#include "pch.h"
#include "CFileUtil.h"
#include "StringEx.h"
#include <iostream>
#include <filesystem>
#include <regex>
#include <cassert>

// インスタンス定義
CFileUtil	CFileUtil::m_instance;

/*! コンストラクタ
*/
CFileUtil::CFileUtil(void)
{
}
/*! デストラクタ
*/
CFileUtil::~CFileUtil(void)
{
}

void CFileUtil::CreateFileList(
	const std::string& strFolderPath,		//!< in		親フォルダパス
	const std::string& strName,				//!< in		ファイル検索の条件
	std::list<std::string>* pLstFilePath			//!< out	ファイルパスのリスト
)
{
	for (const auto& file : std::filesystem::directory_iterator(strFolderPath))
	{
		if (file.is_directory())
		{
			// 何もしない
		}
		// ファイル
		else
		{
			// リストに追加
			pLstFilePath->push_back(file.path().string());
		}
	}

}

void CFileUtil::CreateFileList(
	const std::wstring& strFolderPath,		//!< in		親フォルダパス
	const std::wstring& strName,				//!< in		ファイル検索の条件
	std::list<std::wstring>* pLstFilePath			//!< out	ファイルパスのリスト
)
{
	for (const auto& file : std::filesystem::directory_iterator(strFolderPath))
	{
		if (file.is_directory())
		{
			// 何もしない
		}
		// ファイル
		else
		{
			// リストに追加
			pLstFilePath->push_back(file.path().wstring());
		}
	}

}

/* ファイル名の拡張子を変更する
*/
std::string CFileUtil::ChangeFileNameExt(const std::string& strFileName,	//!< in ファイル名
	const std::string& strExtName		//!< in 拡張子
)
{
	//exeファイルのパスを分解
	char path[_MAX_PATH] = { '\0' };
	char drive[_MAX_DRIVE] = { '\0' };
	char dir[_MAX_DIR] = { '\0' };
	char fname[_MAX_FNAME] = { '\0' };
	char ext[_MAX_EXT] = { '\0' };
	_splitpath_s(strFileName.c_str(), drive, dir, fname, ext);
	_makepath_s(path, drive, dir, fname, strExtName.c_str());
	return path;
}


/* ファイル名の拡張子を変更する
*/
std::wstring CFileUtil::ChangeFileNameExt(const std::wstring& strFileName,	//!< in ファイル名
	const std::wstring& strExtName		//!< in 拡張子
)
{
	//exeファイルのパスを分解
	WCHAR path[_MAX_PATH] = { '\0' };
	WCHAR drive[_MAX_DRIVE] = { '\0' };
	WCHAR dir[_MAX_DIR] = { '\0' };
	WCHAR fname[_MAX_FNAME] = { '\0' };
	WCHAR ext[_MAX_EXT] = { '\0' };
	_wsplitpath_s(strFileName.c_str(), drive, dir, fname, ext);
	_wmakepath_s(path, drive, dir, fname, strExtName.c_str());
	return path;
}

/*! パス(ファイル、ディレクトリ)の存在チェック
@retval	true	存在する
@retval	false	存在しない
@note	ワイルドカード文字が含まれるファイルパスは対応しない。
*/
bool CFileUtil::IsExistPath(const std::string& strPath	//!< in	パス
)
{
	bool bExist = false;

	const int TEXT_LENGTH = (int)strPath.length();
	if (TEXT_LENGTH > 0)
	{
		// 末尾の'\'、もしくは'/'はカットする。
		std::string strTarget = strPath;
		if (strPath.back() == '\\' || strPath.back() == '/')
		{
			strTarget.pop_back();
		}

		if(std::filesystem::exists(strTarget))
		{
			bExist = true;
		}
	}

	return bExist;
}

/*! パス(ファイル、ディレクトリ)の存在チェック
@retval	true	存在する
@retval	false	存在しない
@note	ワイルドカード文字が含まれるファイルパスは対応しない。
*/
bool CFileUtil::IsExistPath(const std::wstring& strPath	//!< in	パス
)
{
	bool bExist = false;

	const int TEXT_LENGTH = (int)strPath.length();
	if (TEXT_LENGTH > 0)
	{
		// 末尾の'\'、もしくは'/'はカットする。
		std::wstring strTarget = strPath;
		if (strPath.back() == '\\' || strPath.back() == '/')
		{
			strTarget.pop_back();
		}

		if (std::filesystem::exists(strTarget))
		{
			bExist = true;
		}
	}

	return bExist;
}

