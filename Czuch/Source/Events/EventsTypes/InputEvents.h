#pragma once
#include"../Event.h"
#include"Core/KeyCodes.h"


namespace Czuch
{
	class CZUCH_API KeyEvent : public Event
	{
	public:
		inline void SetData(KeyCode keyCodeValue) { m_KeyCode = keyCodeValue; }
		inline I32 GetKeyCode() const { return m_KeyCode; }
	protected:
		KeyEvent(KeyCode keyCode) :m_KeyCode(keyCode) {}
	private:
		KeyCode m_KeyCode;
	};

	class CZUCH_API KeyDownEvent : public KeyEvent
	{
	public:
		static void CreateAndDispatch(KeyCode keyCode,bool repeated=false);
	public:
		EVENT_METHODS("KeyDownEvent")
	protected:
		KeyDownEvent(KeyCode keyCode,bool repeated) :KeyEvent(keyCode),m_Repeated(repeated) {}
	private:
		bool m_Repeated;
	};

	class CZUCH_API KeyUpEvent : public KeyEvent
	{
	public:
		static void CreateAndDispatch(KeyCode keyCode);
	public:
		EVENT_METHODS("KeyUpEvent")
	protected:
		KeyUpEvent(KeyCode keyCode) :KeyEvent(keyCode) {}
	};

	class CZUCH_API KeyTypedEvent : public KeyEvent
	{
	public:
		static void CreateAndDispatch(KeyCode keyCode);
	public:
		EVENT_METHODS("KeyTypedEvent")
	protected:
		KeyTypedEvent(KeyCode keyCode) :KeyEvent(keyCode) {}
	};
}