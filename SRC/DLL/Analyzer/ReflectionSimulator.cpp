#include "pch.h"
#include "ReflectionSimulator.h"
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <string>

#include <CommonUtil/CGeoUtil.h>
#include <CommonUtil/CLightRay.h>
#include <CommonUtil/ReadINIParam.h>
#include "ReflectionSimulatorMesh.h"
#include "AnalysisReflectionMesh.h"
#include "AnalyzeData.h"

using namespace std;


CReflectionSimulator::CReflectionSimulator(const CTime& date, CUIParam* pUIParam)
	: m_date(date)
	, m_pParam(pUIParam)
	, m_bExec(eExitCode::Normal)
	, m_buildingIndex(0)
{
	Init();

	if (pUIParam)
	{
		m_roofCorrectLower = *pUIParam->pReflectionParam->pRoof_Lower;
		m_roofCorrectUpper = *pUIParam->pReflectionParam->pRoof_Upper;
	}
}

CReflectionSimulator::~CReflectionSimulator()
{
	if (m_pSunVector)
	{
		delete m_pSunVector;
		m_pSunVector = nullptr;
	}
}

// ���z�ʒu�̈ܓx�o�x��ݒ肷��
void CReflectionSimulator::SetSunVector(double lat, double lon)
{
	// �w����̑��z���𐶐�
	if (m_pSunVector)
		delete m_pSunVector;
	m_pSunVector = new CSunVector(lat, lon, m_date);
}

// ���˃V�~�����[�V������͂����s����
eExitCode CReflectionSimulator::Exec(
	const vector<AREADATA>& analyzeAreas
)
{
	assert(m_pParam);

	InitResult();

	// 24���Ԃ̉�͂��s��
	for (uint8_t hour = 0; hour < 24; ++hour)
	{
		CAnalysisReflection result;
		bool ret = true;

		for (const auto& area : analyzeAreas)
		{
			if (area.analyzeBuild)
			{
				vector<BLDGLIST> targetBuildings{};	// ���ˉ�͂��錚��

				// ��͑Ώۂ̌������X�g�����b�V�����Ƃɍ쐬
				for (const auto& bldList : area.neighborBldgList)
				{
					if (area.targetBuildings.find(bldList->meshID) == area.targetBuildings.end())	continue;

					BLDGLIST tmpBldList = *bldList;
					tmpBldList.buildingList.clear();

					const auto tmpBuildings = area.targetBuildings.at(bldList->meshID);
					for (auto bld : tmpBuildings)
					{
						tmpBldList.buildingList.emplace_back(*bld);
					}

					targetBuildings.emplace_back(tmpBldList);
				}

				ret &= AnalyzeBuildings(targetBuildings, area.neighborBldgList, hour, result);
			}

			// �L�����Z��
			if (IsCancel())
			{
				m_bExec = eExitCode::Cancel;
				return m_bExec;
			}

			if (area.analyzeLand)
			{
				ret &= AnalyzeLand(area, area.neighborBldgList, hour, result);
			}

			// �L�����Z��
			if (IsCancel())
			{
				m_bExec = eExitCode::Cancel;
				return m_bExec;
			}

		}

		// ��͂���ID�̏��Ԃ����ʂɐݒ肷��
		SetBuildingIndex(result);

		// ���ʂ����Ԃ��Ƃ̔z��ɕێ�
		SetResult(hour, result);

		// �L�����Z��
		if (IsCancel())
		{
			m_bExec = eExitCode::Cancel;
			return m_bExec;
		}
	}

	m_bExec = eExitCode::Normal;
	return m_bExec;
}

