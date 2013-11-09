#ifndef __TINY_STRING_DEFINE_H__
#define __TINY_STRING_DEFINE_H__

#ifdef TINY_STRING_DLL_EXPORT
#define TINY_STRING_DLL __declspec(dllexport)
#else
#define TINY_STRING_DLL __declspec(dllimport)
#endif

#define null 0
#define uint unsigned int

#endif