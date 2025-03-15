#pragma once
#include"BaseSubsystem.h"
#include"../Events/Event.h"
#include"../Events/IEventsListener.h"
#include<unordered_map>
#include<queue>

namespace Czuch
{
	class CZUCH_API EventsManager: public BaseSubsystem<EventsManager>
	{
	public:
		void DispatchEvent(Event* event);
		void AddListener(EventTypeID eventType, IEventsListener* listener,int priority=0);
		void AddListenerForAllEvents(IEventsListener* listener,int priority=0);
		void RemoveListener(EventTypeID eventType, IEventsListener* listener);
		void Init(RenderSettings* settings) override ;
		void Shutdown();
		void Update(TimeDelta timeDelta);
	private:
		void Clear();
	private:
		class ListenersContainer;
		std::queue<Event*> m_EventsToDispatch;
		std::unordered_map<EventTypeID, ListenersContainer> m_ListenersContainer;
	private:

		struct Listener
		{
			IEventsListener* m_Listener;
			int m_Priority;
			bool operator==(Listener& comp);
			bool operator!=(Listener& comp);
			bool operator<(Listener& comp);
			bool operator>(const Listener& comp) const;
			Listener(IEventsListener* listener, int priority);
			Listener(const Listener& other);
			Listener& operator=(const Listener&& other) noexcept(true);
			Listener& operator=(const Listener& other);
			void TryInvoke(Event& e);
		};

		class ListenersContainer
		{
		public:
			const ListenersContainer& operator=(const ListenersContainer& other);
			ListenersContainer():m_targetEventID(0){}
			ListenersContainer(EventTypeID eventType);
			void AddListener(IEventsListener* listener,int priority=0);
			const void RemoveListener(IEventsListener* listener);
			void RemoveAll();
			const void Invoke(Event& e);
		private:
			EventTypeID m_targetEventID;
			std::vector<Listener> m_Listeners;
		};
	private:
		ListenersContainer& GetContainerOfType(const EventTypeID& type);
	};

#define DISPATCH_EVENT(evt)EventsManager::GetPtr()->DispatchEvent(evt);
#define LISTEN_TO_EVENT(eventType, listener) EventsManager::GetPtr()->AddListener(eventType, listener);
#define STOP_LISTEN_TO_EVENT(eventType, listener) EventsManager::GetPtr()->RemoveListener(eventType, listener);
#define LISTEN_TO_ALL_EVENTS(listener) EventsManager::GetPtr()->AddListenerForAllEvents(listener);

}

