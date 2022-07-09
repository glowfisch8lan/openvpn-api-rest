// Copyright (c) 2020 Cesanta Software Limited
// All rights reserved
//
// HTTP server example. This server serves both static and dynamic content.
// It opens two ports: plain HTTP on port 8000 and HTTP on port 8443.
// It implements the following endpoints:
//    /api/stats - respond with free-formatted stats on current connections
//    /api/f2/:id - wildcard example, respond with JSON string {"result": "URI"}
//    any other URI serves static files from s_root_dir
//
// To enable SSL/TLS (using self-signed certificates in PEM files),
//    1. make SSL=OPENSSL or make SSL=MBEDTLS
//    2. curl -k https://127.0.0.1:8443

#include "mongoose.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

static const char *s_http_addr = "http://0.0.0.0:8000";    // HTTP port
static const char *s_root_dir = ".";
static const char *path_openvpn_log = nullptr;

/**
 *
 * @param str
 * @param ch
 * @return
 */
vector<string> explode(const string& str, const char& ch) {
    string next;
    vector<string> result;

    // For each character in the string
    for (string::const_iterator it = str.begin(); it != str.end(); it++) {
        // If we've hit the terminal character
        if (*it == ch) {
            // If we have some characters accumulated
            if (!next.empty()) {
                // Add them to the result vector
                result.push_back(next);
                next.clear();
            }
        } else {
            // Accumulate the next character into the sequence
            next += *it;
        }
    }
    if (!next.empty())
        result.push_back(next);
    return result;
}

/**
 *
 * @param c
 * @param ev
 * @param ev_data
 * @param fn_data
 */
static void router(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {

    if (ev != MG_EV_HTTP_MSG) {
        return;
    }
    int status_code = 404;
    std::string result = "no data";

    std::cout << "Request API" << std::endl;
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;

    if (mg_http_match_uri(hm, "/api/openvpn/connected")) {
        status_code = 200;

        string line;
        json j;

        std::ifstream myfile(path_openvpn_log);
        if (myfile.is_open()) {
            int index = 0;
            bool parse = true;

            while (getline(myfile, line)) {
                index += 1;
                std::string endPart = "ROUTING";
                if (line.find(endPart) == 0) {
                    parse = false;
                }

                if (!myfile.eof() && index > 3 && parse){
                    j.push_back(explode(line, ','));
                }

            }
            myfile.close();

            result = to_string(j);

        } else result = "Unable to open file";

    }

    mg_http_reply(c, status_code,
                  "Content-Type: application/json\r\n",
                  "{\"result\": %.*s}\n",
                  result.length(), result.c_str()
    );
}

/**
 *
 * @return
 */
int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");

    if (argv[1] == nullptr) {
        cout << "Error: Need argument path to openvpn-status.log" << endl;
        return 0;
    }

    path_openvpn_log = argv[1];

    struct mg_mgr mgr{};                            // Event manager
    mg_log_set("2");                              // Set to 3 to enable debug
    mg_mgr_init(&mgr);                            // Initialise event manager
    mg_http_listen(&mgr, s_http_addr, router, nullptr);  // Create HTTP listener
    for (;;) mg_mgr_poll(&mgr, 1000);                    // Infinite event loop
    mg_mgr_free(&mgr);
    return 0;
}