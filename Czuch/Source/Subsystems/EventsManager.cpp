#include"czpch.h"
#include "EventsManager.h"

namespace Czuch
{
	void EventsManager::DispatchEvent(Event* event)
	{
		m_EventsToDispatch.push(event);
	}

	void EventsManager::AddListener(EventTypeID eventType, IEventsListener* listener)
	{
		auto &result = GetContainerOfType(eventType);
		result.AddListener(listener);
	}

	void EventsManager::RemoveListener(EventTypeID eventType, IEventsListener* listener)
	{
		auto result=m_listeners.find(eventType);
		if (result != m_listeners.end())
		{
			result->second.RemoveListener(listener);
		}
	}

	void EventsManager::Init()
	{
		Clear();
	}

	void EventsManager::Shutdown()
	{
		Clear();
	}

	void EventsManager::Update()
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
		for (U32 i = 0; i < m_listeners.size(); i++)
		{
			m_listeners[i].RemoveAll();
		}
		m_listeners.clear();
	}

	EventsManager::ListenersContainer& EventsManager::GetContainerOfType(const EventTypeID& type)
	{
		auto iterator=m_listeners.find(type);
		if (iterator != m_listeners.end())
		{
			return iterator->second;
		}
		m_listeners.insert(std::make_pair(type, EventsManager::ListenersContainer(type)));
		auto it = m_listeners.find(type);
		return it->second;
	}

	const EventsManager::ListenersContainer& EventsManager::ListenersContainer::operator=(const ListenersContainer& other)
	{
		return *this;
	}

	EventsManager::ListenersContainer::ListenersContainer(EventTypeID eventType):m_targetEventID(eventType)
	{
	}

	void EventsManager::ListenersContainer::AddListener(IEventsListener* listener)
	{
		m_listeners.push_back({listener});
	}

	const void EventsManager::ListenersContainer::RemoveListener(IEventsListener* listener)
	{
		int at = -1;
		Listener toSearch = { listener };
		for (size_t i = 0; i < m_listeners.size(); i++)
		{
			if (m_listeners[i] == toSearch)
			{
				at = i;
				break;
			}
		}

		if (at != -1)
		{
			m_listeners.erase(m_listeners.begin()+at);
		}
	}

	void EventsManager::ListenersContainer::RemoveAll()
	{
		m_listeners.clear();
	}

	const void EventsManager::ListenersContainer::Invoke(Event& e)
	{
		for each (Listener list in m_listeners)
		{
			list.TryInvoke(e);
		}
	}

	bool EventsManager::Listener::operator==(const Listener& comp)
	{
		return m_listener == comp.m_listener;
	}

	void EventsManager::Listener::TryInvoke(Event& e)
	{
		if (m_listener != nullptr)
		{
			m_listener->OnEvent(e);
		}
	}

}
