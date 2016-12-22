#pragma once

#include "WebPlugin.h"

class CommonPlugin : public WebPlugin
{
public:
	CommonPlugin(StdString, StdString, StdString, StdString);

	virtual bool Load();
	virtual void Unload();
	virtual bool LoadInfo(json & info);
	virtual long GetInfo(LPCTSTR lpValueName, PDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
	virtual bool GetClassObject(REFCLSID rclsid, DWORD dwClsContext, COSERVERINFO *pServerInfo, REFIID riid, LPVOID *ppv);
};

