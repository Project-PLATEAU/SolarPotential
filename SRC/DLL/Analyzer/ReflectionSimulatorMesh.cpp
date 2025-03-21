#include "pch.h"
#include "ReflectionSimulatorMesh.h"
#include <CommonUtil/ReadINIParam.h>
#include <filesystem>

using namespace std;

CReflectionSimulatorMesh::CReflectionSimulatorMesh(CUIParam* pUIParam)
	: m_pParam(pUIParam)
{

}

CReflectionSimulatorMesh::~CReflectionSimulatorMesh()
{

}

bool CReflectionSimulatorMesh::Exec(
	const CVector3D& inputVec,
	const vector<CVector3D>& roofMesh,
	const std::vector<BLDGLIST>& buildingsList
)
{
	// �����̗L������
	const double LIGHT_LENGTH = m_pParam->pReflectionParam->dReflectionRange;

	// �������b�V���̍��W
	CVector3D roofMeshPos;
	for (const auto& mesh : roofMesh)
		roofMeshPos += mesh;
	roofMeshPos *= 0.25;	// 4�_�̕���
	// �������b�V���̖@��
	CVector3D n;
	CGeoUtil::OuterProduct(
		CVector3D(roofMesh[1], roofMesh[0]),
		CVector3D(roofMesh[2], roofMesh[1]), n);
	if (n.z < 0) n *= -1;

	// �����ʃ��b�V���̗���������ˌ����������Ă���Ƃ��͔��˂��Ȃ��̂ŉ�͏I��
	if (CGeoUtil::InnerProduct(n, inputVec) >= 0.0)
		return false;

	// ���ˌ��̌������Z�o
	// �������b�V�����W�̉�������500m�ɐݒ肷��
	CVector3D inputInverseVec = CGeoUtil::Normalize(inputVec) * ((-1) * LIGHT_LENGTH);
	CVector3D sunPos = roofMeshPos + inputInverseVec;
	CLightRay lightRay(sunPos, CGeoUtil::Normalize(inputVec) * LIGHT_LENGTH);

	// ���ˌ�������̌����Ɏז����ꂸ�ɉ����ʂɓ����邩�`�F�b�N
	double dist = 0.0;
	if (IntersectBuildings(lightRay, buildingsList, nullptr, &dist, nullptr, nullptr))
	{
		return false;	// �������b�V���Ɍ������������Ă��Ȃ��̂ŉ�͏I��
	}

	// �L�����Z��
	if (IsCancel())
	{
		return false;
	}

	// ���ˌ�
	CLightRay reflectedLightRay = lightRay.Reflect(roofMeshPos, n);

	// ���ˌ��������ɂ�����Ƃ��ǂ��ɂ�����̂�
	CVector3D targetPos;		// �����������W
	string strTargetBuilding;	// ������������ID
	vector<CVector3D> targetPosList;	// ���������ʍ��W
	if (IntersectBuildings(reflectedLightRay, buildingsList, &targetPos, &dist, &strTargetBuilding, &targetPosList))
	{
		// �ǂ��������ɂ�����Ƃ��A���ʂɋL�^����
		m_reflectionMesh.inputVec = inputVec;
		m_reflectionMesh.outputVec = reflectedLightRay.GetVector();
		m_reflectionMesh.reflectionRoof.targetPos = roofMeshPos;
		m_reflectionMesh.reflectionTarget.buildingId = strTargetBuilding;
		m_reflectionMesh.reflectionTarget.targetPos = targetPos;
		m_reflectionMesh.reflectionPosList = targetPosList;

		return true;
	}

	// �L�����Z��
	if (IsCancel())
	{
		return false;
	}

	return false;
}

const CAnalysisReflectionMesh& CReflectionSimulatorMesh::GetResult() const
{
	return m_reflectionMesh;
}

// �����Q�Ɍ������������Ă��邩
// �������Ă���Ƃ��̍��W�ƌ�������̋������擾
bool CReflectionSimulatorMesh::IntersectBuildings(
	const CLightRay& lightRay,
	const std::vector<BLDGLIST>& buildingsList,
	CVector3D* targetPos,
	double* dist,
	string* strTargetBuilding,
	vector<CVector3D>* posList
)
{
	bool result = false;
	double minDist = DBL_MAX;
	double tempDist;
	CVector3D tempTargetPos;
	BUILDINGS tempTargetBuilding;
	vector<CVector3D> tempPosList;
	for (const auto& bldglist : buildingsList)
	{
		// LOD2
		const vector<BUILDINGS>& buildings = bldglist.buildingList;
		for (const auto& building : buildings)
		{
			// �L�����Z��
			if (IsCancel())
			{
				return false;
			}

			// �ǖʂƉ����ʂ̃��X�g
			vector<SURFACEMEMBERS> surfaceList;
			// �ǖ�
			for (const auto& wall : building.wallSurfaceList)
				surfaceList.insert(surfaceList.end(), wall.wallSurfaceList.begin(), wall.wallSurfaceList.end());
			// ������
			for (const auto& roof : building.roofSurfaceList)
				surfaceList.insert(surfaceList.end(), roof.roofSurfaceList.begin(), roof.roofSurfaceList.end());

			// �����Ɩʂ̓����蔻��
			if (IntersectSurface(lightRay, surfaceList, &tempTargetPos, &tempDist, &tempPosList))
			{
				// ��ԋ߂��ʒu�̃f�[�^���̗p����
				if (tempDist < minDist)
				{
					if (targetPos)		*targetPos = tempTargetPos;
					if (dist)			*dist = tempDist;
					if (strTargetBuilding)	*strTargetBuilding = building.building;
					if (posList)		*posList = tempPosList;
					minDist = tempDist;
				}
				result = true;
			}
		}

		// LOD1
		const vector<BUILDINGSLOD1>& buildingsLOD1 = bldglist.buildingListLOD1;
		for (const auto& building : buildingsLOD1)
		{
			// �L�����Z��
			if (IsCancel())
			{
				return false;
			}

			// �ǖʂ̃��X�g
			vector<SURFACEMEMBERS> surfaceList;
			for (const auto& wall : building.wallSurfaceList)
				surfaceList.insert(surfaceList.end(), wall.wallSurfaceList.begin(), wall.wallSurfaceList.end());

			// �����Ɩʂ̓����蔻��
			if (IntersectSurface(lightRay, surfaceList, &tempTargetPos, &tempDist, &tempPosList))
			{
				// ��ԋ߂��ʒu�̃f�[�^���̗p����
				if (tempDist < minDist)
				{
					if (targetPos)		*targetPos = tempTargetPos;
					if (dist)			*dist = tempDist;
					if (strTargetBuilding)	*strTargetBuilding = building.building;
					if (posList)		*posList = tempPosList;
					minDist = tempDist;
				}
				result = true;
			}
		}
	}

	return result;
}

