#pragma once
#include"Entity.h"
#include"Core/TimeDelta.h"

namespace Czuch
{

	class NativeBehaviour
	{
	public:
		template<typename T>
		T& GetComponent()
		{
			return m_Entity.GetComponent<T>();
		}

		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate(TimeDelta delta) {}
		virtual void OnEnable() {}
		virtual void OnDisable() {}
	private:
		friend class Scene;
		Entity m_Entity;
	};

}


