#ifndef SMOBJECT_H
#define SMOBJECT_H
#include <Windows.h>
#include <tchar.h>
#include <string>

// #ifndef UNICODE  
// typedef std::string String;
// #else
// typedef std::wstring String;
// #endif

class SMObject
{
	HANDLE CreateHandle;
	HANDLE AccessHandle;
	TCHAR *szName;
	int Size;
public:
	void *pData;
	bool SMCreateError;
	bool SMAccessError;
public:
	SMObject();
	SMObject(TCHAR* szname, int size);

	~SMObject();
	int SMCreate();
	int SMAccess();
	void SetSzname(TCHAR* szname);
	void SetSize(int size);
};
#endif


