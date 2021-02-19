#pragma once

#include <algorithm>
#include <iostream>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#include "util.h"
#include "slice.h"
#include "port_posix.h"

namespace handy {

struct net {
    template <class T>
    static T hton(T v) {
        return port::htobe(v);
    }
    template <class T>
    static T ntoh(T v) {
        return port::htobe(v);
    }
    static int setNonBlock(int fd, bool value = true){
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0) {
            return errno;
        }
        if (value) {
            return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
        }
        return fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    }
    static int setReuseAddr(int fd, bool value = true){
        int flag = value;
        int len = sizeof flag;
        return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &flag, len);
    }
    static int setReusePort(int fd, bool value = true){
        #ifndef SO_REUSEPORT
            fatalif(value, "SO_REUSEPORT not supported");
            return 0;
        #else
            int flag = value;
            int len = sizeof flag;
            return setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &flag, len);
        #endif
    }
    static int setNoDelay(int fd, bool value = true){
        int flag = value;
        int len = sizeof flag;
        return setsockopt(fd, SOL_SOCKET, TCP_NODELAY, &flag, len);
    }
};

struct Ip4Addr {
    Ip4Addr(const std::string &host, unsigned short port){
        memset(&addr_, 0, sizeof addr_);
        addr_.sin_family = AF_INET;
        addr_.sin_port = htons(port);
        if (host.size()) {
            addr_.sin_addr = port::getHostByName(host);
        } else {
            addr_.sin_addr.s_addr = INADDR_ANY;
        }
        if (addr_.sin_addr.s_addr == INADDR_NONE){
            std::cout << "cannot resove " << host.c_str() << " to ip" << std::endl;
        }
    }
    Ip4Addr(unsigned short port = 0) : Ip4Addr("", port) {}
    Ip4Addr(const struct sockaddr_in &addr) : addr_(addr){}

    std::string toString() const {
        uint32_t uip = addr_.sin_addr.s_addr;
        return util::format("%d.%d.%d.%d:%d",
            (uip >> 0)&0xff,
            (uip >> 8)&0xff,
            (uip >> 16)&0xff,
            (uip >> 24)&0xff,
            ntohs(addr_.sin_port));
    }
    std::string ip() const {
        uint32_t uip = addr_.sin_addr.s_addr;
        return util::format("%d.%d.%d.%d",
            (uip >> 0)&0xff,
            (uip >> 8)&0xff,
            (uip >> 16)&0xff,
            (uip >> 24)&0xff);
    }
    unsigned short port() const {
        return ntohs(addr_.sin_port);
    }
    unsigned int ipInt() const {
        return ntohl(addr_.sin_addr.s_addr);
    }
    // if you pass a hostname to constructor, then use this to check error
    bool isIpValid() const {
        return addr_.sin_addr.s_addr != INADDR_NONE;
    }
    struct sockaddr_in &getAddr() {
        return addr_;
    }
    static std::string hostToIp(const std::string &host) {
        Ip4Addr addr(host, 0);
        return addr.ip();
    }

   private:
    struct sockaddr_in addr_;
};

struct Buffer {
    Buffer() : buf_(NULL), b_(0), e_(0), cap_(0), exp_(512) {}
    ~Buffer() { delete[] buf_; }
    void clear() {
        delete[] buf_;
        buf_ = NULL;
        cap_ = 0;
        b_ = e_ = 0;
    }
    size_t size() const { return e_ - b_; }
    bool empty() const { return e_ == b_; }
    char *data() const { return buf_ + b_; }
    char *begin() const { return buf_ + b_; }
    char *end() const { return buf_ + e_; }
    char *makeRoom(size_t len) {
        if (e_ + len <= cap_) {
        } else if (size() + len < cap_ / 2) {
            moveHead();
        } else {
            expand(len);
        }
        return end();
    }
    void makeRoom() {
        if (space() < exp_)
            expand(0);
    }
    size_t space() const { return cap_ - e_; }
    void addSize(size_t len) { e_ += len; }
    char *allocRoom(size_t len) {
        char *p = makeRoom(len);
        addSize(len);
        return p;
    }
    Buffer &append(const char *p, size_t len) {
        memcpy(allocRoom(len), p, len);
        return *this;
    }
    Buffer &append(Slice slice) { return append(slice.data(), slice.size()); }
    Buffer &append(const char *p) { return append(p, strlen(p)); }
    template <class T>
    Buffer &appendValue(const T &v) {
        append((const char *) &v, sizeof v);
        return *this;
    }
    Buffer &consume(size_t len) {
        b_ += len;
        if (size() == 0)
            clear();
        return *this;
    }
    Buffer &absorb(Buffer &buf) {
        if (&buf != this) {
            if (size() == 0) {
                char b[sizeof buf];
                memcpy(b, this, sizeof b);
                memcpy(this, &buf, sizeof b);
                memcpy(&buf, b, sizeof b);
                std::swap(exp_, buf.exp_); //keep the origin exp_
            } else {
                append(buf.begin(), buf.size());
                buf.clear();
            }
        }
        return *this;
    }
    void setSuggestSize(size_t sz) { exp_ = sz; }
    Buffer(const Buffer &b) { copyFrom(b); }
    Buffer &operator=(const Buffer &b) {
        if (this == &b)
            return *this;
        delete[] buf_;
        buf_ = NULL;
        copyFrom(b);
        return *this;
    }
    operator Slice() { return Slice(data(), size()); }

   private:
    char *buf_;
    size_t b_, e_, cap_, exp_;
    void moveHead() {
        std::copy(begin(), end(), buf_);
        e_ -= b_;
        b_ = 0;
    }
    void expand(size_t len){
        size_t ncap = std::max(exp_, std::max(2*cap_, size()+len));
        char* p = new char[ncap];
        std::copy(begin(), end(), p);
        e_ -= b_;
        b_ = 0;
        delete[] buf_;
        buf_ = p;
        cap_ = ncap;
    }
    void copyFrom(const Buffer &b){
        memcpy(this, &b, sizeof b); 
        if (size()) { 
            buf_ = new char[cap_]; 
            memcpy(data(), b.begin(), b.size());
        }
    }
};

}  // namespace handy
