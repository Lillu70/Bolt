#include "Sandbox.h"


int main()
{
	auto sb = new Sandbox();
	sb->Run();

	delete sb;
}