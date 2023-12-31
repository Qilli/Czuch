#pragma once
#include"../Core/EngineCore.h"
#include"../Core/StringID.h"

namespace Czuch
{
	typedef unsigned int EventTypeID;

#define EVENT_METHODS(name) EventTypeID GetEventType() const override { return sID(#name).GetGuid(); }\
	const char* GetName() const override { return #name; }\
	static EventTypeID GetStaticEventType() { return sID(#name).GetGuid(); }

	class CZUCH_API Event
	{
	public:
		virtual std::string ToString() const { return GetName(); }
		virtual EventTypeID GetEventType() const { return sID("BaseEvent").GetGuid(); }
		virtual const char* GetName() const { return "BaseEvent"; }
		static void Release(Event* evt);
	protected:
		Event() {}
	};

}