/*! ファイルパス分解
@retval	true	成功
@retval	false	失敗
@note	ファイルパスを分解します。
@note	出力引数のpstrDrive,pstrDir,pstrName,pstrExtのうち、不要なものはNULLをセット可能です。
*/
bool CFileUtil::SplitPath(const std::string& strPath,				//!< in		パス
	std::string* pstrDrive		/*= NULL*/,	//!< out	ドライブ
	std::string* pstrDir		/*= NULL*/,	//!< out	ディレクトリ
	std::string* pstrName		/*= NULL*/,	//!< out	名前
	std::string* pstrExt		/*= NULL*/	//!< out	拡張子
)
{
	bool bRet = false;

	char	drive[_MAX_DRIVE],
		dir[_MAX_DIR],
		name[_MAX_FNAME],
		ext[_MAX_EXT];
	if (_splitpath_s(strPath.c_str(), drive, dir, name, ext) == 0)
	{
		if (pstrDrive != NULL)
		{
			*pstrDrive = drive;
		}
		if (pstrDir != NULL)
		{
			*pstrDir = dir;
		}
		if (pstrName != NULL)
		{
			*pstrName = name;
		}
		if (pstrExt != NULL)
		{
			*pstrExt = ext;
		}
		bRet = true;
	}

	return bRet;
}


/*! ファイルパス分解
@retval	true	成功
@retval	false	失敗
@note	ファイルパスを分解します。
@note	出力引数のpstrDrive,pstrDir,pstrName,pstrExtのうち、不要なものはNULLをセット可能です。
*/
bool CFileUtil::SplitPath(const std::wstring& strPath,				//!< in		パス
	std::wstring* pstrDrive		/*= NULL*/,	//!< out	ドライブ
	std::wstring* pstrDir		/*= NULL*/,	//!< out	ディレクトリ
	std::wstring* pstrName		/*= NULL*/,	//!< out	名前
	std::wstring* pstrExt		/*= NULL*/	//!< out	拡張子
)
{
	bool bRet = false;

	WCHAR	drive[_MAX_DRIVE],
		dir[_MAX_DIR],
		name[_MAX_FNAME],
		ext[_MAX_EXT];
	if (_wsplitpath_s(strPath.c_str(), drive, dir, name, ext) == 0)
	{
		if (pstrDrive != NULL)
		{
			*pstrDrive = drive;
		}
		if (pstrDir != NULL)
		{
			*pstrDir = dir;
		}
		if (pstrName != NULL)
		{
			*pstrName = name;
		}
		if (pstrExt != NULL)
		{
			*pstrExt = ext;
		}
		bRet = true;
	}

	return bRet;
}

/*!
@brief	パスの連結
@return	連結したパス
*/
std::string CFileUtil::Combine(
	const std::string& strPath1,		//!< [in]	パス1
	const std::string& strPath2		//!< [in]	パス2
)
{
	LPCSTR pPath2Header = strPath2.c_str();
	
	if (strPath2.front() == '\\' || *pPath2Header == '/')
	{
		// パス2の先頭が\の場合はエラー
		return "";
	}
	// パスを連結
	char	szPath[MAX_PATH] = {'\0'};
	errno_t _err = _makepath_s(szPath, NULL, strPath1.c_str(), strPath2.c_str(), NULL);
	return szPath;
}

/*!
@brief	パスの連結
@return	連結したパス
*/
std::wstring CFileUtil::Combine(
	const std::wstring& strPath1,		//!< [in]	パス1
	const std::wstring& strPath2		//!< [in]	パス2
)
{
	LPCWSTR pPath2Header = strPath2.c_str();

	if (strPath2.front() == '\\' || *pPath2Header == '/')
	{
		// パス2の先頭が\の場合はエラー
		return L"";
	}
	// パスを連結
	WCHAR	szPath[MAX_PATH] = { '\0' };
	errno_t _err = _wmakepath_s(szPath, NULL, strPath1.c_str(), strPath2.c_str(), NULL);
	return szPath;
}

/*! フォルダ削除
*/
bool CFileUtil::RemoveMultiFolder(const std::string& strFolderPath	//!< in フォルダ名
)
{
	if (strFolderPath.length() == 0) {
		return true;
	}

	bool		bReturn = true;
	std::string		strFolderName(strFolderPath);

	// 最後に '\'or'/' を付加
	if (strFolderName.back() != '\\' && strFolderName.back() != '/')
		strFolderName.push_back('\\');

	// 再帰的に削除
	std::filesystem::remove_all(strFolderName);

	// ディレクトリの削除
	bReturn = std::filesystem::remove(strFolderName);

	return bReturn;
}

