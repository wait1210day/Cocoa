#ifndef __EXCEPT_H__
#define __EXCEPT_H__

#include <string>
#include <sstream>
#include <vector>
#include <cstdarg>

#include "core/Journal.h"

namespace cocoa
{
#define ARTEXCEPT_BODY_REF      __except
#define ARTEXCEPT_TRY           try
#define ARTEXCEPT_CATCH(type)   catch (type *ARTEXCEPT_BODY_REF)
#define ARTEXCEPT_CATCH_END     delete ARTEXCEPT_BODY_REF;

#define ARTEXCEPT_THROWER_CONSTRUCTOR "<Constructor>"
#define ARTEXCEPT_THROWER_DESTRUCTOR  "<Destructor>"

#define THROWABLE_ASSERT(expr)        Throwable::__assert(expr, __FUNCTION__, #expr)

enum class ExceptionColors : std::uint8_t
{
    Red, Green,
    Yellow, Blue,
    White, None,
    End
};

struct StackBacktraceFrame
{
    /* The address of function */
    void *func;
    /* Call Frame Area pointer (stack pointer) */
    void *sp;
    /* IP register's value */
    void *ip;
    std::string symbol;
    std::string file;
};

/**
 * The ARTException class is the most important one
 * in this exception module. It provides a standard
 * and basic interface to describe an exception.
 * Others can extend this class so that you can throw
 * an specific exception based on ARTException.
 */
class ARTException
{
public:
    typedef std::vector<StackBacktraceFrame> BacktraceListT;
public:
    // ARTException(char const *who, char const *what, BacktraceListT *stacktrace);
    ARTException();
    virtual ~ARTException();

    const std::string& who();
    const std::string& what();
    void backtrace(bool color = true);
    virtual void dumpError(bool color = true);

    void setColor(ExceptionColors who, ExceptionColors what, ExceptionColors head);
    void setAttributes(std::ostringstream& who, std::ostringstream& what, BacktraceListT *stacktrace);

protected:
    std::string m_who;
    std::string m_what;

private:
    ExceptionColors m_colorWho;
    ExceptionColors m_colorWhat;
    ExceptionColors m_colorHead;

    BacktraceListT *m_stacktrace;
};

StreamHolder& operator<<(StreamHolder&, const ExceptionColors);

class Throwable
{
protected:
    Throwable(char const *clname);
    Throwable();

    std::ostringstream& artfmt(char const *funcName);

    template<typename _T>
    void arthrow(ARTException *except)
    {
        if (!m_expectedThrow)
        {
            throw std::runtime_error(
                "LowlevelException: Failed in Throwable::arthrow(): Unexpected state.");
        }

        std::ostringstream ss;
        ss << m_className << "::" << m_funcName;
        except->setAttributes(ss, m_osstream, trace());

        _T *__except = static_cast<_T*>(except);
        throw __except;
    }

    void __assert(bool cond, const std::string& func, const std::string& msg);

private:
    ARTException::BacktraceListT *trace();

private:
    std::string         m_className;
    std::string         m_funcName;
    std::ostringstream  m_osstream;
    bool                m_expectedThrow;
};

} // namespace cocoa

#endif // __EXCEPT_H__
