// Copyright (c) 2026, the Luxon Server contributors
// SPDX-License-Identifier: BSD-3-Clause

#include "sock_selector.hpp"

#include <tracy/Tracy.hpp>

namespace server {
namespace {
#if defined(__linux__) || defined(_WIN32)
uint32_t make_epoll_events(uint8_t interest) {
    uint32_t events = EPOLLHUP;

    if (interest & SockSelector::read)
        events |= EPOLLIN | EPOLLRDHUP;
    if (interest & SockSelector::write)
        events |= EPOLLOUT;
    if (interest & SockSelector::except)
        events |= EPOLLERR;

    return events;
}
#else

timeval ms_to_timeval(unsigned long long ms) {
    timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    return tv;
}
#endif
} // namespace

SockSelector::SockSelector() {
#if defined(__linux__) || defined(_WIN32)
    epoll_fd_ = epoll_create1(0);
    events_.resize(128);
#else
    FD_ZERO(&read_fd_set_);
    FD_ZERO(&write_fd_set_);
    FD_ZERO(&except_fd_set_);
#ifndef _WIN32
    highest_fd_ = 0;
#endif
#endif
}

SockSelector::~SockSelector() {
#if defined(__linux__)
    if (epoll_fd_ >= 0)
        close(epoll_fd_);
#else
#ifdef _WIN32
    if (epoll_fd_)
        epoll_close(epoll_fd_);
#endif
#endif
}

bool SockSelector::run(unsigned timeout_ms) {
#if defined(__linux__) || defined(_WIN32)
    int t = (timeout_ms == 0) ? -1 : static_cast<int>(timeout_ms);
    int nfds = epoll_wait(epoll_fd_, events_.data(), events_.size(), t);

    ZoneScoped;

    if (nfds < 0)
        return false;

    for (int i = 0; i < nfds; ++i) {
        const socket_t fd = static_cast<socket_t>(events_[i].data.u64);
        callbacks_[fd](fd);
    }

    if (nfds == (int)events_.size())
        events_.resize(events_.size() * 2);

    return true;
#else
    timeval tv;
    if (timeout_ms)
        tv = ms_to_timeval(timeout_ms);

    // Work copy
    fd_set read_fd_wset = read_fd_set_;
    fd_set write_fd_wset = write_fd_set_;
    fd_set except_fd_wset = except_fd_set_;
    int fd_count = select(highest_fd_ + 1, &read_fd_wset, &write_fd_wset, &except_fd_wset, timeout_ms ? &tv : nullptr);
    if (fd_count < 0)
        return false;

    for (auto fd : fds_)
        if (FD_ISSET(fd, &read_fd_wset) || FD_ISSET(fd, &write_fd_wset) || FD_ISSET(fd, &except_fd_wset))
            callbacks_[fd](fd);

    return true;
#endif
}

bool SockSelector::add_fd(socket_t fd, callback_t&& callback, uint8_t interest) {
    ZoneScoped;

#if defined(__linux__) || defined(_WIN32)
    struct epoll_event ev{};
    ev.events = make_epoll_events(interest);
    ev.data.u64 = static_cast<uint64_t>(fd);

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) < 0)
        return false;
#else
    if (fd < 0 || fd >= FD_SETSIZE)
        return false; // Prevent stack corruption

    fds_.push_back(fd);

    if (interest & read)
        FD_SET(fd, &read_fd_set_);
    if (interest & write)
        FD_SET(fd, &write_fd_set_);
    if (interest & except)
        FD_SET(fd, &except_fd_set_);

    if (fd > highest_fd_)
        highest_fd_ = fd;
#endif

    callbacks_[fd] = std::move(callback);

    return true;
}

void SockSelector::change_fd(socket_t fd, uint8_t interest) {
#if defined(__linux__) || defined(_WIN32)
    struct epoll_event ev{};
    ev.events = make_epoll_events(interest);
    ev.data.u64 = static_cast<uint64_t>(fd);
    (void)epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
#else
    if (interest & read)
        FD_SET(fd, &read_fd_set_);
    else
        FD_CLR(fd, &read_fd_set_);

    if (interest & write)
        FD_SET(fd, &write_fd_set_);
    else
        FD_CLR(fd, &write_fd_set_);

    if (interest & except)
        FD_SET(fd, &except_fd_set_);
    else
        FD_CLR(fd, &except_fd_set_);
#endif
}

void SockSelector::remove_fd(socket_t fd) {
    ZoneScoped;

#if defined(__linux__) || defined(_WIN32)
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
#else
    FD_CLR(fd, &read_fd_set_);
    FD_CLR(fd, &write_fd_set_);
    FD_CLR(fd, &except_fd_set_);

    // O(1) removal (swap and pop)
    for (size_t i = 0; i < fds_.size(); ++i) {
        if (fds_[i] == fd) {
            fds_[i] = fds_.back();
            fds_.pop_back();
            break;
        }
    }

    if (fd == highest_fd_) {
        highest_fd_ = 0;
        for (auto s : fds_)
            if (s > highest_fd_)
                highest_fd_ = s;
    }
#endif

    callbacks_.erase(fd);
}
} // namespace server