// ���˃V�~�����[�V�������ʂ�CSV�t�@�C���ɏo�͂���
bool CReflectionSimulator::OutResult(const std::string& csvfile) const
{
	ofstream ofs;
	ofs.open(csvfile);
	if (!ofs.is_open())
		return false;

	// �w�b�_�[��
	ofs << "����/�y�nID,������ID,�Эڰ��ݓ���,\
���˓_���W.X(��),���˓_���W.Y(��),���˓_���W.Z(��),\
���ː���W.X(��),���ː���W.Y(��),���ː���W.Z(��),\
���ː�" << endl;

	// �f�[�^��
	// m_result�ɂ͔z�񏇂�0�`23���̌��ʃf�[�^�������Ă���
	int hour = 0;
	for (const auto& resultVector : m_result)
	{
		for (const auto& result : resultVector)
		{
			// ���ꌚ���͏o�͂��Ȃ�
			if (result.reflectionTarget.buildingId == result.reflectionRoof.buildingId)
				continue;

			string strDate = CStringEx::Format("%d/%02d/%02d %02d:%02d",
				m_date.iYear, m_date.iMonth, m_date.iDay, hour, 0);

			ofs << fixed << setprecision(3)
				<< result.reflectionRoof.buildingId << ","
				<< result.reflectionRoof.roofSurfaceId << ","
				<< strDate << ","
				<< result.reflectionRoof.targetPos.x << ","
				<< result.reflectionRoof.targetPos.y << ","
				<< result.reflectionRoof.targetPos.z << ","
				<< result.reflectionTarget.targetPos.x << ","
				<< result.reflectionTarget.targetPos.y << ","
				<< result.reflectionTarget.targetPos.z << ",";
			if (result.reflectionTarget.buildingId == result.reflectionRoof.buildingId)
				ofs << "���ꌚ��" << endl;
			else
				ofs << result.reflectionTarget.buildingId << endl;
		}
		hour++;
	}

	ofs.close();

	return true;
}

// ���˃V�~�����[�V�������ʂ�CZML�t�@�C���ɏo�͂���
bool CReflectionSimulator::OutResultCZML(const std::string& czmlfile) const
{
	return OutResultCZML(czmlfile, m_result);
}

// CZML�t�@�C���ɏo�͂���
bool CReflectionSimulator::OutResultCZML(
	const std::string& czmlfile,			// czml�t�@�C���p�X
	const CAnalysisReflectionOneDay& result	// czml�ɏo�͂���f�[�^
)
{
	ofstream ofs;
	ofs.open(czmlfile);
	if (!ofs.is_open())
		return false;

	ofs << "[{\"id\":\"document\",\"name\":\"Light Trail\",\"version\":\"1.0\"}";

	// �g���q�Ȃ��t�@�C����
	filesystem::path filepath(czmlfile);
	string czmlfilename = filepath.stem().string();

	// �F��czml�t�@�C�����Ō��߂�
	string rgba;
	if (czmlfilename == "summer")
		rgba = "255,80,80,204";
	else if (czmlfilename == "spring")
		rgba = "169,208,142,204";
	else if (czmlfilename == "winter")
		rgba = "91,155,213,204";
	else // �w���
		rgba = "128,128,128,204";

	// ���W�n�ԍ����擾
	const int JPZONE = GetINIParam()->GetJPZone();

	// �W�I�C�h�t�@�C����ǂ�
	CGeoid* pGeoid = nullptr;
	double dOriginLat, dOriginLon;
	CGeoUtil::XYToLatLon(JPZONE, 0.0, 0.0, dOriginLat, dOriginLon);
	wstring strFilePath = GetFUtil()->Combine(GetFUtil()->GetModulePathW(), L"data/Geoid/gsigeo2011_ver1.asc");
	pGeoid = new CGeoid(CGeoid::GEOID_GEOID2000, strFilePath.data(), dOriginLat, dOriginLon);
	pGeoid->Load();

	// CZML��Line�o��
	OutResultCZMLLine(ofs, result, JPZONE, pGeoid, rgba);
	
	// CZML��Point�o��
	OutResultCZMLPoint(ofs, result, JPZONE, pGeoid, rgba, czmlfilename);

	delete pGeoid;

	ofs << "]";

	ofs.close();

	return true;
}

