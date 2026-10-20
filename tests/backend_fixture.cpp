// SPDX-License-Identifier: AGPL-3.0-or-later
// Exercise the real desktop command controller with offline engine services.
#include "GuiBackend.h"
#include "Http.h"
#include "Options.h"
#include "Security/Init.h"
#include "Startup.h"
#include <nlohmann/json.hpp>
#include <chrono>
#include <iostream>
#include <thread>

Options options;
bool LoginAuth = false;
std::string Username;
std::string UserRole;
void InitLog() {}
void ConfigInit() {}
void InitLauncher() {}
void LegitimacyCheck() {}
void PreGame(const beammp_fs_string&) {}
void InitGame(const beammp_fs_string&) {}
beammp_fs_string GetEP(const beammp_fs_char*) { return {}; }
beammp_fs_string GetGameDir() { return {}; }
void HTTP::StartProxy() {}
[[noreturn]] void CoreNetwork() {
    for (;;) std::this_thread::sleep_for(std::chrono::seconds(1));
}
void info(const std::string& message) { std::cout << message << '\n'; }
void error(const std::string& message) { std::cout << message << '\n'; }
void warn(const std::string& message) { std::cout << message << '\n'; }
void debug(const std::string&) {}
#ifdef _WIN32
void debug(const std::wstring&) {}
#endif
std::string Login(const std::string& fields) {
    if (fields == "LO") {
        LoginAuth = false;
        Username.clear();
        return {};
    }
    const auto request = fields.empty() ? nlohmann::json{} : nlohmann::json::parse(fields);
    LoginAuth = fields.empty() || (request.value("username", "") == "tester"
        && request.value("password", "") == "not-a-real-password");
    Username = LoginAuth ? (fields.empty() ? "guest123" : "tester") : "";
    UserRole = "USER";
    // These must never appear in the frontend control channel.
    return nlohmann::json{{"success", LoginAuth}, {"message", LoginAuth ? "OK" : "Rejected"},
        {"private_key", "TEST_PRIVATE_KEY"}, {"public_key", "TEST_PUBLIC_KEY"}}.dump();
}
int main(int argc, const char** argv) { return RunGuiBackend(argc, argv); }
