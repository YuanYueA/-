#include <string>
#include <memory>

#include "conn.h"

// Tcp服务器
struct TcpServer : private handy::noncopyable {
    TcpServer(handy::EventBases *bases):
        base_(bases->allocBase()),
        bases_(bases),
        listen_channel_(NULL),
        createcb_([]{ return TcpConnPtr(new TcpConn); }) {}
    ~TcpServer() { delete listen_channel_; }

    // 成功返回 0
    int bind(const std::string &host, unsigned short port, bool reusePort = false);
    static TcpServerPtr startServer(handy::EventBases *bases, const std::string &host, unsigned short port, bool reusePort = false){
        TcpServerPtr p(new TcpServer(bases));
        int r = p->bind(host, port, reusePort);
        return r == 0 ? p : NULL;
    }
    
    handy::Ip4Addr getAddr() { return addr_; }
    handy::EventBase *getBase() { return base_; }

    void onConnCreate(const std::function<TcpConnPtr()> &cb) { createcb_ = cb; }
    void onConnState(const TcpCallBack &cb) { statecb_ = cb; }
    void onConnRead(const TcpCallBack &cb) {
        readcb_ = cb;
        assert(!msgcb_);
    }
    // 消息处理与Read回调冲突，只能调用一个
    void onConnMsg(handy::CodecBase *codec, const MsgCallBack &cb) {
        codec_.reset(codec);
        msgcb_ = cb;
        assert(!readcb_);
    }

   private:
    handy::EventBase *base_;
    handy::EventBases *bases_;
    handy::Ip4Addr addr_;
    Channel* listen_channel_;
    TcpCallBack statecb_, readcb_;
    MsgCallBack msgcb_;
    std::function<TcpConnPtr()> createcb_;
    std::unique_ptr<handy::CodecBase> codec_;
    void handleAccept();
};