// ����CSV��CZML�ɕϊ�����
bool CReflectionSimulator::ConvertResultCSVtoCZML(
	const std::string& csvfile,		// �ϊ�����CSV�t�@�C���p�X
	const std::string& czmlfile		// �o�͂���CZML�t�@�C���p�X
)
{
	// ����CSV�t�@�C���̑S�s�����X�g�Ɋi�[
	vector<string> csvlines;
	if (!ReadResultCSVLines(csvfile, csvlines))
		return false;

	// ����CSV�̃f�[�^�����ʂɊi�[����
	CAnalysisReflectionOneDay result;
	CAnalysisReflection resultReflection;
	// CZML�ɏo�͂���Ƃ��́A���Ԃ≮���ʂɂ���ă��X�g�𕪂���K�v�Ȃ����߁A���ׂĂ̈�̃��X�g�Ɋi�[���Ă���
	vector<string> datas;
	for (const auto& csvline : csvlines)
	{
		CFileUtil::SplitCSVData(csvline, &datas);
		if (datas.size() < 9)
			return false;
		// ����ID
		string buildingId = datas[0];
		// ���˓_���W
		double x1 = stod(datas[3]);
		double y1 = stod(datas[4]);
		double z1 = stod(datas[5]);
		// ���ː���W
		double x2 = stod(datas[6]);
		double y2 = stod(datas[7]);
		double z2 = stod(datas[8]);

		// ���ʃ��X�g�Ɋi�[
		CAnalysisReflectionMesh resultReflectionMesh;
		resultReflectionMesh.reflectionRoof.buildingId = buildingId;
		resultReflectionMesh.reflectionRoof.targetPos.x = x1;
		resultReflectionMesh.reflectionRoof.targetPos.y = y1;
		resultReflectionMesh.reflectionRoof.targetPos.z = z1;
		resultReflectionMesh.reflectionTarget.targetPos.x = x2;
		resultReflectionMesh.reflectionTarget.targetPos.y = y2;
		resultReflectionMesh.reflectionTarget.targetPos.z = z2;
		resultReflection.emplace_back(resultReflectionMesh);
	}
	result.emplace_back(resultReflection);

	// CZML�t�@�C���o��
	return OutResultCZML(czmlfile, result);
}

// CZML��Line���o��
void CReflectionSimulator::OutResultCZMLLine(
	ofstream& ofs,
	const CAnalysisReflectionOneDay& result,
	int JPZONE,
	CGeoid* pGeoid,
	const string& rgba
)
{
	int id_counter = 0;
	for (const auto& resultVector : result)
	{
		for (const auto& result : resultVector)
		{
			// ���ꌚ���͏o�͂��Ȃ�
			if (result.reflectionTarget.buildingId == result.reflectionRoof.buildingId)
				continue;

			// ���˓_���W���ܓx�o�x�ɕϊ�
			double roof_lat, roof_lon, roof_z;
			ConvertXYZToLatLonZ(
				result.reflectionRoof.targetPos.x,
				result.reflectionRoof.targetPos.y,
				result.reflectionRoof.targetPos.z,
				JPZONE, pGeoid,
				roof_lat, roof_lon, roof_z
			);

			// ���ː���W���ܓx�o�x�ɕϊ�
			double target_lat, target_lon, target_z;
			ConvertXYZToLatLonZ(
				result.reflectionTarget.targetPos.x,
				result.reflectionTarget.targetPos.y,
				result.reflectionTarget.targetPos.z,
				JPZONE, pGeoid,
				target_lat, target_lon, target_z
			);

			ofs << fixed << setprecision(6)
				<< ",{"
				<< "\"id\":\"line_" << id_counter << "\","
				<< "\"name\":\"" << result.reflectionRoof.buildingId << "\","
				<< "\"description\":null,"
				<< "\"polyline\":{"
				<< "\"positions\":{"
				<< "\"cartographicDegrees\":["
				<< roof_lon << ","
				<< roof_lat << ",";
			ofs << fixed << setprecision(3)
				<< roof_z << ",";
			ofs << fixed << setprecision(6)
				<< target_lon << ","
				<< target_lat << ",";
			ofs << fixed << setprecision(3)
				<< target_z << "]"
				<< "},"
				<< "\"width\":2,"
				<< "\"material\":{"
				<< "\"solidColor\":{"
				<< "\"color\":{\"rgba\":[" << rgba << "]}"
				<< "}"
				<< "}"
				<< "}"
				<< "}";

			id_counter++;
		}
	}
}

