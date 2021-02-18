#ifndef COCOA_THREAD_H
#define COCOA_THREAD_H

#include <string>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <queue>
#include <condition_variable>
#include <atomic>

#include <Poco/Dynamic/Var.h>

#include "ciallo/GrBase.h"
CIALLO_BEGIN_NS

class Worker;

class Thread
{
public:
    enum InternalOpcode
    {
        kTaskOp_Exit    = -1
    };

    enum class CmdExecuteResult
    {
        kNormal,
        kStopExecution
    };
    
    class Fence
    {
    public:
        explicit Fence(uint32_t rank = 1);
        Fence(const Fence&) = delete;
        Fence& operator=(const Fence&) = delete;

        void wait();
        void signal();

    private:
        std::condition_variable fCondition;
        std::mutex              fMutex;
        uint32_t                fRank;
        uint32_t                fCount;
    };

    class Command
    {
    public:
        Command(int32_t taskOp, const Poco::Dynamic::Var& userdata);
        Command(Command&& that) noexcept;
        Command();

        Command(const Command&) = delete;
        Command& operator=(const Command&) = delete;

        inline bool valid() const
        { return fValid; }

        inline int32_t opcode() const
        { return fOpcode; }

        inline const Poco::Dynamic::Var& userdata() const
        { return fUserdata; }

        inline std::shared_ptr<Fence> fence()
        { return fFence; }

        void notifyFinish();

    private:
        bool                   fValid;
        int32_t                fOpcode;
        std::shared_ptr<Fence> fFence;
        Poco::Dynamic::Var     fUserdata;
    };

    Thread(const std::string& name, Worker *worker);
    virtual ~Thread();

    std::shared_ptr<Fence> enqueueCmd(Command cmd);

private:
    void thread_worker_entry();
    CmdExecuteResult cmdExecute(const Command& cmd);

private:
    std::string             fName;
    std::mutex              fQueueMutex;
    std::queue<Command>     fCommandQueue;
    Worker                 *fWorker;
    std::thread             fThread;
    int                     fEventFd;
};

class Worker
{
public:
    virtual ~Worker() = default;

    virtual void init();
    virtual void final();
    virtual Thread::CmdExecuteResult execute(const Thread::Command& cmd) = 0;
};

CIALLO_END_NS
#endif //COCOA_THREAD_H
