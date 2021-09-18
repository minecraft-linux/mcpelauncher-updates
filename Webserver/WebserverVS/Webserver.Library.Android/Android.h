#pragma once
#include "../../../Http/SocketListener.h"
extern "C" 
{
	struct _Files
	{
		const char ** names;
		int size;
	};
	struct _File
	{
		const char * name;
		int length;
		int(*read)(char * data, int length);
	};
	void * Initialize();
	char * Start(void * obj, int port, void(*callback)(const char * ip, int port), _Files(*getFilenames)(), _File(*getFile)(const char * name));
	void Cancel(void * obj);
	void Destroy(void * obj);
}