// CZML��Point���o��
void CReflectionSimulator::OutResultCZMLPoint(
	ofstream& ofs,
	const CAnalysisReflectionOneDay& result,
	int JPZONE,
	CGeoid* pGeoid,
	const string& rgba,
	const string& czmlfilename
)
{
	// gltf
	string gltf = "model/" + czmlfilename + ".glb";

	int id_counter = 0;
	for (const auto& resultVector : result)
	{
		for (const auto& result : resultVector)
		{
			// ���ꌚ���͏o�͂��Ȃ�
			if (result.reflectionTarget.buildingId == result.reflectionRoof.buildingId)
				continue;

			// ���ː���W���ܓx�o�x�ɕϊ�
			double target_lat, target_lon, target_z;
			ConvertXYZToLatLonZ(
				result.reflectionTarget.targetPos.x,
				result.reflectionTarget.targetPos.y,
				result.reflectionTarget.targetPos.z,
				JPZONE, pGeoid,
				target_lat, target_lon, target_z
			);

			ofs << fixed << setprecision(6)
				<< ",{"
				<< "\"id\":\"point_" << id_counter << "\","
				<< "\"name\":\"" << result.reflectionRoof.buildingId << "\","
				<< "\"description\":null,"
				<< "\"position\":{"
				<< "\"cartographicDegrees\":["
				<< target_lon << ","
				<< target_lat << ",";
			ofs << fixed << setprecision(3)
				<< target_z << "]"
				<< "},"
				<< "\"model\":{"
				<< "\"gltf\":\"" << gltf << "\","
				<< "\"scale\":0.25,"
				<< "\"color\":{"
				<< "\"rgba\":[" << rgba << "]},"
				<< "\"colorBlendMode\":{"
				<< "\"colorBlendMode\":\"REPLACE\"},"
				<< "\"shadows\":{\"shadowMode\":\"DISABLED\"}"
				<< "}"
				<< "}";

			id_counter++;
		}
	}
}

// XYZ��CZML�̈ܓx�o�x�A�����ɕϊ�����
void CReflectionSimulator::ConvertXYZToLatLonZ(
	double x, double y, double z,
	int JPZONE, CGeoid* pGeoid,
	double& lat, double& lon, double& h
)
{
	// �ܓx�o�x�ɕϊ�
	CGeoUtil::XYToLatLon(
		JPZONE,	// ���W�n
		y,		// �k
		x,		// ��
		lat,	// �ܓx
		lon		// �o�x
	);
	
	// ����
	h = z;
	if (pGeoid)
	{
		// �W�I�C�h��������Ƃ��͕W����ȉ~�̍��ɕϊ�
		double dGeoidHeight;
		if (pGeoid->Extract(lat, lon, &dGeoidHeight) == CGeoid::NOERROR_)
		{
			h = z + dGeoidHeight;
		}
	}
}

// ���ʃt�@�C���̑S�s��ǂݍ���
bool CReflectionSimulator::ReadResultCSVLines(
	const string& csvfile,		// ����CSV�t�@�C��
	std::vector<string>& lines	// �ǂݍ��ݍs�̃��X�g
)
{
	lines.clear();

	ifstream ifs;
	ifs.open(csvfile);
	if (!ifs.is_open())
		return false;

	string line;
	// 1�s�ڂ̓w�b�_�[���Ȃ̂œǂݔ�΂�
	getline(ifs, line);
	// 2�s�ڈȍ~�̃f�[�^��
	while (getline(ifs, line))
	{
		lines.emplace_back(line);
	}

	ifs.close();

	return true;
}

// ��͌��ʂ��擾����
const CAnalysisReflectionOneDay& CReflectionSimulator::GetResult() const
{
	return m_result;
}

