#pragma once

#include <vector>

#include <CommonUtil/CGeoUtil.h>
#include <CommonUtil/CLightRay.h>
#include "AnalysisReflectionMesh.h"
#include "AnalyzeData.h"
#include "UIParam.h"

class CReflectionSimulatorMesh
{
public:
	CReflectionSimulatorMesh(CUIParam* pUIParam);
	virtual ~CReflectionSimulatorMesh();

	// ���ˌ��������ɂ����邩��͂���B
	// �������Ă���Ƃ���true��Ԃ��B
	// �������Ă����͌��ʂ�GetResult()�Ŏ擾�ł���^�Ŋm�F�ł���B
	// ���ˌ��̔��˂�������ID�͎����Ă��Ȃ��̂�
	// GetResult()�Ŏ擾�ł���l�ɂ͊܂܂�Ȃ��̂Œ��ӁB
	bool Exec(
		const CVector3D& inputVec,					// ���ˌ�
		const vector<CVector3D>& roofMesh, 			// ���ˉ������b�V��
		//const BUILDINGS& building,					// �������b�V�������錚��
		const std::vector<BLDGLIST>& buildingsList	// ���ˌ��������邩���ׂ錚��
	);

	// ���ʎ擾
	const CAnalysisReflectionMesh& GetResult() const;


private:
	CAnalysisReflectionMesh m_reflectionMesh;	// ��͌���

	CUIParam* m_pParam;

	// �����������Q�ɂ������Ă��邩�ǂ���
	bool IntersectBuildings(
		const CLightRay& lightRay,				// ����
		const std::vector<BLDGLIST>& buildingsList,// �������������Ă��邩�`�F�b�N���錚���Q
		CVector3D* targetPos,					// [out]�������������Ă�����W
		double* dist,							// [out]�������炠�����Ă���ʒu�܂ł̋���
		std::string* strTargetBuilding,			// [out]�������������Ă��錚��ID
		std::vector<CVector3D>* wallPosList		// [out]�ǖ�
	);

	// ���ʂɌ������������Ă��邩�ǂ���
	bool IntersectSurface(
		const CLightRay& lightRay,				// ����
		const std::vector<SURFACEMEMBERS>& surfaceList,	// �������������Ă��邩�`�F�b�N���镽�ʃ��X�g
		CVector3D* targetPos,					// [out]�������������Ă�����W
		double* dist,							// [out]�������炠�����Ă���ʒu�܂ł̋���
		vector<CVector3D>* posList				// [out]�������Ă����
	);

	// �ʂ�����������������̑�܂��Ȕ͈͓��ɂ��邩�`�F�b�N
	bool CheckDistance(const CLightRay& lightRay, const vector<SURFACEMEMBERS>& roofSurfaceList);

	// �w��_������������������͈͓̔��ɂ��邩�`�F�b�N
	bool CheckDistance(const CLightRay& lightRay, const CPointBase& pos, const double& dist);

	// �L�����Z���`�F�b�N
	bool IsCancel();

};
