#include "stdafx.h"

#include "LoadWebPlugin.h"
#include <fstream>
#include <atlbase.h>
#include "CommonPlugin.h"
#include "../../CommonFunction/DetoursWrapper.h"


DETOURS_FUNC_DECLARE(HRESULT, WINAPI, CoGetClassObject,
	_In_     REFCLSID     rclsid,
	_In_     DWORD        dwClsContext,
	_In_opt_ COSERVERINFO *pServerInfo,
	_In_     REFIID       riid,
	_Out_    LPVOID       *ppv)
{
	LPOLESTR strRefClsID;
	::StringFromCLSID(rclsid, &strRefClsID);
	if (WebPlugin * plugin = WebPluginManager::Instance()->GetLoadedPlugin(strRefClsID))
	{
		CoTaskMemFree(strRefClsID);
		if (plugin->GetClassObject(rclsid, dwClsContext, pServerInfo, riid, ppv))
			return S_OK;
	}
	CoTaskMemFree(strRefClsID);
	HRESULT hr = DETOURS_FUNC_CALLREAL(CoGetClassObject, rclsid, dwClsContext, pServerInfo, riid, ppv);
	return hr;
}

DETOURS_FUNC_DECLARE(HRESULT, WINAPI, CLSIDFromProgID, __in LPCOLESTR lpszProgID, __out LPCLSID lpclsid)
{
	if (!WebPluginManager::Instance()->IsPluginInstalled(lpszProgID))
	{
		if (WebPlugin * plugin = WebPluginManager::Instance()->InitSupportedPlugin(lpszProgID))
		{
			*lpclsid = plugin->clsID;
			plugin->Load();
			return S_OK;
		}
	}

	return DETOURS_FUNC_CALLREAL(CLSIDFromProgID, lpszProgID, lpclsid);
}


bool LoadWebPlugin(LPCTSTR configFile)
{
	if (WebPluginManager::Instance()->Init(configFile))
	{
		DETOURS_FUNC_ATTACH(CoGetClassObject);
		DETOURS_FUNC_ATTACH(CLSIDFromProgID);
	}
	return true;
}

void UnloadWebPlugin()
{
	DETOURS_FUNC_DETACH(CoGetClassObject);
	DETOURS_FUNC_DETACH(CLSIDFromProgID);

	WebPluginManager::Instance()->UnloadPlugin();
	WebPluginManager::Instance()->Uninit();
}

bool WebPluginManager::Init()
{
	return true;
}

void WebPluginManager::Uninit()
{
	for (auto it = pluginsClsIDMap.begin(); it != pluginsClsIDMap.end(); it++)
		delete it->second;
	pluginsClsIDMap.clear();
	pluginsProgIDMap.clear();
	pluginsLoadedMap.clear();
	pluginsLoadedHKeyMap.clear();
}

bool WebPluginManager::Init(LPCTSTR configFile)
{
	json config;
	std::ifstream configStream(configFile);
	if (!configStream.is_open())
		return false;

	try
	{
		config << configStream;
		for (auto it = config.begin(); it != config.end(); it++)
		{
			auto value = *it;
			StdString strName = Utility::ToStdString(it.key().c_str());
			StdString strClass = Utility::ToStdString(value["class"].get<string>().c_str());
			StdString strClsID = Utility::ToStdString(value["clsid"].get<string>().c_str());
			StdString strProgID = Utility::ToStdString(value["progid"].get<string>().c_str());
			StdString strFile = Utility::ToStdString(value["file"].get<string>().c_str());

			MacroReplace(strFile);
			if (_taccess(strFile.c_str(), 0) == -1)
				continue;

			WebPlugin * plugin = NULL;
			if(strClass == _T("common"))
				plugin = new CommonPlugin(strName, strClsID, strProgID, strFile);
			else
				_ASSERT_EXPR(false, _T("unsupport class"));

			if (plugin)
			{
				RegisterPlugin(plugin);
				plugin->LoadInfo(value["info"]);
			}
		}
	}
	catch (exception& e)
	{
		_CRT_UNUSED(e);
		_ASSERT_EXPR(false, e.what());
		return false;
	}

	return true;
}

void WebPluginManager::RegisterPlugin(WebPlugin* plugin)
{
	pluginsClsIDMap[plugin->strClsID] = plugin;
	pluginsProgIDMap[plugin->strProgID] = plugin;
}

WebPlugin * WebPluginManager::InitSupportedPlugin(LPCOLESTR progID)
{
	auto it = pluginsProgIDMap.find(progID);
	if(it == pluginsProgIDMap.end())
		return NULL;
	pluginsLoadedMap[it->second->strClsID] = it->second;
	pluginsLoadedHKeyMap[it->second->hKey] = it->second;
	return it->second;
}

bool WebPluginManager::IsPluginInstalled(LPCOLESTR progID)
{
	ATL::CRegKey reg;
	TCHAR szValue[512] = { 0 };
	TCHAR szRegPath[512] = { 0 };
	ULONG nSize = 512;
	_stprintf_s(szRegPath, _T("%s\\CLSID"), progID);
	long lRet = reg.Open(HKEY_CLASSES_ROOT, szRegPath, KEY_READ);
	if (lRet != ERROR_SUCCESS)
		return false;
	memset(szValue, 0, nSize);
	lRet = reg.QueryStringValue(_T(""), szValue, &nSize);
	if (lRet != ERROR_SUCCESS)
		return false;
	reg.Close();

	nSize = 512;
	_stprintf_s(szRegPath, _T("CLSID\\%s\\InprocServer32"), szValue);
	lRet = reg.Open(HKEY_CLASSES_ROOT, szRegPath, KEY_READ);
	if (lRet != ERROR_SUCCESS)
		return false;
	memset(szValue, 0, nSize);
	lRet = reg.QueryStringValue(_T(""), szValue, &nSize);
	if (lRet != ERROR_SUCCESS)
		return false;
	reg.Close();

	return _taccess(szValue, 0) != -1;
}

WebPlugin * WebPluginManager::GetLoadedPluginByName(LPCTSTR name) {
	for (auto it = pluginsLoadedMap.begin(); it != pluginsLoadedMap.end(); it++)
	{
		if (Utility::findStringIC(it->second->strName, name))
			return it->second;
	}
	return NULL;
}