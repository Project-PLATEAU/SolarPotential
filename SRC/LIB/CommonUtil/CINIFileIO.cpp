#include "pch.h"
#include "CINIFileIO.h"
#include "CFileUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*! コンストラクタ
*/
CINIFileIO::CINIFileIO(void)
{
}

/*! デストラクタ
*/
CINIFileIO::~CINIFileIO(void)
{
	Close();
}

/*! Open
@retval	true	成功
@retval	false	失敗
*/
bool CINIFileIO::Open(const std::string& strFilePath	//!< in	ファイルパス
)
{
	bool bOpen = true;
	m_strLocalFilePath = strFilePath;

	return bOpen;
}

/*! Close
*/
void CINIFileIO::Close()
{
	m_strLocalFilePath = "";
}


/*! 整数値取得
@return	取得した値
@note	INIファイルから整数を取得します。
@note	Openしていない場合はデフォルト値が返ります。
*/
UINT CINIFileIO::GetInt(const std::string& strAppName,		//!< in	セクション名
	const std::string& strKeyName,		//!< in	キー名
	INT				iDefault		//!< in	デフォルト値
) const
{
	UINT uRet = iDefault;
	if (!m_strLocalFilePath.empty())
	{
		uRet = ::GetPrivateProfileIntA(strAppName.c_str(), strKeyName.c_str(),
			iDefault,
			m_strLocalFilePath.c_str());
	}
	return uRet;
}

/*! 文字列取得
@return	取得した値
@note	INIファイルから文字列を取得します。
@note	Openしていない場合はデフォルト値が返ります。
*/
std::string CINIFileIO::GetString(const std::string& strAppName,	//!< in	セクション名
	const std::string& strKeyName,	//!< in	キー名
	const std::string& strDefault	//!< in	デフォルト値
) const
{
	std::string strRet = strDefault;

	if (!m_strLocalFilePath.empty())
	{
		int iBuffSize = 256;	// バッファはひとまず256バイトで・・・
		char* pBuff = (char*)malloc(iBuffSize);

		// INIファイルから文字列を取得
		// 取得バッファが足りない場合はバッファサイズ-1が返るので、その場合はバッファを2倍にしてリトライする
		while ((iBuffSize - 1) == ::GetPrivateProfileStringA(strAppName.c_str(),
			strKeyName.c_str(),
			strDefault.c_str(),
			pBuff, iBuffSize,
			m_strLocalFilePath.c_str()))
		{
			free(pBuff);
			iBuffSize *= 2;
			pBuff = (char*)malloc(iBuffSize);
		}

		strRet = pBuff;
		free(pBuff);
	}

	return strRet;
}

/*! 実数値取得
@return	取得した値
@note	INIファイルから実数を取得します。
@note	Openしていない場合はデフォルト値が返ります。
*/
double CINIFileIO::GetDouble(const std::string& strAppName,	//!< in	セクション名
	const std::string& strKeyName,	//!< in	キー名
	const double& dDefault	//!< in	デフォルト値
) const
{
	double dRet = dDefault;
	if (!m_strLocalFilePath.empty())
	{
		std::string strDefault =CStringEx::Format("%f", dDefault);
		dRet = stof(GetString(strAppName, strKeyName, strDefault));
	}
	return dRet;
}
