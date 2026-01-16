#ifndef LOGGER_H
#define LOGGER_H
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#include <string>

class Logger {
public:
    /**
     * 初始化日志系统
     * @param log_dir 日志文件目录
     * @param level 日志级别 (trace, debug, info, warn, err, critical)
     */
    static void Init(const std::string& log_dir = "./logs",
                     const std::string& level = "info");

    /**
     * 获取日志对象
     */
    static std::shared_ptr<spdlog::logger> Get();

    /**
     * 关闭日志系统
     */
    static void Shutdown();

private:
    static std::shared_ptr<spdlog::logger> s_logger;
};

// 便捷日志宏
#define LOG_TRACE(... ) Logger::Get()->trace(__VA_ARGS__)
#define LOG_DEBUG(...) Logger::Get()->debug(__VA_ARGS__)
#define LOG_INFO(...)  Logger::Get()->info(__VA_ARGS__)
#define LOG_WARN(...)  Logger::Get()->warn(__VA_ARGS__)
#define LOG_ERROR(... ) Logger::Get()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) Logger::Get()->critical(__VA_ARGS__)
#endif // LOGGER_H