// ���ʂɌ������������Ă��邩�ǂ���
bool CReflectionSimulatorMesh::IntersectSurface(
	const CLightRay& lightRay,
	const vector<SURFACEMEMBERS>& surfaceList,
	CVector3D* targetPos,
	double* dist,
	vector<CVector3D>* surfacePosList
)
{
	// �������߂�����Ƃ��͌덷���̌������g�Ƃ��Ĕ��菜�O���邽�߂̋���
	constexpr double MINIMUM_DIST = 0.001;

	// ������������艓�����Ȃ������ׂ�
	if (!CheckDistance(lightRay, surfaceList))
		return false;

	bool result = false;
	double minDist = DBL_MAX;
	double tempDist;
	CVector3D tempTargetPos;
	for (const auto& polygon : surfaceList)
	{
		vector<CVector3D> posList(polygon.posList.size());
		int i = 0;
		for (const auto& pos : polygon.posList)
		{
			posList[i] = CVector3D(pos.x, pos.y, pos.z);
			++i;
		}

		// �����ƃ|���S���̌�_��T��
		if (lightRay.Intersect(posList, &tempTargetPos, &tempDist))
		{
			if ((tempDist > MINIMUM_DIST) &&
				(abs(lightRay.GetVector().Length() - tempDist) > MINIMUM_DIST))
			{
				if (tempDist < minDist)
				{
					if (targetPos)		*targetPos = tempTargetPos;
					if (dist)			*dist = tempDist;
					if (surfacePosList)	*surfacePosList = posList;
					minDist = tempDist;
				}
				result = true;
			}
		}
	}

	return result;
}

// ���ʂ�����������������͈͓̔�����܂��Ƀ`�F�b�N
bool CReflectionSimulatorMesh::CheckDistance(
	const CLightRay& lightRay,
	const vector<SURFACEMEMBERS>& surfaceList
)
{
	// �������͈͓������肷�鋗���͈�
	const double LIGHT_LENGTH = m_pParam->pReflectionParam->dReflectionRange;	// �אڂ���BBox�͈̔�[m]

	for (const auto& polygon : surfaceList)
	{
		for (const auto& pos : polygon.posList)
		{
			if (CheckDistance(lightRay, pos, LIGHT_LENGTH))
				return true;	// �͈͓��������OK
		}
	}

	return false;
}

// �w��_������������������͈͓̔����`�F�b�N
bool CReflectionSimulatorMesh::CheckDistance(
	const CLightRay& lightRay,
	const CPointBase& pos,
	const double& dist
)
{
	// �������͈͓������肷�鋗���͈�
	const double SQUARE_LINGHT_LENGTH = dist * dist;

	bool bDist = false;
	bool bDirect = false;

	double dx = pos.x - lightRay.GetPos().x;
	double dy = pos.y - lightRay.GetPos().y;
	double dz = pos.z - lightRay.GetPos().z;

	// ���ʂ̒��_���t�����ɂ���Ƃ��͔͈͊O�Ƃ���
	double dot = CGeoUtil::InnerProduct(lightRay.GetVector(), CVector3D(dx, dy, dz));
	if (dot > 0.0)
		bDirect = true;

	// �������������Ȃ����`�F�b�N
	double len = dx * dx + dy * dy + dz * dz;
	if (len <= SQUARE_LINGHT_LENGTH)
		bDist = true;

	if (bDirect && bDist)
		return true;

	return false;
}

// �L�����Z���`�F�b�N
bool CReflectionSimulatorMesh::IsCancel()
{
	assert(m_pParam);

	// �L�����Z���t�@�C���̃f�B���N�g���p�X
	filesystem::path filepath = m_pParam->strOutputDirPath;
	filepath = filepath.parent_path().parent_path();

	// �L�����Z���t�@�C���̃p�X
	filepath /= CANCELFILE;
	string cancelPath = filepath.string();

	if (std::filesystem::exists(cancelPath))
		return true;

	return false;
}