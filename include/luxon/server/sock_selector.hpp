// Copyright (c) 2026, the Luxon Server contributors
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "offset_map.hpp"

#include <vector>
#include <functional>
#include <unordered_map>
#include <utility>
#include <cstdint>

#if defined(__linux__)
#include <unistd.h>
#include <sys/epoll.h>
#elif defined(_WIN32)
#include "wepoll.h"
#else
#include <sys/select.h>
#endif

namespace server {
class SockSelector {
public:
    enum Interest : uint8_t { none = 0b000, read = 0b001, write = 0b010, except = 0b100 };

#ifdef _WIN32
    using socket_t = SOCKET;
    using epoll_t = HANDLE;
    template <typename V> using callback_map_t = std::unordered_map<SOCKET, V>;
#else
    using socket_t = int;
    using epoll_t = int;
    template <typename V> using callback_map_t = offset_map<unsigned, V>;
#endif

    using callback_t = std::function<void(socket_t)>;

private:
#if defined(__linux__) || defined(_WIN32)
    epoll_t epoll_fd_;
    std::vector<struct epoll_event> events_;
#else
    fd_set read_fd_set_, write_fd_set_, except_fd_set_;
    std::vector<socket_t> fds_;
#ifndef _WIN32
    socket_t highest_fd_;
#else
    constexpr static socket_t highest_fd_ = 0;
#endif
#endif

    callback_map_t<callback_t> callbacks_;

public:
    SockSelector();
    ~SockSelector();

    // Zero-timeout means potentially forever-blocking
    bool run(unsigned timeout_ms = 0);

    bool add_fd(socket_t fd, callback_t&& callback, uint8_t interest);
    void change_fd(socket_t fd, uint8_t interest);
    void remove_fd(socket_t fd);

    bool add_read_fd(socket_t fd, callback_t&& callback) { return add_fd(fd, std::move(callback), read | except); }
};
} // namespace server
