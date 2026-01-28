#pragma once


#include <stdio.h>

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif


#ifdef _WIN32
#include <windows.h>
#else
#include "Common.h"
#endif


class CReadFileChanges
{
public:
	CReadFileChanges() {
		_lastFileInfo.dwFileAttributes = INVALID_FILE_ATTRIBUTES;
	}
	~CReadFileChanges() {}
	void AddFile(LPCTSTR szDirectory, DWORD dwNotifyFilter);
	BOOL DetectChanges();
	void Terminate();

private:
	LPCTSTR _szFile = nullptr;
	DWORD _dwNotifyFilter = 0;
	WIN32_FILE_ATTRIBUTE_DATA _lastFileInfo = {};
};

