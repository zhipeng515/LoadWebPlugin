#pragma once

#include <unordered_map>
#include <ObjBase.h>
#include "../../CommonFunction/Utility.h"
#include "../../json/src/json.hpp"

using json = nlohmann::json;

class WebPlugin
{
public:
	WebPlugin(StdString name, StdString clsid, StdString progid, StdString filename) {
		strClsID = clsid;
		strProgID = progid;
		strFileName = filename;
		strName = name;
		::CLSIDFromString(strClsID.c_str(), &clsID);
		hKey = (HKEY)(0x8765 + rand() % 0xFFFF);
	}
	virtual bool Load() = 0;
	virtual void Unload() = 0;
	virtual bool LoadInfo(json & info) = 0;
	virtual long GetInfo(LPCTSTR lpValueName, PDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) = 0;
	virtual bool GetClassObject(REFCLSID rclsid, DWORD dwClsContext, COSERVERINFO *pServerInfo, REFIID riid, LPVOID *ppv) = 0;

	StdString	strClsID;
	StdString	strProgID;
	StdString	strFileName;
	CLSID		clsID;

	StdString	strName;
	HKEY		hKey;

	std::unordered_map<StdString, StdString> infoMap;
};

static void MacroReplace(StdString & input)
{
	Utility::replaceString(input, _T("${EXE_PATH}"), Utility::GetExePath());
}