// Exec()���s����
eExitCode CReflectionSimulator::GetExecResult() const
{
	return m_bExec;
}

void CReflectionSimulator::Init()
{
	InitResult();

	if (m_pSunVector)
	{
		delete m_pSunVector;
		m_pSunVector = nullptr;
	}
}

void CReflectionSimulator::InitResult()
{
	m_result.clear();
	m_result.resize(24);	// 24���Ԃ̉�͌���
}

void CReflectionSimulator::SetResult(uint8_t hour, const CAnalysisReflection& result)
{
	m_result[hour] = result;
}

// �S�����ł̉��
bool CReflectionSimulator::AnalyzeBuildings(
	const std::vector<BLDGLIST>& targetBuildings,
	const std::vector<BLDGLIST*>& buildings,
	uint8_t hour,
	CAnalysisReflection& result
)
{
	bool ret = false;	// ��͌��ʂ��Ȃ��Ƃ�false

	for (const auto& targetBldgList : targetBuildings)
	{
		double lat, lon;
		CGeoUtil::MeshIDToLatLon(targetBldgList.meshID, lat, lon);

		HorizontalCoordinate sunPos;
		SetSunVector(lat, lon);
		m_pSunVector->GetPos(hour, sunPos);

		// ���x��0�ȏ�̏ꍇ�̂ݔ��˃V�~�����[�V������͂��s��
		if (sunPos.altitude >= 0)
		{
			// ���ˌ��̃x�N�g��
			CVector3D sunVector;
			m_pSunVector->GetVector(hour, sunVector);

			// �Ώۃ��b�V���̗אڂ��郁�b�V�����擾
			vector<BLDGLIST> neighborBuildings;
			GetNeighborBuildings(targetBldgList, buildings, neighborBuildings);

			for (const auto& building : targetBldgList.buildingList)
			{
				// ����ID�̏��������L�^����
				m_buildingIndex++;
				if (m_mapBuildingIndex.find(building.building) == m_mapBuildingIndex.end())
					m_mapBuildingIndex[building.building] = m_buildingIndex;

				// 1�����̔��ˉ��
				if (AnalyzeBuilding(building, neighborBuildings, sunVector, result))
					ret = true;

				// �L�����Z��
				if (IsCancel())
				{
					return false;
				}
			}
		}
	}

	return ret;
}

// 1�����ł̉��
bool CReflectionSimulator::AnalyzeBuilding(
	const BUILDINGS& building,
	const std::vector<BLDGLIST>& buildings,
	const CVector3D& sunVector,
	CAnalysisReflection& result
)
{
	bool ret = false;
	for (const auto& roof : building.roofSurfaceList)
	{
		if (AnalyzeRoof(roof, building, buildings, sunVector, result))
			ret = true;

		// �L�����Z��
		if (IsCancel())
		{
			return false;
		}
	}

	return ret;
}

// 1�����ł̉��
bool CReflectionSimulator::AnalyzeRoof(
	const ROOFSURFACES& roof,
	const BUILDINGS& building,
	const std::vector<BLDGLIST>& buildings,
	const CVector3D& sunVector,
	CAnalysisReflection& result
)
{
	bool ret = false;

	// �����ʑS�̂̍��W���X�g���쐬
	vector<CPointBase> posList = {};
	for (const auto& polygon : roof.roofSurfaceList)
	{
		posList.insert(posList.end(), polygon.posList.begin(), polygon.posList.end() - 1);	// �\���_�̎n�_�ƏI�_�͓����_�Ȃ̂ŏ��O����
	}

	for (const auto& mesh : roof.meshPosList)
	{
		CAnalysisReflectionMesh resultMesh;
		bool res = AnalyzeMesh(mesh, posList, /*building,*/ buildings, sunVector, resultMesh);
		if (res)
		{
			resultMesh.reflectionRoof.buildingId = building.building;
			resultMesh.reflectionRoof.roofSurfaceId = roof.roofSurfaceId;
			result.emplace_back(resultMesh);
			ret = true;
		}

		// �L�����Z��
		if (IsCancel())
		{
			return false;
		}
	}

	return ret;
}

