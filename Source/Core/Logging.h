#pragma once

#include "Common.h"

#include <chrono>
#include <filesystem>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/chrono.h>

namespace RT {

	class Logging {
	public:
		static void Init() {
			auto CurrentTime = std::chrono::system_clock::now();
			std::string LogFileName = fmt::format("Log/Log_{:%Y%m%d%H%M}.txt", CurrentTime);

			auto ConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
			auto FileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(LogFileName);

			spdlog::sinks_init_list SinkList = { ConsoleSink, FileSink };

			LoggerHandle = std::make_shared<spdlog::logger>("Log", SinkList);
			LoggerHandle->set_pattern("%^[%T][%l]: %v%$");
			LoggerHandle->set_level(spdlog::level::level_enum::info);
		}

		static void Shuwdown() {
			
		}

		template<typename... Args>
		static void Info(Args... Arguments) {
			LoggerHandle->info(Arguments...);
		}

		template<typename... Args>
		static void Warning(Args... Arguments) {
			LoggerHandle->warn(Arguments...);
		}

		template<typename... Args>
		static void Error(Args... Arguments) {
			LoggerHandle->error(Arguments...);
		}

		template<typename... Args>
		static void Critical(Args... Arguments) {
			LoggerHandle->critical(Arguments...);
		}

	private:
		inline static Ref<spdlog::logger> LoggerHandle;
	};

}

#define LOG_INFO(...)		Logging::Info(__VA_ARGS__)
#define LOG_WARN(...)		Logging::Info(__VA_ARGS__)
#define LOG_ERROR(...)		Logging::Info(__VA_ARGS__)
#define LOG_CRITICAL(...)	Logging::Info(__VA_ARGS__)