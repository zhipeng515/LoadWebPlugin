#pragma once

#include <Objbase.h>
#include <unordered_map>
#include "../../CommonFunction/Singleton.h"
#include "../../CommonFunction/Utility.h"
#include "WebPlugin.h"


bool LoadWebPlugin(LPCTSTR configFile);
void UnloadWebPlugin();

class WebPluginManager : public Singleton<WebPluginManager>
{
public:
	virtual bool Init();
	virtual void Uninit();

	bool Init(LPCTSTR configFile);

	void RegisterPlugin(WebPlugin* plugin);
	WebPlugin * InitSupportedPlugin(LPCOLESTR progID = NULL);
	bool IsPluginInstalled(LPCOLESTR progID);

	inline void UnloadPlugin() {
		for (auto it = pluginsLoadedMap.begin(); it != pluginsLoadedMap.end(); it++)
			it->second->Unload();
		pluginsLoadedMap.clear();
		pluginsLoadedHKeyMap.clear();
	}

	inline WebPlugin * GetLoadedPlugin(LPCOLESTR clsID) {
		auto it = pluginsLoadedMap.find(clsID);
		if (it == pluginsLoadedMap.end())
			return NULL;
		return it->second; 
	}
	
	WebPlugin * GetLoadedPluginByName(LPCTSTR name);

	WebPlugin * GetLoadedPluginByHKey(HKEY hKey) {
		auto it = pluginsLoadedHKeyMap.find(hKey);
		if (it == pluginsLoadedHKeyMap.end())
			return NULL;
		return it->second;
	}

private:
	std::unordered_map<StdString, WebPlugin*> pluginsClsIDMap;
	std::unordered_map<StdString, WebPlugin*> pluginsProgIDMap;
	std::unordered_map<StdString, WebPlugin*> pluginsLoadedMap;
	std::unordered_map<HKEY, WebPlugin*> pluginsLoadedHKeyMap;
};
