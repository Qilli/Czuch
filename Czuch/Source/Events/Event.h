#pragma once
#include"../Core/EngineCore.h"
#include"../Core/StringID.h"

namespace Czuch
{
	typedef unsigned int EventTypeID;

	class CZUCH_API Event
	{
	public:
		virtual std::string ToString() const { return GetName(); }
		virtual EventTypeID GetEventType() const { return sID("BaseEvent").GetID(); }
		virtual const char* GetName() const { return "BaseEvent"; }
		static void Release(Event* evt);
	protected:
		Event() {}
	};

}

