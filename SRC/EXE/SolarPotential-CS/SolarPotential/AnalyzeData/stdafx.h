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

VC_DLL_EXPORTS int __cdecl AnalizeBldgFiles(const char* str, const char* strOut);
VC_DLL_EXPORTS int __cdecl AnalizeDemFiles(const char* str, const char* strOut);
VC_DLL_EXPORTS int __cdecl LOD2DataOut(const char* str, const char* strOut);
VC_DLL_EXPORTS void __cdecl SetJPZone();

VC_DLL_EXPORTS void* __cdecl GetAllList();
VC_DLL_EXPORTS void* __cdecl GetAllDemList();