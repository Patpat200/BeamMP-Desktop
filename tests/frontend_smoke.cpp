// SPDX-License-Identifier: AGPL-3.0-or-later
#include "LauncherWindow.h"
#include "Localization.h"
#include "LaunchArguments.h"
#include "../include/WindowsCommandLine.h"
#include <QComboBox>
#include <QApplication>
#include <QDir>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>
#include <functional>
#include <iostream>
#include <stdexcept>

static void require(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}
static bool until(const std::function<bool()>& condition, int timeout = 5000) {
    for (int elapsed = 0; elapsed < timeout; elapsed += 20) {
        QApplication::processEvents();
        if (condition()) return true;
        QTest::qWait(20);
    }
    return condition();
}
int main(int argc, char** argv) try {
    QApplication application(argc, argv);
    require(argc == 2, "Expected fake backend path");
    QCoreApplication::setOrganizationName("BeamMPTest");
    QCoreApplication::setApplicationName("FrontendSmoke");
    QTemporaryDir temporary;
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, temporary.path());
    {
        QSettings settings;
        settings.setValue("core", QString::fromLocal8Bit(argv[1]));
    }
    require(!DesktopI18n::useEnglish("auto", QLocale("fr_FR")), "French system default failed");
    require(!DesktopI18n::useEnglish("auto", QLocale("fr_CA")), "French Canadian default failed");
    require(DesktopI18n::useEnglish("auto", QLocale("de_DE")), "English fallback failed");
    require(DesktopI18n::useEnglish("en", QLocale("fr_FR")), "Explicit English failed");
    require(!DesktopI18n::useEnglish("fr", QLocale("en_US")), "Explicit French failed");
    require(DesktopArguments::build("--game -gfx dx11", "vk") == QStringList{"--game", "-gfx", "dx11"}, "Custom renderer did not override menu");
    require(DesktopArguments::build("--verbose --game -level \"C:/my maps/test\"", "dx11") == QStringList{"--verbose", "--game", "-gfx", "dx11", "-level", "C:/my maps/test"}, "Quoted game arguments lost");
    require(DesktopArguments::build("--no-download", "default") == QStringList{"--no-download"}, "Launcher argument lost");
    require(DesktopArguments::build("-- -gfx vk", "dx11") == QStringList{"--game", "-gfx", "vk"}, "Double dash separator failed");
    require(!DesktopArguments::balancedQuotes("--game \"unfinished"), "Unclosed quotes accepted");
    require(QuoteWindowsArgument(L"C:\\my maps\\") == L"\"C:\\my maps\\\\\"", "Windows path quoting failed");
    require(QuoteWindowsArgument(L"a\"b") == L"\"a\\\"b\"", "Windows embedded quote failed");
    require(QuoteWindowsArgument(L"") == L"\"\"", "Windows empty argument failed");
    LauncherWindow window;
    window.show();
    auto* login = window.findChild<QPushButton*>("login");
    auto* guest = window.findChild<QPushButton*>("guest");
    auto* logout = window.findChild<QPushButton*>("logout");
    auto* username = window.findChild<QLineEdit*>("username");
    auto* password = window.findChild<QLineEdit*>("password");
    auto* console = window.findChild<QPlainTextEdit*>("console");
    auto* restart = window.findChild<QPushButton*>("restart");
    auto* nav = window.findChild<QListWidget*>("navigation");
    auto* play = window.findChild<QPushButton*>("primary");
    auto* language = window.findChild<QComboBox*>("language");
    auto* arguments = window.findChild<QLineEdit*>("launchArguments");
    require(language && arguments, "New settings missing");
    language->setCurrentIndex(language->findData("en"));
    require(nav->item(0)->text() == "Home" && login->text() == "Sign in", "English switch failed");
    require(password->placeholderText() == "Password", "English placeholder failed");
    language->setCurrentIndex(language->findData("fr"));
    require(nav->item(0)->text() == "Accueil" && login->text() == "Se connecter", "French switch failed");
    require(QSettings().value("language").toString() == "fr", "Language not saved");
    require(login && guest && logout && username && password && console && restart && nav && play, "Widgets missing");
    if (!until([&] { return login->isEnabled(); })) {
        std::cerr << console->toPlainText().toStdString() << "\nSTATUS: " << window.findChild<QLabel*>("status")->text().toStdString() << '\n';
        require(false, "Ready handshake failed");
    }
    require(!play->isEnabled(), "Unauthenticated Play must be disabled");
    require(until([&] { return console->toPlainText().contains("aucune connexion réseau"); }), "UTF-8 console stream missing");
    require(password->echoMode() == QLineEdit::Password, "Password must be masked");
    nav->setCurrentRow(1);
    username->setText("tester");
    password->setText("wrong-password");
    login->click();
    require(password->text().isEmpty(), "Password not cleared after submit");
    require(until([&] { return login->isEnabled(); }), "Failed auth did not recover");
    require(!play->isEnabled(), "Failed auth enabled Play");
    password->setText("not-a-real-password");
    login->click();
    require(until([&] { return play->isEnabled(); }), "Successful auth did not enable Play");
    require(!login->isEnabled() && logout->isEnabled(), "Account actions out of sync");
    language->setCurrentIndex(language->findData("en"));
    require(window.findChild<QLabel*>("homeState")->text().contains("Signed in as Test Driver"), "Dynamic account text not translated");
    if (const auto path = qEnvironmentVariable("BEAMMP_TEST_SETTINGS_SCREENSHOT"); !path.isEmpty()) {
        nav->setCurrentRow(3);
        QApplication::processEvents();
        require(window.grab().save(path + "-en.png"), "English settings screenshot failed");
        language->setCurrentIndex(language->findData("fr"));
        QApplication::processEvents();
        require(window.grab().save(path + "-fr.png"), "French settings screenshot failed");
    }
    language->setCurrentIndex(language->findData("fr"));
    require(!console->toPlainText().contains("not-a-real-password"), "Credential leaked into console");
    nav->setCurrentRow(0);
    QApplication::processEvents();
    if (const auto path = qEnvironmentVariable("BEAMMP_TEST_SCREENSHOT"); !path.isEmpty()) {
        require(window.grab().save(path), "Could not save UI screenshot");
    }
    logout->click();
    require(until([&] { return guest->isEnabled(); }), "Logout failed");
    guest->click();
    require(until([&] { return play->isEnabled(); }), "Guest auth failed");
    play->click();
    require(until([&] { return !restart->isEnabled() && !play->isEnabled(); }), "Session launch did not lock controls");
    require(until([&] { return restart->isEnabled() && !play->isEnabled(); }), "Backend exit did not recover UI");
    arguments->setText("--game -gfx dx11");
    restart->click();
    require(until([&] { return guest->isEnabled(); }), "Backend restart failed");
    require(QSettings().value("launchArguments").toString() == "--game -gfx dx11", "Arguments not saved");
    require(console->toPlainText().contains("[test argv] --gui-backend --no-update --game -gfx dx11"), "Arguments not passed to child process");
    window.close();
    std::cout << "PASS: ready/framing, UTF-8 logs, failed and successful login, no password logging, logout, guest, play, backend exit and restart.\n";
    return 0;
} catch (const std::exception& error) {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
}
