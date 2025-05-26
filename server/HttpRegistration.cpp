#include "HttpRegistration.h"
#include "Auth.h"
#include "UserDB.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <iostream>
#include <thread>
#include <map>
#include <sstream>

namespace beast = boost::beast;
namespace http  = beast::http;
namespace asio  = boost::asio;
namespace ssl   = asio::ssl;
using tcp       = asio::ip::tcp;

static const unsigned short PORT = 8443;
// Stub: we don’t currently have per‐instance config for registration.
// This will be called by ServerApp::loadConfig(), so we need an empty implementation.
void HttpRegistration::loadServerConfig(const std::string& /*configPath*/) {
    // no‐op for now
}

void HttpRegistration::start() {
    std::thread([](){
        try {
            asio::io_context ioc{1};
            ssl::context ctx{ssl::context::tlsv12};
            ctx.use_certificate_chain_file("cert.pem");
            ctx.use_private_key_file("key.pem", ssl::context::pem);

            tcp::acceptor acceptor{ioc, {tcp::v4(), PORT}};
            while(true) {
                auto stream = std::make_shared<ssl::stream<tcp::socket>>(ioc, ctx);
                acceptor.accept(stream->next_layer());
                std::thread([stream](){
                    try {
                        stream->handshake(ssl::stream_base::server);
                        beast::flat_buffer buffer;
                        http::request<http::string_body> req;
                        http::read(*stream, buffer, req);

                        http::response<http::string_body> res;

                        // ---- JSON-POST login ----
                        if(req.method() == http::verb::post &&
                           req[http::field::content_type] == "application/json")
                        {
                            using boost::property_tree::ptree;
                            std::stringstream js(req.body());
                            ptree tree;
                            try {
                                read_json(js, tree);
                                std::string type = tree.get<std::string>("type");
                                ptree resp;
                                res.version(req.version());
                                res.set(http::field::content_type, "application/json");
                                if(type == "login") {
                                    auto user = tree.get<std::string>("user");
                                    auto pass = tree.get<std::string>("pass");
                                    bool ok = UserDB::instance().verifyUser(user, pass);
                                    resp.put("type",   "login_response");
                                    resp.put("status", ok ? "ok" : "fail");
                                    std::ostringstream out;
                                    write_json(out, resp, false);
                                    res.result(http::status::ok);
                                    res.body() = out.str();
                                } else {
                                    res.result(http::status::bad_request);
                                    resp.put("error","unknown type");
                                    std::ostringstream out;
                                    write_json(out, resp, false);
                                    res.body() = out.str();
                                }
                            } catch(...) {
                                res = http::response<http::string_body>{
                                    http::status::bad_request, req.version()};
                                res.set(http::field::content_type, "application/json");
                                res.body() = R"({"error":"invalid json"})";
                            }
                        }
                        // ---- Browser form GET ----
                        else if(req.method() == http::verb::get) {
                            res.result(http::status::ok);
                            res.set(http::field::content_type, "text/html");
                            res.body() = R"(
<html><body>
<h1>Register</h1>
<form method="POST">
  Code: <input name="code"><br>
  Username: <input name="user"><br>
  Password: <input type="password" name="pwd"><br>
  <button type="submit">Submit</button>
</form>
</body></html>
)";
                        }
                        // ---- Browser form POST ----
                        else if(req.method() == http::verb::post) {
                            // parse x-www-form-urlencoded
                            std::map<std::string,std::string> params;
                            auto body = req.body();
                            size_t pos = 0;
                            while(pos < body.size()) {
                                auto eq  = body.find('=', pos);
                                auto amp = body.find('&', pos);
                                if(eq==std::string::npos) break;
                                auto key = body.substr(pos, eq-pos);
                                std::string val;
                                if(amp==std::string::npos) {
                                    val = body.substr(eq+1);
                                    pos = body.size();
                                } else {
                                    val = body.substr(eq+1, amp-(eq+1));
                                    pos = amp+1;
                                }
                                params[key] = val;
                            }
                            auto c= params.find("code"), u=params.find("user"), p=params.find("pwd");
                            if(c!=params.end() && u!=params.end() && p!=params.end() &&
                               Auth::validateRegCode(c->second))
                            {
                                auto hash = Auth::hashPassword(p->second);
                                UserDB::instance().addUser(u->second, hash);
                                res.result(http::status::ok);
                                res.set(http::field::content_type, "text/html");
                                res.body() = "<html><body><h2>Registration successful!</h2></body></html>";
                            } else {
                                res = http::response<http::string_body>{
                                    http::status::bad_request, req.version()};
                                res.set(http::field::content_type, "text/html");
                                res.body() = "<html><body><h2>Invalid registration.</h2></body></html>";
                            }
                        }
                        else {
                            res = http::response<http::string_body>{
                                http::status::method_not_allowed, req.version()};
                            res.set(http::field::content_type, "text/html");
                            res.body() = "<html><body><h2>Only GET, POST supported.</h2></body></html>";
                        }

                        res.content_length(res.body().size());
                        http::write(*stream, res);
                    }
                    catch(const std::exception& e) {
                        std::cerr << "Handler error: " << e.what() << "\n";
                    }
                }).detach();
            }
        } catch(const std::exception& e) {
            std::cerr << "Service error: " << e.what() << "\n";
        }
    }).detach();
}