#pragma once
#include <iostream>

#ifdef _DEBUG
#define ASSERT(STATEMENT, MESSAGE)						\
if(!(STATEMENT))										\
{														\
std::cout << "\n[FATAL ERROR]: " << MESSAGE << "!\n";	\
abort();												\
}														\
0 + 0
#else
#define ASSERT(STATEMENT, MESSAGE)
#endif

#ifdef _DEBUG
#define TRACE(MESSAGE)									\
{														\
std::cout << "[TRACE] " << MESSAGE << "\n";				\
}														\
0 + 0
#else
#define TRACE(STATEMENT, MESSAGE)
#endif

#define ERROR(MESSAGE)									\
{														\
std::cout << "\n[FATAL ERROR]: " << MESSAGE << "!\n";	\
abort();												\
}														\
0 + 0

#ifdef _DEBUG
#define WARN(MESSAGE)									\
{														\
std::cout << "\n[WARNING]: " << MESSAGE << "!\n";		\
}														\
0 + 0
#else
#define WARN(MESSAGE)
#endif

#define ERROR_NO_SUITABLE_DEVICE_MEMORY()				\
{														\
const char* MESSAGE = "Failed to find suitable memory";	\
std::cout << "\n[FATAL ERROR]: " << MESSAGE << "!\n";	\
abort();												\
}														\
0 + 0