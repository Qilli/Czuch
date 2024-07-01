#include"czpch.h"
#include "EventsManager.h"
#include "Events/EventsTypes/ApplicationEvents.h"
#include "Events/EventsTypes/InputEvents.h"
#include "Events/EventsTypes/MouseEvents.h"

namespace Czuch
{
	void EventsManager::DispatchEvent(Event* event)
	{
		m_EventsToDispatch.push(event);
	}

	void EventsManager::AddListener(EventTypeID eventType, IEventsListener* listener, int priority)
	{
		auto &result = GetContainerOfType(eventType);
		result.AddListener(listener,priority);
	}

	void EventsManager::AddListenerForAllEvents(IEventsListener* listener, int priority)
	{
		AddListener(WindowClosedEvent::GetStaticEventType(), listener,priority);
		AddListener(WindowSizeChangedEvent::GetStaticEventType(), listener,priority);
		AddListener(KeyUpEvent::GetStaticEventType(), listener, priority);
		AddListener(KeyDownEvent::GetStaticEventType(), listener, priority);
		AddListener(KeyTypedEvent::GetStaticEventType(), listener, priority);
		AddListener(MouseMovedEvent::GetStaticEventType(), listener, priority);
		AddListener(MouseButtonDownEvent::GetStaticEventType(), listener, priority);
		AddListener(MouseButtonUpEvent::GetStaticEventType(), listener, priority);
		AddListener(MouseScrolledEvent::GetStaticEventType(), listener, priority);
	}

	void EventsManager::RemoveListener(EventTypeID eventType, IEventsListener* listener)
	{
		auto result=m_ListenersContainer.find(eventType);
		if (result != m_ListenersContainer.end())
		{
			result->second.RemoveListener(listener);
		}
	}

	void EventsManager::Init(RenderSettings* settings)
	{
		BaseSubsystem::Init(settings);
		Clear();
	}

	void EventsManager::Shutdown()
	{
		Clear();
	}

	void EventsManager::Update(TimeDelta timeDelta)
	{
		while (!m_EventsToDispatch.empty())
		{
			auto &current = m_EventsToDispatch.front();
			auto &listenersContainer=GetContainerOfType(current->GetEventType());
			listenersContainer.Invoke(*current);
			Event::Release(current);
			m_EventsToDispatch.pop();
		}
	}

	void EventsManager::Clear()
	{
		for (auto current= m_ListenersContainer.begin(); current != m_ListenersContainer.end();current++)
		{
			current->second.RemoveAll();
		}
		m_ListenersContainer.clear();
	}

	EventsManager::ListenersContainer& EventsManager::GetContainerOfType(const EventTypeID& type)
	{
		auto iterator=m_ListenersContainer.find(type);
		if (iterator != m_ListenersContainer.end())
		{
			return iterator->second;
		}
		m_ListenersContainer.insert(std::make_pair(type, EventsManager::ListenersContainer(type)));
		auto it = m_ListenersContainer.find(type);
		return it->second;
	}

	const EventsManager::ListenersContainer& EventsManager::ListenersContainer::operator=(const ListenersContainer& other)
	{
		return *this;
	}

	EventsManager::ListenersContainer::ListenersContainer(EventTypeID eventType):m_targetEventID(eventType)
	{
	}

	void EventsManager::ListenersContainer::AddListener(IEventsListener* listener,int priority)
	{
		m_Listeners.push_back({listener,priority});
		std::sort(m_Listeners.begin(), m_Listeners.end(), std::greater<EventsManager::Listener>());
	}

	const void EventsManager::ListenersContainer::RemoveListener(IEventsListener* listener)
	{
		int at = -1;
		Listener toSearch = { listener,0 };
		for (size_t i = 0; i < m_Listeners.size(); i++)
		{
			if (m_Listeners[i] == toSearch)
			{
				at = i;
				break;
			}
		}

		if (at != -1)
		{
			m_Listeners.erase(m_Listeners.begin()+at);
		}
	}

	void EventsManager::ListenersContainer::RemoveAll()
	{
		m_Listeners.clear();
	}

	const void EventsManager::ListenersContainer::Invoke(Event& e)
	{
		for(Listener list: m_Listeners)
		{
			if (e.IsHandled())
			{
				break;
			}
			list.TryInvoke(e);
		}
	}

	bool EventsManager::Listener::operator==(Listener& comp)
	{
		return m_Listener == comp.m_Listener;
	}

	bool EventsManager::Listener::operator!=(Listener& comp)
	{
		return !(*this == comp);
	}

	bool EventsManager::Listener::operator<(Listener& comp)
	{
		return (m_Priority < comp.m_Priority);
	}

	bool EventsManager::Listener::operator>(const Listener& comp) const
	{
		return (m_Priority > comp.m_Priority);
	}

	EventsManager::Listener::Listener(IEventsListener* listener, int priority)
	{
		m_Listener = listener;
		m_Priority = priority;
	}

	EventsManager::Listener::Listener(const Listener& other)
	{
		m_Listener = other.m_Listener;
		m_Priority = other.m_Priority;
	}

	EventsManager::Listener& EventsManager::Listener::operator=(const Listener&& other) noexcept(true)
	{
		m_Listener = other.m_Listener;
		m_Priority = other.m_Priority;
		return *this;
	}

	EventsManager::Listener& EventsManager::Listener::operator=(const Listener& other)
	{
		m_Listener = other.m_Listener;
		m_Priority = other.m_Priority;
		return *this;
	}

	void EventsManager::Listener::TryInvoke(Event& e)
	{
		if (m_Listener != nullptr)
		{
			m_Listener->OnEvent(e);
		}
	}

}
