#ifndef VULNSHOP_HTTP_SERVER_H
#define VULNSHOP_HTTP_SERVER_H
#include "vulnshop.h"
#include <functional>
#include <regex>
namespace vulnshop {

// A very small HTTP/1.1 server. Deliberately dependency-free so that the
// exercise is about the application's own defects rather than a framework's.
class Server {
public:
    using Handler = std::function<void(const Request&, Response&)>;
    void Get (const std::string& pattern, Handler h)  { routes_.push_back({"GET",  pattern, h}); }
    void Post(const std::string& pattern, Handler h)  { routes_.push_back({"POST", pattern, h}); }
    void set_pre_routing(std::function<bool(const Request&, Response&)> f) { pre_ = f; }
    bool listen(const std::string& host, int port);
    void stop() { running_ = false; }
private:
    struct Route { std::string method; std::string pattern; Handler handler; };
    std::vector<Route> routes_;
    std::function<bool(const Request&, Response&)> pre_;
    bool running_ = true;
    void handle(int fd);
};

} // namespace vulnshop
#endif
