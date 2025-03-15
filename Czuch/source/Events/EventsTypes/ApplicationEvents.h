#pragma once
#include"../Event.h"

namespace Czuch
{

	class CZUCH_API WindowSizeChangedEvent : public Event
	{
	public:
		static void CreateAndDispatch(U32 width,U32 height);
	public:
		inline void SetData(U32 width, U32 height) { m_Width = width; m_Height = height; }
		inline U32 GetWidth() const { return m_Width; }
		inline U32 GetHeight() const { return m_Height; }
		EVENT_METHODS("WindowSizeChangedEvent",EventCategoryType::Application)
	protected:
		WindowSizeChangedEvent() :m_Width(0),m_Height(0) {}
	private:
		U32 m_Width;
		U32 m_Height;
	};


	class CZUCH_API WindowClosedEvent : public Event
	{
	public:
		static void CreateAndDispatch();
	public:
		EVENT_METHODS("WindowClosedEvent", EventCategoryType::Application)
	protected:
		WindowClosedEvent(){}

	};
}
