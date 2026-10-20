// SPDX-License-Identifier: AGPL-3.0-or-later
#include "GuiBackend.h"
#include "Http.h"
#include "Logger.h"
#include "Network/network.hpp"
#include "Options.h"
#include "Security/Init.h"
#include "Startup.h"
#include "Utils.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <mutex>
#include <thread>

extern bool LoginAuth;
extern std::string Username;
extern std::string UserRole;
std::string Login(const std::string& fields);

namespace {
using Json = nlohmann::json;
constexpr const char* Prefix = "@@BEAMMP_GUI@@";
std::mutex eventMutex;

// stderr is a separate, line-delimited control channel. stdout remains the
// ordinary launcher console. Never send authentication responses or keys here.
void Emit(const Json& event) {
    std::lock_guard<std::mutex> lock(eventMutex);
    std::cerr << Prefix << event.dump() << '\n' << std::flush;
}

void Account(const char* event, bool success = true, const std::string& message = "") {
    Emit({{"event", event}, {"success", success}, {"authenticated", LoginAuth},
        {"username", Username}, {"role", UserRole}, {"message", message}});
}
}

int RunGuiBackend(int argc, const char** argv) {
    std::cout << std::unitbuf;
    std::wcout << std::unitbuf;
    try {
        curl_global_init(CURL_GLOBAL_ALL);
        GetEP(Utils::ToWString(std::string(argv[0])).c_str());
        InitLog();
        ConfigInit();
        InitOptions(argc, argv, options);
        // Do not let the upstream updater replace this custom protocol-enabled
        // executable. Mod updates still follow the normal no_download option.
        options.no_update = true;
        options.no_launch = false;
        InitLauncher();
        LegitimacyCheck();
        Account("ready");

        bool started = false;
        std::string line;
        while (std::getline(std::cin, line)) {
            const auto command = Json::parse(line, nullptr, false);
            // This is an inherited anonymous pipe, never a TCP/HTTP endpoint.
            if (command.is_discarded() || !command.is_object()) {
                Emit({{"event", "error"}, {"message", "Invalid desktop command."}});
                continue;
            }
            try {
                const auto action = command.value("action", std::string{});
                // GUI account changes stop before the network threads begin.
                // This avoids adding another concurrent writer to login state.
                if (started) {
                    Emit({{"event", "error"}, {"message", "Close the game before changing the launcher session."}});
                } else if (action == "login" || action == "guest") {
                    std::string fields;
                    if (action == "login") {
                        const auto username = command.value("username", std::string{});
                        const auto password = command.value("password", std::string{});
                        if (username.empty() || password.empty()) {
                            Account("auth", false, "Username and password are required.");
                            continue;
                        }
                        fields = Json{{"username", username}, {"password", password}}.dump();
                    }
                    // Empty payload is the existing in-game guest login flow.
                    const auto response = Json::parse(Login(fields), nullptr, false);
                    const bool ok = response.is_object() && response.value("success", false);
                    const std::string message = response.is_object() && response.contains("message")
                        && response["message"].is_string() ? response["message"].get<std::string>()
                                                           : "Authentication service returned an invalid response.";
                    Account("auth", ok, message);
                } else if (action == "logout") {
                    Login("LO");
                    Account("auth");
                } else if (action == "play") {
                    if (!LoginAuth) {
                        Emit({{"event", "error"}, {"message", "Sign in or choose guest mode first."}});
                        continue;
                    }
                    Emit({{"event", "preparing"}});
                    PreGame(GetGameDir());
                    try {
                        HTTP::StartProxy();
                    } catch (const std::exception& e) {
                        warn(std::string("HTTP proxy unavailable: ") + e.what());
                    }
                    started = true;
                    std::thread(CoreNetwork).detach();
                    InitGame(GetGameDir());
                    Emit({{"event", "playing"}});
                } else {
                    Emit({{"event", "error"}, {"message", "Unknown desktop command."}});
                }
            } catch (const std::exception&) {
                // Do not echo command bodies, credentials, tokens or responses.
                Emit({{"event", "error"}, {"message", "Operation failed. See the launcher console for details."}});
                if (started) return 1;
            }
            line.clear();
        }
        // Closing the frontend closes stdin. All existing backend threads then
        // end with this subprocess; no orphaned local listener is left behind.
        return 0;
    } catch (const std::exception& e) {
        error(std::string("Desktop backend startup failed: ") + e.what());
        Emit({{"event", "error"}, {"message", "Launcher initialization failed. See the console."}});
        return 1;
    }
}
