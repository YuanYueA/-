#include <fcntl.h>

#include "../include/TcpServer.h"

int TcpServer::bind(const std::string &host, unsigned short port, bool reusePort) {
    addr_ = handy::Ip4Addr(host, port);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int r = handy::net::setReuseAddr(fd);
    if(r){
        std::cout << "set socket reuse option failed" << std::endl;
    }
    r = handy::net::setReusePort(fd, reusePort);
    if(r){
        std::cout << "set socket reuse port option failed" << std::endl;
    }
    r = handy::util::addFdFlag(fd, FD_CLOEXEC);
    if(r){
        std::cout << "addFdFlag FD_CLOEXEC failed" << std::endl;
    }
    r = ::bind(fd,(struct sockaddr *)&addr_.getAddr(),sizeof(struct sockaddr));
    if (r) {
        close(fd);
        std::cout << "bind to " << addr_.toString().c_str() << ": " << port << " failed " 
            << errno << " " << strerror(errno) << std::endl;
        return errno;
    }
    r = listen(fd, 20);
    if(r){
        std::cout << "listen failed " << errno << ": " << strerror(errno) << std::endl;
        return -1;
    }
    std::cout << "fd: " << fd << "listening at " << addr_.toString().c_str() << std::endl;
    listen_channel_ = new Channel(fd, EPOLLIN);
    listen_channel_->onRead([this]{ handleAccept(); });
    return 0;
}

void TcpServer::handleAccept() {
    struct sockaddr_in raddr;
    socklen_t rsz = sizeof(raddr);
    int lfd = listen_channel_->fd();
    int cfd;
    while (lfd >= 0 && (cfd = accept(lfd,(struct sockaddr *)&raddr,&rsz))>=0) {
        sockaddr_in peer, local;
        socklen_t alen = sizeof(peer);
        int r = getpeername(cfd, (sockaddr*)&peer, &alen);
        if (r < 0) {
            std::cout << "get peer name failed " << errno << " " << strerror(errno) << std::endl;
            continue;
        }
        r = getsockname(cfd, (sockaddr*)&local, &alen);
        if (r < 0) {
            std::cout << "get sock name failed " << errno << " " << strerror(errno) << std::endl;
            continue;
        }
        r = handy::util::addFdFlag(cfd, FD_CLOEXEC);
        if(r){
            std::cout << "addFdFlag FD_CLOEXEC failed" << std::endl;
        }
        handy::EventBase* b = bases_->allocBase();
        auto addcon = [=] {
            TcpConnPtr con = createcb_();
            con->attach(b, cfd, local, peer);
            if (readcb_) {
                con->onRead(readcb_);
            }
            if (msgcb_) {
                con->onMsg(codec_->clone(), msgcb_);
            }
        };
        if (b == base_) {
            addcon();
        } else {
            b->safeCall(std::move(addcon));
        }
    }
    if (lfd >= 0 && errno != EAGAIN && errno != EINTR) {
        std::cout << "accept return " << cfd << " " << errno << " " << strerror(errno) << std::endl;
    }
}