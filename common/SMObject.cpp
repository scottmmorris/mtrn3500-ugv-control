#include <Windows.h>
#include <SMObject.h>

//Code framework by A/Prof. J.Katupitiya(c)2013
HANDLE SMCreateReadWrite(int objectSize, TCHAR szName[])
{
	HANDLE h;
	h = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,      // read/write access
		0,
		objectSize,
		szName);

	return h;
}

HANDLE SMOpenReadWrite(TCHAR szName[])
{
	HANDLE h;
	h = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,   // read/write access
		FALSE,
		szName);
	return h;
}

void* SMAccessReadWrite(HANDLE openedHandle, int objectSize)
{
	void* p;
	p = MapViewOfFile(openedHandle,
		FILE_MAP_ALL_ACCESS,    // read/write permission
		0,
		0,
		objectSize);

	return p;
}

SMObject::SMObject()
{
	CreateHandle = NULL;
	AccessHandle = NULL;
	SMCreateError = false;
	SMAccessError = false;
	szName = _TEXT("");
	pData = NULL;
	Size = 1;
}

SMObject::SMObject(TCHAR* szname, int size)
{
	CreateHandle = NULL;
	AccessHandle = NULL;
	SMCreateError = false;
	SMAccessError = false;
	pData = NULL;
	szName = szname;
	Size = size;
}

SMObject::~SMObject()
{
	if (CreateHandle != NULL)
		CloseHandle(CreateHandle);
	if (AccessHandle != NULL)
		CloseHandle(AccessHandle);
	if (pData != NULL)
		UnmapViewOfFile(pData);

}

int SMObject::SMCreate()
{
	CreateHandle = SMCreateReadWrite(Size, szName);
	if (CreateHandle == NULL)
		SMCreateError = true;
	else
		SMCreateError = false;

	return SMCreateError;
}

int SMObject::SMAccess()
{
	// Requesting a view of Modules memory
	AccessHandle = SMOpenReadWrite(szName);
	if (AccessHandle == NULL)
		SMAccessError = true;
	else
		SMAccessError = false;

	// Requesting access to Modules memory
	pData = SMAccessReadWrite(AccessHandle, Size);
	if (pData == NULL)
		SMAccessError = true;
	else
		SMAccessError = false;

	return SMAccessError;
}

void SMObject::SetSzname(TCHAR *szname)
{
	szName = szname;
}

void SMObject::SetSize(int size)
{
	Size = size;
}

