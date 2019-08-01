#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace gai {

class Result {
 public:
  Result(addrinfo* info, int err) : info_(info), err_(err) {}

  // non-copyable
  Result(const Result&) = delete;
  Result& operator=(const Result&) = delete;
  // movable
  Result(Result&&) = default;

  ~Result() {
    if (info_ != nullptr) {
      freeaddrinfo(info_);
    }
  }

  explicit operator bool() const { return err_ == 0; }

  addrinfo* raw_addrinfo() const { return info_; }

  std::string address() const {
    auto& addr = info_->ai_addr;
    switch (addr->sa_family) {
      case AF_INET: {
        char buf[INET_ADDRSTRLEN] = {};
        inet_ntop(addr->sa_family, addr->sa_data, buf, sizeof(buf));
        return std::string(buf);
      }
      case AF_INET6: {
        char buf[INET6_ADDRSTRLEN] = {};
        inet_ntop(addr->sa_family, addr->sa_data, buf, sizeof(buf));
        return std::string(buf);
      }
    }
    return "";
  }

  const char* error() const { return gai_strerror(err_); }

 private:
  struct addrinfo* info_ = nullptr;
  int err_ = 0;
};

inline Result Resolve(const std::string& host, int port) {
  addrinfo hints = {};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;  // TCP
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  addrinfo* result = nullptr;
  auto port_str = std::to_string(port);
  auto r = getaddrinfo(host.c_str(), port_str.c_str(), &hints, &result);

  return Result(result, r);
}

}  // namespace gai

int Connect(const gai::Result& gai_result) {
  assert(!!gai_result);

  int fd = 0;
  addrinfo* rp = gai_result.raw_addrinfo();
  for (; rp != nullptr; rp = rp->ai_next) {
    fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (fd == -1) {
      continue;
    }

    if (connect(fd, rp->ai_addr, rp->ai_addrlen) == -1) {
      close(fd);
      continue;
    }

    break;  // success
  }

  return rp == nullptr ? -1 : fd;
}

int main(int argc, char** argv) {
  const char* host = "localhost";
  int port = 80;

  if (argc > 1) {
    host = argv[1];
  }
  if (argc > 2) {
    port = std::atoi(argv[2]);
  }

  auto r = gai::Resolve(host, port);
  if (!r) {
    std::cout << "gai: error: " << r.error() << std::endl;
    return 1;
  }
  std::cout << "gai: " << host << " => " << r.address() << std::endl;

  auto fd = Connect(r);
  std::cout << "Connect: fd = " << fd << std::endl;
  if (fd == -1) {
    return 1;
  }

  {
    std::stringstream ss;
    ss << "GET / HTTP/1.1\n"
       << "Host: " << host << ":" << port << "\n"
       << "\n";
    auto data = ss.str();
    auto size = static_cast<ssize_t>(data.size());
    if (write(fd, data.c_str(), size) != size) {
      std::cerr << "failed to write" << std::endl;
      return 1;
    }
  }
  {
    std::cout << "=====" << std::endl;
    while (true) {
      char buf[1024];
      auto nread = read(fd, buf, sizeof(buf));
      if (nread == 0) {
        break;
      }
      if (nread == -1) {
        perror("read");
        break;
      }
      std::cout.write(buf, nread);
    }
    std::cout << "=====" << std::endl;
  }

  close(fd);
  return 0;
}
