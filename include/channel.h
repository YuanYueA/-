#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <iostream>
#include <memory.h>
#include <utility>
#include "poller.h"
#include "util.h"

struct Channel : private noncopyable {
    // base为事件管理器，fd为通道内部的fd，events为通道关心的事件
    Channel(int fd, int events);
    ~Channel();
    int fd() { return fd_; }
    //通道id
    int64_t id() { return id_; }
    short events() { return events_; }
    //关闭通道
    void close();

    //挂接事件处理器
    void onRead(const auto &readcb) { readcb_ = readcb; }
    void onWrite(const auto &writecb) { writecb_ = writecb; }
    void onRead(auto &&readcb) { readcb_ = readcb; }
    void onWrite(auto &&writecb) { writecb_ = writecb; }

    //启用读写监听
    void enableRead(bool enable){
        if (enable) {
            events_ |= EPOLLIN;
        } else {
            events_ &= ~EPOLLIN;
        }
        poller_->updateChannel(this);
    }
    void enableWrite(bool enable){
        if (enable) {
            events_ |= EPOLLOUT;
        } else {
            events_ &= ~EPOLLOUT;
        }
        poller_->updateChannel(this);
    }
    void enableReadWrite(bool readable, bool writable){
        if (enable) {
            events_ |= EPOLLIN;
        } else {
            events_ &= ~EPOLLIN;
        }
        if (enable) {
            events_ |= EPOLLOUT;
        } else {
            events_ &= ~EPOLLOUT;
        }
        poller_->updateChannel(this);
    };
    bool getReadStatus(){ return events_ & EPOLLIN; }
    bool getWriteStatus(){ return events_ & EPOLLOUT; };

    //处理读写事件
    void handleRead() { readcb_(); }
    void handleWrite() { writecb_(); }

   protected:
    PollerBase *poller_;
    int fd_;
    short events_;
    int64_t id_;
    std::function<void()> readcb_, writecb_, errorcb_;
};

#endif