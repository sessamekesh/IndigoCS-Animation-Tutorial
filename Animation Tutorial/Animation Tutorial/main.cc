#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN

#include "DemoApp.h"

int main(int argc, char** argv)
{
	Run(GetModuleHandle(nullptr));

	return EXIT_SUCCESS;
}