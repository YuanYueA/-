#pragma once

#include <netinet/in.h>
#include <string>
#include <netdb.h>
#include <cstring>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

namespace handy {
namespace port {
static const int kLittleEndian = LITTLE_ENDIAN;
inline uint16_t htobe(uint16_t v) {
    if (!kLittleEndian) {
        return v;
    }
    unsigned char *pv = (unsigned char *) &v;
    return uint16_t(pv[0]) << 8 | uint16_t(pv[1]);
}
inline uint32_t htobe(uint32_t v) {
    if (!kLittleEndian) {
        return v;
    }
    unsigned char *pv = (unsigned char *) &v;
    return uint32_t(pv[0]) << 24 | uint32_t(pv[1]) << 16 | uint32_t(pv[2]) << 8 | uint32_t(pv[3]);
}
inline uint64_t htobe(uint64_t v) {
    if (!kLittleEndian) {
        return v;
    }
    unsigned char *pv = (unsigned char *) &v;
    return uint64_t(pv[0]) << 56 | uint64_t(pv[1]) << 48 | uint64_t(pv[2]) << 40 | uint64_t(pv[3]) << 32 | uint64_t(pv[4]) << 24 | uint64_t(pv[5]) << 16 |
           uint64_t(pv[6]) << 8 | uint64_t(pv[7]);
}
inline int16_t htobe(int16_t v) {
    return (int16_t) htobe((uint16_t) v);
}
inline int32_t htobe(int32_t v) {
    return (int32_t) htobe((uint32_t) v);
}
inline int64_t htobe(int64_t v) {
    return (int64_t) htobe((uint64_t) v);
}
struct in_addr getHostByName(const std::string& host) {
    struct in_addr addr;
    char buf[1024];
    struct hostent hent;
    struct hostent* he = NULL;
    int herrno = 0;
    memset(&hent, 0, sizeof hent);
    int r = gethostbyname_r(host.c_str(), &hent, buf, sizeof buf, &he, &herrno);
    if (r == 0 && he && he->h_addrtype==AF_INET) {
        addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
    } else {
        addr.s_addr = INADDR_NONE;
    }
    return addr;
}
uint64_t gettid() { return syscall(SYS_gettid); }
}  // namespace port
}  // namespace handy