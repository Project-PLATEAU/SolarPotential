#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "StringEx.h"

#pragma once
class CINIFileIO
{
public:
	CINIFileIO(void);
	virtual ~CINIFileIO(void);

	virtual bool Open(const std::string& strFilePath);
	virtual void Close();

	UINT GetInt(const std::string& strAppName, const std::string& strKeyName, INT iDefault) const;
	std::string GetString(const std::string& strAppName, const std::string& strKeyName, const std::string& strDefault) const;
	double GetDouble(const std::string& strAppName, const std::string& strKeyName, const double& dDefault) const;

private:
	std::string		m_strLocalFilePath;		//!< ローカルのファイルパス
};