// 1�������b�V���ł̉��
bool CReflectionSimulator::AnalyzeMesh(
	const MESHPOSITION_XY& mesh,
	const vector<CPointBase>& posList,
	//const BUILDINGS& building,
	const vector<BLDGLIST>& buildings,
	const CVector3D& sunVector,
	CAnalysisReflectionMesh& result
)
{
	// ���b�V����z���W���Z�o(CLightRay::Intersect()�̕��ʂƐ����̌�_�v�Z���Q�l)
	vector<CVector3D> meshXYZ {
		{mesh.leftDownX, mesh.leftDownY, 0.0},
		{mesh.leftTopX, mesh.leftTopY, 0.0},
		{mesh.rightDownX, mesh.rightDownY, 0.0},
		{mesh.rightTopX, mesh.rightTopY, 0.0}
	};
	// �����ʂ̖@��
	CVector3D n;
	{
		CVector3D vec1;
		CVector3D vec2;
		// [0]����̊e�_�̃x�N�g��
		vector<CVector3D> vecPolyList;
		for (int i = 1; i < posList.size(); ++i)
		{
			CVector3D vec(posList[i], posList[0]);
			vecPolyList.emplace_back(vec);
		}
		sort(
			vecPolyList.begin(),
			vecPolyList.end(),
			[](const CVector3D& x, const CVector3D& y) { return x.Length() > y.Length(); }
		);
		vec1 = vecPolyList[0];
		vec1.Normalize();
		for (const auto& pos : vecPolyList)
		{
			CVector3D tempVec = pos;
			tempVec.Normalize();
			// �����������t�����̂Ƃ��͖@�����܂�Ȃ�
			if (abs(CGeoUtil::InnerProduct(vec1, tempVec)) > 0.999)
				continue;
			vec2 = tempVec;
			break;
		}
		CGeoUtil::OuterProduct(vec1, vec2, n);
		if (n.z < 0.0) n *= -1;
	}

	// ���ʂ̎�
	CVector3D p(posList[0].x, posList[0].y, posList[0].z);
	double d = CGeoUtil::InnerProduct(p, n);
	// �����Ɩ@��
	CVector3D inVec(0.0, 0.0, 1.0);
	double dot = CGeoUtil::InnerProduct(n, inVec);
	CVector3D center;		// ���S�̍��W
	for (auto& meshPos : meshXYZ)
	{
		// ���ʂƐ����̌�_
		CVector3D p0(meshPos.x, meshPos.y, 0.0);
		double t = (d - CGeoUtil::InnerProduct(p0, n)) / dot;
		// ��_
		CVector3D tempPoint = p0 + t * inVec;
		meshPos.z = tempPoint.z;
		// ���S
		center += meshPos;
	}
	center *= 0.25;

	// �����X�΂̕␳
	{
		// �X�Ίp�����߂�
		double degree = acos(CGeoUtil::InnerProduct(CGeoUtil::Normalize(n), CVector3D(0.0, 0.0, 1.0))) * _COEF_RAD_TO_DEG;
		CReflectionCorrect* pRoofCorrect = (degree < 3.0) ? &m_roofCorrectLower : &m_roofCorrectUpper;
		// mesh���X�΂�����
		if (pRoofCorrect->bCustom)
		{
			for (auto& meshPos : meshXYZ)
			{
				CVector3D orgPos(meshPos - center);
				orgPos.z = 0.0;
				CVector3D rotPos;
				double theta = pRoofCorrect->dSlopeAngle * _COEF_DEG_TO_RAD;
				switch (pRoofCorrect->eAzimuth)
				{
				case eDirections::EAST:
					rotPos.x = orgPos.x * cos(-theta);
					rotPos.y = orgPos.y;
					rotPos.z = orgPos.x * sin(-theta);
					break;
				case eDirections::WEST:
					rotPos.x = orgPos.x * cos(theta);
					rotPos.y = orgPos.y;
					rotPos.z = orgPos.x * sin(theta);
					break;
				case eDirections::SOUTH:
					rotPos.x = orgPos.x;
					rotPos.y = orgPos.y * cos(theta);
					rotPos.z = orgPos.y * sin(theta);
					break;
				case eDirections::NORTH:
					rotPos.x = orgPos.x;
					rotPos.y = orgPos.y * cos(-theta);
					rotPos.z = orgPos.y * sin(-theta);
					break;
				default:
					rotPos = orgPos;
					break;
				}
				rotPos += center;
				meshPos = rotPos;
			}
		}
		
	}

	CReflectionSimulatorMesh reflectionSimMesh(m_pParam);
	bool bResult = reflectionSimMesh.Exec(sunVector, meshXYZ, /*building,*/ buildings);
	if (bResult)
		result = reflectionSimMesh.GetResult();

	return bResult;
}

