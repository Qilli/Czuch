#pragma once
#include"../Core/EngineCore.h"
#include"../Core/StringID.h"

namespace Czuch
{
	typedef unsigned int EventTypeID;

	enum class EventCategoryType
	{
		General,
		Application,
		Mouse,
		Keyboard
	};

#define EVENT_METHODS(name,category) EventTypeID GetEventType() const override { return sID(#name).GetGuid(); }\
	const char* GetName() const override { return #name; }\
	EventCategoryType GetCategory() const { return category; }\
	static EventTypeID GetStaticEventType() { return sID(#name).GetGuid(); }

	class CZUCH_API Event
	{
	public:
		virtual std::string ToString() const { return GetName(); }
		virtual EventTypeID GetEventType() const { return sID("BaseEvent").GetGuid(); }
		virtual const char* GetName() const { return "BaseEvent"; }\
		virtual EventCategoryType GetCategory() const { return EventCategoryType::General; }
		void SetHandled(bool handled) { m_Handled = handled; }
		bool IsHandled() const { return m_Handled; }
		static void Release(Event* evt);
	protected:
		Event() {}
		virtual ~Event() = default;
	private:
		bool m_Handled = false;
	};

}

