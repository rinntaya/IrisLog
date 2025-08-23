#include <iostream>
#include <chrono>


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
                  << "  " << record.args
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
        ilog_info("This is info message");
        ilog(Level::Info) << "Stream Log" << std::endl;
    }

}


using namespace IrisLog;

class PerformanceTest {
    std::chrono::high_resolution_clock::time_point start_;
    std::string name_;
public:
    PerformanceTest(const std::string& name) : name_(name) {
        start_ = std::chrono::high_resolution_clock::now();
    }

    ~PerformanceTest() {
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_).count();
        std::cout << "[PerformanceTest] " << name_ << " finished in " << elapsed_ms << " ms\n";
    }
};


int main()
{
    // 设置日志器
    static ConsoleLogger console_logger;
    LoggerInstance::instance().set_logger(&console_logger);

    double pi = 3.14159;

    // 使用日志宏
    ilog_trace("This is trace message");
    ilog_debug("This is debug message");
    Test::First::func();
    ilog_warn("This is warn message");
    ilog_error("float is : %.2f {}", pi);
    int x = 42;
    double y = 3.14159;
    std::string name = "Alice";

    ilog_debug("Hello, {}!\n", name);
    ilog_debug("x = {:04}, y = {:.2f}\n", x, y);


    std::cout << std::endl;

    ilog_t(iL::Info, "master") << "test" << (1+1) << std::endl;
    logger.set_target(std::nullopt);
    ilog(Level::Debug) << "test" << (1+1) << std::endl; {
        size_t log_count = 10000;
        PerformanceTest test("Logging 10000 messages");
        for (size_t i = 0; i < log_count; ++i) {
            // ilog_debug("Test log number: %zu", i);
            // ilog(IL::Debug) << "test" << "for" << i << "hello, world!" << std::endl;
        }
    } // 出作用域，自动输出耗时

    return 0;
}

