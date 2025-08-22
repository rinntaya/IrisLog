#include "IrisLog.h"

namespace IrisLog
{

    inline NonLogger default_logger;

    LoggerInstance& instance = LoggerInstance::instance();
    Logstream logger(&default_logger);

    bool NonLogger::enable(const Metadata& metadata) noexcept
    { return false; }

    void NonLogger::log(const Record& record) noexcept
    {
    }

    void NonLogger::flush() noexcept
    {
    }

    Logstream::Logstream(Logger* logger, const std::string_view target): target_(target), logger_(logger)
    {}

    Logstream::Logstream(Logger* logger): logger_(logger)
    {
    }

    void Logstream::reset(const Level level)
    {
        level_ = level;
        get_oss().str({});
        get_oss().clear();
    }

    void Logstream::flush() const
    {
        auto& local_oss = get_oss();
        if (const auto logger = logger_.load(std::memory_order_acquire)) {
            if (const Metadata md{level_, target_.value_or(local_target_)}; logger->enable(md)) {
                const std::string msg = local_oss.str(); // 局部复制，线程安全
                const Record rec{md, msg, file_, line_};
                logger->log(rec);
            }
        }
        local_oss.str({});
        local_oss.clear();
    }

    bool LoggerInstance::set_logger(Logger* logger) noexcept
    {
        if (logger == nullptr) return false;
        logger_.store(logger, std::memory_order_release);
        IrisLog::logger.set_logger(logger);
        return true;
    }

    Logger* LoggerInstance::logger() const noexcept
    {
        return logger_.load(std::memory_order_acquire);
    }

}