// �y�n�ʂł̉��
bool CReflectionSimulator::AnalyzeLand(
	const AREADATA& targetArea,
	const std::vector<BLDGLIST*>& buildings,
	uint8_t hour,
	CAnalysisReflection& result
)
{
	bool ret = false;	// ��͌��ʂ��Ȃ��Ƃ�false

	double lat, lon;
	int JPZONE = GetINIParam()->GetJPZone();
	CGeoUtil().XYToLatLon(JPZONE, targetArea.bbMinY, targetArea.bbMinX, lat, lon);

	HorizontalCoordinate sunPos;
	SetSunVector(lat, lon);
	m_pSunVector->GetPos(hour, sunPos);

	// ���x��0�ȏ�̏ꍇ�̂ݔ��˃V�~�����[�V������͂��s��
	if (sunPos.altitude >= 0)
	{
		// ���ˌ��̃x�N�g��
		CVector3D sunVector;
		m_pSunVector->GetVector(hour, sunVector);

		// �Ώۃ��b�V���̗אڂ��郁�b�V�����擾
		vector<BLDGLIST> neighborBuildings;
		GetNeighborBuildings(targetArea, buildings, neighborBuildings);

		// �G���AID�̏������L�^����
		m_buildingIndex++;		// �b��F�����ƈꏏ�ɕێ�����
		if (m_mapBuildingIndex.find(targetArea.areaID) == m_mapBuildingIndex.end())
			m_mapBuildingIndex[targetArea.areaID] = m_buildingIndex;

		// �L���ȓy�n�ʑS�̂̍��W���X�g���쐬
		vector<CPointBase> posList = {};
		for (const auto& polygon : targetArea.landSurface.landSurfaceList)
		{
			posList.insert(posList.end(), polygon.posList.begin(), polygon.posList.end());
		}

		// 1�y�n���b�V���̔��ˉ��
		for (const auto& mesh : targetArea.landSurface.meshPosList)
		{
			CAnalysisReflectionMesh resultMesh;
			bool res = AnalyzeMesh(mesh, posList, neighborBuildings, sunVector, resultMesh);
			if (res)
			{
				resultMesh.reflectionRoof.buildingId = targetArea.areaID;		// �G���AID
				resultMesh.reflectionRoof.roofSurfaceId = "";					// ��
				result.emplace_back(resultMesh);
				ret = true;
			}

			// �L�����Z��
			if (IsCancel())
			{
				return false;
			}
		}

	}

	return ret;
}

