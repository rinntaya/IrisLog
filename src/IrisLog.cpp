#include "IrisLog.h"

namespace IrisLog
{
    LoggerInstance& instance = LoggerInstance::instance();


    bool NonLogger::enable(const Metadata& metadata) noexcept
    { return false; }

    void NonLogger::log(const Record& record) noexcept
    {
    }

    void NonLogger::flush() noexcept
    {
    }

    Logstream::Logstream(const std::string_view target, Logger* logger): target_(target), logger_(logger)
    {}

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
            if (const Metadata md{level_, target_}; logger->enable(md)) {
                const std::string msg = local_oss.str(); // 局部复制，线程安全
                const Record rec{md, msg, __FILE__, __LINE__};
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
        return true;
    }

    Logger* LoggerInstance::logger() const noexcept
    {
        return logger_.load(std::memory_order_acquire);
    }

}
