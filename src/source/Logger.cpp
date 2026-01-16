#include "Logger.h"
#include <iostream>
#include <filesystem>

std::shared_ptr<spdlog::logger> Logger::s_logger = nullptr;

void Logger::Init(const std::string& log_dir, const std::string& level) {
    try {
        // 创建日志目录
        std::filesystem::create_directories(log_dir);

        // 日志文件路径
        std::string log_file = log_dir + "/watchdog_service.log";

        // 创建控制台输出 sink（彩色）
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_pattern("[%H:%M:%S] [%l] %v");

        // 创建文件 sink（轮转日志，最大 10MB，保留 5 个文件）
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            log_file, 10 * 1024 * 1024, 5);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");

        // 合并 sink
        std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};

        // 创建日志对象
        s_logger = std::make_shared<spdlog::logger>("watchdog", sinks.begin(), sinks.end());
        s_logger->set_level(spdlog::level::from_str(level));
        s_logger->flush_on(spdlog::level::err);

        // 注册为全局日志
        spdlog::register_logger(s_logger);

        LOG_INFO("Logger initialized successfully");
    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Failed to initialize logger: " << ex.what() << std::endl;
    }
}

std::shared_ptr<spdlog::logger> Logger::Get() {
    if (!s_logger) {
        Logger::Init();
    }
    return s_logger;
}

void Logger::Shutdown() {
    if (s_logger) {
        spdlog::drop("watchdog");
        s_logger.reset();
    }
    spdlog::shutdown();
}
