#pragma once

// �I���R�[�h(�eDLL����)
enum eExitCode
{
	// ����
	Normal = 0,					// ����I��
	Error = 1,					// �����G���[
	Cancel = 2,					// �L�����Z��

	// ��́E�V�~�����[�V����
	Err_KashoData = 10,			// �Ǝ��ԃf�[�^�s���G���[
	Err_NisshoData = 11,		// ���Ǝ��ԃf�[�^�s���G���[
	Err_SnowDepthData = 12,		// �ϐ�[�f�[�^�s���G���[
	Err_NoTarget = 13,			// �Ώۃf�[�^����


};
