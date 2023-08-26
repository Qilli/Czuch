#include<Czuch.h>

int main()
{
	Czuch::EngineRoot* root = new Czuch::EngineRoot();
	root->Run();
	delete root;
	return 0;
}