#include "sylar/log.h"
#include "sylar/util.h"

#include <algorithm>
#include <complex>
#include <cstdarg>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <sys/time.h>


namespace sylar {

const char* LogLevel::ToString(LogLevel::Level level)
{
    switch (level) {
#define XX(name) \
    case LogLevel::Level::name: return #name;
        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
        XX(FATAL);
#undef XX
        default: return "UNKNOW";
    }

    return "UNKNOW";
}

LogLevel::Level LogLevel::FromString(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    if (str == "DEBUG") {
        return LogLevel::Level::DEBUG;
    }

#define XX(name)                      \
    if (str == #name) {               \
        return LogLevel::Level::name; \
    }

    XX(DEBUG)
    XX(INFO)
    XX(WARN)
    XX(ERROR)
    XX(FATAL)
#undef XX

    return LogLevel::Level::UNKNOW;
}

}   // namespace sylar



namespace sylar {

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file,
                   int32_t line, uint32_t elapse, uint32_t threadId, uint32_t fiberId,
                   const std::string& threadName)
    : file_(file)
    , line_(line)
    , elapse_(elapse)
    , threadId_(threadId)
    , fiberId_(fiberId)
    , threadName_(threadName)
    , logger_(logger)
    , level_(level)
{
    if (gettimeofday(&time_, nullptr) != 0) {
        std::cerr << "failed to call gettimeofday." << std::endl;
        exit(EXIT_SUCCESS);
    }
}

std::string LogEvent::getTime() const
{
    struct tm tm;
    int64_t second = time_.tv_sec;
    int64_t microSecond = time_.tv_usec;

    localtime_r(&second, &tm);
    char buf[128] = {0};
    snprintf(buf,
             sizeof(buf),
             "%4d-%02d-%02d %02d:%02d:%02d.%06ld",
             tm.tm_year + 1900,
             tm.tm_mon + 1,
             tm.tm_mday,
             tm.tm_hour,
             tm.tm_min,
             tm.tm_sec,
             microSecond);

    return buf;
}


void LogEvent::format(const char* fmt, ...)
{
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}

void LogEvent::format(const char* fmt, va_list al)
{
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if (len != -1) {
        ss_ << std::string(buf, len);
        free(buf);
    }
}

}   // namespace sylar




namespace sylar {

LogEventWrap::LogEventWrap(LogEvent::ptr event)
    : event_(event)
{}

LogEventWrap::~LogEventWrap()
{
    event_->getLogger()->log(event_->getLevel(), event_);
}

std::stringstream& LogEventWrap::getSS()
{
    return event_->getSS();
}

}   // namespace sylar


namespace sylar {

class MessageFormatItem : public LogFormatter::FormatItem {
public:
    explicit MessageFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,
                LogEvent::ptr event) override
    {
        os << event->getContent();
    }
};


class LevelFormatItem : public LogFormatter::FormatItem {
public:
    explicit LevelFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,
                LogEvent::ptr event) override
    {
        os << LogLevel::ToString(level);
    }
};

class ElapseFormatItem : public LogFormatter::FormatItem {
public:
    explicit ElapseFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,
                LogEvent::ptr event) override
    {
        os << event->getElapse();
    }
};


class NameFormatItem : public LogFormatter::FormatItem {
public:
    explicit NameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,
                LogEvent::ptr event) override
    {
        os << event->getLogger()->getName();
        // os << "root";
    }
};


class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    explicit ThreadIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,
                LogEvent::ptr event) override
    {
        os << event->getThreadId();
    }
};


class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    explicit FiberIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,
                LogEvent::ptr event) override
    {
        os << event->getFiberId();
    }
};

class ThreadNameFormatItem : public LogFormatter::FormatItem {
public:
    explicit ThreadNameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,
                LogEvent::ptr event) override
    {
        os << event->getThreadName();
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    explicit DateTimeFormatItem(const std::string& str) {};
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,
                LogEvent::ptr event) override
    {
        os << event->getTime();
    }
};


class FilenameFormatItem : public LogFormatter::FormatItem {
public:
    explicit FilenameFormatItem(const std::string& str) {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,
                LogEvent::ptr event) override
    {
        os << event->getFile();
    }
};


class LineFormatItem : public LogFormatter::FormatItem {
public:
    explicit LineFormatItem(const std::string& str) {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,
                LogEvent::ptr event) override
    {
        os << event->getLine();
    }
};


