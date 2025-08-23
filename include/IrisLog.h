#pragma once

#include <atomic>
#include <optional>
#include <sstream>
#include <string_view>

#ifdef IRISLOG_STATIC_MAX_LEVEL
#else
#define IRISLOG_STATIC_MAX_LEVEL ::IrisLog::Level::Trace
#endif


namespace IrisLog
{
    enum class Level: uint8_t
    {
        Fatal = 0,
        Error = 1,
        Warn = 2,
        Info = 3,
        Debug = 4,
        Trace = 5,
    };

    inline const char* ToString(const Level level)
    {
        switch (level) {
            case Level::Trace: return "Trace";
            case Level::Debug: return "Debug";
            case Level::Info: return "Info";
            case Level::Warn: return "Warn";
            case Level::Error: return "Error";
            case Level::Fatal: return "Fatal";
            default: return "Unknown";
        }
    }

    inline std::ostream& operator<<(std::ostream& os, const Level level)
    {
        return os << std::string_view(ToString(level));
    }

    struct Metadata
    {
        Level level;
        std::string_view target;
    };

    struct Record
    {
        Metadata metadata;
        std::string_view message;
        std::string_view file;
        uint32_t line;
        // std::string_view module_path;
    };

    class Logger
    {
    public:
        virtual ~Logger() = default;

        virtual bool enable(const Metadata& metadata) noexcept = 0;
        virtual void log(const Record& record) noexcept = 0;
        virtual void flush() noexcept = 0;
    };


    class NonLogger final : public Logger
    {
    public:
        bool enable(const Metadata& metadata) noexcept override
        { return false; }

        void log(const Record& record) noexcept override{}
        void flush() noexcept override {}
    };


    class LoggerInstance;

    class Logstream
    {
    public:
        explicit Logstream(Logger* logger, const std::optional<std::string_view>& target = std::nullopt): target_(target), logger_(logger)
        {}
        void reset(const Level level)
        {
            level_ = level;
            get_oss().str({});
            get_oss().clear();
        }

        template<typename T>
        Logstream& operator<<(T&& value)
        {
            get_oss() << std::forward<T>(value);
            return *this;
        }

        Logstream& operator<<(std::ostream& (*manip)(std::ostream&))
        {
            if (manip == static_cast<std::ostream& (*)(std::ostream&)>(std::endl)) {
                flush();
            } manip(get_oss());
            return *this;
        }

        void flush() const
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


        template<Level T>
        Logstream& log(const std::string_view file, const uint32_t line, const std::string_view target)
        {
            file_ = file;
            line_ = line;
            local_target_ = target;
            reset(T); return *this;
        }

        void set_logger(Logger* logger) {
            logger_ = logger;
        }
        void set_target(const std::optional<std::string_view>& target) {
            target_ = target;
        }

    private:
        Level level_ = Level::Info;
        std::optional<std::string_view> target_;
        std::string_view file_{"unknown"};
        std::string_view local_target_{"unknown"};
        uint32_t line_{0};
        std::atomic<Logger *> logger_;

        static std::ostringstream& get_oss() {
            thread_local std::ostringstream oss;
            return oss;
        }
    };
    inline NonLogger default_logger;
    inline Logstream logger(&default_logger);

    class LoggerInstance
    {
    public:
        static LoggerInstance& instance() noexcept
        {
            static LoggerInstance instance;
            return instance;
        }

        bool set_logger(Logger* logger) noexcept
        {
            if (logger == nullptr) return false;
            logger_.store(logger, std::memory_order_release);
            IrisLog::logger.set_logger(logger);
            return true;
        }

        [[nodiscard]] Logger* logger() const noexcept
        {
            return logger_.load(std::memory_order_acquire);
        }

    private:
        LoggerInstance() noexcept = default;
        std::atomic<Logger *> logger_{nullptr};
    };


    inline extern LoggerInstance& instance = LoggerInstance::instance();
    namespace Detail
    {
        constexpr bool _static_level_check(const Level level) noexcept
        {
            return level <= IRISLOG_STATIC_MAX_LEVEL;
        };

        template<typename... Args>
        constexpr std::string _format_string(const char* fmt, Args&&... args)
        {
            const int len = std::snprintf(nullptr, 0, fmt, args...);
            if (len <= 0) return {};
            std::string result(len, '\0');
            std::snprintf(result.data(), len + 1, fmt, args...);
            return result;
        }

        template<typename... Args>
        void _dispatch_log(const Level level, const std::string_view target, const std::string_view file,
                           const uint32_t line, Args&&... args) noexcept
        {
            const Metadata metadata{level, target};
            if (auto* logger = instance.logger()) {
                if (!logger->enable(metadata)) return;
                const std::string s = _format_string(std::forward<Args>(args)...);
                logger->log({metadata, s, file, line});
            }
        }
    }}

#define IRISLOG_LOGGER(level, target, ...) \
                do { \
                    if (::IrisLog::Detail::_static_level_check(level)) { \
                        ::IrisLog::Detail::_dispatch_log(level, target, __FILE__, __LINE__, __VA_ARGS__); \
                    } \
                } while (false)

#define LOG_TRACE(...) IRISLOG_LOGGER(::IrisLog::Level::Trace, __func__, __VA_ARGS__)
#define LOG_DEBUG(...) IRISLOG_LOGGER(::IrisLog::Level::Debug, __func__, __VA_ARGS__)
#define LOG_INFO(...)  IRISLOG_LOGGER(::IrisLog::Level::Info, __func__, __VA_ARGS__)
#define LOG_WARN(...)  IRISLOG_LOGGER(::IrisLog::Level::Warn, __func__, __VA_ARGS__)
#define LOG_ERROR(...) IRISLOG_LOGGER(::IrisLog::Level::Error, __func__, __VA_ARGS__)

#define LOG_TRACE_T(target, ...) IRISLOG_LOGGER(::IrisLog::Level::Trace, target, __VA_ARGS__)
#define LOG_DEBUG_T(target, ...) IRISLOG_LOGGER(::IrisLog::Level::Debug, target, __VA_ARGS__)
#define LOG_INFO_T(target, ...)  IRISLOG_LOGGER(::IrisLog::Level::Info, target, __VA_ARGS__)
#define LOG_WARN_T(target, ...)  IRISLOG_LOGGER(::IrisLog::Level::Warn, target, __VA_ARGS__)
#define LOG_ERROR_T(target, ...) IRISLOG_LOGGER(::IrisLog::Level::Error, target, __VA_ARGS__)

#define LOGGER(x) ::IrisLog::logger.log<x>(__FILE__, __LINE__, __func__)


#ifdef IRISLOG_ORIGIN
#else
using IL = IrisLog::Level;
#endif