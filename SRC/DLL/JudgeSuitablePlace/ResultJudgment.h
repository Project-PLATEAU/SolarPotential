#pragma once
#include <string>
#include <vector>

// �Ώ�
enum class eTarget
{
	TARGET_NONE = 0,		// �s��(�G���[)
	TARGET_BUILD = 1,		// ����
	TARGET_LAND = 2,		// �y�n
};


// ���z�\���̎��
enum class ePriority
{
	PRIORITY_RANK_UNKNOWN = 0,		// �D��x�����N�Ȃ�
	PRIORITY_RANK_1 = 1,			// �D��x�����N1
	PRIORITY_RANK_2 = 2,			// �D��x�����N2
	PRIORITY_RANK_3 = 3,			// �D��x�����N3
	PRIORITY_RANK_4 = 4,			// �D��x�����N4
	PRIORITY_RANK_5 = 5,			// �D��x�����N5
};

struct ResultJudgment
{
	// �G���AID
	std::string m_strAreaId;
	// ���b�V��ID
	int m_iMeshId;
	// ����ID
	std::string m_strBuildingId;
	// �D��x
	ePriority m_ePriority;
	// �e��������̓K�E�s�K
	std::string m_strSuitable1_1_1;			// �������1_1_1
	std::string m_strSuitable1_1_2;			// �������1_1_2
	std::string m_strSuitable1_2;			// �������1_2
	std::string m_strSuitable1_3;			// �������1_3
	std::string m_strSuitable2_1;			// �������2_1
	std::string m_strSuitable2_2;			// �������2_2
	std::string m_strSuitable2_3;			// �������2_3
	std::string m_strSuitable2_4;			// �������2_4
	std::string m_strSuitable3_1;			// �������3_1
	std::string m_strSuitable3_2;			// �������3_2
	std::string m_strSuitable3_3;			// �������3_3

	// �R���X�g���N�^
	ResultJudgment()
		: m_strAreaId("")
		, m_iMeshId(0)
		, m_strBuildingId("")
		, m_ePriority(ePriority::PRIORITY_RANK_5)
		, m_strSuitable1_1_1("-")
		, m_strSuitable1_1_2("-")
		, m_strSuitable1_2("-")
		, m_strSuitable1_3("-")
		, m_strSuitable2_1("-")
		, m_strSuitable2_2("-")
		, m_strSuitable2_3("-")
		, m_strSuitable2_4("-")
		, m_strSuitable3_1("-")
		, m_strSuitable3_2("-")
		, m_strSuitable3_3("-")
	{
	};
};

class CResultJudgment
{
public:
	CResultJudgment(void);
	~CResultJudgment(void);

	// ���ʃf�[�^�̒ǉ�
	void Add(const ResultJudgment& result)
	{
		m_result.push_back(result);
	}
	// ���ʃf�[�^��
	size_t GetSize()
	{
		return m_result.size();
	};

	// �D��x��ݒ肷��
	void Prioritization();

	// �D��x���擾����
	ePriority GetPriority(std::string strBuildingId);

	// ���茋��CSV�o��
	bool OutputResultCSV(const std::wstring& filepath);

	// �Ώۂ�ݒ肷��
	void SetTarget(eTarget target) { m_eTarget = target; };

private:
	std::vector<ResultJudgment> m_result;
	eTarget m_eTarget;

};