class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    explicit NewLineFormatItem(const std::string& str) {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,
                LogEvent::ptr event) override
    {
        os << std::endl;
    }
};


class StringFormatItem : public LogFormatter::FormatItem {
public:
    explicit StringFormatItem(const std::string& str)
        : str_(str)
    {}

    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,
                LogEvent::ptr event) override
    {
        os << str_;
    }

private:
    std::string str_;
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
    explicit TabFormatItem(const std::string& str) {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,
                LogEvent::ptr event) override
    {
        os << "  ";
    }
};

}   // namespace sylar



namespace sylar {

LogFormatter::LogFormatter(const std::string& pattern)
    : pattern_(pattern)
{
    init();
}

void LogFormatter::init()
{
    std::string pattern = pattern_;
    std::vector<std::pair<std::string, int32_t>> matchItem;
    while (!pattern.empty()) {
        size_t pos = pattern.find_first_of('%');
        if ((pos == std::string::npos) || (pos == pattern.size() - 1)) {
            matchItem.emplace_back(std::make_pair(pattern, 0));
            break;
        }

        if (isalpha(pattern[pos + 1])) {
            if (pos == 0) {
                matchItem.emplace_back(std::make_pair(std::string(1, pattern[pos + 1]), 1));
            }
            else {
                matchItem.emplace_back(std::make_pair(pattern.substr(0, pos), 0));
                matchItem.emplace_back(std::make_pair(std::string(1, pattern[pos + 1]), 1));
            }
        }
        else {
            matchItem.emplace_back(std::make_pair(pattern.substr(0, pos + 1), 0));
        }


        pattern.erase(0, pos + 2);
    }

    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)>>
        s_format_items{
#define XX(str, C)                                                         \
    {                                                                      \
        #str, [](const std::string& fmt) {                                 \
            return static_cast<FormatItem::ptr>(std::make_shared<C>(fmt)); \
        }                                                                  \
    }

            XX(m, MessageFormatItem),
            XX(p, LevelFormatItem),
            XX(r, ElapseFormatItem),
            XX(c, NameFormatItem),
            XX(t, ThreadIdFormatItem),
            XX(n, NewLineFormatItem),
            XX(d, DateTimeFormatItem),
            XX(f, FilenameFormatItem),
            XX(l, LineFormatItem),
            XX(T, TabFormatItem),
            XX(F, FiberIdFormatItem),
            XX(N, ThreadNameFormatItem),
        };

    for (auto& [str, type] : matchItem) {
        if (type == 0) {
            items_.emplace_back(std::make_shared<StringFormatItem>(str));
        }
        else {
            auto it = s_format_items.find(str);
            if (it == s_format_items.end()) {
                items_.emplace_back(
                    std::make_shared<StringFormatItem>("<<error_format %" + str + ">>"));
                error_ = true;
            }
            else {
                items_.emplace_back(it->second(str));
            }
        }
    }
}


std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level,
                                 LogEvent::ptr event)
{
    std::stringstream ss;
    for (auto& i : items_) {
        i->format(ss, logger, level, event);
    }

    return ss.str();
}

std::ostream& LogFormatter::format(std::ostream& os, std::shared_ptr<Logger> logger,
                                   LogLevel::Level level, LogEvent::ptr event)
{
    for (auto& i : items_) {
        i->format(os, logger, level, event);
    }

    return os;
}

}   // namespace sylar


namespace sylar {

void LogAppender::setFormatter(LogFormatter::ptr val)
{
    Mutex_t::Lock lock(mutex_);
    formatter_ = val;
    if (formatter_) {
        hasFormatter_ = true;
    }
    else {
        hasFormatter_ = false;
    }
}

LogFormatter::ptr LogAppender::getFormatter()
{
    Mutex_t::Lock lock(mutex_);
    return formatter_;
}

}   // namespace sylar


namespace sylar {

void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                            LogEvent::ptr event)
{
    if (level >= level_) {
        Mutex_t::Lock lock(mutex_);
        formatter_->format(std::cout, logger, level, event);
    }
}

std::string StdoutLogAppender::toYamlString()
{
    return "";
}



}   // namespace sylar



