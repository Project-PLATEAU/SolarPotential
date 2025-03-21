#pragma once
#include <string>
#include <CommonUtil/CGeoUtil.h>

// ���˃V�~�����[�V����
// 1���b�V�����Ƃ̔��˃V�~�����[�V�����f�[�^
class CAnalysisReflectionMesh
{
public:
	CVector3D inputVec;		// ���ˌ��x�N�g��

	// �����ʃ��b�V���Ƃ̖@���x�N�g���Ɠ��˃x�N�g���̂Ȃ��p�x���
	// �����ʂ̍��W�n�ɒu�������ċ��߂�B
	CVector3D outputVec;	// ���˃x�N�g��


	// �Փˉ���
	struct TargetRoof
	{
		std::string buildingId{ "" };		// ����ID
		int buildingIndex{ 0 };				// ����ID�̏���
		std::string roofSurfaceId{ "" };	// ����ID
		CPointBase targetPos;				// �Փ˓_

	};

	// ���ː�
	TargetRoof reflectionTarget;
	// ���ː�ʍ��W
	std::vector<CVector3D> reflectionPosList;

	// ���ˉ������b�V��
	TargetRoof reflectionRoof;

};

//
//// ���˃V�~�����[�V����
//// �G���A���Ƃ̔��˃V�~�����[�V�����f�[�^
//class CAnalysisReflectionArea
//{
//public:
//	std::string areaID;
//	CAnalysisReflection refList;
//
//};
