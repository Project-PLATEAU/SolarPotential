#include "pch.h"
#include "CSunVector.h"

CSunVector::CSunVector(double lat, double lon, const CTime& date)
	:m_lat(lat), m_lon(lon), m_date(date)
{
	// 24時間のデータ
	m_position.resize(24);

	// 毎時の高度、方位の算出
	CalcHorizontalCoordinates();
}

// ベクトル取得
bool CSunVector::GetVector(uint8_t hour, CVector3D& vec) const
{
	// 高度・方位を取得
	HorizontalCoordinate pos;
	if (!GetPos(hour, pos))
		return false;

	// 高度と方位のベクトル計算
	// 方位は真南が0, xプラス方向は東, yプラス方向は北
	vec.x = cos(-pos.azimuth - _PI * 0.5);
	vec.y = sin(-pos.azimuth - _PI * 0.5);
	vec.z = sin(pos.altitude);
	// 高度と方位の位置からの光の方向のため、ベクトルは逆向き
	vec *= -1;

	return true;
}

// 高度・方位を取得
bool CSunVector::GetPos(uint8_t hour, HorizontalCoordinate& pos) const
{
	if (hour >= 24)
		return false;

	pos = m_position[hour];

	return true;
}

// 高度・方位の24時間データを取得
std::vector<HorizontalCoordinate>& CSunVector::GetPosAry()
{
	return m_position;
}

// 太陽の0時～23時の1時間単位の位置(高度・方位）を算出
void CSunVector::CalcHorizontalCoordinates()
{
	// 太陽光ベクトル算出用
	constexpr double def_SunVecA[] = { 0.006918, 0.399912, 0.070257, 0.006758, 0.000907, 0.002697, 0.001480 };
	constexpr double def_SunVecB[] = { 0.000075, 0.001868, 0.032077, 0.014615, 0.040849 };

	// ラジアンに変換
	double dLat = m_lat * _COEF_DEG_TO_RAD;
	double dLon = m_lon * _COEF_DEG_TO_RAD;

	double dTheta = 2 * _PI * ((int64_t)m_date.iYDayCnt - 1) / 365.0;

	// 太陽赤緯
	double dSekii = def_SunVecA[0] - def_SunVecA[1] * cos(dTheta) + def_SunVecA[2] * sin(dTheta)
		- def_SunVecA[3] * cos(2 * dTheta) + def_SunVecA[4] * sin(2 * dTheta)
		- def_SunVecA[5] * cos(3 * dTheta) + def_SunVecA[6] * sin(3 * dTheta);
	// 均時差
	double dEq = def_SunVecB[0] + def_SunVecB[1] * cos(dTheta)
		- def_SunVecB[2] * sin(dTheta)
		- def_SunVecB[3] * cos(2 * dTheta)
		- def_SunVecB[4] * sin(2 * dTheta);

	// 毎時の計算
	for (short hour = 0; hour < 24; ++hour)
	{
		// 時角[ラジアン]
		int diffHour = hour - 12;
		double dH = diffHour * _PI / 12 + (m_lon - 135) * _COEF_DEG_TO_RAD + dEq;

		// 高度[ラジアン]
		double dAlpha = asin(sin(dLat) * sin(dSekii) + cos(dLat) * cos(dSekii) * cos(dH));

		// 太陽方位[ラジアン]
		double dPsi = atan2(cos(dLat) * cos(dSekii) * sin(dH), sin(dLat) * sin(dAlpha) - sin(dSekii));

		m_position[hour].altitude = dAlpha;	// 高度
		m_position[hour].azimuth = dPsi;	// 方位
	}
}
