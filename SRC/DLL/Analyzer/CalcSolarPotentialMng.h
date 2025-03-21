#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/CSunVector.h"
#include "../../LIB/CommonUtil/TiffDataManager.h"
#include "../../LIB/CommonUtil/CLightRay.h"
#include "../../LIB/CommonUtil/ExitCode.h"
#include "AnalysisRadiationData.h"
#include "ImportMetpvData.h"
#include "ImportPossibleSunshineData.h"
#include "ImportAverageSunshineData.h"
#include "UIParam.h"

#include "AnalyzeData.h"

#include <iostream>
#include <fstream>

class CCalcSolarPotentialMng
{
public:
	// ���ʔ͈͂������l(����16���ʂ�22�x����)
	const double AZ_RANGE_JUDGE_DEGREE = 22.0;

	enum class eOutputImageTarget
	{
		SOLAR_RAD = 0,
		SOLAR_POWER,
	};

	/// <summary>
	/// ��͑Ώ�
	/// ���O�͑Ώۂ��Ƃ̏o�̓t�H���_��
	/// </summary>
	enum class eAnalyzeTarget
	{
		NONE = -1,
		ROOF = 0,
		LAND,

		// ��͑Ώې�
		TARGET_COUNT,
	};

	CCalcSolarPotentialMng(
		CImportPossibleSunshineData* pSunshineData,
		CImportAverageSunshineData* pPointData,
		CImportMetpvData* pMetpvData,
		CUIParam* m_pUIParam,
		const int& iYear
	);
	~CCalcSolarPotentialMng(void);


	typedef std::map<std::string, CResultData> CResultDataMap;

public:
	bool AnalyzeSolarPotential();	// ���d�|�e���V�������v���C������
	void AnalyzeBuild(const AREADATA& areaData, CBuildListDataMap*& resultDataMap);	// �������
	void AnalyzeLand(const AREADATA& areaData, CPotentialData*& resultData);		// �y�n���

	eExitCode GetExitCode()				{ return m_eExitCode; };
	void SetExitCode(eExitCode code)	{ m_eExitCode = code; };

	CAnalysisRadiationCommon*	GetRadiationData()	{ return m_pRadiationData; };
	CUIParam*					GetUIParam()		{ return m_pUIParam; };
	CImportPossibleSunshineData*	GetSunshineData()	{ return m_pSunshineData; };
	CImportAverageSunshineData*		GetPointData()		{ return m_pPointData; };
	CImportMetpvData*				GetMetpvData()		{ return m_pMetpvData; };
	
	// ���ӂ̒n���Ɏז����ꂸ�ΏۖʂɌ����������邩�ǂ����̔���
	bool IntersectSurfaceCenter(
		const CVector3D& inputVec,						// ���ˌ�
		const std::vector<CVector3D>& surfaceBB,		// �Ώۖ�BB
		const CVector3D& center,						// �Ώۖʒ��S
		const std::string& strId,						// �Ώۂ̉���ID
		const vector<BLDGLIST>& neighborBuildings,		// ���ӂ̌������X�g
		const vector<CTriangle>& neighborDems			// ���ӂ̒n�`TIN���X�g
	);

	// �Ώۂ̌����ɗאڂ��錚�����擾
	void GetNeighborBuildings(
		const CVector3D& bldCenter,						// �Ώۂ̌������S
		std::vector<BLDGLIST>& neighborBuildings		// �ߗ׌���
	);

	// �Ώۂ̌����ɗאڂ���DEM���擾
	void GetNeighborDems(
		const CVector3D& bldCenter,						// �Ώۂ̌������S
		std::vector<CTriangle>& neighborDems,			// �ߗ�TIN
		eAnalyzeTarget target
	);

	const int GetYear() { return m_iYear; };

	// �L�����Z������
	bool IsCancel();

	// DEM�f�[�^�̎g�p�L��
	bool IsEnableDEMData();

private:
	double calcLength(double dx, double dy, double dz)
	{
		return sqrt(dx * dx + dy * dy + dz * dz);
	}

	void initialize();			// �v�Z�p�f�[�^���̏�����

	// �X�Ίp�A���ʊp���Z�o����
	void calcRoofAspect(const std::vector<BUILDINGS*>& targetBuildings, CPotentialDataMap& bldDataMap);
	bool calcRansacPlane(const std::vector<CPointBase>& vecAry, CVector3D& vNormal);
	void calcLandAspect(const AREADATA& area, CPotentialData& landData);

	// �����Ƃ̓��Ɨ����v�Z
	void calcMonthlyRate();

	// �G���A���Ƃ̏o�͏���
	bool outputAreaBuildResult(const AREADATA& areaData);
	bool outputAreaLandResult(const AREADATA& areaData);

