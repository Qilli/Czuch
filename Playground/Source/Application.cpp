#include"czpch.h"
#include<Czuch.h>
#include <typeinfo> 
#include <iostream> 
class EventHandler : public Czuch::IEventsListener
{
	// Odziedziczono za poœrednictwem elementu IEventsListener
	virtual void OnEvent(const Czuch::Event& e) override
	{
		LOG_BE_ERROR(e.ToString());
		LOG_BE_INFO("Test log only gfdgdfgdfg");
	}
};

int main()
{
	Czuch::EngineRoot* root = new Czuch::EngineRoot();
	root->Init("engineConfig.cfg");
	LOG_BE_INFO("Test log only {0}", 12);

	EventHandler handler;

	Czuch::EventsManager::GetPtr()->AddListener(Czuch::KeyDownEvent::GetStaticEventType(), &handler);

	Czuch::KeyDownEvent::CreateAndDispatch(12);

	root->Run();
	root->Shutdown();
	delete root;
	return 0;
}