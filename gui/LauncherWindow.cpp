// SPDX-License-Identifier: AGPL-3.0-or-later
#include "LauncherWindow.h"
#include "Localization.h"
#include "LaunchArguments.h"
#include <QAbstractButton>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTextCursor>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QCoreApplication>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace {
QLabel* heading(const QString& text) {
    auto* label = new QLabel(text);
    label->setObjectName("heading");
    return label;
}
QLabel* paragraph(const QString& text) {
    auto* label = new QLabel(text);
    label->setWordWrap(true);
    label->setTextFormat(Qt::PlainText);
    label->setObjectName("muted");
    return label;
}
QVBoxLayout* pageLayout(QWidget* page) {
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(18);
    return layout;
}
constexpr auto EventPrefix = "@@BEAMMP_GUI@@";
}

LauncherWindow::LauncherWindow(QWidget* parent) : QMainWindow(parent) {
    english_ = DesktopI18n::useEnglish(QSettings().value("language", "auto").toString());
    setWindowTitle("BeamMP — Desktop Preview");
    resize(1100, 730);
    setMinimumSize(880, 620);
    setStyleSheet(R"(
        QMainWindow, QWidget { background: #10151d; color: #e9edf3; }
        QLabel { background: transparent; }
        QLabel#heading { font-size: 28px; font-weight: 700; }
        QLabel#muted { color: #a1adbd; line-height: 1.5; }
        QLabel#brand { font-size: 27px; font-weight: 800; color: #ff773e; }
        QLabel#eyebrow { color: #ff966b; font-weight: 600; }
        QFrame#sidebar { background: #0b1017; border-right: 1px solid #273140; }
        QFrame#card { background: #192331; border: 1px solid #334357; border-radius: 16px; }
        QFrame#card QLabel { background: transparent; }
        QLabel#hero { font-size: 35px; font-weight: 750; }
        QListWidget { background: transparent; border: none; outline: none; font-size: 15px; }
        QListWidget::item { padding: 15px 18px; margin: 3px 0; border-radius: 8px; }
        QListWidget::item:selected { background: #263244; color: #ff9569; }
        QListWidget::item:hover { background: #1a2432; }
        QLineEdit, QComboBox, QPlainTextEdit { background: #0d121a; border: 1px solid #3b4758; border-radius: 7px; padding: 10px; selection-background-color: #a64722; }
        QLineEdit:focus, QComboBox:focus { border-color: #ff773e; }
        QPushButton { background: #263244; border: 1px solid #3a495e; border-radius: 7px; padding: 11px 18px; font-weight: 600; }
        QPushButton:hover { background: #33445b; }
        QPushButton:disabled { color: #748091; background: #202731; border-color: #2b3441; }
        QPushButton#primary { background: #f67437; color: #111820; border-color: #f67437; }
        QPushButton#primary:hover { background: #ff925b; }
        QPushButton#primary:disabled { background: #654431; color: #a6978e; border-color: #654431; }
        QCheckBox { spacing: 9px; padding: 5px 0; }
        QPlainTextEdit { font-family: Consolas, monospace; font-size: 12px; }
        QComboBox::drop-down { border: none; width: 24px; }
        QStatusBar { background: #0b1017; }
    )");

    auto* root = new QWidget;
    auto* row = new QHBoxLayout(root);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(0);
    auto* sidebar = new QFrame;
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(218);
    auto* side = new QVBoxLayout(sidebar);
    side->setContentsMargins(20, 28, 20, 22);
    side->setSpacing(16);
    auto* brand = new QLabel("BeamMP");
    brand->setObjectName("brand");
    side->addWidget(brand);
    side->addWidget(paragraph("DESKTOP PREVIEW\nVersion communautaire"));
    navigation_ = new QListWidget;
    navigation_->setObjectName("navigation");
    navigation_->addItems({"Accueil", "Compte", "Console", "Paramètres"});
    side->addWidget(navigation_, 1);
    account_ = paragraph("Compte non connecté");
    side->addWidget(account_);
    auto* website = new QPushButton("Site BeamMP ↗");
    connect(website, &QPushButton::clicked, this, [] { QDesktopServices::openUrl(QUrl("https://beammp.com")); });
    side->addWidget(website);
    row->addWidget(sidebar);
    auto* right = new QWidget;
    auto* column = new QVBoxLayout(right);
    column->setContentsMargins(0, 0, 0, 0);
    pages_ = new QStackedWidget;
    pages_->addWidget(makeHome());
    pages_->addWidget(makeAccount());
    pages_->addWidget(makeConsole());
    pages_->addWidget(makeSettings());
    column->addWidget(pages_, 1);
    status_ = new QLabel("Initialisation…");
    status_->setObjectName("status");
    status_->setContentsMargins(32, 14, 32, 18);
    status_->setWordWrap(true);
    column->addWidget(status_);
    row->addWidget(right, 1);
    setCentralWidget(root);
    connect(navigation_, &QListWidget::currentRowChanged, pages_, &QStackedWidget::setCurrentIndex);
    navigation_->setCurrentRow(0);

    handshake_ = new QTimer(this);
    handshake_->setSingleShot(true);
    handshake_->setInterval(90000);
    connect(handshake_, &QTimer::timeout, this, [this] {
        showStatus("Le moteur ne répond pas. Vérifie la console et utilise le moteur compilé avec cette interface.", true);
        process_.kill();
    });
    operationTimer_ = new QTimer(this);
    operationTimer_->setSingleShot(true);
    connect(operationTimer_, &QTimer::timeout, this, [this] {
        showStatus("Opération trop longue. Le moteur a été arrêté ; tu peux le redémarrer.", true);
        process_.kill();
    });
    connect(&process_, &QProcess::readyReadStandardOutput, this, [this] {
        appendLog(outputDecoder_.decode(process_.readAllStandardOutput()));
    });
    connect(&process_, &QProcess::readyReadStandardError, this, &LauncherWindow::consumeEvents);
    connect(&process_, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (closing_) return;
        if (error == QProcess::FailedToStart) {
            handshake_->stop();
            busy_ = false;
            showStatus("Impossible de démarrer le moteur : %1", true, process_.errorString());
            appendLog(process_.errorString() + "\n");
            updateControls();
        }
    });
    connect(&process_, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
        [this](int code, QProcess::ExitStatus) {
            handshake_->stop();
            operationTimer_->stop();
            const bool hadSession = playing_;
            ready_ = authenticated_ = busy_ = playing_ = false;
            accountName_.clear();
            if (!closing_) {
                appendLog(localized("\n[Moteur arrêté — code %1]\n").arg(code));
                showStatus(hadSession ? "Session terminée. Redémarre le moteur pour rejouer."
                                      : "Moteur arrêté. Consulte la console puis redémarre-le.");
                updateControls();
            }
        });
    connect(language_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] {
        QSettings().setValue("language", language_->currentData());
        applyLanguage();
    });
    statusSource_ = "Initialisation…";
    applyLanguage();
    QTimer::singleShot(0, this, &LauncherWindow::startBackend);
}

QWidget* LauncherWindow::makeHome() {
    auto* page = new QWidget;
    auto* layout = pageLayout(page);
    auto* eyebrow = new QLabel("MULTIJOUEUR POUR BEAMNG.DRIVE");
    eyebrow->setObjectName("eyebrow");
    layout->addWidget(eyebrow);
    layout->addWidget(heading("La route se partage."));
    layout->addWidget(paragraph("Ton compte, le lancement du jeu et les journaux, réunis dans une seule fenêtre."));
    auto* card = new QFrame;
    card->setObjectName("card");
    auto* hero = new QVBoxLayout(card);
    hero->setContentsMargins(28, 30, 28, 30);
    hero->setSpacing(18);
    auto* title = new QLabel("Prêt pour la prochaine\nvirée ensemble ?");
    title->setObjectName("hero");
    hero->addWidget(title);
    homeState_ = paragraph("Préparation du launcher…");
    homeState_->setObjectName("homeState");
    hero->addWidget(homeState_);
    play_ = new QPushButton("Jouer à BeamMP  →");
    play_->setObjectName("primary");
    play_->setMinimumHeight(54);
    connect(play_, &QPushButton::clicked, this, [this] {
        if (!ready_ || !authenticated_ || busy_ || playing_) return;
        busy_ = true;
        updateControls();
        showStatus("Préparation du mod et démarrage de BeamNG…");
        operationTimer_->start(15 * 60 * 1000);
        send({{"action", "play"}});
    });
    hero->addWidget(play_);
    layout->addWidget(card);
    auto* buttons = new QHBoxLayout;
    auto* account = new QPushButton("Gérer mon compte");
    auto* console = new QPushButton("Voir la console");
    connect(account, &QPushButton::clicked, this, [this] { navigation_->setCurrentRow(1); });
    connect(console, &QPushButton::clicked, this, [this] { navigation_->setCurrentRow(2); });
    buttons->addWidget(account);
    buttons->addWidget(console);
    layout->addLayout(buttons);
    layout->addStretch();
    layout->addWidget(paragraph("Laisse cette fenêtre ouverte pendant la partie. Les serveurs et les favoris restent accessibles dans le jeu."));
    return page;
}

QWidget* LauncherWindow::makeAccount() {
    auto* page = new QWidget;
    auto* layout = pageLayout(page);
    layout->addWidget(heading("Ton compte"));
    layout->addWidget(paragraph("Connecte-toi avec ton compte BeamMP ou continue en invité. Le mot de passe n’est pas enregistré par l’interface."));
    username_ = new QLineEdit;
    username_->setObjectName("username");
    username_->setPlaceholderText("Nom d’utilisateur ou adresse e-mail");
    password_ = new QLineEdit;
    password_->setObjectName("password");
    password_->setPlaceholderText("Mot de passe");
    password_->setEchoMode(QLineEdit::Password);
    auto* form = new QFormLayout;
    form->setSpacing(14);
    form->addRow("Identifiant", username_);
    form->addRow("Mot de passe", password_);
    layout->addLayout(form);
    login_ = new QPushButton("Se connecter");
    login_->setObjectName("login");
    auto doLogin = [this] {
        if (!login_->isEnabled()) return;
        if (username_->text().trimmed().isEmpty() || password_->text().isEmpty()) {
            showStatus("Renseigne ton identifiant et ton mot de passe.", true);
            return;
        }
        busy_ = true;
        updateControls();
        send({{"action", "login"}, {"username", username_->text().trimmed()}, {"password", password_->text()}});
        password_->clear();
        operationTimer_->start(90000);
        showStatus("Connexion au service BeamMP…");
    };
    connect(login_, &QPushButton::clicked, this, doLogin);
    connect(password_, &QLineEdit::returnPressed, this, doLogin);
    guest_ = new QPushButton("Continuer en invité");
    guest_->setObjectName("guest");
    connect(guest_, &QPushButton::clicked, this, [this] {
        busy_ = true;
        password_->clear();
        updateControls();
        send({{"action", "guest"}});
        operationTimer_->start(90000);
        showStatus("Ouverture d’une session invitée…");
    });
    logout_ = new QPushButton("Se déconnecter");
    logout_->setObjectName("logout");
    connect(logout_, &QPushButton::clicked, this, [this] {
        busy_ = true;
        updateControls();
        send({{"action", "logout"}});
        operationTimer_->start(90000);
    });
    auto* buttons = new QHBoxLayout;
    buttons->addWidget(login_);
    buttons->addWidget(guest_);
    layout->addLayout(buttons);
    layout->addWidget(logout_);
    layout->addStretch();
    layout->addWidget(paragraph("La session mémorisée est gérée par le launcher BeamMP existant. Les paramètres de cette interface ne contiennent aucun mot de passe."));
    return page;
}

QWidget* LauncherWindow::makeConsole() {
    auto* page = new QWidget;
    auto* layout = pageLayout(page);
    layout->addWidget(heading("Console du launcher"));
    layout->addWidget(paragraph("Les messages du moteur BeamMP s’affichent ici en direct. Il s’agit d’un journal, pas d’une console Lua du jeu."));
    auto* tools = new QHBoxLayout;
    search_ = new QLineEdit;
    search_->setPlaceholderText("Rechercher dans les messages…");
    auto* next = new QPushButton("Suivant");
    auto* clear = new QPushButton("Effacer");
    auto* save = new QPushButton("Exporter…");
    tools->addWidget(search_, 1);
    tools->addWidget(next);
    tools->addWidget(clear);
    tools->addWidget(save);
    layout->addLayout(tools);
    console_ = new QPlainTextEdit;
    console_->setObjectName("console");
    console_->setReadOnly(true);
    console_->setMaximumBlockCount(10000);
    layout->addWidget(console_, 1);
    auto find = [this] {
        if (!console_->find(search_->text())) {
            console_->moveCursor(QTextCursor::Start);
            console_->find(search_->text());
        }
    };
    connect(next, &QPushButton::clicked, this, find);
    connect(search_, &QLineEdit::returnPressed, this, find);
    connect(clear, &QPushButton::clicked, console_, &QPlainTextEdit::clear);
    connect(save, &QPushButton::clicked, this, [this] {
        const auto path = QFileDialog::getSaveFileName(this, localized("Exporter la console"), "beammp-desktop.log", localized("Journal (*.log);;Texte (*.txt)"));
        if (path.isEmpty()) return;
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            showStatus("Impossible d’écrire le journal.", true);
            return;
        }
        file.write(console_->toPlainText().toUtf8());
        showStatus("Journal exporté.");
    });
    return page;
}

QWidget* LauncherWindow::makeSettings() {
    auto* page = new QWidget;
    auto* layout = pageLayout(page);
    layout->addWidget(heading("Paramètres"));
    layout->addWidget(paragraph("Ces réglages prennent effet au redémarrage du moteur. Le jeu est détecté par le launcher BeamMP."));
    QSettings settings;
#ifdef Q_OS_WIN
    const auto executable = "BeamMP-Launcher.exe";
#else
    const auto executable = "BeamMP-Launcher";
#endif
    corePath_ = new QLineEdit(settings.value("core", QDir(QCoreApplication::applicationDirPath()).filePath(executable)).toString());
    corePath_->setObjectName("corePath");
    auto* browseCore = new QPushButton("Parcourir…");
    connect(browseCore, &QPushButton::clicked, this, [this] {
        const auto path = QFileDialog::getOpenFileName(this, localized("Moteur BeamMP compilé avec l’interface"), corePath_->text());
        if (!path.isEmpty()) corePath_->setText(path);
    });
    auto* coreRow = new QWidget;
    auto* coreLayout = new QHBoxLayout(coreRow);
    coreLayout->setContentsMargins(0, 0, 0, 0);
    coreLayout->addWidget(corePath_, 1);
    coreLayout->addWidget(browseCore);
    userPath_ = new QLineEdit(settings.value("userPath").toString());
    userPath_->setPlaceholderText("Automatique — laisser vide");
    renderer_ = new QComboBox;
    renderer_->addItem("Par défaut", "default");
    renderer_->addItem("DirectX 11 (Windows)", "dx11");
    renderer_->addItem("DirectX 12 (Windows)", "d3d12");
    renderer_->addItem("Vulkan", "vk");
    const int index = renderer_->findData(settings.value("renderer", "default"));
    renderer_->setCurrentIndex(index < 0 ? 0 : index);
    language_ = new QComboBox;
    language_->setObjectName("language");
    language_->addItem("Automatique (langue du PC)", "auto");
    language_->addItem("Français", "fr");
    language_->addItem("English", "en");
    const int languageIndex = language_->findData(settings.value("language", "auto"));
    language_->setCurrentIndex(languageIndex < 0 ? 0 : languageIndex);
    launchArguments_ = new QLineEdit(settings.value("launchArguments").toString());
    launchArguments_->setObjectName("launchArguments");
    launchArguments_->setPlaceholderText("Exemple : --game -gfx dx11");
    auto* form = new QFormLayout;
    form->setSpacing(14);
    form->addRow("Langue", language_);
    form->addRow("Moteur BeamMP", coreRow);
    form->addRow("Dossier utilisateur BeamNG", userPath_);
    form->addRow("Rendu graphique", renderer_);
    form->addRow("Arguments de lancement", launchArguments_);
    layout->addLayout(form);
    layout->addWidget(paragraph("Arguments du launcher ; après --game, arguments du jeu. Entoure les chemins contenant des espaces de guillemets. Un -gfx personnalisé remplace le rendu choisi ci-dessus."));
    noDownload_ = new QCheckBox("Conserver mon mod local sans le retélécharger (développement)");
    noDownload_->setChecked(settings.value("noDownload", false).toBool());
    verbose_ = new QCheckBox("Afficher les messages de débogage du launcher");
    verbose_->setChecked(settings.value("verbose", false).toBool());
    layout->addWidget(noDownload_);
    layout->addWidget(verbose_);
    apply_ = new QPushButton("Enregistrer et redémarrer le moteur");
    connect(apply_, &QPushButton::clicked, this, &LauncherWindow::restartBackend);
    layout->addWidget(apply_);
    restart_ = new QPushButton("Redémarrer le moteur");
    restart_->setObjectName("restart");
    connect(restart_, &QPushButton::clicked, this, &LauncherWindow::restartBackend);
    layout->addWidget(restart_);
    layout->addStretch();
    layout->addWidget(paragraph("Cette version personnelle ne met pas automatiquement à jour son exécutable moteur, afin de conserver l’interface. Le mod reste mis à jour normalement, sauf si l’option de développement est cochée."));
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(page);
    return scroll;
}

void LauncherWindow::saveSettings() {
    QSettings settings;
    settings.setValue("language", language_->currentData());
    settings.setValue("launchArguments", launchArguments_->text());
    settings.setValue("core", corePath_->text());
    settings.setValue("userPath", userPath_->text());
    settings.setValue("renderer", renderer_->currentData());
    settings.setValue("noDownload", noDownload_->isChecked());
    settings.setValue("verbose", verbose_->isChecked());
}

void LauncherWindow::startBackend() {
    if (process_.state() != QProcess::NotRunning) return;
    const QFileInfo core(corePath_->text().trimmed());
    if (!core.exists() || !core.isFile()) {
        showStatus("Moteur introuvable. Dans Paramètres, sélectionne le BeamMP-Launcher compilé avec ce projet.", true);
        navigation_->setCurrentRow(3);
        return;
    }
    if (core.canonicalFilePath() == QFileInfo(QCoreApplication::applicationFilePath()).canonicalFilePath()) {
        showStatus("Sélectionne BeamMP-Launcher, pas BeamMP-Desktop lui-même.", true);
        return;
    }
    const auto userPath = userPath_->text().trimmed();
    if (!userPath.isEmpty() && !QFileInfo(userPath).isDir()) {
        showStatus("Le dossier utilisateur indiqué n’existe pas.", true);
        return;
    }
    if (!DesktopArguments::balancedQuotes(launchArguments_->text())) {
        showStatus("Ferme le guillemet dans les arguments de lancement.", true);
        return;
    }
    ready_ = authenticated_ = playing_ = false;
    busy_ = true;
    controlBuffer_.clear();
    outputDecoder_ = QStringDecoder(QStringDecoder::Utf8);
    QStringList args{"--gui-backend", "--no-update"};
    if (noDownload_->isChecked()) args << "--no-download";
    if (verbose_->isChecked()) args << "--verbose";
    if (!userPath.isEmpty()) args << "--user-path" << userPath;
    args << DesktopArguments::build(launchArguments_->text(), renderer_->currentData().toString());
    process_.setProgram(core.absoluteFilePath());
    process_.setArguments(args);
    process_.setWorkingDirectory(core.absolutePath());
    process_.setProcessChannelMode(QProcess::SeparateChannels);
#ifdef Q_OS_WIN
    process_.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments* args) {
        args->flags |= CREATE_NO_WINDOW;
    });
#endif
    updateControls();
    showStatus("Initialisation du moteur BeamMP…");
    appendLog(localized("\n[Démarrage du moteur]\n"));
    process_.start();
    handshake_->start();
}

void LauncherWindow::restartBackend() {
    if (playing_ || busy_) return;
    saveSettings();
    if (process_.state() != QProcess::NotRunning) {
        process_.terminate();
        if (!process_.waitForFinished(1000)) {
            process_.kill();
            process_.waitForFinished(1000);
        }
    }
    startBackend();
}

void LauncherWindow::send(const QJsonObject& command) {
    if (process_.state() != QProcess::Running) return;
    process_.write(QJsonDocument(command).toJson(QJsonDocument::Compact) + '\n');
}

void LauncherWindow::consumeEvents() {
    controlBuffer_ += process_.readAllStandardError();
    if (controlBuffer_.size() > 1024 * 1024) {
        controlBuffer_.clear();
        appendLog(localized("[Message moteur trop volumineux ignoré]\n"));
        return;
    }
    qsizetype newline;
    while ((newline = controlBuffer_.indexOf('\n')) >= 0) {
        const auto line = controlBuffer_.left(newline).trimmed();
        controlBuffer_.remove(0, newline + 1);
        if (!line.startsWith(EventPrefix)) {
            if (!line.isEmpty()) appendLog(QString::fromUtf8(line) + '\n');
            continue;
        }
        const auto document = QJsonDocument::fromJson(line.mid(QByteArray(EventPrefix).size()));
        if (document.isObject()) handleEvent(document.object());
    }
}

void LauncherWindow::handleEvent(const QJsonObject& event) {
    const auto type = event.value("event").toString();
    if (type == "ready" || type == "auth") {
        handshake_->stop();
        operationTimer_->stop();
        ready_ = true;
        busy_ = false;
        authenticated_ = event.value("authenticated").toBool();
        accountName_ = event.value("username").toString();
        const bool success = event.value("success").toBool();
        if (!success) {
            // Treat backend response as plain text; never render HTML here.
            showStatus(event.value("message").toString("Connexion refusée."), true);
        } else {
            showStatus(authenticated_ ? "Compte prêt. Tu peux lancer le jeu." : "Choisis ton compte ou le mode invité.");
        }
    } else if (type == "preparing") {
        busy_ = true;
        showStatus("Vérification du mod et préparation du jeu…");
    } else if (type == "playing") {
        operationTimer_->stop();
        busy_ = false;
        playing_ = true;
        showStatus("Lancement demandé. Garde le launcher ouvert pendant la partie.");
    } else if (type == "error") {
        operationTimer_->stop();
        busy_ = false;
        showStatus(event.value("message").toString("Erreur du moteur."), true);
    }
    updateControls();
}

void LauncherWindow::updateControls() {
    const bool available = ready_ && !busy_ && !playing_;
    play_->setEnabled(available && authenticated_);
    login_->setEnabled(available && !authenticated_);
    guest_->setEnabled(available && !authenticated_);
    logout_->setEnabled(available && authenticated_);
    username_->setEnabled(available && !authenticated_);
    password_->setEnabled(available && !authenticated_);
    apply_->setEnabled(!playing_ && !busy_);
    restart_->setEnabled(!playing_ && !busy_);
    corePath_->setEnabled(!playing_ && !busy_);
    userPath_->setEnabled(!playing_ && !busy_);
    renderer_->setEnabled(!playing_ && !busy_);
    launchArguments_->setEnabled(!playing_ && !busy_);
    noDownload_->setEnabled(!playing_ && !busy_);
    verbose_->setEnabled(!playing_ && !busy_);
    const auto name = accountName_.isEmpty() ? localized("Session invitée") : accountName_;
    account_->setText(authenticated_ ? name : localized("Compte non connecté"));
    homeState_->setText(playing_ ? localized("Session en cours — laisse le launcher ouvert.")
        : busy_ ? localized("Une opération est en cours…")
        : authenticated_ ? localized("Connecté : %1. Le jeu est prêt à être lancé.").arg(name)
        : ready_ ? localized("Connecte-toi dans l’onglet Compte pour commencer.")
                 : localized("Le moteur doit être démarré depuis l’onglet Paramètres."));
}

void LauncherWindow::showStatus(const QString& message, bool error, const QString& detail) {
    statusSource_ = message;
    statusDetail_ = detail;
    statusError_ = error;
    auto translated = localized(message);
    if (!detail.isNull()) translated = translated.arg(detail);
    status_->setText(translated);
    status_->setStyleSheet(error ? "color: #ff9c91;" : "color: #b9c8d9;");
    status_->setTextFormat(Qt::PlainText);
}

QString LauncherWindow::localized(const QString& source) const {
    return DesktopI18n::text(source, english_);
}

void LauncherWindow::applyLanguage() {
    english_ = DesktopI18n::useEnglish(language_->currentData().toString());
    // Keep original strings on widgets so a switch in either direction never
    // translates an already translated value. No account or log data is changed.
    for (auto* label : findChildren<QLabel*>()) {
        if (label == account_ || label == homeState_ || label == status_) continue;
        if (!label->property("sourceText").isValid()) label->setProperty("sourceText", label->text());
        label->setText(localized(label->property("sourceText").toString()));
    }
    for (auto* button : findChildren<QAbstractButton*>()) {
        if (!button->property("sourceText").isValid()) button->setProperty("sourceText", button->text());
        button->setText(localized(button->property("sourceText").toString()));
    }
    for (auto* edit : findChildren<QLineEdit*>()) {
        if (!edit->property("sourcePlaceholder").isValid()) edit->setProperty("sourcePlaceholder", edit->placeholderText());
        edit->setPlaceholderText(localized(edit->property("sourcePlaceholder").toString()));
    }
    for (auto* combo : findChildren<QComboBox*>()) {
        for (int i = 0; i < combo->count(); ++i) {
            if (!combo->itemData(i, Qt::UserRole + 1).isValid())
                combo->setItemData(i, combo->itemText(i), Qt::UserRole + 1);
            combo->setItemText(i, localized(combo->itemData(i, Qt::UserRole + 1).toString()));
        }
    }
    for (int i = 0; i < navigation_->count(); ++i) {
        auto* item = navigation_->item(i);
        if (!item->data(Qt::UserRole).isValid()) item->setData(Qt::UserRole, item->text());
        item->setText(localized(item->data(Qt::UserRole).toString()));
    }
    showStatus(statusSource_, statusError_, statusDetail_);
    updateControls();
}

void LauncherWindow::appendLog(const QString& text) {
    if (text.isEmpty()) return;
    static const QRegularExpression ansi("\x1b\\[[0-9;]*[a-zA-Z]");
    QString clean = text;
    clean.remove(ansi);
    auto cursor = console_->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(clean);
    console_->setTextCursor(cursor);
    console_->ensureCursorVisible();
}

void LauncherWindow::closeEvent(QCloseEvent* event) {
    if (playing_ && QMessageBox::question(this, localized("Quitter BeamMP ?"),
            localized("Fermer le launcher coupe la connexion multijoueur. Quitter quand même ?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
        event->ignore();
        return;
    }
    closing_ = true;
    saveSettings();
    event->accept();
}

LauncherWindow::~LauncherWindow() {
    closing_ = true;
    if (process_.state() != QProcess::NotRunning) {
        process_.closeWriteChannel();
        if (!process_.waitForFinished(500)) {
            process_.kill();
            process_.waitForFinished(1000);
        }
    }
}
