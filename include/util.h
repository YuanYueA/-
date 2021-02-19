#pragma once
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <functional>
#include <string>
#include <utility>
#include <memory>

namespace handy {

// 继承此类的对象无法拷贝构造或赋值
struct noncopyable {
   protected:
    noncopyable() = default;
    virtual ~noncopyable() = default;

    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;
};

struct util {
    // 对输入的参数进行格式化输出
    static std::string format(const char *fmt, ...) {
        char buffer[500];
        std::unique_ptr<char[]> release1;
        char* base;
        for (int iter = 0; iter < 2; iter++) {
            int bufsize;
            if (iter == 0) {
                bufsize = sizeof(buffer);
                base = buffer;
            } else {
                bufsize = 30000;
                base = new char[bufsize];
                release1.reset(base);
            }
            char* p = base;
            char* limit = base + bufsize;
            if (p < limit) {
                va_list ap;
                va_start(ap, fmt);
                p += vsnprintf(p, limit - p, fmt, ap);
                va_end(ap);
            }
            // Truncate to available space if necessary
            if (p >= limit) {
                if (iter == 0) {
                    continue;       // Try again with larger buffer
                } else {
                    p = limit - 1;
                    *p = '\0';
                }
            }
            break;
        }
        return base;
    }

    // 精度为微秒，获取当前系统时间戳
    static int64_t timeMicro() {
        std::chrono::time_point<std::chrono::system_clock> p = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(p.time_since_epoch()).count();
    }
    // 精度为毫秒
    static int64_t timeMilli() { return timeMicro() / 1000; }
    // 精度为微秒，计时器时间戳，用来记录系统耗时
    static int64_t steadyMicro() {
        std::chrono::time_point<std::chrono::steady_clock> p = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(p.time_since_epoch()).count();
    }
    // 精度为毫秒
    static int64_t steadyMilli() { return steadyMicro() / 1000; }
    // 精度为秒，时间戳转化为可读时间
    static std::string readableTime(time_t t) {
        struct tm tm1;
        localtime_r(&t, &tm1);
        return format("%04d-%02d-%02d %02d:%02d:%02d",
            tm1.tm_year+1900, tm1.tm_mon+1, tm1.tm_mday, tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
    }

    // 字符串转整型值 （可直接使用 c 语言库的 atoi 等函数，这里进行了重写）
    static int64_t atoi(const char *b, const char *e) { return strtol(b, (char **) &e, 10); }
    static int64_t atoi(const char *b) { return atoi(b, b + strlen(b)); }
    static int64_t atoi2(const char *b, const char *e) {
        char **ne = (char **) &e;
        int64_t v = strtol(b, ne, 10);
        return ne == (char **) &e ? v : -1;
    }

    // 改变已打开的文件性质
    static int addFdFlag(int fd, int flag) {
        int ret = fcntl(fd, F_GETFD);
        return fcntl(fd, F_SETFD, ret | flag);
    }
};

struct ExitCaller : private noncopyable {
    ~ExitCaller() { functor_(); }
    ExitCaller(std::function<void()> &&functor) : functor_(std::move(functor)) {}

   private:
    std::function<void()> functor_;
};

}  // namespace handy
