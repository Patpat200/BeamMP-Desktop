// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <QMainWindow>
#include <QProcess>
#include <QStringConverter>
#include <QJsonObject>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QPushButton;
class QStackedWidget;
class QTimer;

class LauncherWindow : public QMainWindow {
public:
    explicit LauncherWindow(QWidget* parent = nullptr);
    ~LauncherWindow() override;
protected:
    void closeEvent(QCloseEvent* event) override;
private:
    void applyLanguage();
    QString localized(const QString& source) const;
    void startBackend();
    void restartBackend();
    void send(const QJsonObject& command);
    void consumeEvents();
    void handleEvent(const QJsonObject& event);
    void appendLog(const QString& text);
    void updateControls();
    void saveSettings();
    void showStatus(const QString& message, bool error = false, const QString& detail = {});
    QWidget* makeHome();
    QWidget* makeAccount();
    QWidget* makeConsole();
    QWidget* makeSettings();

    QProcess process_;
    QByteArray controlBuffer_;
    QStringDecoder outputDecoder_{QStringDecoder::Utf8};
    bool ready_ = false;
    bool authenticated_ = false;
    bool busy_ = false;
    bool playing_ = false;
    bool closing_ = false;
    QString accountName_;
    bool english_ = false;
    QString statusSource_;
    QString statusDetail_;
    bool statusError_ = false;
    QListWidget* navigation_ = nullptr;
    QStackedWidget* pages_ = nullptr;
    QLabel* status_ = nullptr;
    QLabel* account_ = nullptr;
    QLabel* homeState_ = nullptr;
    QPushButton* play_ = nullptr;
    QPushButton* login_ = nullptr;
    QPushButton* guest_ = nullptr;
    QPushButton* logout_ = nullptr;
    QPushButton* restart_ = nullptr;
    QPushButton* apply_ = nullptr;
    QLineEdit* username_ = nullptr;
    QLineEdit* password_ = nullptr;
    QLineEdit* corePath_ = nullptr;
    QLineEdit* userPath_ = nullptr;
    QLineEdit* launchArguments_ = nullptr;
    QComboBox* language_ = nullptr;
    QLineEdit* search_ = nullptr;
    QComboBox* renderer_ = nullptr;
    QCheckBox* noDownload_ = nullptr;
    QCheckBox* verbose_ = nullptr;
    QPlainTextEdit* console_ = nullptr;
    QTimer* handshake_ = nullptr;
    QTimer* operationTimer_ = nullptr;
};
