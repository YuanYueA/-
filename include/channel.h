#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <iostream>
#include <functional>
#include <memory.h>
#include <utility>
#include "poller.h"
#include "util.h"

typedef std::function<void()> Task;
struct Channel : private handy::noncopyable {
    // base为事件管理器，fd为通道内部的fd，events为通道关心的事件
    Channel(int fd, int events);
    virtual ~Channel();

    int fd() const { return fd_; }
    // 通道 id
    int64_t id() const { return id_; }
    short events() const { return events_; }
    // 关闭通道
    void close();

    // 挂接事件处理器
    void onRead(const Task &readcb) { readcb_ = readcb; }
    void onWrite(const Task &writecb) { writecb_ = writecb; }
    void onRead(Task &&readcb) { readcb_ = readcb; }
    void onWrite(Task &&writecb) { writecb_ = writecb; }

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
        if (readable) {
            events_ |= EPOLLIN;
        } else {
            events_ &= ~EPOLLIN;
        }
        if (writable) {
            events_ |= EPOLLOUT;
        } else {
            events_ &= ~EPOLLOUT;
        }
        poller_->updateChannel(this);
    }
    bool getReadStatus(){ return events_ & EPOLLIN; }
    bool getWriteStatus(){ return events_ & EPOLLOUT; }

    //处理读写事件
    void handleRead() { readcb_(); }
    void handleWrite() { writecb_(); }

   protected:
    handy::PollerBase *poller_;
    int fd_;
    short events_;
    int64_t id_;
    Task readcb_, writecb_, errorcb_;
};

#endif