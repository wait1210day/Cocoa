#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <sys/signalfd.h>
#include <unistd.h>
#include <csignal>
#include <memory>
#include <cstdio>

#include "core/IODispatcher.h"
#include "core/Journal.h"

namespace cocoa
{

IOSchedulable::~IOSchedulable()
{
    if (IODispatcher::instance())
        IODispatcher::instance()->removeCallables(this);
}

GOM_DEFINE(IODispatcher)
IODispatcher::IODispatcher()
    : Throwable("cocoa::IODispatcher")
{
    fFd = ::epoll_create(MAX_EPOLL_SIZE);
    dp_assert(fFd >= 0, "Failed to create an epoll instance");
    GOM_CONSTRUCT_DONE
}

IODispatcher::~IODispatcher()
{
    for (CallableHolder *pHolder : fCallables)
    {
        if (pHolder->closeOnRemove)
            ::close(pHolder->fd);
        delete pHolder;
    }
    ::close(fFd);
}

void IODispatcher::dp_assert(bool value, char const *fmt, ...)
{
    if (value)
        return;

    {
        va_list va;
        va_start(va, fmt);
        auto size = std::vsnprintf(nullptr, 0, fmt, va);
        std::unique_ptr<char> sp(new char[size + 1]);
        std::vsnprintf(sp.get(), size + 1, fmt, va);
        va_end(va);

        artfmt(__FUNCTION__) << sp.get();
    }
    arthrow<DispatchException>(new DispatchException());
}

int IODispatcher::castTriggerAsFlags(int trigger)
{
    int ret = 0;
    if (trigger & IOTrigger::kTrigger_Readable)
        ret |= EPOLLIN;
    if (trigger & IOTrigger::kTrigger_Writable)
        ret |= EPOLLOUT;
    if (trigger & IOTrigger::kTrigger_Hup)
        ret |= EPOLLHUP;
    if (trigger & IOTrigger::kTrigger_Error)
        ret |= EPOLLERR;
    return ret;
}

void IODispatcher::new_callable(IOSchedulable *pthis, IOCallable cb, int fd, bool closeOnRemove, uint32_t trigger, CallableHolder::Target target)
{
    CallableHolder *pHolder = new CallableHolder;
    pHolder->pthis = pthis;
    pHolder->method = cb;
    pHolder->fd = fd;
    pHolder->closeOnRemove = closeOnRemove;
    pHolder->target = target;

    struct epoll_event evt;
    evt.events = castTriggerAsFlags(trigger) | EPOLLET;
    evt.data.ptr = pHolder;

    int ret = ::epoll_ctl(fFd, EPOLL_CTL_ADD, fd, &evt);
    if (ret < 0)
    {
        delete pHolder;
        dp_assert(false, "Failed to register a new file descriptor to epoll");
        /* Execution is aborted */
    }

    fCallables.push_back(pHolder);
}

void IODispatcher::new_fd_callable(IOSchedulable *pthis, IOCallable cb, int fd, uint32_t trigger)
{
    new_callable(pthis, cb, fd, false, trigger, CallableHolder::Target::kFd);
}

void IODispatcher::new_timer_callable(IOSchedulable *pthis, IOCallable cb, const ::itimerspec& spec)
{
    int fd = ::timerfd_create(CLOCK_MONOTONIC, 0);
    dp_assert(fd >= 0, "Failed to create timer");
    ::timerfd_settime(fd, 0, &spec, nullptr);

    new_callable(pthis, cb, fd, true, IOTrigger::kTrigger_Readable, CallableHolder::Target::kTimer);
}

void IODispatcher::new_signal_callable(IOSchedulable *pthis, IOCallable cb, const std::set<int>& stl_sigset)
{
    int ret;

    ::sigset_t mask;
    ::sigemptyset(&mask);
    for (int sig : stl_sigset)
        ::sigaddset(&mask, sig);
    
    /* Avoiding signals being handled by default handler */
    ret = ::sigprocmask(SIG_BLOCK, &mask, nullptr);
    dp_assert(ret >= 0, "Failed to sigprocmask(SIG_BLOCK)");

    int sfd = ::signalfd(-1, &mask, SFD_NONBLOCK);
    dp_assert(sfd >= 0, "Failed to crate a signal file descriptor");

    new_callable(pthis, cb, sfd, true, IOTrigger::kTrigger_Readable, CallableHolder::Target::kSignal);
}

void IODispatcher::schedule()
{
    struct epoll_event evts[MAX_EPOLL_SIZE];
    int nfds = ::epoll_wait(fFd, evts, MAX_EPOLL_SIZE, -1);
    if (nfds <= 0)
        return;
    
    for (int i = 0; i < nfds; i++)
    {
        struct epoll_event *pEvent = &evts[i];
        CallableHolder *pHolder = reinterpret_cast<CallableHolder*>(pEvent->data.ptr);
        CallableArgs args;
        fillCallableArgs(pHolder, args, pEvent->events);

        pHolder->method(&args);
    }
}

void IODispatcher::fillCallableArgs(CallableHolder *pHolder, CallableArgs& args, int __events)
{
    uint32_t trigger = 0;
    if (__events & EPOLLIN)
        trigger |= IOTrigger::kTrigger_Readable;
    if (__events & EPOLLOUT)
        trigger |= IOTrigger::kTrigger_Writable;
    if (__events & EPOLLHUP)
        trigger |= IOTrigger::kTrigger_Hup;
    if (__events & EPOLLERR)
        trigger |= IOTrigger::kTrigger_Error;
    args.trigger = trigger;
    args.fd = pHolder->fd;

    switch (pHolder->target)
    {
    case CallableHolder::Target::kSignal:
        fillCallableArgsSignalfd(pHolder, args);
        break;
    case CallableHolder::Target::kTimer:
        fillCallableArgsTimerfd(pHolder, args);
        break;
    default:
        break;
    }
}

void IODispatcher::fillCallableArgsTimerfd(CallableHolder *pHolder, CallableArgs& args)
{
    uint64_t tm;
    int ret = ::read(pHolder->fd, &tm, sizeof(uint64_t));
    dp_assert(ret == sizeof(uint64_t), "Failed to read timer");
    args.timer_timeoutCount = tm;
}

void IODispatcher::fillCallableArgsSignalfd(CallableHolder *pHolder, CallableArgs& args)
{
    ::signalfd_siginfo siginfo;
    while (::read(pHolder->fd, &siginfo, sizeof(::signalfd_siginfo)) == sizeof(::signalfd_siginfo))
        args.signal_siginfo.push_back(siginfo);
    
    dp_assert(!args.signal_siginfo.empty(), "Failed to read signal file descriptor");
}

void IODispatcher::removeCallableHolder(CallableHolder *pHolder, bool removeFromList)
{
    if (pHolder->closeOnRemove)
        ::close(pHolder->fd);
    delete pHolder;
    if (removeFromList)
        fCallables.remove(pHolder);
}

void IODispatcher::removeCallables(IOSchedulable *pthis)
{
    auto itr = fCallables.begin();
    while (itr != fCallables.end())
    {
        CallableHolder *ptr = *itr;
        if (ptr->pthis == pthis)
        {
            removeCallableHolder(ptr, false);
            itr = fCallables.erase(itr);
            if (itr == fCallables.end())
                break;
        }
        itr++;
    }
}

void IODispatcher::removeFdCallable(int fd)
{
    auto itr = fCallables.begin();
    while (itr != fCallables.end())
    {
        CallableHolder *ptr = *itr;
        if (ptr->target == CallableHolder::Target::kFd && ptr->fd == fd)
        {
            removeCallableHolder(ptr, false);
            itr = fCallables.erase(itr);
            if (itr == fCallables.end())
                break;
        }
        itr++;
    }
}

} // namespace cocoa
