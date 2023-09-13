#pragma once
#include<cassert>
#include<string>

#ifdef CZUCH_PLATFORM_WINDOWS
	#ifdef CZUCH_BUILD_DLL
		#define CZUCH_API __declspec(dllexport)
	#else
		#define CZUCH_API __declspec(dllimport)
	#endif
#else
	#error Czuch do not support this platform
#endif

#ifdef CZUCH_PLATFORM_WINDOWS
#include <windows.h>
#endif

#define AssertCheck(x) if(!(x))assert(false);
#define AssertWithMsg(xm,msg) if(!(x))assert(false);


typedef unsigned long U64;
typedef unsigned int U32;
typedef int I32;
typedef float F32;
typedef double F64;
typedef unsigned short U16;
typedef short I16;
typedef unsigned char U8;
typedef char I8;
typedef unsigned int ID;
typedef std::string CzuchStr;

const CzuchStr EMPTY_STRING = "";