#include "http_server.h"
#include "log.h"
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <cstdlib>
#include <unistd.h>

namespace vulnshop {

// Percent-decoding, applied before routing. No normalisation of "." or ".."
// is performed here or anywhere else - see the /api/export route.
static std::string url_decode(const std::string& in) {
    std::string out;
    for (std::size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '%' && i + 2 < in.size()) {
            char hex[3] = { in[i + 1], in[i + 2], '\0' };
            out.push_back(static_cast<char>(std::strtol(hex, nullptr, 16)));
            i += 2;
        } else if (in[i] == '+') {
            out.push_back(' ');
        } else {
            out.push_back(in[i]);
        }
    }
    return out;
}

std::string Request::header(const std::string& key) const {
    auto it = headers.find(key);
    return it == headers.end() ? std::string() : it->second;
}

std::string Request::query(const std::string& key) const {
    std::size_t q = path.find('?');
    if (q == std::string::npos) return {};
    std::string qs = path.substr(q + 1);
    std::size_t pos = 0;
    while (pos < qs.size()) {
        std::size_t amp = qs.find('&', pos);
        std::string pair = qs.substr(pos, amp == std::string::npos ? std::string::npos : amp - pos);
        std::size_t eq = pair.find('=');
        if (eq != std::string::npos && pair.substr(0, eq) == key) return url_decode(pair.substr(eq + 1));
        if (amp == std::string::npos) break;
        pos = amp + 1;
    }
    return {};
}

std::string Request::form(const std::string& key) const {
    std::size_t pos = 0;
    while (pos < body.size()) {
        std::size_t amp = body.find('&', pos);
        std::string pair = body.substr(pos, amp == std::string::npos ? std::string::npos : amp - pos);
        std::size_t eq = pair.find('=');
        if (eq != std::string::npos && pair.substr(0, eq) == key)
            return url_decode(pair.substr(eq + 1));
        if (amp == std::string::npos) break;
        pos = amp + 1;
    }
    return {};
}

std::string Request::cookie(const std::string& key) const {
    const std::string c = header("Cookie");
    std::size_t pos = 0;
    while (pos < c.size()) {
        std::size_t semi = c.find(';', pos);
        std::string pair = c.substr(pos, semi == std::string::npos ? std::string::npos : semi - pos);
        while (!pair.empty() && pair.front() == ' ') pair.erase(pair.begin());
        std::size_t eq = pair.find('=');
        if (eq != std::string::npos && pair.substr(0, eq) == key)
            return url_decode(pair.substr(eq + 1));
        if (semi == std::string::npos) break;
        pos = semi + 1;
    }
    return {};
}

void Response::set_content(const std::string& b, const std::string& ct) {
    body = b; content_type = ct;
}

static std::string read_all(int fd) {
    std::string data;
    char buf[4096];
    ssize_t n;
    while ((n = ::recv(fd, buf, sizeof(buf), 0)) > 0) {
        data.append(buf, static_cast<size_t>(n));
        if (data.find("\r\n\r\n") != std::string::npos) {
            std::size_t hdr_end = data.find("\r\n\r\n") + 4;
            std::size_t cl = data.find("Content-Length:");
            if (cl == std::string::npos) break;
            long want = std::strtol(data.c_str() + cl + 15, nullptr, 10);
            if (static_cast<long>(data.size() - hdr_end) >= want) break;
        }
        if (n < static_cast<ssize_t>(sizeof(buf))) {
            if (data.find("\r\n\r\n") != std::string::npos) break;
        }
    }
    return data;
}

void Server::handle(int fd) {
    std::string raw = read_all(fd);
    if (raw.empty()) { ::close(fd); return; }

    Request req;
    std::istringstream in(raw);
    std::string line;
    std::getline(in, line);
    {
        std::istringstream rl(line);
        std::string version;
        rl >> req.method >> req.path >> version;
    }
    while (std::getline(in, line) && line != "\r" && !line.empty()) {
        std::size_t colon = line.find(':');
        if (colon == std::string::npos) continue;
        std::string k = line.substr(0, colon);
        std::string v = line.substr(colon + 1);
        while (!v.empty() && (v.front() == ' ' || v.front() == '\t')) v.erase(v.begin());
        while (!v.empty() && (v.back() == '\r' || v.back() == '\n')) v.pop_back();
        req.headers[k] = v;
    }
    std::size_t body_at = raw.find("\r\n\r\n");
    if (body_at != std::string::npos) req.body = raw.substr(body_at + 4);

    Response res;
    std::string bare = url_decode(req.path.substr(0, req.path.find('?')));

    bool handled = false;
    if (!pre_ || pre_(req, res)) {
        for (const Route& r : routes_) {
            if (r.method != req.method) continue;
            std::smatch m;
            if (std::regex_match(bare, m, std::regex(r.pattern))) {
                req.matches.clear();
                for (std::size_t i = 1; i < m.size(); ++i) req.matches.push_back(m[i].str());
                r.handler(req, res);
                handled = true;
                break;
            }
        }
        if (!handled) { res.status = 404; res.set_content("not found", "text/plain"); }
    }

    std::ostringstream out;
    out << "HTTP/1.1 " << res.status << " \r\n"
        << "Content-Type: " << res.content_type << "\r\n";
    for (const std::string& c : res.set_cookies) out << "Set-Cookie: " << c << "\r\n";
    out
        << "Content-Length: " << res.body.size() << "\r\n"
        << "Connection: close\r\n\r\n" << res.body;
    std::string s = out.str();
    ::send(fd, s.data(), s.size(), 0);
    ::close(fd);
}

bool Server::listen(const std::string& host, int port) {
    int srv = ::socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) return false;
    int yes = 1;
    ::setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(static_cast<uint16_t>(port));
    addr.sin_addr.s_addr = ::inet_addr(host.c_str());

    if (::bind(srv, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) { ::close(srv); return false; }
    if (::listen(srv, 16) < 0) { ::close(srv); return false; }
    log_info("listening on %s:%d", host.c_str(), port);

    while (running_) {
        int fd = ::accept(srv, nullptr, nullptr);
        if (fd < 0) continue;
        handle(fd);
    }
    ::close(srv);
    return true;
}

} // namespace vulnshop
