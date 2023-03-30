#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

/*!
@brief	比較関数ライブラリ
@note	このクラスではfloat,doubleをはじめ、
<br>	各種変数の最大値や最小値を定義し、それらの比較関数を提供する。
<br>	各種プロジェクトではここで提供される変数、
<br>	関数を使って比較演算を行うこと。
<br>	すべてinlineで定義するため対規模計算が必要な処理はここでは書かない
*/
class  CEpsUtil
{
public:
	static bool Zero(const double& dValue);
	static bool Equal(const double& dSrc, const double& dDest);
	static bool Less(const double& dSrc, const double& dDest);
	static bool LessEqual(const double& dSrc, const double& dDest);
	static bool Greater(const double& dSrc, const double& dDest);
	static bool GreaterEqual(const double& dSrc, const double& dDest);

	// FLOAT
	const static float _FLTMAX;		//!< floatの最大値
	const static float _FLTMIN;		//!< floatの最小値
	const static float _FLTEPS;		//!< floatで表現可能な絶対値の最小値

	static bool Zero(const float& dValue);
	static bool Equal(const float& fSrc, const float& fDest);
	static bool Less(const float& fSrc, const float& fDest);
	static bool LessEqual(const float& fSrc, const float& fDest);
	static bool Greater(const float& fSrc, const float& fDest);
	static bool GreaterEqual(const float& fSrc, const float& fDest);

	// 乱数取得
	//   最小値、最大値を与えその範囲内の乱数を戻す
	static int    Rand(int minValue, int maxValue);
	static double Rand(const double& minValue, const double& maxValue);

	// Log関数
	static double LogBase(const double& a, const double& base);

	// 最大・最小
	static double Min(const double& a, const double& b);
	static double Max(const double& a, const double& b);

};

/*! 0値判断
@retval	true	0
@retval	false	0以外
@note	doubleの値が0かを判断します。
*/
inline bool CEpsUtil::Zero(const double& dValue	//!< in	評価する値
)
{
	return(fabs(dValue) < DBL_EPSILON);
}

/*! <判断
@retval	true	dSrc <  dDest
@retval	false	dSrc >= dDest
@note	2つのdouble値を比較します。
*/
inline bool CEpsUtil::Less(const double& dSrc,	//!< in	比較先
	const double& dDest	//!< in	比較先
)
{
	return ((dSrc < dDest) && !Equal(dSrc, dDest));
}

/*! <=判断
@retval	true	dSrc <= dDest
@retval	false	dSrc >  dDest
@note	2つのdouble値を比較します。
*/
inline bool CEpsUtil::LessEqual(const double& dSrc,	//!< in	比較元
	const double& dDest	//!< in	比較先
)
{
	return ((dSrc < dDest) || Equal(dSrc, dDest));
}

/*! >判断
@retval	true	dSrc >  dDest
@retval	false	dSrc <= dDest
@note	2つのdouble値を比較します。
*/
inline bool CEpsUtil::Greater(const double& dSrc,	//!< in	比較元
	const double& dDest	//!< in	比較先
)
{
	return (!LessEqual(dSrc, dDest));
}

/*! >=判断
@retval	true	dSrc >= dDest
@retval	false	dSrc <  dDest
@note	2つのdouble値を比較します。
*/
inline bool CEpsUtil::GreaterEqual(const double& dSrc,	//!< in	比較元
	const double& dDest	//!< in	比較先
)
{
	return (!Less(dSrc, dDest));
}


/*! =判断
@retval	true	dSrc == dDest
@retval	false	dSrc != dDest
@note	2つのdouble値を比較します。
*/
inline bool CEpsUtil::Equal(const double& dSrc,	//!< in	比較元
	const double& dDest)	//!< in	比較先
{
	if (Zero(dSrc)) {
		if (Zero(dDest)) return(true);
		else  	     return(false);
	}
	else {
		double d = DBL_EPSILON * dSrc;
		if (_isnan(d)) return false;
		return (fabs(dSrc - dDest) < fabs(DBL_EPSILON));
	}
}

