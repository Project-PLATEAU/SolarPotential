#pragma once

#include <vector>
#include "CGeoUtil.h"
#include "CTime.h"

// 平地座標
struct HorizontalCoordinate
{
	double altitude;	// 高度[rad]

	// 方位[rad]
	// 真南を0とする
	// 東側がマイナス方向、西側がプラス方向
	double azimuth;
};


// 太陽光ベクトル
class CSunVector
{
public:
	CSunVector() = delete;
	CSunVector(double lat, double lon, const CTime& date);

	// 指定時間の太陽光のベクトル取得
	bool GetVector(uint8_t hour, CVector3D& vec) const;

	// 指定時間の高度と方位を取得
	bool GetPos(uint8_t hour, HorizontalCoordinate& pos) const;

	// 高度と方位の24時間データ取得
	std::vector<HorizontalCoordinate>& GetPosAry();
	

private:
	// 太陽の緯度経度
	double m_lat;
	double m_lon;

	// 日付
	CTime m_date;

	// 24時間の太陽の高度と方位
	std::vector<HorizontalCoordinate> m_position;


	// 緯度経度と日付から太陽の高度と方位を算出
	void CalcHorizontalCoordinates();
};
