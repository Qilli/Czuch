#pragma once
#include"Core/Singleton.h"

namespace Czuch
{
	template <typename T > class CZUCH_API BaseSubsystem : public Singleton<T>
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;
		virtual void Update() = 0;
	};
}

