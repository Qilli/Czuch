#pragma once
#include"EngineCore.h"

namespace Czuch
{
	template <typename T> class Singleton
	{
	protected:
		static T* m_Target;
	public:
		Singleton(const Singleton<T>&) = delete;
		Singleton& operator=(const Singleton<T>&) = delete;

		Singleton(void)
		{
			AssertCheck(!m_Target);
			m_Target = static_cast<T*>(this);
		}

		~Singleton()
		{
			AssertCheck(m_Target);
			m_Target = nullptr;
		}

		static T& Get(void) 
		{
			AssertCheck(m_Target);
			return *m_Target;
		}

		static T* GetPtr(void) 
		{
			AssertCheck(m_Target);
			return m_Target;
		}
	};

	template <class T> T* Singleton<T>::m_Target;
}