namespace sylar {

FileLogAppender::FileLogAppender(const std::string& filename)
    : filename_(filename)
{
    FileLogAppender::reopen();
}

void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                          LogEvent::ptr event)
{
    if (level >= level_) {
        uint64_t now = event->getSeconds();
        if (now >= (lastTime_ + 3)) {
            reopen();
            lastTime_ = now;
        }

        Mutex_t::Lock lock(mutex_);
        if (!formatter_->format(filestream_, logger, level, event)) {
            std::cerr << "error to format log event" << std::endl;
        }
    }
}

std::string FileLogAppender::toYamlString()
{
    return "";
}

bool FileLogAppender::reopen()
{
    Mutex_t::Lock lock(mutex_);
    if (filestream_) {
        filestream_.close();
    }

    return FSUtil::OpenForWrite(filestream_, filename_, std::ios::app | std::ios::out);
}

}   // namespace sylar



namespace sylar {

Logger::Logger(const std::string& name)
    : name_(name)
    , level_(LogLevel::Level::DEBUG)
{
    formatter_ = std::make_shared<LogFormatter>("%d%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n");
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event)
{
    if (level >= level_) {
        auto self = shared_from_this();
        RWMutex_t::ReadLock lock(mutex_);
        if (!appenders_.empty()) {
            for (auto& i : appenders_) {
                i->log(self, level, event);
            }
        }
        else if (root_) {
            root_->log(level, event);
        }
    }
}

void Logger::addAppender(LogAppender::ptr appender)
{
    RWMutex_t::WriteLock lock(mutex_);
    if (!appender->getFormatter()) {
        LogAppender::Mutex_t::Lock ll(appender->mutex_);
        appender->formatter_ = formatter_;
    }

    appenders_.emplace_back(appender);
}


void Logger::delAppender(LogAppender::ptr appender)
{
    RWMutex_t::WriteLock lock(mutex_);
    for (auto it = appenders_.begin(); it != appenders_.end(); ++it) {
        if (*it == appender) {
            appenders_.erase(it);
            break;
        }
    }
}

void Logger::clearAppenders()
{
    RWMutex_t::WriteLock lock(mutex_);
    appenders_.clear();
}

void Logger::setFormatter(LogFormatter::ptr val)
{
    RWMutex_t::WriteLock lock(mutex_);
    formatter_ = val;

    for (auto& appender : appenders_) {
        LogAppender::Mutex_t::Lock ll(appender->mutex_);
        if (!appender->hasFormatter_) {
            appender->formatter_ = formatter_;
        }
    }
}

void Logger::setFormatter(const std::string& val)
{
    LogFormatter::ptr new_val = std::make_shared<LogFormatter>(val);
    if (new_val->isError()) {
        std::cerr << "Logger setFormatter name=" << name_ << " value=" << val << " invalid"
                  << std::endl;
        return;
    }

    formatter_ = new_val;
    setFormatter(new_val);
}

LogFormatter::ptr Logger::getFormatter()
{
    RWMutex_t::ReadLock lock(mutex_);
    return formatter_;
}

std::string Logger::toYamlString()
{
    return "";
}

bool Logger::reopen()
{
    RWMutex_t::ReadLock lock(mutex_);
    for (auto& appender : appenders_) {
        appender->reopen();
    }

    return true;
}


}   // namespace sylar


namespace sylar {

LoggerManager::LoggerManager()
{
    root_ = std::make_shared<Logger>();
    root_->addAppender(std::make_shared<StdoutLogAppender>());
    loggers_[root_->name_] = root_;
}

Logger::ptr LoggerManager::getLogger(const std::string& name)
{
    do {
        RWMutex_t::ReadLock lock(mutex_);
        if (const auto it = loggers_.find(name); it != loggers_.end()) {
            return it->second;
        }
    } while (false);

    RWMutex_t::WriteLock lock(mutex_);
    if (const auto it = loggers_.find(name); it != loggers_.end()) {
        return it->second;
    }

    Logger::ptr logger = std::make_shared<Logger>(name);
    logger->root_ = root_;
    loggers_[name] = logger;

    return logger;
}

void LoggerManager::init() {}


std::string LoggerManager::toYamlString()
{
    return "";
}

bool LoggerManager::reopen()
{
    RWMutex_t::ReadLock lock(mutex_);
    auto loggers = loggers_;
    auto root = root_;
    lock.unlock();

    root_->reopen();
    for (auto& i : loggers) {
        i.second->reopen();
    }

    return true;
}

}   // namespace sylar