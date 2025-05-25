#include "HttpRegistration.h"
#include "Auth.h"
#include "UserDB.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <iostream>
#include <thread>
#include <map>
#include "HttpRegistration.h"

// Stub implementation—no per-instance config needed for registration
void HttpRegistration::loadServerConfig(const std::string& /*configPath*/) {
    // You can parse a config file here if you need custom interfaces/ports.
}

namespace beast = boost::beast;
namespace http  = beast::http;
namespace asio  = boost::asio;
namespace ssl   = asio::ssl;
using tcp       = asio::ip::tcp;

static const unsigned short PORT = 8443;

void HttpRegistration::start() {
    std::thread([](){
        try {
            asio::io_context ioc{1};
            ssl::context ctx{ssl::context::tlsv12};
            ctx.use_certificate_chain_file("cert.pem");
            ctx.use_private_key_file("key.pem", ssl::context::pem);

            tcp::acceptor acceptor{ioc, {tcp::v4(), PORT}};
            while(true) {
                // Allocate the SSL stream on the heap
                auto stream = std::make_shared<ssl::stream<tcp::socket>>(ioc, ctx);

                // Accept into the underlying socket
                acceptor.accept(stream->next_layer());

                // Handle the session
                std::thread([stream](){
                    try {
                        stream->handshake(ssl::stream_base::server);

                        beast::flat_buffer buffer;
                        http::request<http::string_body> req;
                        http::read(*stream, buffer, req);

                        http::response<http::string_body> res;

                        if(req.method() == http::verb::get) {
                            res.result(http::status::ok);
                            res.set(http::field::content_type, "text/html");
                            res.body() = R"(
<html>
  <body>
    <h1>Register for EChat</h1>
    <form method="POST">
      <label>Registration Code: <input name="code"/></label><br/>
      <label>Username: <input name="user"/></label><br/>
      <label>Password: <input type="password" name="pwd"/></label><br/>
      <button type="submit">Register</button>
    </form>
  </body>
</html>
)";
                        }
                        else if(req.method() == http::verb::post) {
                            // parse application/x-www-form-urlencoded
                            std::map<std::string,std::string> params;
                            std::string body = req.body();
                            size_t pos = 0;
                            while(pos < body.size()) {
                                auto eq = body.find('=', pos);
                                auto amp = body.find('&', pos);
                                if(eq == std::string::npos) break;
                                std::string key = body.substr(pos, eq-pos);
                                std::string val;
                                if(amp == std::string::npos) {
                                    val = body.substr(eq+1);
                                    pos = body.size();
                                } else {
                                    val = body.substr(eq+1, amp-(eq+1));
                                    pos = amp+1;
                                }
                                params[key] = val;
                            }

                            auto it_code = params.find("code");
                            auto it_user = params.find("user");
                            auto it_pwd  = params.find("pwd");
                            if(it_code!=params.end() && it_user!=params.end() && it_pwd!=params.end()
                               && Auth::validateRegCode(it_code->second)) {
                                auto hash = Auth::hashPassword(it_pwd->second);
                                UserDB::instance().addUser(it_user->second, hash);
                                res.result(http::status::ok);
                                res.set(http::field::content_type, "text/html");
                                res.body() = "<html><body><h2>Registration successful!</h2></body></html>";
                            } else {
                                res.result(http::status::bad_request);
                                res.set(http::field::content_type, "text/html");
                                res.body() = "<html><body><h2>Invalid registration code or data.</h2></body></html>";
                            }
                        }
                        else {
                            res.result(http::status::method_not_allowed);
                            res.set(http::field::content_type, "text/html");
                            res.body() = "<html><body><h2>Only GET and POST supported.</h2></body></html>";
                        }

                        res.content_length(res.body().size());
                        http::write(*stream, res);
                    }
                    catch(const std::exception& e) {
                        std::cerr << "Registration handler error: " << e.what() << "\n";
                    }
                }).detach();
            }
        } catch(const std::exception& e) {
            std::cerr << "Registration service error: " << e.what() << "\n";
        }
    }).detach();
}