#include <map>
#include <string.h>
#include <iostream>
#include <errno.h>
#include <fcntl.h>

#include "../include/event_base.h"
//#include "../include/logging.h"
#include "../include/util.h"
#include "../include/poller.h"
#include "../include/conn.h"
namespace handy {

PollerEpoll::PollerEpoll(){
    fd_ = epoll_create1(EPOLL_CLOEXEC);
    if(fd_ < 0){
        std::cout << "epoll_create error " << errno << ": " << strerror(errno) << std::endl;
    }
    else
        std::cout << "poller epoll " << fd_ << " created" << std::endl;
    //fatalif(fd_<0, "epoll_create error %d %s", errno, strerror(errno));
    //info("poller epoll %d created", fd_);
}

PollerEpoll::~PollerEpoll() {
    std::cout << "destroying poller " << fd_ << std::endl;
    //info("destroying poller %d", fd_);
    while (liveChannels_.size()) {
        (*liveChannels_.begin())->close();
    }
    ::close(fd_);
    std::cout << "poller " << fd_ << " destroyed" << std::endl;
    //info("poller %d destroyed", fd_);
}

void PollerEpoll::addChannel(Channel* ch) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev)); //0填充
    ev.events = ch->events();
    ev.data.ptr = ch;
    std::cout << "adding channel:" << ch->id() << " fd:" << ch->fd() << " events:" << ev.events << " epoll:" << fd_ << std::endl;
    //trace("adding channel %lld fd %d events %d epoll %d", (long long)ch->id(), ch->fd(), ev.events, fd_);
    int r = epoll_ctl(fd_, EPOLL_CTL_ADD, ch->fd(), &ev);
    if(r < 0)
        std::cout << "epoll_ctl add failed " << errno << " " << strerror(errno) << std::endl;
    //fatalif(r, "epoll_ctl add failed %d %s", errno, strerror(errno));
    liveChannels_.insert(ch);
}

void PollerEpoll::updateChannel(Channel* ch) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = ch->events();
    ev.data.ptr = ch;
    trace("modifying channel %lld fd %d events read %d write %d epoll %d",
          (long long)ch->id(), ch->fd(), ev.events & POLLIN, ev.events & POLLOUT, fd_);
    int r = epoll_ctl(fd_, EPOLL_CTL_MOD, ch->fd(), &ev);
    fatalif(r, "epoll_ctl mod failed %d %s", errno, strerror(errno));
}

void PollerEpoll::removeChannel(Channel* ch) {
    trace("deleting channel %lld fd %d epoll %d", (long long)ch->id(), ch->fd(), fd_);
    liveChannels_.erase(ch);
    for (int i = lastActive_; i >= 0; i --) {
        if (ch == activeEvs_[i].data.ptr) {
            activeEvs_[i].data.ptr = NULL;
            break;
        }
    }
}

void PollerEpoll::loop_once(int waitMs) {
    int64_t ticks = util::timeMilli();
    lastActive_ = epoll_wait(fd_, activeEvs_, kMaxEvents, waitMs);
    int64_t used = util::timeMilli()-ticks;
    trace("epoll wait %d return %d errno %d used %lld millsecond",
          waitMs, lastActive_, errno, (long long)used);
    fatalif(lastActive_ == -1 && errno != EINTR, "epoll return error %d %s", errno, strerror(errno));
    while (--lastActive_ >= 0) {
        int i = lastActive_;
        Channel* ch = (Channel*)activeEvs_[i].data.ptr;
        int events = activeEvs_[i].events;
        if (ch) {
            if (events & (kReadEvent | POLLERR)) {
                trace("channel %lld fd %d handle read", (long long)ch->id(), ch->fd());
                ch->handleRead();
            } else if (events & kWriteEvent) {
                trace("channel %lld fd %d handle write", (long long)ch->id(), ch->fd());
                ch->handleWrite();
            } else {
                fatal("unexpected poller events");
            }
        }
    }
}
}