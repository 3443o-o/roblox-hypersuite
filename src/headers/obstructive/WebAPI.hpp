#pragma once
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

inline int testrealok() {
    // Create HTTPS client
    httplib::Client cli("https://games.roblox.com");
    cli.enable_server_certificate_verification(false);   // optional but helps on Windows without SSL setup

    // Send GET request
    auto res = cli.Get("/v1/games/multiget-place-details?placeIds=3889763149");

    // Check response
    if (!res) {
        std::cout << "Request failed\n";
        return 1;
    }

    std::cout << "Status: " << res->status << "\n";
    std::cout << "Body:\n" << res->body << "\n";

    return 0;
}
