#pragma once
#include"BaseSubsystem.h"
#include"../Events/Event.h"
#include"../Events/IEventsListener.h"

namespace Czuch
{
	class CZUCH_API EventsManager: public BaseSubsystem<EventsManager>
	{
	public:
		void DispatchEvent(Event* event);
		void AddListener(EventTypeID eventType, IEventsListener* listener);
		void RemoveListener(EventTypeID eventType, IEventsListener* listener);
		void Init();
		void Shutdown();
		void Update(TimeDelta timeDelta);
	private:
		void Clear();
	private:
		class ListenersContainer;
		std::queue<Event*> m_EventsToDispatch;
		std::unordered_map<EventTypeID, ListenersContainer> m_listeners;
	private:

		struct Listener
		{
			IEventsListener* m_listener;
			bool operator==(Listener& comp);
			void TryInvoke(Event& e);
		};

		class ListenersContainer
		{
		public:
			const ListenersContainer& operator=(const ListenersContainer& other);
			ListenersContainer():m_targetEventID(0){}
			ListenersContainer(EventTypeID eventType);
			void AddListener(IEventsListener* listener);
			const void RemoveListener(IEventsListener* listener);
			void RemoveAll();
			const void Invoke(Event& e);
		private:
			EventTypeID m_targetEventID;
			std::vector<Listener> m_listeners;
		};
	private:
		ListenersContainer& GetContainerOfType(const EventTypeID& type);
	};

#define DISPATCH_EVENT(evt)EventsManager::GetPtr()->DispatchEvent(evt);
#define LISTEN_TO_EVENT(eventType, listener) EventsManager::GetPtr()->AddListener(eventType, listener);
#define STOP_LISTEN_TO_EVENT(eventType, listener) EventsManager::GetPtr()->RemoveListener(eventType, listener);

}

