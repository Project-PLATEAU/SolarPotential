#pragma once
#include "pch.h"
#include "TiffData.h"


CTiffData::CTiffData()
{
	// 初期化処理
	Initialize();
}

CTiffData::CTiffData(std::vector<CPointBase>* pData)
{
	m_pData = pData;
}

CTiffData::~CTiffData()
{
}

void CTiffData::Initialize()
{
	// 初期化処理
	m_pData = new std::vector<CPointBase>;
	m_minPoint.x = m_minPoint.y = m_minPoint.z = 0.0;
	m_maxPoint.x = m_maxPoint.y = m_maxPoint.z = 0.0;
}

bool CTiffData::CalcMaxMinPoint()
{
	if (0 == m_pData->size())
	{
		m_maxPoint.x = m_maxPoint.y = m_maxPoint.z = 0.0;
		m_minPoint.x = m_minPoint.y = m_minPoint.z = 0.0;
		return false;
	}

	m_minPoint = m_pData->at(0);
	m_maxPoint = m_pData->at(0);

	// 入力点群の各座標値の最大値と最小値を計算する
	for (size_t i = 0; i < m_pData->size(); i++)
	{
		if (m_maxPoint.x < m_pData->at(i).x)
		{
			m_maxPoint.x = m_pData->at(i).x;
		}

		if (m_maxPoint.y < m_pData->at(i).y)
		{
			m_maxPoint.y = m_pData->at(i).y;
		}

		if (m_maxPoint.z < m_pData->at(i).z)
		{
			m_maxPoint.z = m_pData->at(i).z;
		}

		if (m_minPoint.x > m_pData->at(i).x)
		{
			m_minPoint.x = m_pData->at(i).x;
		}

		if (m_minPoint.y > m_pData->at(i).y)
		{
			m_minPoint.y = m_pData->at(i).y;
		}

		if (m_minPoint.z > m_pData->at(i).z)
		{
			m_minPoint.z = m_pData->at(i).z;
		}
	}

	return true;
}

void CTiffData::Analysis()
{
	if (!this->CalcMaxMinPoint())
	{
		return;
	}
}

const CPointBase CTiffData::GetPointMin()
{
	return m_minPoint;
}

const CPointBase CTiffData::GetPointMax()
{
	return m_maxPoint;
}

