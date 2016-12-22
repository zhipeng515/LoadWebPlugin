#include "stdafx.h"
#include "CommonPlugin.h"
#include "../../CommonFunction/DetoursWrapper.h"
#include "LoadWebPlugin.h"

DETOURS_FUNC_DECLARE(LSTATUS, WINAPI, RegQueryValueEx,
	__in HKEY hKey,
	__in_opt LPCTSTR lpValueName,
	__reserved LPDWORD lpReserved,
	__out_opt LPDWORD lpType,
	__out_bcount_part_opt(*lpcbData, *lpcbData) __out_data_source(REGISTRY) LPBYTE lpData,
	__inout_opt LPDWORD lpcbData);
DETOURS_FUNC_DECLARE(LSTATUS, WINAPI, RegCloseKey,
	__in HKEY hKey);

DETOURS_FUNC_DECLARE(LSTATUS, WINAPI, RegOpenKeyEx,
	__in HKEY hKey,
	__in_opt LPCTSTR lpSubKey,
	__in_opt DWORD ulOptions,
	__in REGSAM samDesired,
	__out PHKEY phkResult)
{
	if (WebPlugin * plugin = WebPluginManager::Instance()->GetLoadedPluginByName(lpSubKey))
	{
		*phkResult = plugin->hKey;
		return ERROR_SUCCESS;
	}
	return DETOURS_FUNC_CALLREAL(RegOpenKeyEx, hKey, lpSubKey, ulOptions, samDesired, phkResult);
}

DETOURS_FUNC_IMPLEMENT(LSTATUS, WINAPI, RegQueryValueEx,
	__in HKEY hKey,
	__in_opt LPCTSTR lpValueName,
	__reserved LPDWORD lpReserved,
	__out_opt LPDWORD lpType,
	__out_bcount_part_opt(*lpcbData, *lpcbData) __out_data_source(REGISTRY) LPBYTE lpData,
	__inout_opt LPDWORD lpcbData)
{
	if (WebPlugin * plugin = WebPluginManager::Instance()->GetLoadedPluginByHKey(hKey))
	{
		return plugin->GetInfo(lpValueName, lpType, lpData, lpcbData);
	}
	return DETOURS_FUNC_CALLREAL(RegQueryValueEx, hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

DETOURS_FUNC_IMPLEMENT(LSTATUS, WINAPI, RegCloseKey,
	__in HKEY hKey)
{
	if (WebPlugin * plugin = WebPluginManager::Instance()->GetLoadedPluginByHKey(hKey))
	{
		return ERROR_SUCCESS;
	}
	return DETOURS_FUNC_CALLREAL(RegCloseKey, hKey);
}

CommonPlugin::CommonPlugin(StdString name, StdString clsid, StdString progid, StdString filename) :
	WebPlugin(name, clsid, progid, filename)
{
}

long CommonPlugin::GetInfo(LPCTSTR lpValueName, PDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
	auto it = infoMap.find(lpValueName);
	if (it != infoMap.end())
	{
		_tcscpy_s((LPTSTR)lpData, *lpcbData, it->second.c_str());
	}
	*lpcbData = (_tcslen((LPTSTR)lpData) + 1) * sizeof(TCHAR);
	*lpType = REG_SZ;

	return ERROR_SUCCESS;
}

bool CommonPlugin::LoadInfo(json & info)
{
	if (info.is_null())
		return false;
	for (auto infoit = info.begin(); infoit != info.end(); infoit++)
	{
		StdString strKey = Utility::ToStdString(infoit.key().c_str());
		StdString strValue = Utility::ToStdString(infoit.value().get<string>().c_str());
		MacroReplace(strValue);
		infoMap[strKey] = strValue;
	}
	return true;
}


bool CommonPlugin::Load()
{
	if (infoMap.size() > 0)
	{
		DETOURS_FUNC_ATTACH(RegOpenKeyEx);
		DETOURS_FUNC_ATTACH(RegQueryValueEx);
		DETOURS_FUNC_ATTACH(RegCloseKey);
	}

	return true;
}

void CommonPlugin::Unload()
{
	DETOURS_FUNC_DETACH(RegOpenKeyEx);
	DETOURS_FUNC_DETACH(RegQueryValueEx);
	DETOURS_FUNC_DETACH(RegCloseKey);
}

bool CommonPlugin::GetClassObject(REFCLSID rclsid, DWORD dwClsContext, COSERVERINFO *pServerInfo, REFIID riid, LPVOID *ppv)
{
	HMODULE hModule = LoadLibrary(strFileName.c_str());
	if (hModule)
	{
		typedef LONG(WINAPI *FuncDefine_DllGetClassObject)(REFCLSID, REFIID, LPVOID*);
		FuncDefine_DllGetClassObject Func_DllGetClassObject = NULL;
		Func_DllGetClassObject = (FuncDefine_DllGetClassObject)GetProcAddress(hModule, "DllGetClassObject");
		if (Func_DllGetClassObject)
		{
			Func_DllGetClassObject(rclsid, riid, ppv);

			return true;
		}
	}
	return false;
}
