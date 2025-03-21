#pragma once

#include <string>
#include <map>

#include <CommonUtil/CTime.h>
#include <CommonUtil/CSunVector.h>
#include "AnalysisReflectionMesh.h"
#include "AnalyzeData.h"
#include "UIParam.h"
#include "CommonUtil/CGeoid.h"
#include "CommonUtil/ExitCode.h"

typedef std::vector<CAnalysisReflectionMesh> CAnalysisReflection;
typedef std::vector<CAnalysisReflection> CAnalysisReflectionOneDay;

class CReflectionSimulator
{
public:
	// ���˃V�~�����[�V������͂��s���N�������w�肷��
	// �ݒ肵������24���Ԃŉ�͂��s��
	CReflectionSimulator(const CTime& date, CUIParam* pUIParam);
	virtual ~CReflectionSimulator();

	// ���˃V�~�����[�V������͂����s����
	// ���ˌ��������ɂ������Ă����͌��ʂ�����Ƃ�true��Ԃ�
	eExitCode Exec(
		const vector<AREADATA>& analyzeAreas);			// ��̓G���A

	// ���ʂ�CSV�t�@�C���ɏo�͂���
	// ���ʂ�Exec()�����{���true���Ԃ����Ƃ��Ɋi�[���ꂽ�f�[�^���o�͂���
	bool OutResult(const std::string& csvfile) const;

	// ���ʂ�CZML�t�@�C���ɏo�͂���
	bool OutResultCZML(const std::string& czmlfile) const;

	// ����CSV��CZML�ɕϊ�����
	static bool ConvertResultCSVtoCZML(const std::string& csvfile, const std::string& czmlfile);

	// ��͌��ʂ�24���ԕ��f�[�^���擾����
	// ���ʂ�Exec()�����{���true���Ԃ����Ƃ��Ɋi�[���ꂽ�f�[�^���o�͂���
	const std::vector<CAnalysisReflection>& GetResult() const;

	// Exec()���s����
	eExitCode GetExecResult() const;

private:
	// �N����
	CTime m_date;

	// ���z���x�N�g��
	CSunVector* m_pSunVector{ nullptr };

	// 24���ԕ��̔��˃V�~�����[�V�����̉�͌��ʂ��i�[
	CAnalysisReflectionOneDay m_result;

	// �����ʂ̌����E�X���̕␳
	CReflectionCorrect m_roofCorrectLower;	// �����ʌX��3�x�����̕␳
	CReflectionCorrect m_roofCorrectUpper;	// �����ʌX��3�x�ȏ�̕␳

	// Exec()���s����
	eExitCode m_bExec;

	CUIParam* m_pParam;

	// �������̌���ID�̐��A����ID�̏��Ԃ��Ǘ�����
	int m_buildingIndex;
	map<string, int> m_mapBuildingIndex;

	// �S�ď�����
	void Init();

	// ��͌��ʂ̏�����
	void InitResult();

	// ���z�ʒu�̈ܓx�o�x��ݒ肷��
	// ���z�ʒu�����ɔ��˃V�~�����[�V�����̉�͂��s��
	void SetSunVector(double lat, double lon);

	// ��͌��ʂ̐ݒ�
	void SetResult(uint8_t hour, const CAnalysisReflection& result);

	// �S�����ł̉��
	bool AnalyzeBuildings(
		const std::vector<BLDGLIST>& targetBuildings,
		const std::vector<BLDGLIST*>& buildings,
		uint8_t hour,
		CAnalysisReflection& result);

	// 1�����ł̉��
	bool AnalyzeBuilding(
		const BUILDINGS& building,
		const std::vector<BLDGLIST>& buildings,
		const CVector3D& sunVector,
		CAnalysisReflection& result);

	// 1�����ł̉��
	bool AnalyzeRoof(
		const ROOFSURFACES& roof, const BUILDINGS& building,
		const std::vector<BLDGLIST>& buildings,
		const CVector3D& sunVector,
		CAnalysisReflection& result);

	// 1�������b�V���ł̉��
	bool AnalyzeMesh(
		const MESHPOSITION_XY& mesh,
		const vector<CPointBase>& posList,
		const std::vector<BLDGLIST>& buildings,
		const CVector3D& sunVector,
		CAnalysisReflectionMesh& result);

	// �y�n�ł̉��
	bool AnalyzeLand(
		const AREADATA& targetArea,
		const std::vector<BLDGLIST*>& buildings,
		uint8_t hour,
		CAnalysisReflection& result);

	// �Ώۃ��b�V���̗אڂ��郁�b�V�����擾
	void GetNeighborBuildings(
		const BLDGLIST& targetBuildings,			// �Ώۃ��b�V��
		const std::vector<BLDGLIST*>& buildings,	// �S���b�V��
		std::vector<BLDGLIST>& neighborBuildings);	// �ߗ׃��b�V��

	// �Ώۃ��b�V���̗אڂ��郁�b�V�����擾(�y�n��)
	void GetNeighborBuildings(
		const AREADATA& targetArea,
		const std::vector<BLDGLIST*>& buildings,
		std::vector<BLDGLIST>& neighborBuildings);

	// �L�����Z�������ǂ���
	bool IsCancel();

	// ���ʂɌ���ID�̏��Ԃ�ݒ肷��
	void SetBuildingIndex(CAnalysisReflection& result);

	// ���ʂɌ���ID�̏��Ԃ�ݒ肷��
	void SetAreaIndex(CAnalysisReflection& result);

	// ���ʂ�CZML�t�@�C���ɏo�͂���
	static bool OutResultCZML(const std::string& czmlfile, const CAnalysisReflectionOneDay& result);

	// CZML��Line���o��
	static void OutResultCZMLLine(
		ofstream& ofs, const CAnalysisReflectionOneDay& result,
		int JPZONE, CGeoid* pGeoid, const string& rgba);

	// CZML��Point���o��
	static void OutResultCZMLPoint(
		ofstream& ofs, const CAnalysisReflectionOneDay& result,
		int JPZONE, CGeoid* pGeoid, const string& rgba, const string& czmlfilename);

	// XYZ��CZML�̈ܓx�o�x�A�����ɕϊ�����
	static void ConvertXYZToLatLonZ(
		double x, double y, double z,
		int JPZONE, CGeoid* pGeoid,
		double& lat, double& lon, double& h);

	// ���ʃt�@�C���̑S�s��ǂݍ���
	static bool ReadResultCSVLines(const string& csvfile, std::vector<string>& lines);
};
