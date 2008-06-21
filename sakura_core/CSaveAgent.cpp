#include "stdafx.h"
#include "CSaveAgent.h"
#include "doc/CDocVisitor.h"
#include "CWaitCursor.h"
#include "io/CBinaryStream.h"

CSaveAgent::CSaveAgent()
{
}


ECallbackResult CSaveAgent::OnCheckSave(SSaveInfo* pSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//	Jun.  5, 2004 genta
	//	�r���[���[�h�̃`�F�b�N��CEditDoc����㏑���ۑ������Ɉړ�
	//	�����ŏ㏑�������̂�h��
	if( CAppMode::Instance()->IsViewMode() && pSaveInfo->IsSamePath(pcDoc->m_cDocFile.GetFilePath()) ){
		ErrorBeep();
		TopErrorMessage( CEditWnd::Instance()->GetHwnd(), _T("�r���[���[�h�ł͓���t�@�C���ւ̕ۑ��͂ł��܂���B") );
		return CALLBACK_INTERRUPT;
	}

	//�I�v�V�����F���ύX�ł��㏑�����邩
	if( !GetDllShareData().m_Common.m_sFile.m_bEnableUnmodifiedOverwrite ){
		// �㏑���̏ꍇ
		if(pSaveInfo->bOverwriteMode){
			// ���ύX�̏ꍇ�͌x�������o���A�I��
			if(!pcDoc->m_cDocEditor.IsModified() && pSaveInfo->cEol==EOL_NONE){ //�����s�R�[�h�w��ۑ������N�G�X�g���ꂽ�ꍇ�́A�u�ύX�����������́v�Ƃ݂Ȃ�
				CEditApp::Instance()->m_cSoundSet.NeedlessToSaveBeep();
				return CALLBACK_INTERRUPT;
			}
		}
	}

	// �����\�`�F�b�N ######### �X�}�[�g����Ȃ��B�z���g�͏������ݎ��G���[�`�F�b�N���o�@�\��p�ӂ�����
	if(!pSaveInfo->IsSamePath(pcDoc->m_cDocFile.GetFilePath()) || !pcDoc->m_cDocFileOperation._ToDoLock()){ //���O��t���ĕۑ� or ���b�N���ĂȂ�
		CFile cFile;
		cFile.SetFilePath(pSaveInfo->cFilePath);
		try{
			if(fexist(pSaveInfo->cFilePath)){
				if(!cFile.IsFileWritable()){
					throw CError_FileOpen();
				}
			}
			else{
				CBinaryOutputStream out(pSaveInfo->cFilePath);
				out.Close();
				::DeleteFile(pSaveInfo->cFilePath);
			}
		}
		catch(CError_FileOpen){
			ErrorMessage(
				CEditWnd::Instance()->GetHwnd(),
				_T("\'%ts\'\n")
				_T("�t�@�C����ۑ��ł��܂���B\n")
				_T("�p�X�����݂��Ȃ����A���̃A�v���P�[�V�����Ŏg�p����Ă���\��������܂��B"),
				pSaveInfo->cFilePath.c_str()
			);
			return CALLBACK_INTERRUPT;
		}
	}

	return CALLBACK_CONTINUE;
}



void CSaveAgent::OnBeforeSave(const SSaveInfo& sSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//���s�R�[�h����
	CDocVisitor(pcDoc).SetAllEol(sSaveInfo.cEol);
}

void CSaveAgent::OnSave(const SSaveInfo& sSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//�J�L�R
	CWriteManager cWriter;
	CEditApp::Instance()->m_pcVisualProgress->CProgressListener::Listen(&cWriter);
	EConvertResult eSaveResult = cWriter.WriteFile_From_CDocLineMgr(
		pcDoc->m_cDocLineMgr,
		sSaveInfo
	);

	//�Z�[�u���̊m��
	pcDoc->SetFilePathAndIcon( sSaveInfo.cFilePath );
	pcDoc->m_cDocFile.m_sFileInfo.eCharCode = sSaveInfo.eCharCode;
	pcDoc->m_cDocFile.m_sFileInfo.bBomExist = sSaveInfo.bBomExist;
	if(sSaveInfo.cEol.IsValid()){
		pcDoc->m_cDocEditor.SetNewLineCode(sSaveInfo.cEol);
	}
}

void CSaveAgent::OnAfterSave(const SSaveInfo& sSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	/* �X�V��̃t�@�C�������̎擾
	 * CloseHandle�O�ł�FlushFileBuffers���Ă�ł��^�C���X�^���v���X�V
	 * ����Ȃ����Ƃ�����B
	 */
	GetLastWriteTimestamp( pcDoc->m_cDocFile.GetFilePath(), &pcDoc->m_cDocFile.m_sFileInfo.cFileTime );

	// �^�C�v�ʐݒ�̕ύX���w���B
	pcDoc->OnChangeSetting();
}



void CSaveAgent::OnFinalSave(ESaveResult eSaveResult)
{
}