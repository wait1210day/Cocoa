#include <iostream>
#include <cmath>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "core/Journal.h"
#include "core/Configurator.h"

namespace cocoa
{

#define DEFINE_OPR(type) StreamHolder& StreamHolder::operator<<(type i0)
#define OPRRET  return *this;

#define OPR_F(type) \
DEFINE_OPR(type) \
{ \
    mBuffer << i0; \
    return *this; \
}

enum Identifier
{
    /* CatchBlock tokens */
    MAYBE_TIMESTAMP_OR_LEVEL_OR_DECLARATIVE,
    MAYBE_MODULE_NAME,
    MAYBE_NUMBER,
    MAYBE_CONTENT_OR_DEFINITION,
    /* Final tokens */
    TIMESTAMP,
    LEVEL_TRACE,
    LEVEL_DEBUG,
    LEVEL_INFO,
    LEVEL_WARN,
    LEVEL_ERROR,
    LEVEL_FATAL,
    LEVEL_OFF,
    DECLARATIVE,
    DEFINITION,
    MODULE_NAME,
    NUMBER,
    STRING,
    SPACE,
    OPERATOR,
    CONTENT
};

enum Colors
{
    NONE        = 0,
    GREEN       = 0x001,
    RED         = 0x002,
    YELLOW      = 0x004,
    BLUE        = 0x008,
    PURPLE      = 0x010,
    DEEP_GREEN  = 0x020,
    WHITE       = 0x040,
    GRAY        = 0x080,
    HIGHLIGHT   = 0x100,
    DISABLE     = 0x200
};

typedef struct
{
    int color;
    char const *attribute;
} ColorAttributeMap;

typedef struct
{
    Identifier ident;
    int color;
} IdentColorMap;

typedef struct
{
    char const *begin;
    char const *end;
    Identifier ident;
} CatchBlock;

typedef struct
{
    Identifier level;
    char const *str;
} LevelStringMap;

static const ColorAttributeMap __cshader_camap[] = {
    { GREEN,        "\033[32m" },
    { RED,          "\033[31m" },
    { YELLOW,       "\033[33m" },
    { BLUE,         "\033[34m" },
    { PURPLE,       "\033[35m" },
    { DEEP_GREEN,   "\033[36m" },
    { WHITE,        "\033[37m" },
    { GRAY,         "\033[38m" },
    { HIGHLIGHT,    "\033[1m"  },
    { DISABLE,      "\033[0m"  },
    { NONE,         ""         }
};

static const IdentColorMap __cshader_icmap[] = {
    { TIMESTAMP,    PURPLE              },
    { LEVEL_TRACE,  GRAY | HIGHLIGHT    },
    { LEVEL_DEBUG,  PURPLE              },
    { LEVEL_INFO,   GREEN               },
    { LEVEL_WARN,   YELLOW              },
    { LEVEL_ERROR,  RED                 },
    { LEVEL_FATAL,  RED | HIGHLIGHT     },
    { LEVEL_OFF,    WHITE | HIGHLIGHT   },
    { MODULE_NAME,  WHITE | HIGHLIGHT   },
    { NUMBER,       DEEP_GREEN          },
    { STRING,       YELLOW              },
    { SPACE,        NONE                },
    { CONTENT,      NONE                },
    { OPERATOR,     RED                 },
    { DECLARATIVE,  BLUE | HIGHLIGHT    },
    { DEFINITION,   DEEP_GREEN          }
};

static const LevelStringMap __cshader_lsmap[] = {
    { LEVEL_TRACE,  "trace" },
    { LEVEL_DEBUG,  "debug" },
    { LEVEL_INFO,   "info"  },
    { LEVEL_WARN,   "warn"  },
    { LEVEL_ERROR,  "error" },
    { LEVEL_FATAL,  "fatal" },
    { LEVEL_OFF,    "off"   }
};

bool __cshader_matches_catchblock(char const *str, CatchBlock *catchblock)
{
    bool expects = 0;
    bool expectExclude = 0;
    char expect;

    if (*str == '\0')
        return 0;

    while (*str != '\0')
    {
        if (expects)
        {
            if (expect != *str)
                goto next;
            catchblock->end = str;
            if (expectExclude)
                catchblock->end--;
            break;
        }

        catchblock->begin = str;
        if (*str == '[')
        {
            catchblock->ident = MAYBE_TIMESTAMP_OR_LEVEL_OR_DECLARATIVE;
            expects = 1;
            expect = ']';
        }
        else if (*str == '<')
        {
            catchblock->ident = MAYBE_MODULE_NAME;
            expects = 1;
            expect = '>';
        }
        else if (*str >= '0' && *str <= '9')
        {
            catchblock->ident = MAYBE_NUMBER;
            expects = 1;
            expectExclude = 1;
            expect = ' ';
        }
        else if (*str == '\"' || *str == '\'')
        {
            catchblock->ident = STRING;
            expects = 1;
            expect = *str;
        }
        else if (*str == ' ')
        {
            catchblock->begin = str;
            catchblock->end = str;
            catchblock->ident = SPACE;
            break;
        }
        else if (*str == '=' || *str == '+' || *str == '-'
                || *str == '*' || *str == '/' || *str == '>'
                || *str == '<')
        {
            catchblock->end = str;
            catchblock->ident = OPERATOR;
            break;
        }
        else if (*str == '\033')
        {
            catchblock->ident = CONTENT;
            catchblock->end = str + std::strlen(str) - 1;
            break;
        }
        else
        {
            catchblock->ident = MAYBE_CONTENT_OR_DEFINITION;
            expects = 1;
            expect = ' ';
            expectExclude = 1;
        }

    next:
        str++;
    }

    if (expects && expect == ' ' && *str == '\0')
        catchblock->end = str - 1;
    return 1;
}

bool __cshader_is_number(char ch)
{ return (ch >= '0' && ch <= '9'); }
bool __cshader_is_upper(char ch)
{ return (ch >= 'A' && ch <= 'Z'); }
bool __cshader_is_lower(char ch)
{ return (ch >= 'a' && ch <= 'z'); }
char __cshader_to_lower(char ch)
{ return __cshader_is_lower(ch) ? ch : (__cshader_is_upper(ch) ? ('a' + (ch - 'A')) : ch); }

bool __cshader_case_insensitive_compare_equal(const char *begin, const char *end, char const *str)
{
    if (begin > end)
        return 0;
    if (static_cast<size_t>(end - begin + 1) != strlen(str))
        return 0;
    
    char const *p0 = begin, *p1 = str;
    while (p0 <= end && *p1 != '\0')
    {
        char c0 = __cshader_to_lower(*p0);
        char c1 = __cshader_to_lower(*p1);
        if (c0 != c1)
            return 0;
        p0++;
        p1++;
    }
    return 1;
}

bool __cshader_is_integer(char const *begin, char const *end)
{
    if (begin > end)
        return 0;
    while (begin <= end)
    {
        if (!__cshader_is_number(*begin))
            return 0;
        begin++;
    }
    return 1;
}

bool __cshader_is_float(char const *begin, char const *end)
{
    if (begin > end)
        return 0;
    
    bool point = 0;
    while (begin <= end)
    {
        if (*begin == '.')
        {
            if (point)
                return 0;
            else if (begin == end)
                /* Point shouldn't appears at the end */
                return 0;
            point = 1;
        }
        else if (!__cshader_is_number(*begin))
            return 0;
        begin++;
    }
    return 1;
}

bool __cshader_check_number(CatchBlock *cb)
{
    if (__cshader_is_integer(cb->begin, cb->end)
        || __cshader_is_float(cb->begin, cb->end))
    {
        cb->ident = NUMBER;
        return 1;
    }
    return 0;
}

/* There's no regex engine, so we must handle it manually... */
bool __cshader_check_timestamp(CatchBlock *cb)
{
    const char *ptr = cb->begin + 1;
    while (*ptr++ == ' ')
        ;
    
    if (__cshader_is_float(ptr, cb->end - 1))
    {
        cb->ident = TIMESTAMP;
        return 1;
    }
    return 0;
}

#define ARRAY_SIZE(a)   (sizeof(a) / sizeof(a[0]))
bool __cshader_check_level(CatchBlock *cb)
{
    for (size_t i = 0; i < ARRAY_SIZE(__cshader_lsmap); i++)
    {
        if (__cshader_case_insensitive_compare_equal(cb->begin + 1, cb->end - 1,
            __cshader_lsmap[i].str))
        {
            cb->ident = __cshader_lsmap[i].level;
            return 1;
        }
    }
    return 0;
}

bool __cshader_is_llegal_identifier(char const *begin, char const *end)
{
    if (begin > end)
        return 0;

    bool allowNumber = 0;
    while (begin <= end)
    {
        if (__cshader_is_number(*begin) && !allowNumber)
            return 0;
        else if (!__cshader_is_lower(*begin) && !__cshader_is_upper(*begin)
            && !__cshader_is_number(*begin) && *begin != '_')
            return 0;
        allowNumber = 1;
        begin++;
    }
    return 1;
}

bool __cshader_check_modulename(CatchBlock *cb)
{
    /* A module name is like: org.sora.xxx */
    char const *p = cb->begin;
    char const *pIdBegin = cb->begin + 1, *pIdEnd;
    while (p <= cb->end)
    {
        if (*p == '.' || p == cb->end)
        {
            pIdEnd = p - 1;
            if (!__cshader_is_llegal_identifier(pIdBegin, pIdEnd))
                return 0;
            pIdBegin = p + 1;
        }
        p++;
    }
    cb->ident = MODULE_NAME;
    return 1;
}

bool __cshader_check_definition(CatchBlock *cb)
{
    /* First, a definition must be a llegal identifier */
    if (!__cshader_is_llegal_identifier(cb->begin, cb->end))
        return 0;
    
    char const *p = cb->begin;
    while (p <= cb->end)
    {
        if (!__cshader_is_upper(*p) && *p != '_' && !__cshader_is_number(*p))
            return 0;
        p++;
    }
    cb->ident = DEFINITION;
    return 1;
}

void __cshader_catchblock_tochecked(CatchBlock *cb)
{
    switch (cb->ident)
    {
    case MAYBE_TIMESTAMP_OR_LEVEL_OR_DECLARATIVE:
        if (!__cshader_check_timestamp(cb) && !__cshader_check_level(cb))
            cb->ident = DECLARATIVE;
        break;
    case MAYBE_NUMBER:
        if (!__cshader_check_number(cb))
            cb->ident = CONTENT;
        break;
    case MAYBE_MODULE_NAME:
        if (!__cshader_check_modulename(cb))
            cb->ident = CONTENT;
        break;
    case MAYBE_CONTENT_OR_DEFINITION:
        if (!__cshader_check_definition(cb))
            cb->ident = CONTENT;
        break;
    default:
        break;
    }
}

void __cshader_apply(int fd, CatchBlock *cb)
{
    int color = NONE;
    for (size_t i = 0; i < ARRAY_SIZE(__cshader_icmap); i++)
    {
        if (__cshader_icmap[i].ident == cb->ident)
        {
            color = __cshader_icmap[i].color;
            break;
        }
    }

    char const *disable_attribute;
    for (size_t i = 0; i < ARRAY_SIZE(__cshader_camap); i++)
    {
        if (color & __cshader_camap[i].color)
        {
            // std::fprintf(fp, "%s", __cshader_camap[i].attribute);
            ::write(fd, __cshader_camap[i].attribute, std::strlen(__cshader_camap[i].attribute));
        }
        if (__cshader_camap[i].color == DISABLE)
            disable_attribute = __cshader_camap[i].attribute;
    }

    size_t siz = cb->end - cb->begin + 1;
    char buffer[siz + 1];
    std::memcpy(buffer, cb->begin, siz);
    buffer[siz] = '\0';

    // fprintf(fp, "%s%s", buffer, disable_attribute);
    ::write(fd, buffer, std::strlen(buffer));
    ::write(fd, disable_attribute, std::strlen(disable_attribute));
}

void __cshader_transport(int fd, char const *str)
{
    CatchBlock catchblock;
    while (__cshader_matches_catchblock(str, &catchblock))
    {
        __cshader_catchblock_tochecked(&catchblock);
        __cshader_apply(fd, &catchblock);
        str = catchblock.end + 1;
    }
}

StreamHolder::StreamHolder(Journal *obj)
    : mLogger(obj)
{
}

StreamHolder::~StreamHolder()
{
}

DEFINE_OPR(endl_t)
{
    mBuffer << '\n';
    std::string str = mBuffer.str();
    mLogger->write(str.c_str(), str.size());

    mBuffer.str("");
    OPRRET
}

OPR_F(bool)
OPR_F(char)
OPR_F(unsigned char)
OPR_F(short)
OPR_F(unsigned short)
OPR_F(int)
OPR_F(unsigned int)
OPR_F(long)
OPR_F(unsigned long)
OPR_F(long long)
OPR_F(unsigned long long)
OPR_F(float)
OPR_F(double)
OPR_F(long double)
OPR_F(char const *)
OPR_F(void *)
OPR_F(const std::string&);

static const char *__log_syms_list[] = {
    "[Debug]",
    "[Info]",
    "[Warn]",
    "[Error]",
    "[Fatal]"
};

GOM_DEFINE(Journal)
Journal::Journal(int fd, int filter, bool color)
    : mStream(new StreamHolder(this)),
      mColor(color),
      mStartTime(std::chrono::steady_clock::now())
{
    mFd = fd;
    mFilter = filter;
    pthread_mutex_init(&mOutMutex, nullptr);

    welcome();
    GOM_CONSTRUCT_DONE
}

Journal::Journal(char const *file, int filter, bool color)
    : mStream(new StreamHolder(this)),
      mColor(color),
      mStartTime(std::chrono::steady_clock::now())
{
    try
    {
        openfileAsLog(file);
    }
    catch (std::runtime_error& rt)
    {
        delete mStream;
        std::rethrow_exception(std::current_exception());
    }
    mFilter = filter;
    pthread_mutex_init(&mOutMutex, nullptr);
    welcome();
    GOM_CONSTRUCT_DONE
}

Journal::~Journal()
{
    pthread_mutex_lock(&mOutMutex);
    delete mStream;
    pthread_mutex_destroy(&mOutMutex);
}

void Journal::welcome()
{
    out(LOG_INFO) << COCOA_VERSION << logEndl;
    out(LOG_INFO) << COCOA_COPYRIGHT << logEndl;
    out(LOG_INFO) << COCOA_LICENSE << logEndl;
    out(LOG_INFO) << "Components:" << logEndl;
    out(LOG_INFO) << '\t' << COCOA_FRAMEWORK_VERSION << logEndl;
    out(LOG_INFO) << '\t' << COCOA_GOM_VERSION << logEndl;
    out(LOG_INFO) << logEndl;
}

StreamHolder& Journal::out(int type)
{
    pthread_mutex_lock(&mOutMutex);
    mCurType = type;
    return *mStream;
}

void Journal::write(char const *str, size_t size)
{
    // std::cout << "filter() -> " << (mCurType & mFilter) << std::endl;
    if (!(mCurType & mFilter))
    {
        pthread_mutex_unlock(&mOutMutex);
        return ;
    }
    
    int idx = std::log2(mCurType);
    const char *header =  __log_syms_list[idx];
    
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - mStartTime);
    double dt = static_cast<double>(duration.count()) * std::chrono::microseconds::period::num
        / std::chrono::microseconds::period::den;

