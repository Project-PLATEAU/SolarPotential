#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <list>

#include "StringEx.h"


class CFileUtil
{
private:
	static CFileUtil m_instance;

	CFileUtil(void);
	virtual ~CFileUtil(void);

public:
	// インスタンス取得
	static CFileUtil* GetInstance() { return &m_instance; }

	static void CreateFileList(const std::string& strFolderPath, const std::string& strName, std::list<std::string>* pLstFilePath);
	static void CreateFileList(const std::wstring& strFolderPath, const std::wstring& strName, std::list<std::wstring>* pLstFilePath);
	static std::string ChangeFileNameExt(const std::string& strFileName, const std::string& strExtName);
	static std::wstring ChangeFileNameExt(const std::wstring& strFileName, const std::wstring& strExtName);
	static bool IsExistPath(const std::string& strPat);
	static bool IsExistPath(const std::wstring& strPat);
	static bool SplitPath(const std::string& strPath, std::string* pstrDrive = NULL, std::string* pstrDir = NULL, std::string* pstrName = NULL, std::string* pstrExt = NULL);
	static bool SplitPath(const std::wstring& strPath, std::wstring* pstrDrive = NULL, std::wstring* pstrDir = NULL, std::wstring* pstrName = NULL, std::wstring* pstrExt = NULL);
	static std::string Combine(const std::string& strPath1, const std::string& strPath2);
	static std::wstring Combine(const std::wstring& strPath1, const std::wstring& strPath2);
	static bool RemoveMultiFolder(const std::string& strTempPath);
	static bool RemoveMultiFolder(const std::wstring& strTempPath);
	static std::string	GetModulePath(const HINSTANCE hModule = NULL);
	static std::wstring	GetModulePathW(const HINSTANCE hModule = NULL);

	static void SplitCSVData(const std::string& strRecord, std::vector<std::string>* paryData, char cSeparator = ',');
	static void SplitCSVData(const std::wstring& strRecord, std::vector<std::wstring>* paryData, wchar_t cSeparator = ',');

	std::wstring GetParentDir(const std::wstring& strTargetPath) const;

};

#define	GetFUtil()	CFileUtil::GetInstance()
