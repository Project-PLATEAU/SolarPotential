#pragma once
#include "StringEx.h"

/*!
@brief	ファイル読込みクラス
*/
class CFileIO 
{
/* Param */
public:

/* Function */
public:
	int GetFileLength();					//!< ファイルサイズ取得
	void SeekToBegin();					//!< ファイルポインタ先頭移動
	void SeekToCurrent( int nByte );		//!< ファイルポインタカレント移動

	bool WriteLineA(std::string strLine);	//!< 文字列ファイル書き込み
	bool WriteLineW(std::wstring strLine );	//!< 文字列ファイル書き込み
	wchar_t* ReadLineW(wchar_t* pBuff, int nSize);
	char* ReadLineA(char* pBuff, int nSize);
	void Close(void);
	bool Open(const std::wstring& strFileName, const std::wstring& strMode);
	size_t Read(void* pBuff, size_t nSize);

/* Event */
public:
	CFileIO(void);
	~CFileIO(void);

private:
	std::wstring		m_strFileName;				//!< 実際に開いているファイルの名称
	std::wstring		m_strOpenMode;				//!< アクセス許可の種類
	FILE*		m_pFp;						//!< 開いたファイルのポインタ
};
