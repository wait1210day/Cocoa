#include <unistd.h>
#include <sys/eventfd.h>

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <Poco/Dynamic/Var.h>

#include "ciallo/Thread.h"
CIALLO_BEGIN_NS

Thread::Fence::Fence(uint32_t rank)
    : fRank(rank),
      fCount(0)
{
}

void Thread::Fence::wait()
{
    std::unique_lock<std::mutex> scopedLock(fMutex);
    while (fCount < fRank)
        fCondition.wait(scopedLock);
}

void Thread::Fence::signal()
{
    std::scoped_lock<std::mutex> scopedLock(fMutex);
    fCount++;
    fCondition.notify_one();
}

// --------------------------------------------------------------

Thread::Command::Command()
    : fValid(false),
      fFence(nullptr) {}

Thread::Command::Command(int32_t taskOp, const Poco::Dynamic::Var& userdata)
    : fValid(true),
      fOpcode(taskOp),
      fFence(std::make_shared<Fence>()),
      fUserdata(userdata) {}

Thread::Command::Command(Command&& that) noexcept
    : fValid(that.fValid),
      fOpcode(that.fOpcode),
      fFence(std::move(that.fFence)),
      fUserdata(that.fUserdata)
{
    that.fValid = false;
    that.fFence = nullptr;
}

void Thread::Command::notifyFinish()
{
    if (fValid)
        fFence->signal();
}

// -----------------------------------------------------------------

Thread::Thread(const std::string& name, Worker *worker)
    : fName(name),
      fWorker(worker),
      fThread(&Thread::thread_worker_entry, this)
{
    fEventFd = ::eventfd(0, EFD_CLOEXEC | EFD_SEMAPHORE);
}

Thread::~Thread()
{
    if (fThread.joinable())
    {
        enqueueCmd(Command(InternalOpcode::kTaskOp_Exit, Poco::Dynamic::Var()));
        fThread.join();
    }

    if (fEventFd > 0)
        ::close(fEventFd);
}

std::shared_ptr<Thread::Fence> Thread::enqueueCmd(Command cmd)
{
    std::lock_guard<std::mutex> scopedLock(fQueueMutex);

    std::shared_ptr<Fence> fence = cmd.fence();
    fCommandQueue.push(std::move(cmd));

    uint64_t inc = 1;
    ssize_t ret = ::write(fEventFd, &inc, sizeof(uint64_t));
    if (ret != sizeof(uint64_t))
        return nullptr;

    return fence;
}

void Thread::thread_worker_entry()
{
    if constexpr(std::is_same<std::thread::native_handle_type, pthread_t>::value) {
        pthread_setname_np(fThread.native_handle(), fName.c_str());
    }

    if (fEventFd < 0)
        return;
    fWorker->init();

    CmdExecuteResult status = CmdExecuteResult::kNormal;
    while (status != CmdExecuteResult::kStopExecution)
    {
        uint64_t taskCounter;
        ssize_t ret = ::read(fEventFd, &taskCounter, sizeof(uint64_t));
        if (ret < 0)
            continue;

        for (uint64_t i = 0; i < taskCounter; i++)
        {
            fQueueMutex.lock();
            if (fCommandQueue.empty())
            {
                fQueueMutex.unlock();
                break;
            }

            Command currentTask(std::move(fCommandQueue.front()));
            fCommandQueue.pop();
            fQueueMutex.unlock();

            status = cmdExecute(currentTask);
            currentTask.notifyFinish();
        }
    }

    fWorker->final();
}

Thread::CmdExecuteResult Thread::cmdExecute(const Command& cmd)
{
    if (!cmd.valid())
        return CmdExecuteResult::kNormal;

    switch (cmd.opcode())
    {
    case InternalOpcode::kTaskOp_Exit:
        return CmdExecuteResult::kStopExecution;
    }

    return fWorker->execute(cmd);
}

// -------------------------------------------------------------------------

void Worker::init()
{
}

void Worker::final()
{
}

CIALLO_END_NS