/*! フォルダ削除
*/
bool CFileUtil::RemoveMultiFolder(const std::wstring& strFolderPath	//!< in フォルダ名
)
{
	if (strFolderPath.length() == 0) {
		return true;
	}

	bool		bReturn = true;
	std::wstring		strFolderName(strFolderPath);

	// 最後に '\'or'/' を付加
	if (strFolderName.back() != '\\' && strFolderName.back() != '/')
		strFolderName.push_back('\\');

	// 再帰的に削除
	std::filesystem::remove_all(strFolderName);

	// ディレクトリの削除
	bReturn = std::filesystem::remove(strFolderName);

	return bReturn;
}

/*! CSVデータ分割
@note	strDataをカンマ毎に分割します。
<br>	strDataに','が１つも無い場合、paryDataにはstrDataを1件追加します。
@note	""で括られたデータは1フィールドとして扱います。
<br>	""で括られたフィールドは""を除去した状態でデータに格納されます。
*/
void CFileUtil::SplitCSVData(const std::string& strRecord,		//!< in		文字列
	std::vector<std::string>* paryData,					//!< out	分割したデータ
	char			cSeparator /*= ','*/		//!< in		区切り文字
)
{
	assert(paryData != NULL);
	paryData->clear();

	if (!strRecord.empty())
	{
		std::string strData = strRecord;
		// 改行コードが含まれる場合は除去する
		strData = regex_replace(strData, std::regex("\r"), "\0");
		strData = regex_replace(strData, std::regex("\n"), "\0");

		if (!strData.empty())
		{
			bool bDblQuotation = false;						// "始まりの文字列フラグ
			std::string strEndField = "\"" + cSeparator;	// 文字列の終端

			// "始まりだった場合とそれ以外で文字列レコードの扱いが変わる
			bDblQuotation = ('\"' == strData.front()) ? true : false;
			std::string strEndFieldSeparator = (bDblQuotation) ? strEndField : std::string(1, cSeparator);

			std::string strField;
			int iLastFind = 0;
			int iFindIndex = (int)strData.find(strEndFieldSeparator);
			if (iFindIndex >= 0)
			{
				while (iFindIndex >= 0)
				{
					if (bDblQuotation)
					{
						strField = strData.substr((int64_t)iLastFind + 1, (((int64_t)iFindIndex - iLastFind) - 1));
					}
					else
					{
						strField = strData.substr(iLastFind, ((int64_t)iFindIndex - iLastFind));
					}
					paryData->push_back(strField);
					iLastFind = iFindIndex + (int)strEndFieldSeparator.length();

					// "始まりだった場合とそれ以外で文字列レコードの扱いが変わる
					bDblQuotation = ("\"" == strData.substr(iLastFind, 1)) ? true : false;
					strEndFieldSeparator = (bDblQuotation) ? strEndField : std::string(1, cSeparator);

					iFindIndex = (int)strData.find(strEndFieldSeparator, iLastFind);
				}
				strField = strData.substr(iLastFind);
			}
			else
			{
				strField = strData;
			}
			// 2文字以上ある場合は""で括られている可能性がある
			if (strField.length() >= 2)
			{
				// 文字列が""で括られている場合は除去する
				std::string str1 = { strField.front() },
						str2 = strField.substr(strField.length() - 2, 1);
				if (strField.front() == '\"' && strField.substr(strField.length() - 2, 1) == "\"")
				{
					strField = strField.substr(1, strField.length() - 3);
				}
			}
			paryData->push_back(strField);
		}
		else
		{
			// データが空でも、空の1件を追加する
			paryData->push_back(strData);
		}
	}
	else
	{
		// データが空でも、空の1件を追加する
		paryData->push_back(strRecord);
	}
}

