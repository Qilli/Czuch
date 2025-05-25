#include"czpch.h"
#include "Logging.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/common.h"

namespace Czuch
{
	void Logging::Init(EngineSettings* settings)
	{
		BaseSubsystem::Init(settings);
		spdlog::set_pattern("%^[%T] %n: %v%$");

		m_StringListSink = std::make_shared<StringListSink>();
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		m_SpdBackendLogger = std::make_shared<spdlog::logger>("Czuch", spdlog::sinks_init_list{ m_StringListSink,console_sink });
		m_SpdBackendLogger->set_level(spdlog::level::trace);
		m_SpdFrontendLogger = spdlog::stdout_color_mt("App");
		m_SpdFrontendLogger->set_level(spdlog::level::trace);
		m_SpdBackendLogger->info("Logging Initialized");
	}

	void Logging::Shutdown()
	{

	}

	void Logging::Update(TimeDelta timeDelta)
	{

	}
}
