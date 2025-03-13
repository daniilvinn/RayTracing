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

			std::vector<spdlog::sink_ptr> Sinks;

			Sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

			if (!IN_RELEASE_BUILD) {
				Sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(LogFileName));
			}

			LoggerHandle = std::make_shared<spdlog::logger>("Log", Sinks.begin(), Sinks.end());
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