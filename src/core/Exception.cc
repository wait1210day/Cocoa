#include <dlfcn.h>
#include <cxxabi.h>
#include <unwind.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <memory>

#include "core/Exception.h"
#include "core/Journal.h"
#include "core/ObjectHolder.h"

namespace cocoa
{

static _Unwind_Reason_Code unwindBacktraceCallback(_Unwind_Context *context, void *arg)
{
    // StackBacktraceFrame *dst = static_cast<StackBacktraceFrame *>(arg);
    StackBacktraceFrame dst;
    Dl_info dl;
    dst.ip = reinterpret_cast<void*>(::_Unwind_GetIP(context));
    dst.sp = reinterpret_cast<void*>(::_Unwind_GetCFA(context));

    if (!dst.ip)
        return _URC_NORMAL_STOP;

    ::dladdr(dst.ip, &dl);
    dst.func = dl.dli_saddr;
    if (dl.dli_fname)
        dst.file = dl.dli_fname;
    else
        dst.file = "<Unknown>";

    if (dl.dli_sname)
        gom_utils::resolve_address_symbol(dl.dli_saddr, dst.symbol);
    else
        dst.symbol = "<Unknown>";
    
    static_cast<ARTException::BacktraceListT *>(arg)->push_back(dst);
    return _URC_NO_REASON;
}

// Color selector for enumeration
StreamHolder& operator<<(StreamHolder& os, const ExceptionColors e)
{
    switch (e)
    {
    case ExceptionColors::Blue:   os << "\033[34;1m"; break;
    case ExceptionColors::End:    os << "\033[0m"; break;
    case ExceptionColors::Green:  os << "\033[32;1m"; break;
    case ExceptionColors::None:   break;
    case ExceptionColors::Red:    os << "\033[31;1m"; break;
    case ExceptionColors::White:  os << "\033[37;1m"; break;
    case ExceptionColors::Yellow: os << "\033[34;1m"; break;
    }
    return os;
}

ARTException::~ARTException()
{
    // std::cout << "ARTException destructed" << std::endl;
    delete m_stacktrace;
}

ARTException::ARTException()
    : m_colorWho(ExceptionColors::White), m_colorWhat(ExceptionColors::None),
    m_colorHead(ExceptionColors::Red)
{
    // std::cout << "ARTException() => " << this << std::endl;
}

void ARTException::setAttributes(std::ostringstream& who, std::ostringstream& what, BacktraceListT *stacktrace)
{
    m_who = who.str();
    m_what = what.str();
    m_stacktrace = stacktrace;
}

const std::string& ARTException::what()
{ return m_what; }

const std::string& ARTException::who()
{ return m_who; }

void ARTException::setColor(ExceptionColors who, ExceptionColors what, ExceptionColors head)
{
    m_colorWhat = what;
    m_colorWho = who;
    m_colorHead = head;
}

void ARTException::dumpError(bool color)
{
    if (color)
    {
        logOut(LOG_EXCEPTION) << m_colorHead << "Exception: " << ExceptionColors::End
                            << m_colorWho << m_who << ": " << ExceptionColors::End
                            << m_colorWhat << m_what << ExceptionColors::End
                            << logEndl;
    }
    else
    {
        logOut(LOG_EXCEPTION) << "Exception: " << m_who << ": " << m_what << logEndl;
    }
}

void ARTException::backtrace(bool color)
{
    int frame = 0;
    if (color)
    {
        logOut(LOG_EXCEPTION) << "\033[35;1mTraceback:\033[0m Stack backtracing of exception (Thread "
            << gettid() << "):" << logEndl;
    }
    else
        logOut(LOG_EXCEPTION) << "Traceback: Stack backtracing of exception (Thread "
            << gettid() << "):" << logEndl;
    
    for (StackBacktraceFrame& sbf : *m_stacktrace)
    {
        if (color)
        {
            logOut(LOG_EXCEPTION) << "\t#" << frame << " \033[34m" << sbf.func << "\033[0m "
                << "in \033[33m" << sbf.symbol << "\033[0m "
                << "from \033[32m" << sbf.file << "\033[0m" << logEndl;
        }
        else
        {
            logOut(LOG_EXCEPTION) << "\t#" << frame << " " << sbf.func
                 << " in " << sbf.symbol << " from " << sbf.file
                 << logEndl;
        }
        frame++;
    }
}

Throwable::Throwable(char const *clname)
    : m_className(clname), m_expectedThrow(false) { }
Throwable::Throwable()
    : m_className("<UndefinedThrower>"), m_expectedThrow(false) { }

std::ostringstream& Throwable::artfmt(char const *funcName)
{
    m_osstream.clear();
    m_osstream.str("");
    m_funcName = funcName;
    m_expectedThrow = true;
    return m_osstream;
}

void Throwable::__assert(bool cond, const std::string& func, const std::string& expr)
{
    if (!cond)
    {
        artfmt(func.c_str()) << expr;
        arthrow<ARTException>(new ARTException());
    }
}

ARTException::BacktraceListT *Throwable::trace()
{
    ARTException::BacktraceListT *stack = new ARTException::BacktraceListT;
    ::_Unwind_Backtrace(unwindBacktraceCallback, static_cast<void *>(stack));
    return stack;
}

} // namespace cocoa
