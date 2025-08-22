#include <iostream>


#define IRISLOG_STATIC_MAX_LEVEL ::IrisLog::Level::Trace
#include <IrisLog.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>
#include <string_view>

using namespace IrisLog;

// 彩色ANSI转义码
namespace Color {
    constexpr std::string_view Reset   = "\033[0m";
    constexpr std::string_view RedBg   = "\033[41m";
    constexpr std::string_view GreenBg = "\033[42m";
    constexpr std::string_view YellowBg= "\033[43m";
    constexpr std::string_view BlueBg  = "\033[44m";
    constexpr std::string_view MagentaBg="\033[45m";
    constexpr std::string_view CyanBg  = "\033[46m";
    constexpr std::string_view WhiteBg = "\033[47m";
}

// 控制台 Logger
class ConsoleLogger final : public Logger {
public:
    bool enable(const Metadata& metadata) noexcept override {
        return true; // 简单起见，所有日志都打印
    }

    void log(const Record& record) noexcept override {
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

        // 日志等级 + 背景色
        std::string level_str;
        std::string_view color;
        switch (record.metadata.level) {
            case Level::Fatal: level_str = "FATAL"; color = Color::RedBg; break;
            case Level::Error: level_str = "ERROR"; color = Color::MagentaBg; break;
            case Level::Warn:  level_str = "WARN";  color = Color::YellowBg; break;
            case Level::Info:  level_str = "INFO";  color = Color::GreenBg; break;
            case Level::Debug: level_str = "DEBUG"; color = Color::BlueBg; break;
            case Level::Trace: level_str = "TRACE"; color = Color::CyanBg; break;
        }

        std::cout << oss.str() << "  "
                  << color << " " << level_str << " " << Color::Reset
                  << "  " << record.metadata.target
                  << " " << record.file << ":" << record.line
                  << "  " << record.message
                  << std::endl;
    }

    void flush() noexcept override {
        std::cout << std::flush;
    }
};


namespace Test::First
{
    void func()
    {
        LOG_INFO("This is info message");
        LOGGER(Level::Info) << "Stream Log" << std::endl;
    }

}


using namespace IrisLog;

int main()
{
    // 设置日志器
    static ConsoleLogger console_logger;
    LoggerInstance::instance().set_logger(&console_logger);

    double pi = 3.14159;

    // 使用日志宏
    LOG_TRACE("This is trace message");
    LOG_DEBUG("This is debug message");
    Test::First::func();
    LOG_WARN("This is warn message");
    LOG_ERROR("float is : %.2f", pi);

    std::cout << std::endl;

    auto logger = instance.getStream("master");

    logger.log<Level::Info>() << "test" << (1+1) << std::endl;
    logger(Level::Debug) << "test" << (1+1) << std::endl;


    return 0;
}

