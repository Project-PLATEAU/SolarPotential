// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、
// または、参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"
#include <xmllite.h>

#pragma comment(lib, "xmllite.lib")

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーからほとんど使用されていない部分を除外する
// Windows ヘッダー ファイル
//#include <windows.h>
//#include <atlbase.h>


// プログラムに必要な追加ヘッダーをここで参照してください
//#define DllExport extern "C" __declspec(dllexport)

#include <Windows.h>
#include <string.h>

// エクスポートとインポートの切り替え
#ifdef ANALYZEDATA_EXPORTS
#define VC_DLL_EXPORTS extern "C" __declspec(dllexport)
#else
#define VC_DLL_EXPORTS extern "C" __declspec(dllimport)
#endif

// 解析エリア(C#受け渡し用)
typedef struct AnalyzeAreaData
{
    char strAreaId[10];
    char strAreaName[100];
    int nPointCount;
    double pPointArray[256];
    int eDirection;
    double dDegree;
    char* strExplanation;
    bool bWater;
    bool analyzeBuild;
    bool analyzeLand;
};

VC_DLL_EXPORTS void __cdecl AddAnalyzeAreaData(AnalyzeAreaData* p);
VC_DLL_EXPORTS int __cdecl AnalizeBldgFiles(const char* str, const char* strOut, const bool analyzeInterior);
VC_DLL_EXPORTS int __cdecl AnalizeDemFiles(const char* str, const char* strOut, const bool useDem);
VC_DLL_EXPORTS int __cdecl ExtractLandMesh(const bool exclusionRoad, const char* strOut);
VC_DLL_EXPORTS int __cdecl AnalizeTranFiles(const char* str, const char* strOut);
VC_DLL_EXPORTS int __cdecl LOD2DataOut(const char* str, const char* strOut);
VC_DLL_EXPORTS void __cdecl Initialize();
VC_DLL_EXPORTS void __cdecl DllDispose();
VC_DLL_EXPORTS void __cdecl OutputCoordinatesFile(const char* strOut);

VC_DLL_EXPORTS void* __cdecl GetAllAreaList();
VC_DLL_EXPORTS void* __cdecl GetAllBuildList();
VC_DLL_EXPORTS void* __cdecl GetAllDemList();