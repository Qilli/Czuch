#pragma once
#include"BaseSubsystem.h"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace Czuch
{
	class CZUCH_API Logging: public BaseSubsystem<Logging>
	{
	public:
		void Init();
		void Shutdown();
		void Update(TimeDelta timeDelta);
	public:
		inline std::shared_ptr<spdlog::logger> GetBackendLogger() { return m_SpdBackendLogger; }
		inline std::shared_ptr<spdlog::logger> GetFrontendLogger() { return m_SpdFrontendLogger; }
	private:
		std::shared_ptr<spdlog::logger> m_SpdBackendLogger;
		std::shared_ptr<spdlog::logger> m_SpdFrontendLogger;
	};
}

#define LOG_BE_ERROR(...) Czuch::Logging::GetPtr()->GetBackendLogger()->error(__VA_ARGS__)
#define LOG_BE_WARN(...) Czuch::Logging::GetPtr()->GetBackendLogger()->warn(__VA_ARGS__)
#define LOG_BE_INFO(...) Czuch::Logging::GetPtr()->GetBackendLogger()->info(__VA_ARGS__)
#define LOG_BE_TRACE(...) Czuch::Logging::GetPtr()->GetBackendLogger()->trace(__VA_ARGS__)
#define LOG_BE_FATAL(...) Czuch::Logging::GetPtr()->GetBackendLogger()->fatal(__VA_ARGS__)

#define LOG_FE_ERROR(...) Czuch::Logging::GetPtr()->GetFrontendLgger()->error(__VA_ARGS__)
#define LOG_FE_WARN(...) Czuch::Logging::GetPtr()->GetFrontendLgger()->warn(__VA_ARGS__)
#define LOG_FE_INFO(...) Czuch::Logging::GetPtr()->GetFrontendLgger()->info(__VA_ARGS__)
#define LOG_FE_TRACE(...) Czuch::Logging::GetPtr()->GetFrontendLgger()->trace(__VA_ARGS__)
#define LOG_FE_FATAL(...) Czuch::Logging::GetPtr()->GetFrontendLgger()->fatal(__VA_ARGS__)