    std::ostringstream ss;
    ss.setf(std::ios_base::right | std::ios_base::fixed);
    ss.precision(6);
    ss << '[' << std::setw(12) << dt << "] ";
    
    ss << header << " " << str;
    std::string res = ss.str();
    // ::write(mFd, res.c_str(), res.size());
    if (mColor)
        __cshader_transport(mFd, res.c_str());
    else
        ::write(mFd, res.c_str(), res.length());

    pthread_mutex_unlock(&mOutMutex);
}

void Journal::openfileAsLog(const char *path)
{
    int fd = ::open(path, O_RDWR);
    if (fd >= 0)
    {
        ::close(fd);
        char *p = new char[std::strlen(path) + 5];
        sprintf(p, "%s.old", path);
        ::rename(path, p);
        delete[] p;
    }

    fd = ::open(path, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
    if (fd < 0)
        throw std::runtime_error("Could not open log file");

    mFd = fd;
}

bool Journal::color() const
{
    return mColor;
}

void Journal::color(bool enable)
{
    mColor = enable;
}

void Journal::InitializeJournal()
{
    int fd = -1;
    char const *file;
    if (GOM->getNodeUserdata("/config/common/standardOutput")
        == kConfigConstraintCocoaPath)
        fd = GOM->getPropertyInt(GOM->getPropertyString("/config/common/standardOutput"));
    else
        file = GOM->getPropertyString("/config/common/standardOutput");

    std::string level(GOM->getPropertyString("/config/common/level"));
    LogLevel realLevel;
    if (level == "debug")
        realLevel = LogLevel::LOG_LEVEL_DEBUG;
    else if (level == "normal")
        realLevel = LogLevel::LOG_LEVEL_NORMAL;
    else if (level == "quiet")
        realLevel = LogLevel::LOG_LEVEL_QUIET;
    else if (level == "silent")
        realLevel = LogLevel::LOG_LEVEL_SILENT;
    else if (level == "disabled")
        realLevel = LogLevel::LOG_LEVEL_DISABLED;
    else
    {
        throw std::runtime_error("Invalid identifier for log level in configuration");
    }

    bool color = true, specifiedColor = false;
    if (GOM->hasPropertyOrDirectory("/config/common/consoleColor"))
    {
        specifiedColor = true;
        color = GOM->getPropertyBool("/config/common/consoleColor");
    }

    Journal *logger;
    if (fd >= 0)
        logger = new Journal(fd, realLevel, specifiedColor ? color : true);
    else
        logger = new Journal(file, realLevel, specifiedColor ? color : false);
    GOM->addObject(logger);
}

} // namespace cocoa

/*
int main(int argc, char const *argv[])
{
    CsShaderTransport(stdout, "[2197.3242] ", "\"String\" rrss 2.2");
    return 0;
}
*/
