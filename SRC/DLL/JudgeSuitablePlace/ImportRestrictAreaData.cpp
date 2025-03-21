#include "pch.h"
#include "ImportRestrictAreaData.h"
#include "../../LIB/CommonUtil/CFileIO.h"
#include "../../LIB/CommonUtil/CFileUtil.h"
#include "../../LIB/CommonUtil/ReadINIParam.h"
#include "shapefil.h"

#ifdef _DEBUG
#pragma comment(lib,"shapelib_i.lib")
#else
#pragma comment(lib,"shapelib_i.lib")
#endif

CImportRestrictAreaData::CImportRestrictAreaData(void)
	: m_strFilePath(L"")
{

}

CImportRestrictAreaData::~CImportRestrictAreaData(void)
{
	Initialize();
}

void CImportRestrictAreaData::Initialize()
{
	if (m_aryRestrictAreaData.size() > 0)
	{
		for (RestrictArea* pRestrictArea : m_aryRestrictAreaData)
		{
			delete pRestrictArea;
		}
		m_aryRestrictAreaData.clear();
	}
}
bool CImportRestrictAreaData::ReadData(eDatum datum)
{
	if (!GetFUtil()->IsExistPath(m_strFilePath))
	{
		return	false;
	}

	// ������
	Initialize();

	// �t�@�C�����ꗗ���擾
	std::string strFilePath = CStringEx::ToString(m_strFilePath);

	// �V�F�[�v�t�@�C�����I�[�v��
	SHPHandle hSHP;
	hSHP = SHPOpen(strFilePath.c_str(), "r");
	if (hSHP == NULL)
	{
		// �t�@�C���̃I�[�v���Ɏ��s
		return false;
	}

	// ���
	int nShapeType = SHPT_NULL;
	// �v�f��
	int nEntities = 0;
	//�@�o�E���f�B���O
	double adfMinBound[4], adfMaxBound[4];
	SHPGetInfo(hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound);

	if (nEntities <= 0)
	{
		// �V�F�[�v�t�@�C�����R�[�h�̎擾�Ɏ��s
		SHPClose(hSHP);
		return false;
	}
	// ��ʂ̃`�F�b�N
	if (nShapeType != SHPT_POLYGON &&
		nShapeType != SHPT_POLYGONZ &&
		nShapeType != SHPT_POLYGONM)
	{
		// �V�F�[�v�^�C�v���ΏۊO
		SHPClose(hSHP);
		return false;
	}

	// �t�@�C������f�[�^��ǂݍ���Œ��_�z��ɒǉ�
	SHPObject* psElem;
	for (int ic = 0; ic < nEntities; ic++)
	{
		psElem = SHPReadObject(hSHP, ic);

		if (psElem)
		{
			RestrictArea* pRestrictArea = new RestrictArea;

			// ���_��̎擾
			for (int jc = 0; jc < psElem->nVertices; jc++)
			{
				// �ϊ��������W�̒ǉ�
				CPointBase pt;
				if (datum == eDatum::LATLON)
				{
					// �ܓx�o�x -> ���ʒ��p���W
					int JPZONE = GetINIParam()->GetJPZone();
					double x, y;
					CGeoUtil().LonLatToXY(psElem->padfX[jc], psElem->padfY[jc], JPZONE, x, y);
					pt = CPointBase(x, y, psElem->padfZ[jc]);
				}
				else
				{
					pt = CPointBase(psElem->padfX[jc], psElem->padfY[jc], psElem->padfZ[jc]);
				}
				pRestrictArea->vecPolyline.push_back(pt);
			}

			if (pRestrictArea->vecPolyline.size() >= 0)
			{
				m_aryRestrictAreaData.push_back(pRestrictArea);
			}

		}
		SHPDestroyObject(psElem);
	}

	SHPClose(hSHP);
	return true;
}

// ���������O����
bool CImportRestrictAreaData::IsBuildingInRestrictArea(const std::vector<ROOFSURFACES> pRoofSurfaceList)
{
	// ������搔
	size_t iAreaNum = m_aryRestrictAreaData.size();

	// ������悲�Ƃɔ���
	for (RestrictArea* pRestrictArea : m_aryRestrictAreaData)
	{
		std::vector<CPoint2D> restrictArea;
		for(int i = 0; i < pRestrictArea->vecPolyline.size(); i++)
		{
			CPointBase pos = pRestrictArea->vecPolyline.at(i);
			restrictArea.push_back(CPoint2D(pos.x, pos.y));
		}
		
		// �����ʍ��W���Ƃɔ���
		for (const auto& roofSurfaces : pRoofSurfaceList)
		{
			for (const auto& surfaceMembers : roofSurfaces.roofSurfaceList)
			{
				for (const auto& pos : surfaceMembers.posList)
				{
					CPoint2D target2d(pos.x, pos.y);
					// ���O����
					bool bRet = CGeoUtil::IsPointInPolygon(target2d, (int)restrictArea.size(), restrictArea.data());
					// ���������ɂ���
					if (bRet) return true;
				}
			}
		}
	}

	return false;
}

// ���������O����
bool CImportRestrictAreaData::IsLandInRestrictArea(const std::vector<CPoint2D> pSurface)
{
	// ������搔
	size_t iAreaNum = m_aryRestrictAreaData.size();

	// ������悲�Ƃɔ���
	for (RestrictArea* pRestrictArea : m_aryRestrictAreaData)
	{
		std::vector<CPoint2D> restrictArea;
		for (int i = 0; i < pRestrictArea->vecPolyline.size(); i++)
		{
			CPointBase pos = pRestrictArea->vecPolyline.at(i);
			restrictArea.push_back(CPoint2D(pos.x, pos.y));
		}

		// ���W���Ƃɔ���
		for (const auto& pos : pSurface)
		{
			CPoint2D target2d(pos.x, pos.y);
			// ���O����
			bool bRet = CGeoUtil::IsPointInPolygon(target2d, (int)restrictArea.size(), restrictArea.data());
			// ���������ɂ���
			if (bRet) return true;
		}
	}

	return false;
}