#pragma once
#include "Analyzer.h"
#include "../../LIB/CommonUtil/StringEx.h"
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <locale.h>

/*!
@brief	入力データ取り込みクラス
*/
class CDataImport
{

public:
	CDataImport(void);
	~CDataImport(void);

public:
	void SetReadFilePath(std::string path)
	{
		setlocale(LC_ALL, "");
		m_strFilePath = CStringEx::ToWString(path);
	};
	virtual bool ReadData() { return false; };


protected:
	std::wstring		m_strFilePath;		// 入力ファイルパス


};