/*! 0値判断
@retval	true	0
@retval	false	0以外
@note	floatの値が0かを判断します。
*/
inline bool CEpsUtil::Zero(const float& fValue	//!< in	評価する値
)
{
	return(fabs((double)fValue) < _FLTEPS);
}

/*! <判断
@retval	true	fSrc <  fDest
@retval	false	fSrc >= fDest
@note	2つのfloat値を比較します。
*/
inline bool CEpsUtil::Less(const float& fSrc,		//!< in	比較元
	const float& fDest		//!< in	比較先
)
{
	return ((fSrc < fDest) && !Equal(fSrc, fDest));
}

/*! <=判断
@retval	true	fSrc <= fDest
@retval	false	fSrc >  fDest
@note	2つのfloat値を比較します。
*/
inline bool CEpsUtil::LessEqual(const float& fSrc,		//!< in	比較元
	const float& fDest		//!< in	比較先
)
{
	return ((fSrc < fDest) || Equal(fSrc, fDest));
}

/*! >判断
@retval	true	fSrc >  fDest
@retval	false	fSrc <= fDest
@note	2つのfloat値を比較します。
*/
inline bool CEpsUtil::Greater(const float& fSrc,		//!< in	比較元
	const float& fDest		//!< in	比較先
)
{
	return (!LessEqual(fSrc, fDest));
}

/*! >=判断
@retval	true	fSrc >= fDest
@retval	false	fSrc <  fDest
@note	2つのfloat値を比較します。
*/
inline bool CEpsUtil::GreaterEqual(const float& fSrc,	//!< in	比較元
	const float& fDest	//!< in	比較先
)
{
	return (!Less(fSrc, fDest));
}

/*! =判断
@retval	true	fSrc == fDest
@retval	false	fSrc != fDest
@note	2つのfloat値を比較します。
*/
inline bool CEpsUtil::Equal(const float& fSrc,		//!< in	比較元
	const float& fDest		//!< in	比較先
)
{
	if (Zero(fSrc)) {
		if (Zero(fDest)) return(true);
		else  	     return(false);
	}
	else {
		double d = _FLTEPS * (double)fSrc;
		if (_isnan(d)) return false;
		return (fabs((double)fSrc - (double)fDest) < fabs(d));
	}
}

/*! 乱数取得
@return	取得した乱数
@note	最小値、最大値を与えその範囲内の乱数を戻す
*/
inline int CEpsUtil::Rand(int minValue,	//!< in	最小値
	int maxValue	//!< in	最大値
)
{
	int    number = rand();

	if (number == 0)        return minValue;
	if (number == RAND_MAX) return maxValue;

	double d = (double)number / (double)RAND_MAX; // 0.0～1.0の平滑化
	return (int)(min(minValue, maxValue) + d * abs(maxValue - minValue));
}

/*! 乱数取得
@return	取得した乱数
@note	最小値、最大値を与えその範囲内の乱数を戻す
*/
inline double CEpsUtil::Rand(const double& minValue,	//!< in	最小値
	const double& maxValue		//!< in	最大値
)
{
	int    number = rand();

	if (number == 0)
		return minValue;
	if (number == RAND_MAX)
		return maxValue;

	double d = (double)number / (double)RAND_MAX; // 0.0～1.0の平滑化
	return (double)(min(minValue, maxValue) + d * fabs(maxValue - minValue));
}

/*! 数学的LOG計算
@return 計算値
*/
inline double CEpsUtil::LogBase(const double& a, const double& base)
{
	if (Zero(a) || Zero(base)) return - DBL_MAX;

	return log(a) / log(base);

}

/*! 最小
@return 最小を返す
*/
inline double CEpsUtil::Min(const double& a, const double& b)
{
	return Less(a, b) ? a : b;
}

/*! 最大
@return 最大を返す
*/
inline double CEpsUtil::Max(const double& a, const double& b)
{
	return Greater(a, b) ? a : b;
}
