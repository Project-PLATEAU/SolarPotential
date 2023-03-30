#include "pch.h"
#include "CFileIO.h"
#include <iostream>
#include <filesystem>
#include <cassert>

/*! コンストラクタ
*/
CFileIO::CFileIO(void)
{
	// 初期化
	m_pFp = NULL;
	m_strFileName = L"";
	m_strOpenMode = L"";
}

/*! デストラクタ
*/
CFileIO::~CFileIO(void)
{
	this->Close();
}

/*! ファイルサイズを取得する
@retval	ファイルサイズ
*/
int CFileIO::GetFileLength()
{
	uintmax_t nRet = 0;

	nRet = std::filesystem::file_size(m_strFileName);

	return (int)nRet;
}

/*! ファイルポインタ先頭移動
@note	読み込んだファイルのポインタを先頭に移動する。
*/
void CFileIO::SeekToBegin()
{
	// ファイルポインタを先頭に移動
	if (m_pFp != NULL)
	{
		fseek(m_pFp, 0L, (int)SEEK_SET);
	}

}

/*! ファイルポインタカレント移動
@note	現在のファイルポインタ位置から指定バイト数分移動する。
*/
void CFileIO::SeekToCurrent(int nByte	//!< in 指定バイト数
)
{
	// ファイルポインタを指定位置から引数バイト分シフト
	if (m_pFp != NULL)
	{
		fseek(m_pFp, nByte, (int)SEEK_CUR);
	}
}

/*! 文字列ファイル書き込み
@note	指定文字列に改行コードを追加し、ファイルに出力する。
*/
bool CFileIO::WriteLineA(std::string strLine	//!< in 出力対象文字列
)
{
	// 改行コードを追加
	std::string str = strLine + "\n";

	fputs(str.c_str(), m_pFp);

	return true;
}

/*! 文字列ファイル書き込み
@note	指定文字列に改行コードを追加し、ファイルに出力する。
*/
bool CFileIO::WriteLineW(std::wstring strLine	//!< in 出力対象文字列
)
{
	// 改行コードを追加
	std::wstring wstr = strLine + L"\n";
	std::string str = CStringEx::ToString(wstr);

	fputs(str.c_str(), m_pFp);

	return true;
}

/*! ファイルを閉じる
*/
void CFileIO::Close(void)
{
	if (m_pFp != NULL) {
		fclose(m_pFp);
	}
	m_pFp = NULL;
	m_strFileName = L"";
	m_strOpenMode = L"";
}

/*! ファイルを開く
@retval	true	成功
@retval	false	失敗
*/
bool CFileIO::Open(
	const std::wstring& strFileName,			//!< in ファイ名(URL名)
	const std::wstring& strMode				//!< in アクセス許可の種類("r","w+"等、CRTのfopen関数のモードと同様)
)
{
	bool	bRtn = true;

	errno_t		err;

	// ファイルポインタを初期化
	this->Close();

	// ファイルを開く
	err = _wfopen_s(&m_pFp, strFileName.c_str(), strMode.c_str());

	if (err == 0) {
		m_strFileName = strFileName;
		m_strOpenMode = strMode;
	}
	else {
		bRtn = false;
	}
	if (bRtn == false) {
		m_strFileName = L"";
	}
	return	bRtn;
}

/*! 関連付けられたファイルからバッファにデータを読み出す
@return	バッファに転送されたバイト数
*/
size_t CFileIO::Read(void* pBuff,	//!< out データ取得用のユーザーが確保したバッファへのポインタ
	size_t	nSize	//!< in  バッファサイズ
)
{
	assert(pBuff != NULL);
	assert(nSize > 0);
	assert(m_pFp != NULL);

	size_t	nReadSize = 0;

	if (m_pFp != NULL) {
		nReadSize = fread(pBuff, 1, nSize, m_pFp);
	}
	return	nReadSize;
}

wchar_t* CFileIO::ReadLineW(wchar_t* pBuff,	//!< out データ取得用のユーザーが確保したバッファへのポインタ
	int			nSize)	//!< in  バッファサイズ
{
	assert(pBuff != NULL);
	assert(nSize > 0);
	assert(m_pFp != NULL);

	wchar_t* pstr = NULL;

	if (m_pFp != NULL)
	{
		memset(pBuff, '\0', nSize * sizeof(wchar_t));
		pstr = fgetws(pBuff, nSize, m_pFp);
	}
	return	pstr;
}

char* CFileIO::ReadLineA(char* pBuff,	//!< out データ取得用のユーザーが確保したバッファへのポインタ
	int			nSize)	//!< in  バッファサイズ
{
	assert(pBuff != NULL);
	assert(nSize > 0);
	assert(m_pFp != NULL);

	char* pstr = NULL;

	if (m_pFp != NULL)
	{
		memset(pBuff, '\0', nSize * sizeof(wchar_t));
		pstr = fgets(pBuff, nSize, m_pFp);
	}
	return	pstr;
}