# LoadWebPlugin
免安装加载ActiveX插件

```JSON
{
    "avenger":{
        "class":"common",
        "clsid":"{1B030824-BDBC-41C8-9F7E-DE124FB14860}",
        "progid":"AVENGER.AvengerCtrl.1",
        "file":"${EXE_PATH}\\plugin\\GamePlugin\\GamePlugin.dll"
    },
    "unity":{
        "class":"common",
        "clsid":"{444785F1-DE89-4295-863A-D46C3A781394}",
        "progid":"UnityWebPlayer.UnityWebPlayer.1",
        "file":"${EXE_PATH}\\plugin\\WebPlayer\\loader\\UnityWebPluginAX.ocx",
        "info":{
            "Path":"${EXE_PATH}\\plugin\\WebPlayer\\loader\\npUnity3D32.dll",
            "Directory":"${EXE_PATH}\\plugin\\WebPlayer",
            "UnityWebPlayerReleaseChannel":"Stable",
            "UnityWebPlayerDevelopment":"no"
        }
    },
    "fancy":{
        "class":"common",
        "clsid":"{B2E8D85E-C0C5-48DF-8DBC-1359B339AE32}",
        "progid":"FANCY3DOCX.Fancy3DOCXCtrl.1",
        "file":"${EXE_PATH}\\plugin\\Fancy\\fancygame.ocx"
    }
}
```

依赖

* https://github.com/zhipeng515/CommonFunction.git
