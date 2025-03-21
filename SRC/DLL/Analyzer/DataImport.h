#pragma once
#include "../../LIB/CommonUtil/StringEx.h"
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <locale.h>

/*!
@brief	���̓f�[�^��荞�݃N���X
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
	std::wstring		m_strFilePath;		// ���̓t�@�C���p�X


};