/*! CSVデータ分割
@note	strDataをカンマ毎に分割します。
<br>	strDataに','が１つも無い場合、paryDataにはstrDataを1件追加します。
@note	""で括られたデータは1フィールドとして扱います。
<br>	""で括られたフィールドは""を除去した状態でデータに格納されます。
*/
void CFileUtil::SplitCSVData(const std::wstring& strRecord,		//!< in		文字列
	std::vector<std::wstring>* paryData,					//!< out	分割したデータ
	wchar_t			cSeparator /*= ','*/		//!< in		区切り文字
)
{
	assert(paryData != NULL);
	paryData->clear();

	if (!strRecord.empty())
	{
		std::wstring strData = strRecord;
		// 改行コードが含まれる場合は除去する
		strData = regex_replace(strData, std::wregex(L"\r"), L"\0");
		strData = regex_replace(strData, std::wregex(L"\n"), L"\0");

		if (!strData.empty())
		{
			bool bDblQuotation = false;						// "始まりの文字列フラグ
			std::wstring strEndField = L"\"" + cSeparator;	// 文字列の終端

			// "始まりだった場合とそれ以外で文字列レコードの扱いが変わる
			bDblQuotation = ('\"' == strData.front()) ? true : false;
			std::wstring strEndFieldSeparator = (bDblQuotation) ? strEndField : std::wstring(1, cSeparator);

			std::wstring strField;
			int iLastFind = 0;
			int iFindIndex = (int)strData.find(strEndFieldSeparator);
			if (iFindIndex >= 0)
			{
				while (iFindIndex >= 0)
				{
					if (bDblQuotation)
					{
						strField = strData.substr((int64_t)iLastFind + 1, (((int64_t)iFindIndex - iLastFind) - 1));
					}
					else
					{
						strField = strData.substr(iLastFind, ((int64_t)iFindIndex - iLastFind));
					}
					paryData->push_back(strField);
					iLastFind = iFindIndex + (int)strEndFieldSeparator.length();

					// "始まりだった場合とそれ以外で文字列レコードの扱いが変わる
					bDblQuotation = (L"\"" == strData.substr(iLastFind, 1)) ? true : false;
					strEndFieldSeparator = (bDblQuotation) ? strEndField : std::wstring(1, cSeparator);

					iFindIndex = (int)strData.find(strEndFieldSeparator, iLastFind);
				}
				strField = strData.substr(iLastFind);
			}
			else
			{
				strField = strData;
			}
			// 2文字以上ある場合は""で括られている可能性がある
			if (strField.length() >= 2)
			{
				// 文字列が""で括られている場合は除去する
				std::wstring str1 = { strField.front() },
					str2 = strField.substr(strField.length() - 2, 1);
				if (strField.front() == '\"' && strField.substr(strField.length() - 2, 1) == L"\"")
				{
					strField = strField.substr(1, strField.length() - 3);
				}
			}
			paryData->push_back(strField);
		}
		else
		{
			// データが空でも、空の1件を追加する
			paryData->push_back(strData);
		}
	}
	else
	{
		// データが空でも、空の1件を追加する
		paryData->push_back(strRecord);
	}
}

/*! モジュールパス取得関数
@return モジュールパス
*/
std::string CFileUtil::GetModulePath(
	const HINSTANCE hModule /*= NULL*/	//!< in	取得対象モジュールハンドル
)
{
	std::string	szValue;

	char szModuleFileName[MAX_PATH];

	GetModuleFileNameA(hModule, szModuleFileName, MAX_PATH);
	std::string strModulePath = szModuleFileName;
	strModulePath = strModulePath.substr(0, strModulePath.rfind('\\') + 1);

	szValue = strModulePath;
	return szValue;
}

/*! モジュールパス取得関数
@return モジュールパス
*/
std::wstring CFileUtil::GetModulePathW(
	const HINSTANCE hModule /*= NULL*/	//!< in	取得対象モジュールハンドル
)
{
	std::wstring	szValue;

	WCHAR szModuleFileName[MAX_PATH];

	GetModuleFileNameW(hModule, szModuleFileName, MAX_PATH);
	std::wstring strModulePath = szModuleFileName;
	strModulePath = strModulePath.substr(0, strModulePath.rfind('\\') + 1);

	szValue = strModulePath;
	return szValue;
}


/*! 親ディレクトリ取得
@return	親ディレクトリのパス
@note	親ディレクトリを取得したいパスを渡して親ディレクトリを取得します。
*/
std::wstring CFileUtil::GetParentDir(const std::wstring& strTargetPath		//!< in	取得したい対象のパス
) const
{
	// ディレクトリの場合、末尾が'\\'や'/'だと親ディレクトリが取れないので末尾を削除する
	std::wstring strTarget = (strTargetPath.back() == '\\' || strTargetPath.back() == '/')
		? strTargetPath.substr(0, (strTargetPath.length() - 1)) : strTargetPath,
		strParent, strDrive, strDir;

	// パスの分解
	if (SplitPath(strTarget, &strDrive, &strDir))
	{
		strParent = CStringEx::Format(L"%s%s", strDrive.c_str(), strDir.c_str());
	}

	return strParent;
}