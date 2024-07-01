#include"czpch.h"
#include "Logging.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/common.h"

namespace Czuch
{
	void Logging::Init(RenderSettings* settings)
	{
		BaseSubsystem::Init(settings);
		spdlog::set_pattern("%^[%T] %n: %v%$");
		m_SpdBackendLogger = spdlog::stdout_color_mt("Czuch");
		m_SpdBackendLogger->set_level(spdlog::level::trace);
		m_SpdFrontendLogger = spdlog::stdout_color_mt("App");
		m_SpdFrontendLogger->set_level(spdlog::level::trace);
	}

	void Logging::Shutdown()
	{

	}

	void Logging::Update(TimeDelta timeDelta)
	{

	}
}