// �Ώۃ��b�V���̗אڂ��郁�b�V�����擾
void CReflectionSimulator::GetNeighborBuildings(
	const BLDGLIST& targetBuildings,
	const std::vector<BLDGLIST*>& buildings,
	std::vector<BLDGLIST>& neighborBuildings
)
{
	const double DIST = GetINIParam()->GetNeighborBuildDist_Reflection();	// �אڂ���BBox�͈̔�[m]

	// targetBuildings�̒��S
	double targetCenterX = ((int64_t)targetBuildings.bbMaxX + targetBuildings.bbMinX) * 0.5;
	double targetCenterY = ((int64_t)targetBuildings.bbMaxY + targetBuildings.bbMinY) * 0.5;
	// �O�ډ~���a
	double targetR = sqrt((targetCenterX - targetBuildings.bbMaxX) * (targetCenterX - targetBuildings.bbMaxX) +
		(targetCenterY - targetBuildings.bbMaxY) * (targetCenterY - targetBuildings.bbMaxY));

	for (const auto& building : buildings)
	{
		// �͈͓��ɂ��邩
		// bulding�̒��S
		double buildCenterX = ((int64_t)building->bbMaxX + building->bbMinX) * 0.5;
		double buildCenterY = ((int64_t)building->bbMaxY + building->bbMinY) * 0.5;
		// �O�ډ~���a
		double buildR = sqrt((buildCenterX - building->bbMaxX) * (buildCenterX - building->bbMaxX) +
			(buildCenterY - building->bbMaxY) * (buildCenterY - building->bbMaxY));
		// ���S���m�̋���
		double dist = sqrt((buildCenterX - targetCenterX) * (buildCenterX - targetCenterX) + (buildCenterY - targetCenterY) * (buildCenterY - targetCenterY));
		// DIST�ȓ��̋����̂Ƃ��ߗׂƂ���
		if ((dist - targetR - buildR) <= DIST)
		{
			neighborBuildings.emplace_back(*building);
			continue;
		}
	}
}

// �Ώۃ��b�V���̗אڂ��郁�b�V�����擾
void CReflectionSimulator::GetNeighborBuildings(
	const AREADATA& targetArea,
	const std::vector<BLDGLIST*>& buildings,
	std::vector<BLDGLIST>& neighborBuildings
)
{
	const double DIST = GetINIParam()->GetNeighborBuildDist_Reflection();	// �אڂ���BBox�͈̔�[m]

	// targetLand�̒��S
	double targetCenterX = ((int64_t)targetArea.bbMaxX + targetArea.bbMinX) * 0.5;
	double targetCenterY = ((int64_t)targetArea.bbMaxY + targetArea.bbMinY) * 0.5;
	// �O�ډ~���a
	double targetR = sqrt((targetCenterX - targetArea.bbMaxX) * (targetCenterX - targetArea.bbMaxX) +
		(targetCenterY - targetArea.bbMaxY) * (targetCenterY - targetArea.bbMaxY));

	for (const auto& building : buildings)
	{
		// �͈͓��ɂ��邩
		// bulding�̒��S
		double buildCenterX = ((int64_t)building->bbMaxX + building->bbMinX) * 0.5;
		double buildCenterY = ((int64_t)building->bbMaxY + building->bbMinY) * 0.5;
		// �O�ډ~���a
		double buildR = sqrt((buildCenterX - building->bbMaxX) * (buildCenterX - building->bbMaxX) +
			(buildCenterY - building->bbMaxY) * (buildCenterY - building->bbMaxY));
		// ���S���m�̋���
		double dist = sqrt((buildCenterX - targetCenterX) * (buildCenterX - targetCenterX) + (buildCenterY - targetCenterY) * (buildCenterY - targetCenterY));
		// DIST�ȓ��̋����̂Ƃ��ߗׂƂ���
		if ((dist - targetR - buildR) <= DIST)
		{
			neighborBuildings.emplace_back(*building);
			continue;
		}
	}
}

// �L�����Z���`�F�b�N
bool CReflectionSimulator::IsCancel()
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

// ����ID�̏��Ԃ����ʂɐݒ肷��
// �������ɏ��Ԃ�ۑ�����m_mapBuildingIndex�����ʂɐݒ肷��
void CReflectionSimulator::SetBuildingIndex(CAnalysisReflection& result)
{
	for (auto& roofMesh : result)
	{
		// ���ˉ���
		roofMesh.reflectionRoof.buildingIndex = m_mapBuildingIndex[roofMesh.reflectionRoof.buildingId];

		// ���ː�
		roofMesh.reflectionTarget.buildingIndex = m_mapBuildingIndex[roofMesh.reflectionTarget.buildingId];
	}
}
