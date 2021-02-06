#pragma once
#include <poll.h>
#include <assert.h>
#include <iostream>
#include <map>
#include <atomic>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/epoll.h>


namespace handy {

const int kMaxEvents = 2000;

struct PollerBase {
    int64_t id_;
    int lastActive_;
    PollerBase(): lastActive_(-1) { static std::atomic<int64_t> id(0); id_ = ++id; }
    virtual void addChannel(Channel* ch) = 0;
    virtual void removeChannel(Channel* ch) = 0;
    virtual void updateChannel(Channel* ch) = 0;
    virtual void loop_once(int waitMs) = 0;
    virtual ~PollerBase(){};
};

const int kReadEvent = EPOLLIN;
const int kWriteEvent = EPOLLOUT;

struct PollerEpoll : public PollerBase{
    int fd_;
    std::set<Channel*> liveChannels_;
    //for epoll selected active events
    struct epoll_event activeEvs_[kMaxEvents];
    PollerEpoll();
    ~PollerEpoll();
    void addChannel(Channel* ch) override;
    void removeChannel(Channel* ch) override;
    void updateChannel(Channel* ch) override;
    void loop_once(int waitMs) override;
};

#define PlatformPoller PollerEpoll

}