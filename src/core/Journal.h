#ifndef __LOG_H__
#define __LOG_H__

#include "core/ObjectHolder.h"
#include <string>
#include <sstream>
#include <chrono>

#include <cstdint>
#include <pthread.h>

#include "Project.h"

namespace cocoa
{

enum LogType
{
    LOG_DEBUG       = 0x0001,
    LOG_INFO        = 0x0002,
    LOG_WARNING     = 0x0004,
    LOG_ERROR       = 0x0008,
    LOG_EXCEPTION   = 0x0010
};

enum class LogStream
{
    /* Output logs into file stream */
    STREAM_FILE,
    /* Print logs to current tty */
    STREAM_DIRECT_CONSOLE,
    /* Print logs to current Cocoa Terminal */
    STREAM_RENDERED_CONSOLE,
    /* Output logs into blackhole (Give them up) */
    STREAM_VOID
};

enum LogLevel
{
    LOG_LEVEL_DEBUG     = LOG_DEBUG | LOG_INFO | LOG_WARNING | LOG_ERROR | LOG_EXCEPTION,
    LOG_LEVEL_NORMAL    = LOG_INFO | LOG_WARNING | LOG_ERROR | LOG_EXCEPTION,
    LOG_LEVEL_QUIET     = LOG_WARNING | LOG_ERROR | LOG_EXCEPTION,
    LOG_LEVEL_SILENT    = LOG_ERROR | LOG_EXCEPTION,
    LOG_LEVEL_DISABLED  = 0x0000
};

struct __endl_struct {};
typedef struct __endl_struct* endl_t;

#define logEndl    static_cast<cocoa::endl_t>(nullptr)
#define logOut(t)  cocoa::Journal::instance()->out(t)

class Journal;

#define DECLARE_OPR(type)   StreamHolder& operator<<(type i0);

class StreamHolder
{
public:
    StreamHolder(Journal *logObject);
    ~StreamHolder();

    DECLARE_OPR(endl_t)
    DECLARE_OPR(bool)
    DECLARE_OPR(char)
    DECLARE_OPR(unsigned char)
    DECLARE_OPR(short)
    DECLARE_OPR(unsigned short)
    DECLARE_OPR(int)
    DECLARE_OPR(unsigned int)
    DECLARE_OPR(long)
    DECLARE_OPR(unsigned long)
    DECLARE_OPR(long long)
    DECLARE_OPR(unsigned long long)
    DECLARE_OPR(float)
    DECLARE_OPR(double)
    DECLARE_OPR(long double)
    DECLARE_OPR(char const *)
    DECLARE_OPR(void *)

    DECLARE_OPR(const std::string&);

private:
    std::ostringstream mBuffer;
    Journal            *mLogger;
};

#undef DECLARE_OPR

/* Note that this class requires class {GOM}Config before constructing it */
class Journal final : public QObject
{
    GOM_DECLARE(Journal)
public:
    Journal(int fd, int filter, bool color = true);
    Journal(char const *file, int filter, bool color = false);
    ~Journal() override;

    void write(char const *str, size_t size);
    StreamHolder& out(int type);
    bool color() const;
    void color(bool enable);

    static void InitializeJournal();

private:
    void welcome();
    void openfileAsLog(char const *path);

private:
    StreamHolder    *mStream;
    int              mFilter;
    int              mCurType;
    pthread_mutex_t  mOutMutex;
    bool             mColor;
    int              mFd;
    std::chrono::steady_clock::time_point mStartTime;
};

} // namespace cocoa

#endif // __LOG_H__
