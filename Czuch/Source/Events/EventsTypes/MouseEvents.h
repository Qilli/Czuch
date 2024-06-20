#pragma once
#include"Core/EngineCore.h"
#include"Events/Event.h"
#include"Core/MouseCodes.h"


namespace Czuch
{
	class CZUCH_API MouseButtonEvent : public Event
	{
	public:
		inline void SetData(MouseCode mouseCodeValue) { m_MouseCode = mouseCodeValue; }
		inline MouseCode GetMouseCode() const { return m_MouseCode; }
	protected:
		MouseButtonEvent(MouseCode mouseCode) :m_MouseCode(mouseCode) {}
	private:
		MouseCode m_MouseCode;
	};

	class CZUCH_API MouseButtonDownEvent :public MouseButtonEvent
	{
	public:
		static void CreateAndDispatch(MouseCode mouseCode);
	public:
		EVENT_METHODS("MouseButtonDownEvent",EventCategoryType::Mouse)
	protected:
		MouseButtonDownEvent(MouseCode mouseCode) :MouseButtonEvent(mouseCode) {}
	};

	class CZUCH_API MouseButtonUpEvent :public MouseButtonEvent
	{
	public:
		static void CreateAndDispatch(MouseCode mouseCode);
	public:
		EVENT_METHODS("MouseButtonUpEvent", EventCategoryType::Mouse)
	protected:
		MouseButtonUpEvent(MouseCode mouseCode) :MouseButtonEvent(mouseCode) {}
	};

	class CZUCH_API MouseMovedEvent :public Event
	{
	public:
		static void CreateAndDispatch(const F32 x,const F32 y);
		inline F32 GetMouseX() const { return m_X; }
		inline F32 GetMouseY() const { return m_Y; }
	public:
		EVENT_METHODS("MouseMovedEvent", EventCategoryType::Mouse)
	protected:
		MouseMovedEvent(F32 x, F32 y) : m_X(x),m_Y(y) {}
	private:
		F32 m_X, m_Y;
	};

	class CZUCH_API MouseScrolledEvent :public Event
	{
	public:
		static void CreateAndDispatch(const F32 xOffset, const F32 yOffset);
		inline F32 GetMouseXOffset() const { return m_XOffset; }
		inline F32 GetMouseYOffset() const { return m_YOffset; }
	public:
		EVENT_METHODS("MouseScrolledEvent", EventCategoryType::Mouse)
	protected:
		MouseScrolledEvent(F32 xOffset, F32 yOffset) : m_XOffset(xOffset), m_YOffset(yOffset) {}
	private:
		F32 m_XOffset, m_YOffset;
	};
}