	// �o�͗p��3D�|�C���g�f�[�^���쐬
	bool createPointData_Build(
		std::vector<CPointBase>& vecPoint3d,
		const AREADATA& areaData,
		const BLDGLIST& bldList,
		const CPotentialDataMap& bldDataMap,
		double outMeshsize,
		const eOutputImageTarget& eTarget
	);
	// �o�͗p��3D�|�C���g�f�[�^���쐬
	bool createPointData_Land(
		std::vector<CPointBase>& vecPoint3d,
		const AREADATA& areaData,
		const CPotentialData& landData,
		const eOutputImageTarget& eTarget
	);
	// ���˗ʂ̒l�ɉ����Ē��F�����摜���o��
	bool outputImage(
		const std::wstring strFilePath,
		std::vector<CPointBase>* pvecPoint3d,
		double outMeshsize,
		const eOutputImageTarget& eTarget
	);

	bool outputLegendImage();			// �}��摜���o��
	bool outputAzimuthDataCSV();		// �K�n����p ���ʊp���ԃt�@�C���o��
	bool outputAllAreaResultCSV();		// �S�͈͂ɂ�������˗ʁE���d��CSV���o��
	bool outputLandShape();					// �V�F�[�v�t�@�C���ɏo��

	bool outputMonthlyRadCSV(const CPotentialDataMap& dataMap, const std::wstring& wstrOutDir);	// ���ʓ��˗�CSV�o��
	bool outputSurfaceRadCSV(const eAnalyzeTarget analyzeTarget, const CPotentialDataMap& dataMap, const std::wstring& wstrOutDir);	// ���b�V�����Ƃ̓��˗�CSV�o��

	void finalize();

	double calcArea(const std::vector<CPointBase>& vecPos);
	
	// �����������Q�ɂ������Ă��邩�ǂ���
	bool intersectBuildings(
		const CLightRay& lightRay,					// ����
		const std::string& strId,					// �Ώۂ̉���ID
		const std::vector<BLDGLIST>& buildingsList	// �������������Ă��邩�`�F�b�N���錚���Q
	);
	// �����������ɂ������Ă��邩�ǂ���
	bool intersectBuilding(
		const CLightRay& lightRay,					// ����
		const vector<WALLSURFACES>& wallSurfaceList	// �������������Ă��邩�`�F�b�N���錚���̕�
	);
	bool intersectBuilding(
		const CLightRay& lightRay,					// ����
		const std::string& strId,					// �Ώۂ̉���ID
		const BUILDINGS& buildings					// �������������Ă��邩�`�F�b�N���錚���Q
	);
	// �����������͈͓̔���
	bool checkDistance(const CLightRay& lightRay, const vector<WALLSURFACES>& wallSurfaceList);

	// �������n�`�ɂ������Ă��邩�ǂ���
	bool intersectLandDEM(
		const CLightRay& lightRay,					// ����
		const vector<CTriangle>& tinList,			// �������������Ă��邩�`�F�b�N����n�`��TIN
		const std::vector<CVector3D>& surfaceBB		// �Ώۖʂ�BB
	);

	// �F�ݒ�t�@�C���p�X���擾
	std::wstring getColorSettingFileName(const eOutputImageTarget& eTarget);

	// �t�H���_�E�t�@�C������
	const std::wstring GetDirName_AnalyzeTargetDir(eAnalyzeTarget target);
	const std::wstring GetDirName_SolarRadImage() { return L"���˗ʉ摜"; }
	const std::wstring GetDirName_SolarPotentialImage() { return L"���d�|�e���V�����摜"; }
	const std::wstring GetDirName_LandShape() { return L"�V�F�[�v�t�@�C��"; }
	const std::wstring GetFileName_SolarPotentialCsv(eAnalyzeTarget target);	// �����E�y�n���Ɨ\�����d��CSV
	const std::wstring GetFileName_MeshPotentialCsv(eAnalyzeTarget target);		// (�y�n�ʂ̂�)���b�V�����Ɨ\�����d��CSV
	const std::wstring GetFileName_SolarPotentialShp();							// (�y�n�ʂ̂�)�y�n���Ɨ\�����d��SHP
	const std::wstring GetFileName_MeshSolarRadCsv(eAnalyzeTarget target);		// �����ʕʁE�y�n�ʃ��b�V���ʓ��˗�CSV

private:
	// ���̓f�[�^
	CImportPossibleSunshineData* m_pSunshineData;
	CImportAverageSunshineData* m_pPointData;
	CImportMetpvData* m_pMetpvData;
	CUIParam* m_pUIParam;

	std::vector<AREADATA>*		m_pvecAllAreaList;		// ��̓G���A���(�����A�y�n�f�[�^)
	AREADATA*					m_targetArea;			// �������G���A�f�[�^

	CAnalysisRadiationCommon*	m_pRadiationData;		// ���ʂ̌v�Z�p�����[�^
	CResultDataMap*				m_pmapResultData;		// �v�Z���ʃf�[�^�}�b�v

	int m_iYear;

	CTime						m_dateStart;
	CTime						m_dateEnd;

	std::wstring				m_strCancelFilePath;

	eExitCode					m_eExitCode;				// �I���R�[�h
	bool						m_isCancel;					// �L�����Z�����

#ifdef CHRONO
	std::ofstream m_ofs;
#endif

};
