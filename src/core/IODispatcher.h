#ifndef __Included_IODispatcher__
#define __Included_IODispatcher__

#include "core/ObjectHolder.h"
#include "core/Exception.h"

#include <string>
#include <sys/time.h>
#include <list>
#include <set>
#include <vector>
#include <concepts>
#include <functional>
#include <ffi.h>
#include <sys/signalfd.h>

/* Function signature */
#define io_callback
#define slot

namespace cocoa
{

class DispatchException : public ARTException
{
public:
    DispatchException() = default;
    ~DispatchException() = default;
};

class IODispatcher;

class IOSchedulable
{
protected:
    virtual ~IOSchedulable();
};

enum IOTrigger
{
    kTrigger_Readable    = 0x01,
    kTrigger_Writable    = 0x02,
    kTrigger_Hup         = 0x04,
    kTrigger_Error       = 0x08
};

// using IOCallable = void(IOSchedulable::*)(int trigger);
struct CallableArgs
{
    uint32_t trigger;
    
    uint64_t timer_timeoutCount;
    std::vector<signalfd_siginfo> signal_siginfo;
    int fd;
};

using IOCallable = std::function<void(CallableArgs*)>;
struct CallableHolder
{
    enum class Target
    {
        kFd,
        kTimer,
        kSignal
    } target;
    bool           closeOnRemove;
    int            fd;
    IOSchedulable *pthis;
    IOCallable     method;
};

template<typename T>
concept BaseOfSchedulable = std::is_base_of<IOSchedulable, T>::value;

class IODispatcher : public QObject,
                     public Throwable
{
    GOM_DECLARE(IODispatcher)
public:
    static const int MAX_EPOLL_SIZE = 128;

    IODispatcher();
    ~IODispatcher();

    /* Asynchronous IO: */
    void schedule();

    template<BaseOfSchedulable _Tp>
    void newFdCallable(_Tp *pthis, void(_Tp::*func)(CallableArgs*), int fd, int trigger) {
        IOCallable cb = std::bind(func, pthis, std::placeholders::_1);
        new_fd_callable(pthis, cb, fd, trigger);
    }

    template<BaseOfSchedulable _Tp>
    void newTimerCallable(_Tp *pthis, void(_Tp::*func)(CallableArgs*), const ::itimerspec& spec) {
        IOCallable cb = std::bind(func, pthis, std::placeholders::_1);
        new_timer_callable(pthis, cb, spec);
    }

    template<BaseOfSchedulable _Tp>
    void newSoftirqCallable(_Tp *pthis, void(_Tp::*func)(CallableArgs*), const std::set<int>& sigset) {
        IOCallable cb = std::bind(func, pthis, std::placeholders::_1);
        new_signal_callable(pthis, cb, sigset);
    }

    void removeCallables(IOSchedulable *pthis);
    void removeFdCallable(int fd);

private:
    void new_fd_callable(IOSchedulable *pthis, IOCallable cb, int fd, uint32_t trigger);
    void new_timer_callable(IOSchedulable *pthis, IOCallable cb, const ::itimerspec& spec);
    void new_signal_callable(IOSchedulable *pthis, IOCallable cb, const std::set<int>& sigset);
    void new_callable(IOSchedulable *pthis, IOCallable cb, int fd, bool closeOnRemove, uint32_t trigger, CallableHolder::Target target);

    void removeCallableHolder(CallableHolder *holder, bool removeFromList);

    void fillCallableArgs(CallableHolder *pHolder, CallableArgs& args, int epoll_event);
    void fillCallableArgsSignalfd(CallableHolder *pHolder, CallableArgs& args);
    void fillCallableArgsTimerfd(CallableHolder *pHolder, CallableArgs& args);
    int castTriggerAsFlags(int trigger);
    void dp_assert(bool value, char const *fmt, ...);

private:
    int                         fFd;
    std::list<CallableHolder*>  fCallables;
};

} // namespace cocoa

#endif /* __Included_IODispatcher__ */
