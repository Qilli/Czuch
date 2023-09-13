#pragma once
#include"../Event.h"

namespace Czuch
{

	class CZUCH_API KeyDownEvent : public Event
	{
	public:
		static void CreateAndDispatch(int keyCode);
	public:
		inline void SetData(int keyCodeValue) { m_KeyCode = keyCodeValue; }
		inline I32 GetKeyCode() const { return m_KeyCode; }
		EventTypeID GetEventType() const override { return sID("KeyDownEvent").GetID(); }
		const char* GetName() const override { return "KeyDownEvent"; }
		static EventTypeID GetStaticEventType() { return sID("KeyDownEvent").GetID(); }
	protected:
		KeyDownEvent():m_KeyCode(0) {}
	private:
		I32 m_KeyCode;
	};

}
