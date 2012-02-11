/*!	@file
	@brief �����֘A���̊Ǘ�

	@author Norio Nakatani
	@date	1998/03/13 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, genta, YAZAKI, jepro, novice, asa-o, MIK,
	Copyright (C) 2002, YAZAKI, hor, genta, aroka, frozen, Moca, MIK
	Copyright (C) 2003, MIK, genta, ryoji, Moca, zenryaku, naoh, wmlhq
	Copyright (C) 2004, genta, novice, Moca, MIK, zenryaku
	Copyright (C) 2005, genta, naoh, FILE, Moca, ryoji, D.S.Koba, aroka
	Copyright (C) 2006, genta, ryoji, aroka
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, ryoji, nasukoji, bosagami
	Copyright (C) 2009, nasukoji
	Copyright (C) 2010, ryoji, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>	// Apr. 03, 2003 genta
#include <io.h>
#include <cderr.h> // Nov. 3, 2005 genta
#include "CEditDoc.h"
#include "Debug.h"
#include "Funccode.h"
#include "CRunningTimer.h"
#include "charcode.h"
#include "mymessage.h"
#include "CWaitCursor.h"
#include <dlgs.h>
#include "CShareData.h"
#include "CEditWnd.h"
#include "sakura_rc.h"
#include "etc_uty.h"
#include "global.h"
#include "CFuncInfoArr.h" /// 2002/2/3 aroka
#include "CSMacroMgr.h"///
#include "CMarkMgr.h"///
#include "CDocLine.h" /// 2002/2/3 aroka
#include "CPrintPreview.h"
#include "CDlgFileUpdateQuery.h"
#include <assert.h> /// 2002/11/2 frozen
#include "my_icmp.h" // 2002/11/30 Moca �ǉ�
#include "MY_SP.h" // 2005/11/22 aroka �ǉ�
#include "CLayout.h"	// 2007.08.22 ryoji �ǉ�
#include "CMemoryIterator.h"	// 2007.08.22 ryoji �ǉ�


#define IDT_ROLLMOUSE	1

/*!
	May 12, 2000 genta ���������@�ύX
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
	@note m_pcEditWnd �̓R���X�g���N�^���ł͎g�p���Ȃ����ƁD
*/
CEditDoc::CEditDoc() :
	m_cNewLineCode( EOL_CRLF ),		//	New Line Type
	m_cSaveLineCode( EOL_NONE ),		//	�ۑ�����Line Type
	m_bGrepRunning( FALSE ),		/* Grep������ */
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
//	m_bPrintPreviewMode( FALSE ),	/* ����v���r���[���[�h�� */
	m_nCommandExecNum( 0 ),			/* �R�}���h���s�� */
// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
#if 0
	m_hwndReferer( NULL ),			/* �Q�ƌ��E�B���h�E */
	m_nRefererX( 0 ),				/* �Q�ƌ� �s������̃o�C�g�ʒu�� */
	m_nRefererLine( 0 ),			/* �Q�ƌ��s �܂�Ԃ������̕����s�ʒu */
#endif
	m_bReadOnly( FALSE ),			/* �ǂݎ���p���[�h */
	m_bDebugMode( FALSE ),			/* �f�o�b�O���j�^���[�h */
	m_bGrepMode( FALSE ),			/* Grep���[�h�� */
	m_nCharCode( 0 ),				/* �����R�[�h��� */
	m_bBomExist( FALSE ),			//	Jul. 26, 2003 ryoji BOM
	m_nActivePaneIndex( 0 ),
//@@@ 2002.01.14 YAZAKI �s�g�p�̂���
//	m_pcOpeBlk( NULL ),				/* ����u���b�N */
	m_bDoing_UndoRedo( FALSE ),		/* �A���h�D�E���h�D�̎��s���� */
	m_nFileShareModeOld( 0 ),		/* �t�@�C���̔r�����䃂�[�h */
	m_hLockedFile( NULL ),			/* ���b�N���Ă���t�@�C���̃n���h�� */
	m_pszAppName( "EditorClient" ),
	m_hInstance( NULL ),
	m_hWnd( NULL ),
	m_eWatchUpdate( CEditDoc::WU_QUERY ),
	m_nSettingTypeLocked( false ),	//	�ݒ�l�ύX�\�t���O
	m_nSettingType( 0 ),	// Sep. 11, 2002 genta
	m_bInsMode( true ),	// Oct. 2, 2005 genta
	m_bIsModified( false ),	/* �ύX�t���O */ // Jan. 22, 2002 genta �^�ύX
	m_pcDragSourceView( NULL )
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditDoc::CEditDoc" );
//	m_pcDlgTest = new CDlgTest;

	m_szFilePath[0] = '\0';			/* ���ݕҏW���̃t�@�C���̃p�X */
	m_szSaveFilePath[0] = '\0';			/* �ۑ����̃t�@�C���̃p�X�i�}�N���p�j */	// 2006.09.04 ryoji
	strcpy( m_szGrepKey, "" );
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */

	m_pShareData = CShareData::getInstance()->GetShareData();
	//	Sep. 11, 2002 genta �폜
	//	SetDocumentType�̓R���X�g���N�^���ł͎g��Ȃ��D
	//int doctype = CShareData::getInstance()->GetDocumentType( GetFilePath() );
	//SetDocumentType( doctype, true );

	m_nEditViewCount = 0;

	/* ���C�A�E�g�Ǘ����̏����� */
	m_cLayoutMgr.Create( this, &m_cDocLineMgr );
	/* ���C�A�E�g���̕ύX */
//	STypeConfig& ref = GetDocumentAttribute();
	// 2008.06.07 nasukoji	�܂�Ԃ����@�̒ǉ��ɑΉ�
	// �u�w�茅�Ő܂�Ԃ��v�ȊO�̎��͐܂�Ԃ�����MAXLINESIZE�ŏ���������
	// �u�E�[�Ő܂�Ԃ��v�́A���̌��OnSize()�ōĐݒ肳���
	STypeConfig ref = GetDocumentAttribute();
	if( ref.m_nTextWrapMethod != WRAP_SETTING_WIDTH )
		ref.m_nMaxLineSize = MAXLINESIZE;
	
	m_cLayoutMgr.SetLayoutInfo(
		TRUE,
		NULL,/*hwndProgress*/
		ref
	);
//	MYTRACE( "CEditDoc::CEditDoc()�����\n" );

	//	Aug, 21, 2000 genta
	//	�����ۑ��̐ݒ�
	ReloadAutoSaveParam();

	//	Sep, 29, 2001 genta
	//	�}�N��
	m_pcSMacroMgr = new CSMacroMgr;
	//strcpy(m_pszCaption, "sakura");	//@@@	YAZAKI
	
	//	m_FileTime�̏�����
	m_FileTime.dwLowDateTime = 0;
	m_FileTime.dwHighDateTime = 0;

	//	Oct. 2, 2005 genta �}�����[�h
	SetInsMode( m_pShareData->m_Common.m_bIsINSMode != FALSE );

	// 2008.06.07 nasukoji	�e�L�X�g�̐܂�Ԃ����@��������
	m_nTextWrapMethodCur = GetDocumentAttribute().m_nTextWrapMethod;	// �܂�Ԃ����@
	m_bTextWrapMethodCurTemp = false;									// �ꎞ�ݒ�K�p��������

	return;
}


CEditDoc::~CEditDoc()
{
//	delete (CDialog*)m_pcDlgTest;
//	m_pcDlgTest = NULL;

	if( m_hWnd != NULL ){
		DestroyWindow( m_hWnd );
	}
	/* �t�@�C���̔r�����b�N���� */
	delete m_pcSMacroMgr;
	DoFileUnLock();
	return;
}





/////////////////////////////////////////////////////////////////////////////
//
//	CEditDoc::Create
//	BOOL Create(HINSTANCE hInstance, HWND hwndParent)
//
//	����
//	  �E�B���h�E�̍쐬��
//
//	@date Sep. 29, 2001 genta �}�N���N���X��n���悤��
//	@date 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
/////////////////////////////////////////////////////////////////////////////
BOOL CEditDoc::Create(
	HINSTANCE hInstance,
	HWND hwndParent,
	CImageListMgr* pcIcons
 )
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditDoc::Create" );

	HWND		hWndArr[2];
	CEditWnd*	pCEditWnd;
	m_hInstance = hInstance;
	m_hwndParent = hwndParent;

	/* �����t���[���쐬 */
	pCEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta
	m_cSplitterWnd.Create( m_hInstance, m_hwndParent, pCEditWnd );

	/* �r���[ */
	m_cEditViewArr[0].Create( m_hInstance, m_cSplitterWnd.m_hWnd, this, 0, /*FALSE,*/ TRUE  );
	m_nEditViewCount = 1;

#if 0
	YAZAKI �s�v�ȏ����Ǝv����B
	m_cEditViewArr[0].OnKillFocus();
	m_cEditViewArr[1].OnKillFocus();
	m_cEditViewArr[2].OnKillFocus();
	m_cEditViewArr[3].OnKillFocus();
#endif

	m_cEditViewArr[0].OnSetFocus();

	/* �q�E�B���h�E�̐ݒ� */
	hWndArr[0] = m_cEditViewArr[0].m_hWnd;
	hWndArr[1] = NULL;
	m_cSplitterWnd.SetChildWndArr( hWndArr );
	m_hWnd = m_cSplitterWnd.m_hWnd;

	MY_TRACETIME( cRunningTimer, "View created" );

	//	Oct. 2, 2001 genta
	m_cFuncLookup.Init( m_hInstance, m_pShareData->m_MacroTable, &m_pShareData->m_Common );

	/* �ݒ�v���p�e�B�V�[�g�̏������P */
	m_cPropCommon.Create( m_hInstance, m_hWnd, pcIcons, m_pcSMacroMgr, &(pCEditWnd->m_CMenuDrawer) );
	m_cPropTypes.Create( m_hInstance, m_hWnd );

	MY_TRACETIME( cRunningTimer, "End: PropSheet" );

	/* ���͕⊮�E�B���h�E�쐬 */
	m_cHokanMgr.DoModeless( m_hInstance, m_cEditViewArr[0].m_hWnd, (LPARAM)&(m_cEditViewArr[0]) );

	return TRUE;
}





/*
|| ���b�Z�[�W�f�B�X�p�b�`��
*/
LRESULT CEditDoc::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	switch( uMsg ){
	case WM_ENTERMENULOOP:
	case WM_EXITMENULOOP:
		{
			int i;
			for( i = 0; i < GetAllViewCount(); ++i ){
				m_cEditViewArr[i].DispatchEvent( hwnd, uMsg, wParam, lParam );
			}
		}
		return 0L;
	default:
		return m_cEditViewArr[m_nActivePaneIndex].DispatchEvent( hwnd, uMsg, wParam, lParam );
	}
}



void CEditDoc::OnMove( int x, int y, int nWidth, int nHeight )
{
//	m_cSplitterWnd.OnMove( x, y, nWidth, nHeight );
	::MoveWindow( m_cSplitterWnd.m_hWnd, x, y, nWidth, nHeight, TRUE );

	return;
}




/*! �e�L�X�g���I������Ă��邩 */
BOOL CEditDoc::IsTextSelected( void )
{
	return m_cEditViewArr[m_nActivePaneIndex].IsTextSelected();
}




BOOL CEditDoc::SelectFont( LOGFONT* plf )
{
	// 2004.02.16 Moca CHOOSEFONT�������o����O��
	CHOOSEFONT cf;
	/* CHOOSEFONT�̏����� */
	::ZeroMemory( &cf, sizeof( CHOOSEFONT ) );
	cf.lStructSize = sizeof( cf );
	cf.hwndOwner = m_hWnd;
	cf.hDC = NULL;
//	cf.lpLogFont = &(m_pShareData->m_Common.m_lf);
	cf.Flags = CF_FIXEDPITCHONLY | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
//#ifdef _DEBUG
//	cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
//#endif
	cf.lpLogFont = plf;
	if( FALSE == ChooseFont( &cf ) ){
#ifdef _DEBUG
		DWORD nErr;
		nErr = CommDlgExtendedError();
		switch( nErr ){
		case CDERR_FINDRESFAILURE:	MYTRACE( "CDERR_FINDRESFAILURE \n" );	break;
		case CDERR_INITIALIZATION:	MYTRACE( "CDERR_INITIALIZATION \n" );	break;
		case CDERR_LOCKRESFAILURE:	MYTRACE( "CDERR_LOCKRESFAILURE \n" );	break;
		case CDERR_LOADRESFAILURE:	MYTRACE( "CDERR_LOADRESFAILURE \n" );	break;
		case CDERR_LOADSTRFAILURE:	MYTRACE( "CDERR_LOADSTRFAILURE \n" );	break;
		case CDERR_MEMALLOCFAILURE:	MYTRACE( "CDERR_MEMALLOCFAILURE\n" );	break;
		case CDERR_MEMLOCKFAILURE:	MYTRACE( "CDERR_MEMLOCKFAILURE \n" );	break;
		case CDERR_NOHINSTANCE:		MYTRACE( "CDERR_NOHINSTANCE \n" );		break;
		case CDERR_NOHOOK:			MYTRACE( "CDERR_NOHOOK \n" );			break;
		case CDERR_NOTEMPLATE:		MYTRACE( "CDERR_NOTEMPLATE \n" );		break;
		case CDERR_STRUCTSIZE:		MYTRACE( "CDERR_STRUCTSIZE \n" );		break;
		case CFERR_MAXLESSTHANMIN:	MYTRACE( "CFERR_MAXLESSTHANMIN \n" );	break;
		case CFERR_NOFONTS:			MYTRACE( "CFERR_NOFONTS \n" );			break;
		}
#endif
		return FALSE;
	}else{
//		MYTRACE( "LOGFONT.lfPitchAndFamily = " );
//		if( plf->lfPitchAndFamily & DEFAULT_PITCH ){
//			MYTRACE( "DEFAULT_PITCH " );
//		}
//		if( plf->lfPitchAndFamily & FIXED_PITCH ){
//			MYTRACE( "FIXED_PITCH " );
//		}
//		if( plf->lfPitchAndFamily & VARIABLE_PITCH ){
//			MYTRACE( "VARIABLE_PITCH " );
//		}
//		if( plf->lfPitchAndFamily & FF_DECORATIVE  ){
//			MYTRACE( "FF_DECORATIVE " );
//		}
//		if( plf->lfPitchAndFamily & FF_DONTCARE ){
//			MYTRACE( "FF_DONTCARE " );
//		}
//		if( plf->lfPitchAndFamily & FF_MODERN ){
//			MYTRACE( "FF_MODERN " );
//		}
//		if( plf->lfPitchAndFamily & FF_ROMAN ){
//			MYTRACE( "FF_ROMAN " );
//		}
//		if( plf->lfPitchAndFamily & FF_SCRIPT ){
//			MYTRACE( "FF_SCRIPT " );
//		}
//		if( plf->lfPitchAndFamily & FF_SWISS ){
//			MYTRACE( "FF_SWISS " );
//		}
//		MYTRACE( "\n" );

//		MYTRACE( "/* LOGFONT�̏����� */\n" );
//		MYTRACE( "memset( &m_pShareData->m_Common.m_lf, 0, sizeof(LOGFONT) );\n" );
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfHeight			= %d;\n", m_pShareData->m_Common.m_lf.lfHeight			);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfWidth			= %d;\n", m_pShareData->m_Common.m_lf.lfWidth			);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfEscapement		= %d;\n", m_pShareData->m_Common.m_lf.lfEscapement		);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfOrientation		= %d;\n", m_pShareData->m_Common.m_lf.lfOrientation		);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfWeight			= %d;\n", m_pShareData->m_Common.m_lf.lfWeight			);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfItalic			= %d;\n", m_pShareData->m_Common.m_lf.lfItalic			);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfUnderline		= %d;\n", m_pShareData->m_Common.m_lf.lfUnderline		);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfStrikeOut		= %d;\n", m_pShareData->m_Common.m_lf.lfStrikeOut		);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfCharSet			= %d;\n", m_pShareData->m_Common.m_lf.lfCharSet			);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfOutPrecision	= %d;\n", m_pShareData->m_Common.m_lf.lfOutPrecision	);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfClipPrecision	= %d;\n", m_pShareData->m_Common.m_lf.lfClipPrecision	);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfQuality			= %d;\n", m_pShareData->m_Common.m_lf.lfQuality			);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfPitchAndFamily	= %d;\n", m_pShareData->m_Common.m_lf.lfPitchAndFamily	);
//		MYTRACE( "strcpy( m_pShareData->m_Common.m_lf.lfFaceName, \"%s\" );\n", m_pShareData->m_Common.m_lf.lfFaceName	);

	}

	return TRUE;
}




/*! �t�@�C�����J��

	@return ����: TRUE/pbOpened==FALSE,
			���ɊJ����Ă���: FALSE/pbOpened==TRUE
			���s: FALSE/pbOpened==FALSE

	@note genta �߂������Ɍ����������������ȁD

	@date 2000.01.18 �V�X�e�������̃t�@�C�����J���Ȃ����
	@date 2000.05,12 genta ���s�R�[�h�̐ݒ�
	@date 2000.10.25 genta �����R�[�h�ُ̈�Ȓl���`�F�b�N
	@date 2000.11.20 genta IME��Ԃ̐ݒ�
	@date 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	@date 2002.01.16 hor �u�b�N�}�[�N����
	@date 2002.10.19 genta �ǂݎ��s�̃t�@�C���͕����R�[�h���ʂŎ��s����
	@date 2003.03.28 MIK ���s�̐^�񒆂ɃJ�[�\�������Ȃ��悤��
	@date 2003.07.26 ryoji BOM�����ǉ�
	@date 2002.05.26 Moca gm_pszCodeNameArr_1 ���g���悤�ɕύX
	@date 2004.06.18 moca �t�@�C�����J���Ȃ������ꍇ��pbOpened��FALSE�ɏ���������Ă��Ȃ������D
	@date 2004.10.09 genta ���݂��Ȃ��t�@�C�����J�����Ƃ����Ƃ���
					�t���O�ɉ����Čx�����o���i�ȑO�̓���j�悤��
	@date 2006.12.16 ���イ�� �O��̕����R�[�h��D�悷��
	@date 2007.03.12 maru �t�@�C�������݂��Ȃ��Ă��O��̕����R�[�h���p��
						���d�I�[�v��������CEditDoc::IsPathOpened�Ɉړ�
*/
BOOL CEditDoc::FileRead(
	char*	pszPath,	//!< [in/out]
	BOOL*	pbOpened,	//!< [out] ���łɊJ����Ă�����
	int		nCharCode,			/*!< [in] �����R�[�h��� */
	BOOL	bReadOnly,			/*!< [in] �ǂݎ���p�� */
	BOOL	bConfirmCodeChange	/*!< [in] �����R�[�h�ύX���̊m�F�����邩�ǂ��� */
)
{
	int				i;
	HWND			hWndOwner;
	BOOL			bRet;
	EditInfo		fi;
//	EditInfo*		pfi;
	HWND			hwndProgress;
	CWaitCursor		cWaitCursor( m_hWnd );
	BOOL			bIsExistInMRU;
	int				nRet;
	BOOL			bFileIsExist;
	int				doctype;

	*pbOpened = FALSE;	// 2004.06.18 Moca �������~�X
	m_bReadOnly = bReadOnly;	/* �ǂݎ���p���[�h */

//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	CMRU			cMRU;

	/* �t�@�C���̑��݃`�F�b�N */
	bFileIsExist = FALSE;
	if( -1 == _access( pszPath, 0 ) ){
	}else{
		HANDLE			hFind;
		WIN32_FIND_DATA	w32fd;
		hFind = ::FindFirstFile( pszPath, &w32fd );
		::FindClose( hFind );
//? 2000.01.18 �V�X�e�������̃t�@�C�����J���Ȃ����
//?		if( w32fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ){
//?		}else{
			bFileIsExist = TRUE;
//?		}
		/* �t�H���_���w�肳�ꂽ�ꍇ */
		if( w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
			/* �w��t�H���_�Łu�J���_�C�A���O�v��\�� */
			{
				char*		pszPathNew = new char[_MAX_PATH];
//				int			nCharCode;
//				BOOL		bReadOnly;

				strcpy( pszPathNew, "" );

				/* �u�t�@�C�����J���v�_�C�A���O */
				nCharCode = CODE_AUTODETECT;	/* �����R�[�h�������� */
				bReadOnly = FALSE;
//				::ShowWindow( m_hWnd, SW_SHOW );
				if( !OpenFileDialog( m_hWnd, pszPath, pszPathNew, &nCharCode, &bReadOnly ) ){
					delete [] pszPathNew;
					return FALSE;
				}
				strcpy( pszPath, pszPathNew );
				delete [] pszPathNew;
				if( -1 == _access( pszPath, 0 ) ){
					bFileIsExist = FALSE;
				}else{
					bFileIsExist = TRUE;
				}
			}
		}

	}

	//	From Here Oct. 19, 2002 genta
	//	�ǂݍ��݃A�N�Z�X���������ꍇ�ɂ͊����R�[�h����Ńt�@�C����
	//	�J���Ȃ��̂ŕ����R�[�h���ʃG���[�Əo�Ă��܂��D
	//	���K�؂ȃ��b�Z�[�W���o�����߁C�ǂ߂Ȃ��t�@�C����
	//	���O�ɔ���E�r������
	//
	//	_access�ł̓��b�N���ꂽ�t�@�C���̏�Ԃ��擾�ł��Ȃ��̂�
	//	���ۂɃt�@�C�����J���Ċm�F����
	if( bFileIsExist){
		HANDLE hTest = 	CreateFile( pszPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
		if( hTest == INVALID_HANDLE_VALUE ){
			// �ǂݍ��݃A�N�Z�X�����Ȃ�
			::MYMESSAGEBOX(
				m_hWnd, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
				_T("\'%s\'\n�Ƃ����t�@�C�����J���܂���B\n�ǂݍ��݃A�N�Z�X��������܂���B"),
				pszPath
			 );
			return FALSE;
		}
		else {
			CloseHandle( hTest );
		}
	}
	//	To Here Oct. 19, 2002 genta

	CEditWnd*	pCEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta
	if( NULL != pCEditWnd ){
		hwndProgress = pCEditWnd->m_hwndProgressBar;
	}else{
		hwndProgress = NULL;
	}
	bRet = TRUE;
	if( NULL == pszPath ){
		MYMESSAGEBOX(
			m_hWnd,
			MB_YESNO | MB_ICONEXCLAMATION | MB_TOPMOST,
			"�o�O���႟�������I�I�I",
			"CEditDoc::FileRead()\n\nNULL == pszPath\n�y�Ώ��z�G���[�̏o���󋵂���҂ɘA�����Ă��������ˁB"
		);
		return FALSE;
	}
	/* �w��t�@�C�����J����Ă��邩���ׂ� */
	if( CShareData::getInstance()->IsPathOpened(pszPath, &hWndOwner, nCharCode) ){	/* 2007.03.12 maru ���d�I�[�v��������IsPathOpened�ɂ܂Ƃ߂� */
		*pbOpened = TRUE;
		bRet = FALSE;
		goto end_of_func;
	}
	for( i = 0; i < GetAllViewCount(); ++i ){
		if( m_cEditViewArr[i].IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			m_cEditViewArr[i].DisableSelectArea( TRUE );
		}
	}

	//	Sep. 10, 2002 genta
	SetFilePath( pszPath ); /* ���ݕҏW���̃t�@�C���̃p�X */


	/* �w�肳�ꂽ�����R�[�h��ʂɕύX���� */
	//	Oct. 25, 2000 genta
	//	�����R�[�h�Ƃ��Ĉُ�Ȓl���ݒ肳�ꂽ�ꍇ�̑Ή�
	//	-1�ȏ�CODE_MAX�����̂ݎ󂯕t����
	//	Oct. 26, 2000 genta
	//	CODE_AUTODETECT�͂��͈̔͂���O��Ă��邩��ʂɃ`�F�b�N
	if( ( -1 <= nCharCode && nCharCode < CODE_CODEMAX ) || nCharCode == CODE_AUTODETECT )
		m_nCharCode = nCharCode;
	
	/* MRU���X�g�ɑ��݂��邩���ׂ�  ���݂���Ȃ�΃t�@�C������Ԃ� */
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	if ( cMRU.GetEditInfo( pszPath, &fi ) ){
		bIsExistInMRU = TRUE;

		if( -1 == m_nCharCode ){
			/* �O��Ɏw�肳�ꂽ�����R�[�h��ʂɕύX���� */
			m_nCharCode = fi.m_nCharCode;
		}
		
		if( !bConfirmCodeChange && ( CODE_AUTODETECT == m_nCharCode ) ){	// �����R�[�h�w��̍ăI�[�v���Ȃ�O��𖳎�
			m_nCharCode = fi.m_nCharCode;
		}
		if( (FALSE == bFileIsExist) && (CODE_AUTODETECT == m_nCharCode) ){
			/* ���݂��Ȃ��t�@�C���̕����R�[�h�w��Ȃ��Ȃ�O����p�� */
			m_nCharCode = fi.m_nCharCode;
		}
	} else {
		bIsExistInMRU = FALSE;
	}

	/* �����R�[�h�������� */
	if( CODE_AUTODETECT == m_nCharCode ) {
		if( FALSE == bFileIsExist ){	/* �t�@�C�������݂��Ȃ� */
			m_nCharCode = 0;
		} else {
			m_nCharCode = CMemory::CheckKanjiCodeOfFile( pszPath );
			if( -1 == m_nCharCode ){
				::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
					"%s\n�����R�[�h�̔��ʏ����ŃG���[���������܂����B",
					pszPath
				);
				//	Sep. 10, 2002 genta
				SetFilePath( "" );
				bRet = FALSE;
				goto end_of_func;
			}
		}
	}
	/* �����R�[�h���قȂ�Ƃ��Ɋm�F���� */
	if( bConfirmCodeChange && bIsExistInMRU ){
		if (m_nCharCode != fi.m_nCharCode ) {	// MRU �̕����R�[�h�Ɣ��ʂ��قȂ�
			char*	pszCodeName = NULL;
			char*	pszCodeNameNew = NULL;

			// gm_pszCodeNameArr_1 ���g���悤�ɕύX Moca. 2002/05/26
			if( -1 < fi.m_nCharCode && fi.m_nCharCode < CODE_CODEMAX ){
				pszCodeName = (char*)gm_pszCodeNameArr_1[fi.m_nCharCode];
			}
			if( -1 < m_nCharCode && m_nCharCode < CODE_CODEMAX ){
				pszCodeNameNew = (char*)gm_pszCodeNameArr_1[m_nCharCode];
			}
			if( pszCodeName != NULL ){
				ConfirmBeep();
				nRet = MYMESSAGEBOX(
					m_hWnd,
					MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
					"�����R�[�h���",
					"%s\n\n���̃t�@�C���́A�O��͕ʂ̕����R�[�h %s �ŊJ����Ă��܂��B\n�O��Ɠ��������R�[�h���g���܂����H\n\n�E[�͂�(Y)]  ��%s\n�E[������(N)]��%s\n�E[�L�����Z��]���J���܂���",
					GetFilePath(), pszCodeName, pszCodeName, pszCodeNameNew
				);
				if( IDYES == nRet ){
					/* �O��Ɏw�肳�ꂽ�����R�[�h��ʂɕύX���� */
					m_nCharCode = fi.m_nCharCode;
				}else
				if( IDCANCEL == nRet ){
					m_nCharCode = 0;
					//	Sep. 10, 2002 genta
					SetFilePath( "" );
					bRet = FALSE;
					goto end_of_func;
				}
			}else{
				MYMESSAGEBOX(
					m_hWnd,
					MB_YESNO | MB_ICONEXCLAMATION | MB_TOPMOST,
					"�o�O���႟�������I�I�I",
					"�y�Ώ��z�G���[�̏o���󋵂���҂ɘA�����Ă��������B"
				);
				//	Sep. 10, 2002 genta
				SetFilePath( "" );
				bRet = FALSE;
				goto end_of_func;
			}
		}
	}
	if( -1 == m_nCharCode ){
		m_nCharCode = 0;
	}

//		if( (_access( pszPath, 0 )) == -1 ){
//			::MYMESSAGEBOX(
//				m_hwndParent,
//				MB_OK | MB_ICONSTOP | MB_TOPMOST,
//				GSTR_APPNAME,
//				"\'%s\'\n�t�@�C���͑��݂��܂���B �V�K�ɍ쐬���܂��B",
//				pszPath
//			);
//
//			strcpy( m_szFilePath, pszPath ); /* ���ݕҏW���̃t�@�C���̃p�X */
//			m_nCharCode = CODE_SJIS;
//
//			return TRUE;
//		}

	//	Nov. 12, 2000 genta �����O�t�@�C�����̎擾��O���Ɉړ�
	char szWork[MAX_PATH];
	/* �����O�t�@�C�������擾���� */
	if( TRUE == ::GetLongFileName( pszPath, szWork ) ){
		//	Sep. 10, 2002 genta
		SetFilePath( szWork );
	}

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();
	doctype = CShareData::getInstance()->GetDocumentType( GetFilePath() );
	SetDocumentType( doctype, true );

	//	From Here Jul. 26, 2003 ryoji BOM�̗L���̏�����Ԃ�ݒ�
	switch( m_nCharCode ){
	case CODE_UNICODE:
	case CODE_UNICODEBE:
		m_bBomExist = TRUE;
		break;
	case CODE_UTF8:
	default:
		m_bBomExist = FALSE;
		break;
	}
	//	To Here Jul. 26, 2003 ryoji BOM�̗L���̏�����Ԃ�ݒ�

	/* �t�@�C�������݂��Ȃ� */
	if( FALSE == bFileIsExist ){

		//	Oct. 09, 2004 genta �t���O�ɉ����Čx�����o���i�ȑO�̓���j�悤��
		if( m_pShareData->m_Common.GetAlertIfFileNotExist() ){
			InfoBeep();

			//	Feb. 15, 2003 genta Popup�E�B���h�E��\�����Ȃ��悤�ɁD
			//	�����ŃX�e�[�^�X���b�Z�[�W���g���Ă���ʂɕ\������Ȃ��D
			::MYMESSAGEBOX(
				m_hwndParent,
				MB_OK | MB_ICONINFORMATION | MB_TOPMOST,
				GSTR_APPNAME,
				"%s\n�Ƃ����t�@�C���͑��݂��܂���B\n\n�t�@�C����ۑ������Ƃ��ɁA�f�B�X�N��ɂ��̃t�@�C�����쐬����܂��B",	//Mar. 24, 2001 jepro �኱�C��
				pszPath
			);
		}
	}else{
		/* �t�@�C����ǂ� */
		if( NULL != hwndProgress ){
			::ShowWindow( hwndProgress, SW_SHOW );
		}
		//	Jul. 26, 2003 ryoji BOM�����ǉ�
		if( FALSE == m_cDocLineMgr.ReadFile( GetFilePath(), m_hWnd, hwndProgress,
			m_nCharCode, &m_FileTime, m_pShareData->m_Common.GetAutoMIMEdecode(), &m_bBomExist ) ){
			//	Sep. 10, 2002 genta
			SetFilePath( "" );
			bRet = FALSE;
			goto end_of_func;
		}
//#ifdef _DEBUG
//		m_cDocLineMgr.DUMP();
//#endif

	}

	/* ���C�A�E�g���̕ύX */
	{
//		STypeConfig& ref = GetDocumentAttribute();
		// 2008.06.07 nasukoji	�܂�Ԃ����@�̒ǉ��ɑΉ�
		// �u�w�茅�Ő܂�Ԃ��v�ȊO�̎��͐܂�Ԃ�����MAXLINESIZE�ŏ���������
		// �u�E�[�Ő܂�Ԃ��v�́A���̌��OnSize()�ōĐݒ肳���
		STypeConfig ref = GetDocumentAttribute();
		if( ref.m_nTextWrapMethod != WRAP_SETTING_WIDTH )
			ref.m_nMaxLineSize = MAXLINESIZE;

		m_cLayoutMgr.SetLayoutInfo(
			TRUE,
			hwndProgress,
			ref
		);
	}

	/* �S�r���[�̏������F�t�@�C���I�[�v��/�N���[�Y�����ɁA�r���[������������ */
	InitAllView();

	//	Nov. 20, 2000 genta
	//	IME��Ԃ̐ݒ�
	SetImeMode( GetDocumentAttribute().m_nImeState );

	if( bIsExistInMRU && m_pShareData->m_Common.GetRestoreCurPosition() ){
		/*
		  �J�[�\���ʒu�ϊ�
		  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		  ��
		  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		*/
		int		nCaretPosX;
		int		nCaretPosY;
		m_cLayoutMgr.CaretPos_Phys2Log(
			fi.m_nX,
			fi.m_nY,
			&nCaretPosX,
			&nCaretPosY
		);
		if( nCaretPosY >= m_cLayoutMgr.GetLineCount() ){
			/*�t�@�C���̍Ō�Ɉړ� */
//			m_cEditViewArr[m_nActivePaneIndex].Command_GOFILEEND(FALSE);
			m_cEditViewArr[m_nActivePaneIndex].HandleCommand( F_GOFILEEND, 0, 0, 0, 0, 0 );
		}else{
			m_cEditViewArr[m_nActivePaneIndex].m_nViewTopLine = fi.m_nViewTopLine; // 2001/10/20 novice
			m_cEditViewArr[m_nActivePaneIndex].m_nViewLeftCol = fi.m_nViewLeftCol; // 2001/10/20 novice
			// From Here Mar. 28, 2003 MIK
			// ���s�̐^�񒆂ɃJ�[�\�������Ȃ��悤�ɁB
			const CDocLine *pTmpDocLine = m_cDocLineMgr.GetLineInfo( fi.m_nY );	// 2008.08.22 ryoji ���s�P�ʂ̍s�ԍ���n���悤�ɏC��
			if( pTmpDocLine ){
				if( pTmpDocLine->GetLengthWithoutEOL() < fi.m_nX ) nCaretPosX--;
			}
			// To Here Mar. 28, 2003 MIK
			m_cEditViewArr[m_nActivePaneIndex].MoveCursor( nCaretPosX, nCaretPosY, TRUE );
			m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX_Prev =
				m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX;
		}
	}
	// 2002.01.16 hor �u�b�N�}�[�N����
	if( bIsExistInMRU ){
		if( m_pShareData->m_Common.GetRestoreBookmarks() ){
			m_cDocLineMgr.SetBookMarks(fi.m_szMarkLines);
		}
	}else{
		strcpy(fi.m_szMarkLines,"");
	}
	SetFileInfo( &fi );

	//	May 12, 2000 genta
	//	���s�R�[�h�̐ݒ�
	{
		SetNewLineCode( EOL_CRLF );
		CDocLine*	pFirstlineinfo = m_cDocLineMgr.GetLineInfo( 0 );
		if( pFirstlineinfo != NULL ){
			EEolType t = (EEolType)pFirstlineinfo->m_cEol;
			if( t != EOL_NONE && t != EOL_UNKNOWN ){
				SetNewLineCode( t );
			}
		}
	}

	/* MRU���X�g�ւ̓o�^ */
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	cMRU.Add( &fi );
	
	/* �J�����g�f�B���N�g���̕ύX */
	{
		char	szCurDir[_MAX_PATH];
		char	szDrive[_MAX_DRIVE], szDir[_MAX_DIR];
		_splitpath( GetFilePath(), szDrive, szDir, NULL, NULL );
		strcpy( szCurDir, szDrive);
		strcat( szCurDir, szDir );
		::SetCurrentDirectory( szCurDir );
	}

end_of_func:;
	//	2004.05.13 Moca ���s�R�[�h�̐ݒ�����炱���Ɉړ�
	m_cEditViewArr[m_nActivePaneIndex].DrawCaretPosInfo();

	if( NULL != hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
	if( TRUE == bRet && IsFilePathAvailable() ){
		/* �t�@�C���̔r�����b�N */
		DoFileLock();
	}
	//	From Here Jul. 26, 2003 ryoji �G���[�̎��͋K���BOM�ݒ�Ƃ���
	if( FALSE == bRet ){
		switch( m_nCharCode ){
		case CODE_UNICODE:
		case CODE_UNICODEBE:
			m_bBomExist = TRUE;
			break;
		case CODE_UTF8:
		default:
			m_bBomExist = FALSE;
			break;
		}
	}
	//	To Here Jul. 26, 2003 ryoji
	return bRet;
}


/*!	@brief �t�@�C���̕ۑ�
	
	@param pszPath [in] �ۑ��t�@�C����
	@param cEolType [in] ���s�R�[�h���
	
	pszPath��NULL�ł����Ă͂Ȃ�Ȃ��B
	
	@date Feb. 9, 2001 genta ���s�R�[�h�p�����ǉ�
*/
BOOL CEditDoc::FileWrite( const char* pszPath, EEolType cEolType )
{
	BOOL		bRet;
	EditInfo	fi;
	HWND		hwndProgress;
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	CMRU		cMRU;
	//	Feb. 9, 2001 genta
	CEol	cEol( cEolType );

	//	Jun.  5, 2004 genta ������ReadOnly�`�F�b�N������ƁC�t�@�C������ύX���Ă�
	//	�ۑ��ł��Ȃ��Ȃ��Ă��܂��̂ŁC�`�F�b�N���㏑���ۑ������ֈړ��D

	//	Sep. 7, 2003 genta
	//	�ۑ�����������܂ł̓t�@�C���X�V�̒ʒm��}������
	WatchUpdate wuSave = m_eWatchUpdate;
	m_eWatchUpdate = WU_NONE;

	bRet = TRUE;

	CEditWnd*	pCEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta
	if( NULL != pCEditWnd ){
		hwndProgress = pCEditWnd->m_hwndProgressBar;
	}else{
		hwndProgress = NULL;
	}
	if( NULL != hwndProgress ){
		::ShowWindow( hwndProgress, SW_SHOW );
	}


	/* �t�@�C���̔r�����b�N���� */
	DoFileUnLock();

	if( m_pShareData->m_Common.m_bBackUp ){	/* �o�b�N�A�b�v�̍쐬 */
		//	Jun.  5, 2004 genta �t�@�C������^����悤�ɁD�߂�l�ɉ�����������ǉ��D
		switch( MakeBackUp( pszPath )){
		case 2:	//	���f�w��
			return FALSE;
		case 3: //	�t�@�C���G���[
			if( IDYES != ::MYMESSAGEBOX(
				m_hWnd,
				MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
				"�t�@�C���ۑ�",
				"�o�b�N�A�b�v�̍쐬�Ɏ��s���܂����D���t�@�C���ւ̏㏑�����p�����čs���܂����D"
			)){
				return FALSE;
			}
		case 4:	//	�o�b�N�A�b�v�t�@�C���̃p�X����������
			if( IDYES != ::MYMESSAGEBOX(
				m_hWnd,
				MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
				"�t�@�C���ۑ�",
				"�t�@�C���p�X���������邽�߃o�b�N�A�b�v�̍쐬�Ɏ��s���܂����D\n"
				"ANSI �łł� %d �o�C�g�ȏ�̐�΃p�X�������܂���D\n\n"
				"���t�@�C���ւ̏㏑�����p�����čs���܂����D",
				_MAX_PATH
			)){
				return FALSE;
			}
		break;
		}
	}

	CWaitCursor cWaitCursor( m_hWnd );
	//	Jul. 26, 2003 ryoji BOM�����ǉ�
	if( FALSE == m_cDocLineMgr.WriteFile( pszPath, m_hWnd, hwndProgress,
		m_nCharCode, &m_FileTime, cEol , m_bBomExist ) ){
		bRet = FALSE;
		goto end_of_func;
	}
	/* �s�ύX��Ԃ����ׂă��Z�b�g */
	m_cDocLineMgr.ResetAllModifyFlag();
	
#if 0
	/* �����O�t�@�C�������擾����B�i�㏑���ۑ��̂Ƃ��̂݁j */
	char szWork[MAX_PATH];
	if( TRUE == ::GetLongFileName( GetFilePath(), szWork ) ){
		//	Sep. 10, 2002 genta
		SetFilePath( szWork );
	}
#endif

	int	v;
	for( v = 0; v < GetAllViewCount(); ++v ){
		if( m_nActivePaneIndex != v ){
			m_cEditViewArr[v].RedrawAll();
		}
	}
	m_cEditViewArr[m_nActivePaneIndex].RedrawAll();

	//	Sep. 10, 2002 genta
	SetFilePath( pszPath ); /* ���ݕҏW���̃t�@�C���̃p�X */

	SetModified(false,false);	//	Jan. 22, 2002 genta �֐��� �X�V�t���O�̃N���A

	//	Mar. 30, 2003 genta �T�u���[�`���ɂ܂Ƃ߂�
	AddToMRU();

	/* ���݈ʒu�Ŗ��ύX�ȏ�ԂɂȂ������Ƃ�ʒm */
	m_cOpeBuf.SetNoModified();

	m_bReadOnly = FALSE;	/* �ǂݎ���p���[�h */

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	SetParentCaption();
end_of_func:;

	if( IsFilePathAvailable() &&
		FALSE == m_bReadOnly && /* �ǂݎ���p���[�h �ł͂Ȃ� */
		TRUE == bRet
	){
		/* �t�@�C���̔r�����b�N */
		DoFileLock();
	}
	if( NULL != hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
	//	Sep. 7, 2003 genta
	//	�t�@�C���X�V�̒ʒm�����ɖ߂�
	m_eWatchUpdate = wuSave;


	return bRet;
}



/* �u�t�@�C�����J���v�_�C�A���O */
//	Mar. 30, 2003 genta	�t�@�C�������莞�̏����f�B���N�g�����J�����g�t�H���_��
BOOL CEditDoc::OpenFileDialog(
	HWND		hwndParent,
	const char*	pszOpenFolder,	//!< [in]  NULL�ȊO���w�肷��Ə����t�H���_���w��ł���
	char*		pszPath,		//!< [out] �J���t�@�C���̃p�X���󂯎��A�h���X
	int*		pnCharCode,		//!< [out] �w�肳�ꂽ�����R�[�h��ʂ��󂯎��A�h���X
	BOOL*		pbReadOnly		//!< [out] �ǂݎ���p��
)
{
	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( hwndParent );

	const char*	pszDefFolder;
	char*	pszCurDir = NULL;
	char**	ppszMRU;
	char**	ppszOPENFOLDER;
	BOOL	bRet;

	/* MRU���X�g�̃t�@�C���̃��X�g */
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	CMRU cMRU;
	ppszMRU = NULL;
	ppszMRU = new char*[ cMRU.Length() + 1 ];
	cMRU.GetPathList(ppszMRU);


	/* OPENFOLDER���X�g�̃t�@�C���̃��X�g */
//@@@ 2001.12.26 YAZAKI OPENFOLDER���X�g�́ACMRUFolder�ɂ��ׂĈ˗�����
	CMRUFolder cMRUFolder;
	ppszOPENFOLDER = NULL;
	ppszOPENFOLDER = new char*[ cMRUFolder.Length() + 1 ];
	cMRUFolder.GetPathList(ppszOPENFOLDER);

	/* �����t�H���_�̐ݒ� */
	// pszFolder�̓t�H���_�������A�t�@�C�����t���p�X��n���Ă�CDlgOpenFile���ŏ������Ă����
	if( NULL != pszOpenFolder ){
		pszDefFolder = pszOpenFolder;
	}else{
		if( IsFilePathAvailable() ){
			pszDefFolder = GetFilePath();
		// Mar. 28, 2003 genta �J�����g�f�B���N�g����MRU���D�悳����
		//}else if( ppszMRU[0] != NULL && ppszMRU[0][0] != '\0' ){ // Sep. 9, 2002 genta
		//	pszDefFolder = ppszMRU[0];
		}else{ // 2002.10.25 Moca
			int nCurDir;
			pszCurDir = new char[_MAX_PATH];
			nCurDir = ::GetCurrentDirectory( _MAX_PATH, pszCurDir );
			if( 0 == nCurDir || _MAX_PATH < nCurDir ){
				pszDefFolder = "";
			}else{
				pszDefFolder = pszCurDir;
			}
		}
	}
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	m_cDlgOpenFile.Create(
		m_hInstance,
		hwndParent,
		"*.*",
		pszDefFolder,
		(const char **)ppszMRU,
		(const char **)ppszOPENFOLDER
	);
	
	bRet = m_cDlgOpenFile.DoModalOpenDlg( pszPath, pnCharCode, pbReadOnly );

	delete [] ppszMRU;
	delete [] ppszOPENFOLDER;
	delete [] pszCurDir;
	return bRet;
}


//pszOpenFolder pszOpenFolder


/*! �u�t�@�C������t���ĕۑ��v�_�C�A���O

	@param pszPath [out]	�ۑ��t�@�C����
	@param pnCharCode [out]	�ۑ������R�[�h�Z�b�g
	@param pcEol [out]		�ۑ����s�R�[�h

	@date 2001.02.09 genta	���s�R�[�h�����������ǉ�
	@date 2003.03.30 genta	�t�@�C�������莞�̏����f�B���N�g�����J�����g�t�H���_��
	@date 2003.07.20 ryoji	BOM�̗L�������������ǉ�
	@date 2006.11.10 ryoji	���[�U�[�w��̊g���q���󋵈ˑ��ŕω�������
*/
BOOL CEditDoc::SaveFileDialog( char* pszPath, int* pnCharCode, CEol* pcEol, BOOL* pbBomExist )
{
	char**	ppszMRU;		//	�ŋ߂̃t�@�C��
	char**	ppszOPENFOLDER;	//	�ŋ߂̃t�H���_
	const char*	pszDefFolder; // �f�t�H���g�t�H���_
	char*	pszCurDir = NULL;
	char	szDefaultWildCard[_MAX_PATH + 10];	// ���[�U�[�w��g���q
	char	szExt[_MAX_EXT];
	BOOL	bret;

	/* MRU���X�g�̃t�@�C���̃��X�g */
	CMRU cMRU;
	ppszMRU = NULL;
	ppszMRU = new char*[ cMRU.Length() + 1 ];
	cMRU.GetPathList(ppszMRU);

	/* OPENFOLDER���X�g�̃t�@�C���̃��X�g */
	CMRUFolder cMRUFolder;
	ppszOPENFOLDER = NULL;
	ppszOPENFOLDER = new char*[ cMRUFolder.Length() + 1 ];
	cMRUFolder.GetPathList(ppszOPENFOLDER);

	/* �t�@�C���ۑ��_�C�A���O�̏����� */
	/* �t�@�C�����̖����t�@�C����������AppszMRU[0]���f�t�H���g�t�@�C�����Ƃ��āHppszOPENFOLDER����Ȃ��H */
	// �t�@�C�����̖����Ƃ��̓J�����g�t�H���_���f�t�H���g�ɂ��܂��BMar. 30, 2003 genta
	// �f���v�] No.2699 (2003/02/05)
	if( !IsFilePathAvailable() ){
		// 2002.10.25 Moca ����̃R�[�h�𗬗p Mar. 23, 2003 genta
		int nCurDir;
		pszCurDir = new char[_MAX_PATH];
		nCurDir = ::GetCurrentDirectory( _MAX_PATH, pszCurDir );
		if( 0 == nCurDir || _MAX_PATH < nCurDir ){
			pszDefFolder = "";
		}else{
			pszDefFolder = pszCurDir;
		}
		strcpy(szDefaultWildCard, "*.txt");
		if( m_pShareData->m_Common.m_bNoFilterSaveNew )
			strcat(szDefaultWildCard, ";*.*");	// �S�t�@�C���\��
	}else{
		pszDefFolder = GetFilePath();
		_splitpath(GetFilePath(), NULL, NULL, NULL, szExt);
		if( szExt[0] == _T('.') && szExt[1] != _T('\0') ){
			strcpy(szDefaultWildCard, "*");
			strcat(szDefaultWildCard, szExt);
			if( m_pShareData->m_Common.m_bNoFilterSaveFile )
				strcat(szDefaultWildCard, ";*.*");	// �S�t�@�C���\��
		}else{
			strcpy(szDefaultWildCard, "*.*");
		}
	}
	m_cDlgOpenFile.Create( m_hInstance, /*NULL*/m_hWnd, szDefaultWildCard, pszDefFolder,
		(const char **)ppszMRU, (const char **)ppszOPENFOLDER );

	/* �_�C�A���O��\�� */
	//	Jul. 26, 2003 ryoji pbBomExist�ǉ�
	bret = m_cDlgOpenFile.DoModalSaveDlg( pszPath, pnCharCode, pcEol, pbBomExist );

	delete [] ppszMRU;
	delete [] ppszOPENFOLDER;
	delete [] pszCurDir;
	return bret;
}





/*! ���ʐݒ� �v���p�e�B�V�[�g */
BOOL CEditDoc::OpenPropertySheet( int nPageNum/*, int nActiveItem*/ )
{
	int		i;
//	BOOL	bModify;
	
	// 2002.12.11 Moca ���̕����ōs���Ă����f�[�^�̃R�s�[��CPropCommon�Ɉړ��E�֐���
	// ���ʐݒ�̈ꎞ�ݒ�̈��SharaData���R�s�[����
	m_cPropCommon.InitData();
	
	/* �v���p�e�B�V�[�g�̍쐬 */
	if( m_cPropCommon.DoPropertySheet( nPageNum/*, nActiveItem*/ ) ){

		// 2002.12.11 Moca ���̕����ōs���Ă����f�[�^�̃R�s�[��CPropCommon�Ɉړ��E�֐���
		// ShareData �� �ݒ��K�p�E�R�s�[����
		// 2007.06.20 ryoji �O���[�v���ɕύX���������Ƃ��̓O���[�vID�����Z�b�g����
		BOOL bGroup = (m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin);
		m_cPropCommon.ApplyData();
		m_pcSMacroMgr->UnloadAll();	// 2007.10.19 genta �}�N���o�^�ύX�𔽉f���邽�߁C�ǂݍ��ݍς݂̃}�N����j������
		if( bGroup != (m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin ) ){
			CShareData::getInstance()->ResetGroupId();
		}

		/* �A�N�Z�����[�^�e�[�u���̍č쐬 */
		::SendMessage( m_pShareData->m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)0 );

		/* �t�H���g���ς���� */
		for( i = 0; i < GetAllViewCount(); ++i ){
			m_cEditViewArr[i].m_cTipWnd.ChangeFont( &(m_pShareData->m_Common.m_lf_kh) );
		}

		/* �ݒ�ύX�𔽉f������ */
		CShareData::getInstance()->SendMessageToAllEditors( MYWM_CHANGESETTING, (WPARAM)0, (LPARAM)m_hwndParent, m_hwndParent );	/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */

		return TRUE;
	}else{
		return FALSE;
	}
}



/*! �^�C�v�ʐݒ� �v���p�e�B�V�[�g */
BOOL CEditDoc::OpenPropertySheetTypes( int nPageNum, int nSettingType )
{
	m_cPropTypes.SetTypeData( m_pShareData->m_Types[nSettingType] );
	// Mar. 31, 2003 genta �������팸�̂��߃|�C���^�ɕύX��ProperySheet���Ŏ擾����悤��
	//m_cPropTypes.m_CKeyWordSetMgr = m_pShareData->m_CKeyWordSetMgr;

	/* �v���p�e�B�V�[�g�̍쐬 */
	if( m_cPropTypes.DoPropertySheet( nPageNum ) ){
//		/* �ύX���ꂽ���H */
//		if( 0 == memcmp( &m_pShareData->m_Types[nSettingType], &m_cPropTypes.m_Types, sizeof( STypeConfig ) ) ){
//			/* ���ύX */
//			return FALSE;
//		}
//		/* �ύX�t���O(�^�C�v�ʐݒ�) �̃Z�b�g */
//		m_pShareData->m_nTypesModifyArr[nSettingType] = TRUE;
		/* �ύX���ꂽ�ݒ�l�̃R�s�[ */
		int nTextWrapMethodOld = GetDocumentAttribute().m_nTextWrapMethod;
		m_cPropTypes.GetTypeData( m_pShareData->m_Types[nSettingType] );

//		/* �܂�Ԃ��������ύX���ꂽ */
//		if( m_cPropTypes.m_nMaxLineSize_org != m_cPropTypes.m_Types.m_nMaxLineSize){
//			/*�A���h�D�E���h�D�o�b�t�@�̃N���A */
//			/* �S�v�f�̃N���A */
//			m_cOpeBuf.ClearAll();
//		}

		// 2008.06.01 nasukoji	�e�L�X�g�̐܂�Ԃ��ʒu�ύX�Ή�
		// �^�C�v�ʐݒ���Ăяo�����E�B���h�E�ɂ��ẮA�^�C�v�ʐݒ肪�ύX���ꂽ��
		// �܂�Ԃ����@�̈ꎞ�ݒ�K�p�����������ă^�C�v�ʐݒ��L���Ƃ���B
		if( nTextWrapMethodOld != GetDocumentAttribute().m_nTextWrapMethod )		// �ݒ肪�ύX���ꂽ
			m_bTextWrapMethodCurTemp = false;	// �ꎞ�ݒ�K�p��������

		/* �A�N�Z�����[�^�e�[�u���̍č쐬 */
		::SendMessage( m_pShareData->m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)0 );

		/* �ݒ�ύX�𔽉f������ */
		CShareData::getInstance()->SendMessageToAllEditors( MYWM_CHANGESETTING, (WPARAM)0, (LPARAM)m_hwndParent, m_hwndParent );	/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */

		return TRUE;
	}else{
		return FALSE;
	}
}



/* Undo(���ɖ߂�)�\�ȏ�Ԃ��H */
BOOL CEditDoc::IsEnableUndo( void )
{
	return m_cOpeBuf.IsEnableUndo();
}



/*! Redo(��蒼��)�\�ȏ�Ԃ��H */
BOOL CEditDoc::IsEnableRedo( void )
{
	return m_cOpeBuf.IsEnableRedo();
}




/*! �N���b�v�{�[�h����\��t���\���H */
BOOL CEditDoc::IsEnablePaste( void )
{
	UINT uFormatSakuraClip;
	uFormatSakuraClip = ::RegisterClipboardFormat( "SAKURAClip" );

	// 2008/02/16 �N���b�v�{�[�h����̃t�@�C���p�X�\��t���Ή�	bosagami	zlib/libpng license
	if( ::IsClipboardFormatAvailable( CF_OEMTEXT )
	 || ::IsClipboardFormatAvailable( CF_HDROP )
	 || ::IsClipboardFormatAvailable( uFormatSakuraClip )
	){
		return TRUE;
	}
	return FALSE;
}





/*! �e�E�B���h�E�̃^�C�g�����X�V

	@date 2007.03.08 ryoji bKillFocus�p�����[�^������
*/
void CEditDoc::SetParentCaption( void )
{
	if( NULL == m_hWnd ){
		return;
	}
	if( !m_cEditViewArr[m_nActivePaneIndex].m_bDrawSWITCH ){
		return;
	}

	char	pszCap[1024];	//	Nov. 6, 2000 genta �I�[�o�[�w�b�h�y���̂���Heap��Stack�ɕύX

//	/* �A�C�R��������Ă��Ȃ����̓t���p�X */
//	/* �A�C�R��������Ă��鎞�̓t�@�C�����̂� */
//	if( ::IsIconic( m_hWnd ) ){
//		bKillFocus = TRUE;
//	}else{
//		bKillFocus = FALSE;
//	}

	// From Here Apr. 04, 2003 genta / Apr.05 ShareData�̃p�����[�^���p��
	if( !m_pcEditWnd->IsActiveApp() ){	// 2007.03.08 ryoji bKillFocus��IsActiveApp()�ɕύX
		ExpandParameter( m_pShareData->m_Common.m_szWindowCaptionInactive,
			pszCap, sizeof( pszCap ));
	}
	else {
		ExpandParameter( m_pShareData->m_Common.m_szWindowCaptionActive,
			pszCap, sizeof( pszCap ));
	}
	// To Here Apr. 04, 2003 genta

	::SetWindowText( m_hwndParent, pszCap );

	//@@@ From Here 2003.06.13 MIK
	//�^�u�E�C���h�E�̃t�@�C������ʒm
	ExpandParameter( m_pShareData->m_Common.m_szTabWndCaption, pszCap, sizeof( pszCap ));
	m_pcEditWnd->ChangeFileNameNotify( pszCap, m_szFilePath, m_bGrepMode );	// 2006.01.28 ryoji �t�@�C�����AGrep���[�h�p�����[�^��ǉ�
	//@@@ To Here 2003.06.13 MIK

	return;
}




/*! �o�b�N�A�b�v�̍쐬
	@author genta
	@date 2001.06.12 asa-o
		�t�@�C���̎��������Ƀo�b�N�A�b�v�t�@�C�������쐬����@�\
	@date 2001.12.11 MIK �o�b�N�A�b�v�t�@�C�����S�~���ɓ����@�\
	@date 2004.06.05 genta �o�b�N�A�b�v�Ώۃt�@�C���������ŗ^����悤�ɁD
		���O��t���ĕۑ��̎��͎����̃o�b�N�A�b�v������Ă����Ӗ��Ȃ̂ŁD
		�܂��C�o�b�N�A�b�v���ۑ����s��Ȃ��I������ǉ��D
	@date 2005.11.26 aroka �t�@�C����������FormatBackUpPath�ɕ���
	@date 2008.11.23 nasukoji �p�X����������ꍇ�ւ̑Ή��i�߂�l4��ǉ��j

	@retval 0 �o�b�N�A�b�v�쐬���s�D
	@retval 1 �o�b�N�A�b�v�쐬�����D
	@retval 2 �o�b�N�A�b�v�쐬���s�D�ۑ����f�w���D
	@retval 3 �t�@�C������G���[�ɂ��o�b�N�A�b�v�쐬���s�D
	@retval 4 �o�b�N�A�b�v�쐬���s�D�t�@�C���p�X����������D
	
	@todo Advanced mode�ł̐���Ǘ�
*/
int CEditDoc::MakeBackUp( const char* target_file )
{
	char	szPath[_MAX_PATH];
	int		nRet;
	char*	pBase;

	/* �o�b�N�A�b�v�\�[�X�̑��݃`�F�b�N */
	//	Aug. 21, 2005 genta �������݃A�N�Z�X�����Ȃ��ꍇ��
	//	�t�@�C�����Ȃ��ꍇ�Ɠ��l�ɉ������Ȃ�
	if( (_access( target_file, 2 )) == -1 ){
		return 0;
	}

	if( m_pShareData->m_Common.m_bBackUpFolder ){	/* �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� */
		//	Aug. 21, 2005 genta �w��t�H���_���Ȃ��ꍇ�Ɍx��
		if( (_access( m_pShareData->m_Common.m_szBackUpFolder, 0 )) == -1 ){
			if( ::MYMESSAGEBOX(
				m_hWnd,
				MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
				"�o�b�N�A�b�v�G���[",
				"�ȉ��̃o�b�N�A�b�v�t�H���_��������܂���D\n%s\n"
				"�o�b�N�A�b�v���쐬�����ɏ㏑���ۑ����Ă�낵���ł����D",
				m_pShareData->m_Common.m_szBackUpFolder
			) == IDYES ){
				return 0;//	�ۑ��p��
			}
			else {
				return 2;// �ۑ����f
			}
		}
	}

	if( !FormatBackUpPath( szPath, sizeof(szPath), target_file ) ){
		return 4;	// �쐬���ׂ��o�b�N�A�b�v�t�@�C���̃p�X����������
	}

	//@@@ 2002.03.23 start �l�b�g���[�N�E�����[�o�u���h���C�u�̏ꍇ�͂��ݔ��ɕ��荞�܂Ȃ�
	bool dustflag = false;
	if( m_pShareData->m_Common.m_bBackUpDustBox ){
		dustflag = !IsLocalDrive( szPath );
	}
	//@@@ 2002.03.23 end

	if( m_pShareData->m_Common.m_bBackUpDialog ){	/* �o�b�N�A�b�v�̍쐬�O�Ɋm�F */
		ConfirmBeep();
		if( m_pShareData->m_Common.m_bBackUpDustBox && dustflag == false ){	//@@@ 2001.12.11 add start MIK	//2002.03.23
			nRet = ::MYMESSAGEBOX(
				m_hWnd,
				MB_YESNO/*CANCEL*/ | MB_ICONQUESTION | MB_TOPMOST,
				"�o�b�N�A�b�v�쐬�̊m�F",
				"�ύX�����O�ɁA�o�b�N�A�b�v�t�@�C�����쐬���܂��B\n��낵���ł����H  [������(N)] ��I�Ԃƍ쐬�����ɏ㏑���i�܂��͖��O��t���āj�ۑ��ɂȂ�܂��B\n\n%s\n    ��\n%s\n\n�쐬�����o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞�݂܂��B\n",
				target_file,
				szPath
			);
		}else{	//@@@ 2001.12.11 add end MIK
			nRet = ::MYMESSAGEBOX(
				m_hWnd,
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				"�o�b�N�A�b�v�쐬�̊m�F",
				"�ύX�����O�ɁA�o�b�N�A�b�v�t�@�C�����쐬���܂��B\n��낵���ł����H  [������(N)] ��I�Ԃƍ쐬�����ɏ㏑���i�܂��͖��O��t���āj�ۑ��ɂȂ�܂��B\n\n%s\n    ��\n%s\n\n",
				//IsFilePathAvailable() ? GetFilePath() : "�i����j",
				//	Aug. 21, 2005 genta ���݂̃t�@�C���ł͂Ȃ��^�[�Q�b�g�t�@�C�����o�b�N�A�b�v����悤��
				target_file,
				szPath
			);	//Jul. 06, 2001 jepro [���O��t���ĕۑ�] �̏ꍇ������̂Ń��b�Z�[�W���C��
		}	//@@@ 2001.12.11 add MIK
		//	Jun.  5, 2005 genta �߂�l�ύX
		if( IDNO == nRet ){
			return 0;//	�ۑ��p��
		}else if( IDCANCEL == nRet ){
			return 2;// �ۑ����f
		}
	}

	//	From Here Aug. 16, 2000 genta
	//	Jun.  5, 2005 genta 1�̊g���q���c���ł�ǉ�
	if( m_pShareData->m_Common.GetBackupType() == 3 ||
		m_pShareData->m_Common.GetBackupType() == 6 ){
		//	���ɑ��݂���Backup�����炷����
		int				i;

		//	�t�@�C�������p
		HANDLE			hFind;
		WIN32_FIND_DATA	fData;

		pBase = szPath + strlen( szPath ) - 2;	//	2: �g���q�̍Ō��2���̈Ӗ�
		//::MessageBox( NULL, pBase, "���������ꏊ", MB_OK );

		//------------------------------------------------------------------
		//	1. �Y���f�B���N�g������backup�t�@�C����1���T��
		for( i = 0; i <= 99; i++ ){	//	�ő�l�Ɋւ�炸�C99�i2���̍ő�l�j�܂ŒT��
			//	�t�@�C�������Z�b�g
			wsprintf( pBase, "%02d", i );

			hFind = ::FindFirstFile( szPath, &fData );
			if( hFind == INVALID_HANDLE_VALUE ){
				//	�����Ɏ��s���� == �t�@�C���͑��݂��Ȃ�
				break;
			}
			::FindClose( hFind );
			//	���������t�@�C���̑������`�F�b�N
			//	�͖ʓ|���������炵�Ȃ��D
			//	�������O�̃f�B���N�g������������ǂ��Ȃ�̂��낤...
		}
		--i;

		//------------------------------------------------------------------
		//	2. �ő�l���琧����-1�Ԃ܂ł��폜
		int boundary = m_pShareData->m_Common.GetBackupCount();
		boundary = boundary > 0 ? boundary - 1 : 0;	//	�ŏ��l��0
		//::MessageBox( NULL, pBase, "���������ꏊ", MB_OK );

		for( ; i >= boundary; --i ){
			//	�t�@�C�������Z�b�g
			wsprintf( pBase, "%02d", i );
			if( ::DeleteFile( szPath ) == 0 ){
				::MessageBox( m_hWnd, szPath, "�폜���s", MB_OK );
				//	Jun.  5, 2005 genta �߂�l�ύX
				//	���s���Ă��ۑ��͌p��
				return 0;
				//	���s�����ꍇ
				//	��ōl����
			}
		}

		//	���̈ʒu��i�͑��݂���o�b�N�A�b�v�t�@�C���̍ő�ԍ���\���Ă���D

		//	3. ��������0�Ԃ܂ł̓R�s�[���Ȃ���ړ�
		char szNewPath[MAX_PATH];
		char *pNewNrBase;

		strcpy( szNewPath, szPath );
		pNewNrBase = szNewPath + strlen( szNewPath ) - 2;

		for( ; i >= 0; --i ){
			//	�t�@�C�������Z�b�g
			wsprintf( pBase, "%02d", i );
			wsprintf( pNewNrBase, "%02d", i + 1 );

			//	�t�@�C���̈ړ�
			if( ::MoveFile( szPath, szNewPath ) == 0 ){
				//	���s�����ꍇ
				//	��ōl����
				::MessageBox( m_hWnd, szPath, "�ړ����s", MB_OK );
				//	Jun.  5, 2005 genta �߂�l�ύX
				//	���s���Ă��ۑ��͌p��
				return 0;
			}
		}
	}
	//	To Here Aug. 16, 2000 genta

	//::MessageBox( NULL, szPath, "���O�̃o�b�N�A�b�v�t�@�C��", MB_OK );
	/* �o�b�N�A�b�v�̍쐬 */
	//	Aug. 21, 2005 genta ���݂̃t�@�C���ł͂Ȃ��^�[�Q�b�g�t�@�C�����o�b�N�A�b�v����悤��
	char	szDrive[_MAX_DIR];
	char	szDir[_MAX_DIR];
	char	szFname[_MAX_FNAME];
	char	szExt[_MAX_EXT];
	_splitpath( szPath, szDrive, szDir, szFname, szExt );
	char szPath2[MAX_PATH];
	wsprintf( szPath2, "%s%s", szDrive, szDir );
		HANDLE			hFind;
		WIN32_FIND_DATA	fData;

	hFind = ::FindFirstFile( szPath2, &fData );
	if( hFind == INVALID_HANDLE_VALUE ){
		//	�����Ɏ��s���� == �t�@�C���͑��݂��Ȃ�
		::CreateDirectory( szPath2, NULL );
	}
	::FindClose( hFind );

	if( ::CopyFile( target_file, szPath, FALSE ) ){
		/* ����I�� */
		//@@@ 2001.12.11 start MIK
		if( m_pShareData->m_Common.m_bBackUpDustBox && dustflag == false ){	//@@@ 2002.03.23 �l�b�g���[�N�E�����[�o�u���h���C�u�łȂ�
			char	szDustPath[_MAX_PATH+1];
			strcpy(szDustPath, szPath);
			szDustPath[strlen(szDustPath) + 1] = '\0';
			SHFILEOPSTRUCT	fos;
			fos.hwnd   = m_hWnd;
			fos.wFunc  = FO_DELETE;
			fos.pFrom  = szDustPath;
			fos.pTo    = NULL;
			fos.fFlags = FOF_ALLOWUNDO | FOF_SIMPLEPROGRESS | FOF_NOCONFIRMATION;	//�_�C�A���O�Ȃ�
			//fos.fFlags = FOF_ALLOWUNDO | FOF_FILESONLY;
			//fos.fFlags = FOF_ALLOWUNDO;	//�_�C�A���O���\�������B
			fos.fAnyOperationsAborted = true; //false;
			fos.hNameMappings = NULL;
			fos.lpszProgressTitle = NULL; //"�o�b�N�A�b�v�t�@�C�������ݔ��Ɉړ����Ă��܂�...";
			if( ::SHFileOperation(&fos) == 0 ){
				/* ����I�� */
			}else{
				/* �G���[�I�� */
			}
		}
		//@@@ 2001.12.11 end MIK
	}else{
		/* �G���[�I�� */
		//	Jun.  5, 2005 genta �߂�l�ύX
		return 3;
	}
	//	Jun.  5, 2005 genta �߂�l�ύX
	return 1;
}

/*! �o�b�N�A�b�v�̍쐬

	@param[out] szNewPath �o�b�N�A�b�v��p�X��
	@param[in]  dwSize    �o�b�N�A�b�v��p�X���̃o�b�t�@�T�C�Y
	@param[in]  target_file �o�b�N�A�b�v���p�X��

	@author aroka
	@date 2005.11.29 aroka
		MakeBackUp���番���D���������Ƀo�b�N�A�b�v�t�@�C�������쐬����@�\�ǉ�
	@date 2008.11.23 nasukoji	�p�X����������ꍇ�ւ̑Ή�
	@date 2009.10.10 aroka	�K�w���󂢂Ƃ��ɗ�����o�O�̑Ή�

	@retval true
	@retval false	�쐬�����t�@�C���p�X�̒�����dwSize���傫������
	
	@todo Advanced mode�ł̐���Ǘ�
*/
bool CEditDoc::FormatBackUpPath( char* szNewPath, DWORD dwSize, const char* target_file )
{
	char	szDrive[_MAX_DIR];
	char	szDir[_MAX_DIR];
	char	szFname[_MAX_FNAME];
	char	szExt[_MAX_EXT];
	char	szTempPath[1024];		// �p�X���쐬�p�̈ꎞ�o�b�t�@�i_MAX_PATH��肠����x�傫�����Ɓj

	bool	bOverflow = false;		// �o�b�t�@�I�[�o�[�t���[

	/* �p�X�̕��� */
	_splitpath( target_file, szDrive, szDir, szFname, szExt );

	if( m_pShareData->m_Common.m_bBackUpFolder ){	/* �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� */
		strcpy( szTempPath, m_pShareData->m_Common.m_szBackUpFolder );
		/* �t�H���_�̍Ōオ���p����'\\'�łȂ��ꍇ�́A�t������ */
		AddLastYenFromDirectoryPath( szTempPath );
	}
	else{
		wsprintf( szTempPath, "%s%s", szDrive, szDir );
	}

	/* ���΃t�H���_��}�� */
	if( !m_pShareData->m_Common.m_bBackUpPathAdvanced ){
		time_t	ltime;
		struct	tm *today, *gmt;
		char	szTime[64];
		char	szForm[64];
		char*	pBase;

		pBase = szTempPath + strlen( szTempPath );

		/* �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.* */
		switch( m_pShareData->m_Common.GetBackupType() ){
		case 1:
			wsprintf( pBase, "%s.bak", szFname );
			break;
		case 5: //	Jun.  5, 2005 genta 1�̊g���q���c����
			wsprintf( pBase, "%s%s.bak", szFname, szExt );
			break;
		case 2:	//	���t�C����
			_tzset();
			_strdate( szTime );
			time( &ltime );				/* �V�X�e�������𓾂܂� */
			gmt = gmtime( &ltime );		/* �����W�����ɕϊ����� */
			today = localtime( &ltime );/* ���n���Ԃɕϊ����� */

			strcpy( szForm, "" );
			if( m_pShareData->m_Common.GetBackupOpt(BKUP_YEAR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̔N */
				strcat( szForm, "%Y" );
			}
			if( m_pShareData->m_Common.GetBackupOpt(BKUP_MONTH) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̌� */
				strcat( szForm, "%m" );
			}
			if( m_pShareData->m_Common.GetBackupOpt(BKUP_DAY) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̓� */
				strcat( szForm, "%d" );
			}
			if( m_pShareData->m_Common.GetBackupOpt(BKUP_HOUR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̎� */
				strcat( szForm, "%H" );
			}
			if( m_pShareData->m_Common.GetBackupOpt(BKUP_MIN) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕� */
				strcat( szForm, "%M" );
			}
			if( m_pShareData->m_Common.GetBackupOpt(BKUP_SEC) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕b */
				strcat( szForm, "%S" );
			}
			/* YYYYMMDD�����b �`���ɕϊ� */
			strftime( szTime, sizeof( szTime ) - 1, szForm, today );
			wsprintf( pBase, "%s_%s%s", szFname, szTime, szExt );
			break;
	//	2001/06/12 Start by asa-o: �t�@�C���ɕt������t��O��̕ۑ���(�X�V����)�ɂ���
		case 4:	//	���t�C����
			{
				FILETIME	LastWriteTime,
							LocalTime;
				SYSTEMTIME	SystemTime;

				// 2005.10.20 ryoji FindFirstFile���g���悤�ɕύX
				if( ! GetLastWriteTimestamp( target_file, LastWriteTime )){
					LastWriteTime.dwHighDateTime = LastWriteTime.dwLowDateTime = 0;
				}
				::FileTimeToLocalFileTime(&LastWriteTime,&LocalTime);	// ���n�����ɕϊ�
				::FileTimeToSystemTime(&LocalTime,&SystemTime);			// �V�X�e���^�C���ɕϊ�

				strcpy( szTime, "" );
				if( m_pShareData->m_Common.GetBackupOpt(BKUP_YEAR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̔N */
					wsprintf(szTime,"%d",SystemTime.wYear);
				}
				if( m_pShareData->m_Common.GetBackupOpt(BKUP_MONTH) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̌� */
					wsprintf(szTime,"%s%02d",szTime,SystemTime.wMonth);
				}
				if( m_pShareData->m_Common.GetBackupOpt(BKUP_DAY) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̓� */
					wsprintf(szTime,"%s%02d",szTime,SystemTime.wDay);
				}
				if( m_pShareData->m_Common.GetBackupOpt(BKUP_HOUR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̎� */
					wsprintf(szTime,"%s%02d",szTime,SystemTime.wHour);
				}
				if( m_pShareData->m_Common.GetBackupOpt(BKUP_MIN) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕� */
					wsprintf(szTime,"%s%02d",szTime,SystemTime.wMinute);
				}
				if( m_pShareData->m_Common.GetBackupOpt(BKUP_SEC) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕b */
					wsprintf(szTime,"%s%02d",szTime,SystemTime.wSecond);
				}
				wsprintf( pBase, "%s_%s%s", szFname, szTime, szExt );
			}
			break;
	// 2001/06/12 End

		case 3: //	?xx : xx = 00~99, ?�͔C�ӂ̕���
		case 6: //	Jun.  5, 2005 genta 3�̊g���q���c����
			//	Aug. 15, 2000 genta
			//	�����ł͍쐬����o�b�N�A�b�v�t�@�C�����̂ݐ�������D
			//	�t�@�C������Rotation�͊m�F�_�C�A���O�̌�ōs���D
			{
				//	Jun.  5, 2005 genta �g���q���c����悤�ɏ����N�_�𑀍삷��
				char* ptr;
				if( m_pShareData->m_Common.GetBackupType() == 3 ){
					ptr = szExt;
				}
				else {
					ptr = szExt + strlen( szExt );
				}
				*ptr   = '.';
				*++ptr = m_pShareData->m_Common.GetBackupExtChar();
				*++ptr = '0';
				*++ptr = '0';
				*++ptr = '\0';
			}
			wsprintf( pBase, "%s%s", szFname, szExt );
			break;
		}

	}else{ // �ڍאݒ�g�p����
		char szFormat[1024];

		switch( m_pShareData->m_Common.GetBackupTypeAdv() ){
		case 4:	//	�t�@�C���̓��t�C����
			{
				FILETIME	LastWriteTime,
							LocalTime;
				SYSTEMTIME	SystemTime;

				// 2005.10.20 ryoji FindFirstFile���g���悤�ɕύX
				if( ! GetLastWriteTimestamp( target_file, LastWriteTime )){
					LastWriteTime.dwHighDateTime = LastWriteTime.dwLowDateTime = 0;
				}
				::FileTimeToLocalFileTime(&LastWriteTime,&LocalTime);	// ���n�����ɕϊ�
				::FileTimeToSystemTime(&LocalTime,&SystemTime);			// �V�X�e���^�C���ɕϊ�

				GetDateTimeFormat( szFormat, sizeof(szFormat), m_pShareData->m_Common.m_szBackUpPathAdvanced , SystemTime );
			}
			break;
		case 2:	//	���݂̓��t�C����
		default:
			{
				time_t	ltime;
				struct	tm *today;

				time( &ltime );				/* �V�X�e�������𓾂܂� */
				today = localtime( &ltime );/* ���n���Ԃɕϊ����� */

				/* YYYYMMDD�����b �`���ɕϊ� */
				strftime( szFormat, sizeof( szFormat ) - 1, m_pShareData->m_Common.m_szBackUpPathAdvanced , today );
			}
			break;
		}

		{
			// make keys
			// $0-$9�ɑΉ�����t�H���_����؂�o��
			char keybuff[1024];
			strcpy( keybuff, szDir );
			CutLastYenFromDirectoryPath( keybuff );

			const char *folders[10];
			{
				//	Jan. 9, 2006 genta VC6�΍�
				int idx;
				for( idx=0; idx<10; ++idx ){
					folders[idx] = "";		// 2009.10.10 aroka	�K�w���󂢂Ƃ��ɗ�����o�O�̑Ή�
				}
				folders[0] = szFname;

				for( idx=1; idx<10; ++idx ){
					char *cp;
					cp =sjis_strrchr2((unsigned char*)keybuff, '\\', '\\');
					if( cp != NULL ){
						folders[idx] = cp+1;
						*cp = '\0';
					}
					else{
						break;
					}
				}
			}
			{
				// $0-$9��u��
				//strcpy( szNewPath, "" );
				char *q= szFormat;
				const char *q2 = szFormat;
				while( *q ){
					if( *q=='$' ){
						++q;
						if( isdigit(*q) ){
							q[-1] = '\0';

							// 2008.11.25 nasukoji	�o�b�t�@�I�[�o�[�t���[�`�F�b�N
							if( strlen( szTempPath ) + strlen( q2 ) + strlen( folders[*q-'0'] ) >= dwSize ){
								bOverflow = true;
								break;
							}

							if( '\\' == *q2 ){
								// 2010.04.13 Moca \�̏d���`�F�b�N C:\backup\\dir�ɂȂ�̂��ɖh��
								// �������l�b�g���[�N�p�X����菜���ƍ���̂ŁA3�����ȏ�
								int nTempPathLen = strlen( szTempPath );
								if( 3 <= nTempPathLen && *::CharPrev( szTempPath, &szTempPath[nTempPathLen] ) == '\\' ){
									q2 +=1; // \���΂�
								}
							}

							strcat( szTempPath, q2 );
							//if( folders[*q-'0'] != 0 ){	// 2009.10.10 aroka	�o�O�Ή��Ń`�F�b�N�s�v�ɂȂ���
							strcat( szTempPath, folders[*q-'0'] );
							//}
							q2 = q+1;
						}
					}
					++q;
				}

				// 2008.11.25 nasukoji	�o�b�t�@�I�[�o�[�t���[�`�F�b�N
				if( !bOverflow ){
					if( strlen( szTempPath ) + strlen( q2 ) >= dwSize ){
						bOverflow = true;
					}else{
						strcat( szTempPath, q2 );
					}
				}
			}
		}

		if( !bOverflow ){
			char temp[1024];
			char *cp;
			//	2006.03.25 Aroka szExt[0] == '\0'�̂Ƃ��̃I�[�o���������C��
			char *ep = (szExt[0]!=0) ? &szExt[1] : &szExt[0];

			while( strchr( szTempPath, '*' ) ){
				strcpy( temp, szTempPath );
				cp = strchr( temp, '*' );
				*cp = 0;

				// 2008.11.25 nasukoji	�o�b�t�@�I�[�o�[�t���[�`�F�b�N
				if( cp - temp + strlen( ep ) + strlen( cp + 1 ) >= dwSize ){
					bOverflow = true;
					break;
				}

				wsprintf( szTempPath, "%s%s%s", temp, ep, cp+1 );
			}

			if( !bOverflow ){
				//	??�̓o�b�N�A�b�v�A�Ԃɂ������Ƃ���ł͂��邪�C
				//	�A�ԏ����͖�����2���ɂ����Ή����Ă��Ȃ��̂�
				//	�g�p�ł��Ȃ�����?��_�ɕϊ����Ă��������
				while(( cp = strchr( szTempPath, '?' ) ) != NULL){
					*cp = '_';
//					strcpy( temp, szNewPath );
//					cp = strchr( temp, '?' );
//					*cp = 0;
//					wsprintf( szNewPath, "%s00%s", temp, cp+2 );
				}
			}
		}
	}

	// �쐬�����p�X��szNewPath�Ɏ��܂�Ȃ���΃G���[
	if( bOverflow || strlen( szTempPath ) >= dwSize ){
		return false;
	}

	strcpy( szNewPath, szTempPath );	// �쐬�����p�X���R�s�[

	return true;
}

/* �t�@�C���̔r�����b�N */
void CEditDoc::DoFileLock( void )
{
	char*	pszMode;
	int		nAccessMode;
	BOOL	bCheckOnly;

	/* ���b�N���Ă��� */
	if( NULL != m_hLockedFile ){
		/* ���b�N���� */
		::_lclose( m_hLockedFile );
		m_hLockedFile = NULL;
	}

	/* �t�@�C�������݂��Ȃ� */
	if( -1 == _access( GetFilePath(), 0 ) ){
		/* �t�@�C���̔r�����䃂�[�h */
		m_nFileShareModeOld = 0;
		return;
	}else{
		/* �t�@�C���̔r�����䃂�[�h */
		m_nFileShareModeOld = m_pShareData->m_Common.m_nFileShareMode;
	}


	/* �t�@�C�����J���Ă��Ȃ� */
	if( ! IsFilePathAvailable() ){
		return;
	}
	/* �ǂݎ���p���[�h */
	if( TRUE == m_bReadOnly ){
		return;
	}


	nAccessMode = 0;
	if( m_pShareData->m_Common.m_nFileShareMode == OF_SHARE_DENY_WRITE ||
		m_pShareData->m_Common.m_nFileShareMode == OF_SHARE_EXCLUSIVE ){
		bCheckOnly = FALSE;
	}else{
		/* �r�����䂵�Ȃ����ǃ��b�N����Ă��邩�̃`�F�b�N�͍s���̂�return���Ȃ� */
//		return;
		bCheckOnly = TRUE;
	}
	/* �����݋֎~���ǂ������ׂ� */
	if( -1 == _access( GetFilePath(), 2 ) ){	/* �A�N�Z�X���F�������݋��� */
		m_hLockedFile = NULL;
		/* �e�E�B���h�E�̃^�C�g�����X�V */
		SetParentCaption();
		return;
	}


	m_hLockedFile = ::_lopen( GetFilePath(), OF_READWRITE );
	_lclose( m_hLockedFile );
	if( HFILE_ERROR == m_hLockedFile ){
		WarningBeep();
		MYMESSAGEBOX(
			m_hWnd,
			MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST,
			GSTR_APPNAME,
			"%s\n�͌��ݑ��̃v���Z�X�ɂ���ď����݂��֎~����Ă��܂��B",
			IsFilePathAvailable() ? GetFilePath() : "�i����j"
		);
		m_hLockedFile = NULL;
		/* �e�E�B���h�E�̃^�C�g�����X�V */
		SetParentCaption();
		return;
	}
	m_hLockedFile = ::_lopen( GetFilePath(), nAccessMode | m_pShareData->m_Common.m_nFileShareMode );
	if( HFILE_ERROR == m_hLockedFile ){
		switch( m_pShareData->m_Common.m_nFileShareMode ){
		case OF_SHARE_EXCLUSIVE:	/* �ǂݏ��� */
			pszMode = "�ǂݏ����֎~���[�h";
			break;
		case OF_SHARE_DENY_WRITE:	/* ���� */
			pszMode = "�������݋֎~���[�h";
			break;
		default:
			pszMode = "����`�̃��[�h�i��肪����܂��j";
			break;
		}
		WarningBeep();
		MYMESSAGEBOX(
			m_hWnd,
			MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST,
			GSTR_APPNAME,
			"%s\n��%s�Ń��b�N�ł��܂���ł����B\n���݂��̃t�@�C���ɑ΂���r������͖����ƂȂ�܂��B",
			IsFilePathAvailable() ? GetFilePath() : "�i����j",
			pszMode
		);
		/* �e�E�B���h�E�̃^�C�g�����X�V */
		SetParentCaption();
		return;
	}
	/* �r�����䂵�Ȃ����ǃ��b�N����Ă��邩�̃`�F�b�N�͍s���ꍇ */
	if( bCheckOnly ){
		/* ���b�N���������� */
		DoFileUnLock();

	}
	return;
}


/* �t�@�C���̔r�����b�N���� */
void CEditDoc::DoFileUnLock( void )
{
	if( NULL != m_hLockedFile ){
		/* ���b�N���� */
		::_lclose( m_hLockedFile );
		m_hLockedFile = NULL;
		/* �t�@�C���̔r�����䃂�[�h */
		m_nFileShareModeOld = 0;
	}
	return;
}

/*
	C�֐����X�g�쐬��CEditDoc_FuncList1.cpp�ֈړ�
*/

/*! PL/SQL�֐����X�g�쐬 */
void CEditDoc::MakeFuncList_PLSQL( CFuncInfoArr* pcFuncInfoArr )
{
	const char*	pLine;
	int			nLineLen;
	int			nLineCount;
	int			i;
	int			nCharChars;
	char		szWordPrev[100];
	char		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;
	char		szFuncName[100];
	int			nFuncLine;
	int			nFuncId;
	int			nFuncNum;
	int			nFuncOrProc = 0;
	int			nParseCnt = 0;

	szWordPrev[0] = '\0';
	szWord[nWordIdx] = '\0';
	nMode = 0;
	nFuncNum = 0;
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		for( i = 0; i < nLineLen; ++i ){
			/* 1�o�C�g������������������ */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, i );
			if( 0 == nCharChars ){
				nCharChars = 1;
			}
//			if( 1 < nCharChars ){
//				i += (nCharChars - 1);
//				continue;
//			}
			/* �V���O���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( 20 == nMode ){
				if( '\'' == pLine[i] ){
					if( i + 1 < nLineLen && '\'' == pLine[i + 1] ){
						++i;
					}else{
						nMode = 0;
						continue;
					}
				}else{
				}
			}else
			/* �R�����g�ǂݍ��ݒ� */
			if( 8 == nMode ){
				if( i + 1 < nLineLen && '*' == pLine[i] &&  '/' == pLine[i + 1] ){
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* �P��ǂݍ��ݒ� */
			if( 1 == nMode ){
				if( (1 == nCharChars && (
					'_' == pLine[i] ||
					'~' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )
					) )
				 || 2 == nCharChars
				){
//					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);
					}
				}else{
					if( 0 == nParseCnt && 0 == _stricmp( szWord, "FUNCTION" ) ){
						nFuncOrProc = 1;
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;

					}else
					if( 0 == nParseCnt && 0 == _stricmp( szWord, "PROCEDURE" ) ){
						nFuncOrProc = 2;
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}else
					if( 0 == nParseCnt && 0 == _stricmp( szWord, "PACKAGE" ) ){
						nFuncOrProc = 3;
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}else
					if( 1 == nParseCnt && 3 == nFuncOrProc && 0 == _stricmp( szWord, "BODY" ) ){
						nFuncOrProc = 4;
						nParseCnt = 1;
					}else
					if( 1 == nParseCnt ){
						if( 1 == nFuncOrProc ||
							2 == nFuncOrProc ||
							3 == nFuncOrProc ||
							4 == nFuncOrProc ){
							++nParseCnt;
							strcpy( szFuncName, szWord );
//						}else
//						if( 3 == nFuncOrProc ){

						}
					}else
					if( 2 == nParseCnt ){
						if( 0 == _stricmp( szWord, "IS" ) ){
							if( 1 == nFuncOrProc ){
								nFuncId = 11;	/* �t�@���N�V�����{�� */
							}else
							if( 2 == nFuncOrProc ){
								nFuncId = 21;	/* �v���V�[�W���{�� */
							}else
							if( 3 == nFuncOrProc ){
								nFuncId = 31;	/* �p�b�P�[�W�d�l�� */
							}else
							if( 4 == nFuncOrProc ){
								nFuncId = 41;	/* �p�b�P�[�W�{�� */
							}
							++nFuncNum;
							/*
							  �J�[�\���ʒu�ϊ�
							  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
							  ��
							  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
							*/
							int		nPosX;
							int		nPosY;
							m_cLayoutMgr.CaretPos_Phys2Log(
								0,
								nFuncLine - 1,
								&nPosX,
								&nPosY
							);
							pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1, szFuncName, nFuncId );
							nParseCnt = 0;
						}
						if( 0 == _stricmp( szWord, "AS" ) ){
							if( 3 == nFuncOrProc ){
								nFuncId = 31;	/* �p�b�P�[�W�d�l�� */
								++nFuncNum;
								/*
								  �J�[�\���ʒu�ϊ�
								  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
								  ��
								  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
								*/
								int		nPosX;
								int		nPosY;
								m_cLayoutMgr.CaretPos_Phys2Log(
									0,
									nFuncLine - 1,
									&nPosX,
									&nPosY
								);
								pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1 , szFuncName, nFuncId );
								nParseCnt = 0;
							}else
							if( 4 == nFuncOrProc ){
								nFuncId = 41;	/* �p�b�P�[�W�{�� */
								++nFuncNum;
								/*
								  �J�[�\���ʒu�ϊ�
								  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
								  ��
								  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
								*/
								int		nPosX;
								int		nPosY;
								m_cLayoutMgr.CaretPos_Phys2Log(
									0,
									nFuncLine - 1,
									&nPosX,
									&nPosY
								);
								pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1 , szFuncName, nFuncId );
								nParseCnt = 0;
							}
						}
					}
					strcpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = '\0';
					nMode = 0;
					i--;
					continue;
				}
			}else
			/* �L����ǂݍ��ݒ� */
			if( 2 == nMode ){
				if( '_' == pLine[i] ||
					'~' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )||
					'\t' == pLine[i] ||
					 ' ' == pLine[i] ||
					  CR == pLine[i] ||
					  LF == pLine[i] ||
					 '{' == pLine[i] ||
					 '}' == pLine[i] ||
					 '(' == pLine[i] ||
					 ')' == pLine[i] ||
					 ';' == pLine[i] ||
					'\'' == pLine[i] ||
					 '/' == pLine[i] ||
					 '-' == pLine[i]
				){
					strcpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = '\0';
					nMode = 0;
					i--;
					continue;
				}else{
//					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);
					}
				}
			}else
			/* ���߂���P�ꖳ���� */
			if( 999 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( '\t' == pLine[i] ||
					 ' ' == pLine[i] ||
					  CR == pLine[i] ||
					  LF == pLine[i]
				){
					nMode = 0;
					continue;
				}
			}else
			/* �m�[�}�����[�h */
			if( 0 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( '\t' == pLine[i] ||
					 ' ' == pLine[i] ||
					  CR == pLine[i] ||
					  LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen - 1 && '-' == pLine[i] &&  '-' == pLine[i + 1] ){
					break;
				}else
				if( i < nLineLen - 1 && '/' == pLine[i] &&  '*' == pLine[i + 1] ){
					++i;
					nMode = 8;
					continue;
				}else
				if( '\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( ';' == pLine[i] ){
					if( 2 == nParseCnt ){
						if( 1 == nFuncOrProc ){
							nFuncId = 10;	/* �t�@���N�V�����錾 */
						}else{
							nFuncId = 20;	/* �v���V�[�W���錾 */
						}
						++nFuncNum;
						/*
						  �J�[�\���ʒu�ϊ�
						  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
						  ��
						  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						*/
						int		nPosX;
						int		nPosY;
						m_cLayoutMgr.CaretPos_Phys2Log(
							0,
							nFuncLine - 1,
							&nPosX,
							&nPosY
						);
						pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1 , szFuncName, nFuncId );
						nParseCnt = 0;
					}
					nMode = 0;
					continue;
				}else{
					if( (1 == nCharChars && (
						'_' == pLine[i] ||
						'~' == pLine[i] ||
						('a' <= pLine[i] &&	pLine[i] <= 'z' )||
						('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
						('0' <= pLine[i] &&	pLine[i] <= '9' )
						) )
					 || 2 == nCharChars
					){
						strcpy( szWordPrev, szWord );
						nWordIdx = 0;

//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);

						nMode = 1;
					}else{
						strcpy( szWordPrev, szWord );
						nWordIdx = 0;
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';

						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);

						nMode = 2;
					}
				}
			}
			i += (nCharChars - 1);
		}
	}
	return;
}





/*!	�e�L�X�g�E�g�s�b�N���X�g�쐬
	
	@date 2002.04.01 YAZAKI CDlgFuncList::SetText()���g�p����悤�ɉ����B
	@date 2002.11.03 Moca �K�w���ő�l�𒴂���ƃo�b�t�@�I�[�o�[��������̂��C��
		�ő�l�ȏ�͒ǉ������ɖ�������
*/
void CEditDoc::MakeTopicList_txt( CFuncInfoArr* pcFuncInfoArr )
{
	const unsigned char*	pLine;
	int						nLineLen;
	int						nLineCount;
	int						i;
	int						j;
	int						nCharChars;
	int						nCharChars2;
	const char*				pszStarts;
	int						nStartsLen;
	char*					pszText;


	pszStarts = m_pShareData->m_Common.m_szMidashiKigou; 	/* ���o���L�� */
	nStartsLen = lstrlen( pszStarts );

	/*	�l�X�g�̐[���́AnMaxStack���x���܂ŁA�ЂƂ̃w�b�_�́A�Œ�32�����܂ŋ��
		�i32�����܂œ����������瓯�����̂Ƃ��Ĉ����܂��j
	*/
	const int nMaxStack = 32;	//	�l�X�g�̍Ő[
	int nDepth = 0;				//	���܂̃A�C�e���̐[����\�����l�B
	char pszStack[nMaxStack][32];
	char szTitle[32];			//	�ꎞ�̈�
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = (const unsigned char *)m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( NULL == pLine ){
			break;
		}
		for( i = 0; i < nLineLen; ++i ){
			if( pLine[i] == ' ' ||
				pLine[i] == '\t'){
				continue;
			}else
			if( i + 1 < nLineLen && pLine[i] == 0x81 && pLine[i + 1] == 0x40 ){
				++i;
				continue;
			}
			break;
		}
		if( i >= nLineLen ){
			continue;
		}
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( (const char *)pLine, nLineLen, i );
		for( j = 0; j < nStartsLen; j+=nCharChars2 ){
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars2 = CMemory::GetSizeOfChar( pszStarts, nStartsLen, j );
			if( nCharChars == nCharChars2 ){
				if( 0 == memcmp( &pLine[i], &pszStarts[j], nCharChars ) ){
					strncpy( szTitle, &pszStarts[j], nCharChars);	//	szTitle�ɕێ��B
					szTitle[nCharChars] = '\0';
					break;
				}
			}
		}
		if( j >= nStartsLen ){
			continue;
		}
		/* ���o��������(���܂܂�Ă��邱�Ƃ��O��ɂȂ��Ă���! */
		if( nCharChars == 1 && pLine[i] == '(' ){
			if( pLine[i + 1] >= '0' && pLine[i + 1] <= '9' )  {
				strcpy( szTitle, "(0)" );
			}
			else if ( pLine[i + 1] >= 'A' && pLine[i + 1] <= 'Z' ) {
				strcpy( szTitle, "(A)" );
			}
			else if ( pLine[i + 1] >= 'a' && pLine[i + 1] <= 'z' ) {
				strcpy( szTitle, "(a)" );
			}
			else {
				continue;
			}
		}else
		if( 2 == nCharChars ){
			// 2003.06.28 Moca 1���ڂ���n�܂��Ă��Ȃ��Ɠ��ꃌ�x���ƔF�����ꂸ��
			//	�ǂ�ǂ�q�m�[�h�ɂȂ��Ă��܂��̂��C�������ɂ�炸���ꃌ�x���ƔF�������悤��
			/* �S�p���� */
			if( pLine[i] == 0x82 && ( pLine[i + 1] >= 0x4f && pLine[i + 1] <= 0x58 ) ) {
				strcpy( szTitle, "�O" );
			}
			/* �@�`�S */
			else if( pLine[i] == 0x87 && ( pLine[i + 1] >= 0x40 && pLine[i + 1] <= 0x53 ) ){
				strcpy( szTitle, "�@" );
			}
			/* �T�`�] */
			else if( pLine[i] == 0x87 && ( pLine[i + 1] >= 0x54 && pLine[i + 1] <= 0x5d ) ){
				strcpy( szTitle, "�T" );
			}
			// 2003.06.28 Moca ������������K�w��
			//	���������قȂ遁�ԍ����قȂ�ƈقȂ錩�o���L���ƔF������Ă����̂�
			//	�F�����K�w�Ǝ��ʂ����悤��
			else{
				char szCheck[3];
				szCheck[0] = pLine[i];
				szCheck[1] = pLine[i + 1];
				szCheck[2] = '\0';
				/* ��`�\ */
				if( NULL != strstr( "�Z���O�l�ܘZ������\�S����Q��", szCheck ) ){
					strcpy( szTitle, "��" );
				}
			}
		}
		/*	�u���o���L���v�Ɋ܂܂�镶���Ŏn�܂邩�A
			(0�A(1�A...(9�A(A�A(B�A...(Z�A(a�A(b�A...(z
			�Ŏn�܂�s�́A�A�E�g���C�����ʂɕ\������B
		*/
		pszText = new char[nLineLen + 1];
		memcpy( pszText, (const char *)&pLine[i], nLineLen );
		pszText[nLineLen] = '\0';
		for( i = 0; i < (int)lstrlen(pszText); ++i ){
			if( pszText[i] == CR ||
				pszText[i] == LF ){
				pszText[i] = '\0';
			}
		}
		/*
		  �J�[�\���ʒu�ϊ�
		  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		  ��
		  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		*/
		int		nPosX;
		int		nPosY;
		m_cLayoutMgr.CaretPos_Phys2Log(
			0,
			nLineCount,
			&nPosX,
			&nPosY
		);
		/* nDepth���v�Z */
		int k;
		BOOL bAppend;
		bAppend = TRUE;
		for ( k = 0; k < nDepth; k++ ){
			int nResult = strcmp( pszStack[k], szTitle );
			if ( nResult == 0 ){
				break;
			}
		}
		if ( k < nDepth ){
			//	���[�v�r����break;���Ă����B�����܂łɓ������o�������݂��Ă����B
			//	�̂ŁA�������x���ɍ��킹��AppendData.
			nDepth = k;
		}
		else if( nMaxStack > k ){
			//	���܂܂łɓ������o�������݂��Ȃ������B
			//	�̂ŁApszStack�ɃR�s�[����AppendData.
			strcpy(pszStack[nDepth], szTitle);
		}else{
			// 2002.11.03 Moca �ő�l�𒴂���ƃo�b�t�@�I�[�o�[����
			// nDepth = nMaxStack;
			bAppend = FALSE;
		}
		
		if( FALSE != bAppend ){
			pcFuncInfoArr->AppendData( nLineCount + 1, nPosY + 1 , (char *)pszText, 0, nDepth );
			nDepth++;
		}
		delete [] pszText;

	}
	return;
}


/*! ���[���t�@�C����1�s���Ǘ�����\����

	@date 2002.04.01 YAZAKI
*/
struct oneRule {
	char szMatch[256];
	int  nLength;
	char szGroupName[256];
};

/*! ���[���t�@�C����ǂݍ��݁A���[���\���̂̔z����쐬����

	@date 2002.04.01 YAZAKI
	@date 2002.11.03 Moca ����nMaxCount��ǉ��B�o�b�t�@���`�F�b�N������悤�ɕύX
*/
int CEditDoc::ReadRuleFile( const char* pszFilename, oneRule* pcOneRule, int nMaxCount )
{
	long	i;
	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	FILE*	pFile = _tfopen_absini( pszFilename, "r" );
	if( NULL == pFile ){
		return 0;
	}
	char	szLine[LINEREADBUFSIZE];
	const char*	pszDelimit = " /// ";
	const char*	pszKeySeps = ",\0";
	char*	pszWork;
	int nDelimitLen = strlen( pszDelimit );
	int nCount = 0;
	while( NULL != fgets( szLine, sizeof(szLine), pFile ) && nCount < nMaxCount ){
		pszWork = strstr( szLine, pszDelimit );
		if( NULL != pszWork && szLine[0] != ';' ){
			*pszWork = '\0';
			pszWork += nDelimitLen;

			/* �ŏ��̃g�[�N�����擾���܂��B */
			char* pszToken = strtok( szLine, pszKeySeps );
			while( NULL != pszToken ){
//				nRes = _stricmp( pszKey, pszToken );
				for( i = 0; i < (int)lstrlen(pszWork); ++i ){
					if( pszWork[i] == '\r' ||
						pszWork[i] == '\n' ){
						pszWork[i] = '\0';
						break;
					}
				}
				strncpy( pcOneRule[nCount].szMatch, pszToken, 255 );
				strncpy( pcOneRule[nCount].szGroupName, pszWork, 255 );
				pcOneRule[nCount].szMatch[255] = '\0';
				pcOneRule[nCount].szGroupName[255] = '\0';
				pcOneRule[nCount].nLength = strlen(pcOneRule[nCount].szMatch);
				nCount++;
				pszToken = strtok( NULL, pszKeySeps );
			}
		}
	}
	fclose( pFile );
	return nCount;
}

/*! ���[���t�@�C�������ɁA�g�s�b�N���X�g���쐬

	@date 2002.04.01 YAZAKI
	@date 2002.11.03 Moca �l�X�g�̐[�����ő�l�𒴂���ƃo�b�t�@�I�[�o�[��������̂��C��
		�ő�l�ȏ�͒ǉ������ɖ�������
*/
void CEditDoc::MakeFuncList_RuleFile( CFuncInfoArr* pcFuncInfoArr )
{
	const unsigned char*	pLine;
	int						nLineLen;
	int						nLineCount;
	int						i;
	int						j;
	char*					pszText;

	/* ���[���t�@�C���̓��e���o�b�t�@�ɓǂݍ��� */
	const int nRuleSize = 1024;
	oneRule* test = new oneRule[nRuleSize];	//	1024���B // 516*1024 = 528,384 byte
	int nCount = ReadRuleFile(GetDocumentAttribute().m_szOutlineRuleFilename, test, nRuleSize );
	if ( nCount < 1 ){
		return;
	}

	/*	�l�X�g�̐[���́A32���x���܂ŁA�ЂƂ̃w�b�_�́A�Œ�256�����܂ŋ��
		�i256�����܂œ����������瓯�����̂Ƃ��Ĉ����܂��j
	*/
	const int nMaxStack = 32;	//	�l�X�g�̍Ő[
	int nDepth = 0;				//	���܂̃A�C�e���̐[����\�����l�B
	char pszStack[nMaxStack][256];
	char szTitle[256];			//	�ꎞ�̈�
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = (const unsigned char *)m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( NULL == pLine ){
			break;
		}
		for( i = 0; i < nLineLen; ++i ){
			if( pLine[i] == ' ' ||
				pLine[i] == '\t'){
				continue;
			}else
			if( i + 1 < nLineLen && pLine[i] == 0x81 && pLine[i + 1] == 0x40 ){
				++i;
				continue;
			}
			break;
		}
		if( i >= nLineLen ){
			continue;
		}
		for( j = 0; j < nCount; j++ ){
			if ( 0 == strncmp( (const char*)&pLine[i], test[j].szMatch, test[j].nLength ) ){
				strcpy( szTitle, test[j].szGroupName );
				break;
			}
		}
		if( j >= nCount ){
			continue;
		}
		/*	���[���Ƀ}�b�`�����s�́A�A�E�g���C�����ʂɕ\������B
		*/
		pszText = new char[nLineLen + 1];
		memcpy( pszText, (const char *)&pLine[i], nLineLen );
		pszText[nLineLen] = '\0';
		int nTextLen = lstrlen( pszText );
		for( i = 0; i < nTextLen; ++i ){
			if( pszText[i] == CR ||
				pszText[i] == LF ){
				pszText[i] = '\0';
				break;
			}
		}
		/*
		  �J�[�\���ʒu�ϊ�
		  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		  ��
		  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		*/
		int		nPosX;
		int		nPosY;
		m_cLayoutMgr.CaretPos_Phys2Log(
			0,
			nLineCount,
			&nPosX,
			&nPosY
		);
		/* nDepth���v�Z */
		int k;
		BOOL bAppend;
		bAppend = TRUE;
		for ( k = 0; k < nDepth; k++ ){
			int nResult = strcmp( pszStack[k], szTitle );
			if ( nResult == 0 ){
				break;
			}
		}
		if ( k < nDepth ){
			//	���[�v�r����break;���Ă����B�����܂łɓ������o�������݂��Ă����B
			//	�̂ŁA�������x���ɍ��킹��AppendData.
			nDepth = k;
		}
		else if( nMaxStack> k ){
			//	���܂܂łɓ������o�������݂��Ȃ������B
			//	�̂ŁApszStack�ɃR�s�[����AppendData.
			strcpy(pszStack[nDepth], szTitle);
		}else{
			// 2002.11.03 Moca �ő�l�𒴂���ƃo�b�t�@�I�[�o�[�������邩��K������
			// nDepth = nMaxStack;
			bAppend = FALSE;
		}
		
		if( FALSE != bAppend ){
			pcFuncInfoArr->AppendData( nLineCount + 1, nPosY + 1 , (char *)pszText, 0, nDepth );
			nDepth++;
		}
		delete [] pszText;

	}
	delete [] test;
	return;
}



/*! COBOL �A�E�g���C����� */
void CEditDoc::MakeTopicList_cobol( CFuncInfoArr* pcFuncInfoArr )
{
	const unsigned char*	pLine;
	int						nLineLen;
	int						nLineCount;
	int						i;
//	int						j;
//	int						nCharChars;
//	int						nCharChars2;
//	int						nStartsLen;
//	char*					pszText;
	int						k;
//	int						m;
	char					szDivision[1024];
	char					szLabel[1024];
	const char*				pszKeyWord;
	int						nKeyWordLen;
	BOOL					bDivision;

	szDivision[0] = '\0';
	szLabel[0] =  '\0';


	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = (const unsigned char *)m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( NULL == pLine ){
			break;
		}
		/* �R�����g�s�� */
		if( 7 <= nLineLen && pLine[6] == '*' ){
			continue;
		}
		/* ���x���s�� */
		if( 8 <= nLineLen && pLine[7] != ' ' ){
			k = 0;
			for( i = 7; i < nLineLen; ){
				if( pLine[i] == '.'
				 || pLine[i] == CR
				 || pLine[i] == LF
				){
					break;
				}
				szLabel[k] = pLine[i];
				++k;
				++i;
				if( pLine[i - 1] == ' ' ){
					for( ; i < nLineLen; ++i ){
						if( pLine[i] != ' ' ){
							break;
						}
					}
				}
			}
			szLabel[k] = '\0';
//			MYTRACE( "szLabel=[%s]\n", szLabel );



			pszKeyWord = "division";
			nKeyWordLen = lstrlen( pszKeyWord );
			bDivision = FALSE;
			for( i = 0; i <= (int)lstrlen( szLabel ) - nKeyWordLen; ++i ){
				if( 0 == memicmp( &szLabel[i], pszKeyWord, nKeyWordLen ) ){
					szLabel[i + nKeyWordLen] = '\0';
					strcpy( szDivision, szLabel );
					bDivision = TRUE;
					break;
				}
			}
			if( bDivision ){
				continue;
			}
			/*
			  �J�[�\���ʒu�ϊ�
			  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
			  ��
			  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
			*/

			int		nPosX;
			int		nPosY;
			char	szWork[1024];
			m_cLayoutMgr.CaretPos_Phys2Log(
				0,
				nLineCount,
				&nPosX,
				&nPosY
			);
			wsprintf( szWork, "%s::%s", szDivision, szLabel );
			pcFuncInfoArr->AppendData( nLineCount + 1, nPosY + 1 , (char *)szWork, 0 );
		}
	}
	return;
}


/*! �A�Z���u�� �A�E�g���C�����

	@author MIK
	@date 2004.04.12 ��蒼��
*/
void CEditDoc::MakeTopicList_asm( CFuncInfoArr* pcFuncInfoArr )
{
	int nTotalLine;

	nTotalLine = m_cDocLineMgr.GetLineCount();

	for( int nLineCount = 0; nLineCount < nTotalLine; nLineCount++ ){
		const TCHAR* pLine;
		int nLineLen;
		TCHAR* pTmpLine;
		int length;
		int offset;
#define MAX_ASM_TOKEN 2
		TCHAR* token[MAX_ASM_TOKEN];
		int j;
		TCHAR* p;

		//1�s�擾����B
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( pLine == NULL ) break;

		//��Ɨp�ɃR�s�[���쐬����B�o�C�i�����������炻�̌��͒m��Ȃ��B
		pTmpLine = _tcsdup( pLine );
		if( pTmpLine == NULL ) break;
		if( _tcslen( pTmpLine ) >= (unsigned int)nLineLen ){	//�o�C�i�����܂�ł�����Z���Ȃ�̂�...
			pTmpLine[ nLineLen ] = _T('\0');	//�w�蒷�Ő؂�l��
		}

		//�s�R�����g�폜
		p = _tcsstr( pTmpLine, _T(";") );
		if( p ) *p = _T('\0');

		length = _tcslen( pTmpLine );
		offset = 0;

		//�g�[�N���ɕ���
		for( j = 0; j < MAX_ASM_TOKEN; j++ ) token[ j ] = NULL;
		for( j = 0; j < MAX_ASM_TOKEN; j++ ){
			token[ j ] = my_strtok( pTmpLine, length, &offset, _T(" \t\r\n") );
			if( token[ j ] == NULL ) break;
			//�g�[�N���Ɋ܂܂��ׂ������łȂ����H
			if( _tcsstr( token[ j ], _T("\"")) != NULL
			 || _tcsstr( token[ j ], _T("\\")) != NULL
			 || _tcsstr( token[ j ], _T("'" )) != NULL ){
				token[ j ] = NULL;
				break;
			}
		}

		if( token[ 0 ] != NULL ){	//�g�[�N����1�ȏ゠��
			int nFuncId = -1;
			TCHAR* entry_token = NULL;

			length = _tcslen( token[ 0 ] );
			if( length >= 2
			 && token[ 0 ][ length - 1 ] == _T(':') ){	//���x��
				token[ 0 ][ length - 1 ] = _T('\0');
				nFuncId = 51;
				entry_token = token[ 0 ];
			}else
			if( token[ 1 ] != NULL ){	//�g�[�N����2�ȏ゠��
				if( my_stricmp( token[ 1 ], _T("proc") ) == 0 ){	//�֐�
					nFuncId = 50;
					entry_token = token[ 0 ];
				}else
				if( my_stricmp( token[ 1 ], _T("endp") ) == 0 ){	//�֐��I��
					nFuncId = 52;
					entry_token = token[ 0 ];
				//}else
				//if( my_stricmp( token[ 1 ], _T("macro") ) == 0 ){	//�}�N��
				//	nFuncId = -1;
				//	entry_token = token[ 0 ];
				//}else
				//if( my_stricmp( token[ 1 ], _T("struc") ) == 0 ){	//�\����
				//	nFuncId = -1;
				//	entry_token = token[ 0 ];
				}
			}

			if( nFuncId >= 0 ){
				/*
				  �J�[�\���ʒu�ϊ�
				  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
				  ��
				  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
				*/
				int		nPosX;
				int		nPosY;
				m_cLayoutMgr.CaretPos_Phys2Log(
					0,
					nLineCount/*nFuncLine - 1*/,
					&nPosX,
					&nPosY
				);
				pcFuncInfoArr->AppendData( nLineCount + 1/*nFuncLine*/, nPosY + 1, entry_token, nFuncId );
			}
		}

		free( pTmpLine );
	}

	return;
}



/*! �K�w�t���e�L�X�g �A�E�g���C�����

	@author zenryaku
	@date 2003.05.20 zenryaku �V�K�쐬
	@date 2003.05.25 genta �������@�ꕔ�C��
	@date 2003.06.21 Moca �K�w��2�i�ȏ�[���Ȃ�ꍇ���l��
*/
void CEditDoc::MakeTopicList_wztxt(CFuncInfoArr* pcFuncInfoArr)
{
	int levelPrev = 0;

	for(int nLineCount=0;nLineCount<m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		const char*	pLine;
		int			nLineLen;

		pLine = m_cDocLineMgr.GetLineStr(nLineCount,&nLineLen);
		if(!pLine)
		{
			break;
		}
		//	May 25, 2003 genta ���菇���ύX
		if( *pLine == '.' )
		{
			const char* pPos;	//	May 25, 2003 genta
			int			nLength;
			char		szTitle[1024];

			//	�s���I�h�̐����K�w�̐[���𐔂���
			for( pPos = pLine + 1 ; *pPos == '.' ; ++pPos )
				;

			int	nPosX;
			int	nPosY;
			m_cLayoutMgr.CaretPos_Phys2Log(
				0,
				nLineCount,
				&nPosX,
				&nPosY
			);
			
			int level = pPos - pLine;

			// 2003.06.27 Moca �K�w��2�i�ʏ�[���Ȃ�Ƃ��́A����̗v�f��ǉ�
			if( levelPrev < level && level != levelPrev + 1  ){
				int dummyLevel;
				// (����)��}��
				//	�������CTAG�ꗗ�ɂ͏o�͂���Ȃ��悤��
				for( dummyLevel = levelPrev + 1; dummyLevel < level; dummyLevel++ ){
					pcFuncInfoArr->AppendData( nLineCount+1, nPosY+1,
						"(����)", FUNCINFO_NOCLIPTEXT, dummyLevel - 1 );
				}
			}
			levelPrev = level;

			nLength = wsprintf(szTitle,"%d - ", level );
			
			char *pDest = szTitle + nLength; // �������ݐ�
			char *pDestEnd = szTitle + sizeof(szTitle) - 2;
			
			while( pDest < pDestEnd )
			{
				if( *pPos =='\r' || *pPos =='\n' || *pPos == '\0')
				{
					break;
				}
				//	May 25, 2003 genta 2�o�C�g�����̐ؒf��h��
				else if( _IS_SJIS_1( *pPos )){
					*pDest++ = *pPos++;
					*pDest++ = *pPos++;
				}
				else {
					*pDest++ = *pPos++;
				}
			}
			*pDest = '\0';
			pcFuncInfoArr->AppendData(nLineCount+1,nPosY+1,szTitle, 0, level - 1);
		}
	}
}

/*! HTML �A�E�g���C�����

	@author zenryaku
	@date 2003.05.20 zenryaku �V�K�쐬
	@date 2004.04.19 zenryaku ��v�f�𔻒�
	@date 2004.04.20 Moca �R�����g�����ƁA�s���ȏI���^�O�𖳎����鏈����ǉ�
	@date 2008.08.15 aroka ���o���ƒi���̐[�������ǉ� 2008.09.07�C��
*/
void CEditDoc::MakeTopicList_html(CFuncInfoArr* pcFuncInfoArr)
{
	const unsigned char*	pLineBuf;	//	pLineBuf �͍s�S�̂��w���A
	const unsigned char*	pLine;		//	pLine �͏������̕����ȍ~�̕������w���܂��B
	int						nLineLen;
	int						nLineCount;
	int						i;
	int						j;
	int						k;
	bool					bEndTag;
	bool					bCommentTag = false;
	bool					bParaTag = false;	//	2008.08.15 aroka

	/*	�l�X�g�̐[���́AnMaxStack���x���܂ŁA�ЂƂ̃w�b�_�́A�Œ�32�����܂ŋ��
		�i32�����܂œ����������瓯�����̂Ƃ��Ĉ����܂��j
	*/
	const int				nMaxStack = 32;	//	�l�X�g�̍Ő[
	int						nDepth = 0;				//	���܂̃A�C�e���̐[����\�����l�B
	char					pszStack[nMaxStack][32];
	char					szTitle[32];			//	�ꎞ�̈�
	char					szTag[32];				//	�ꎞ�̈�  �������ŕێ����č��������Ă��܂��B
	int						nLabelType;				// default, inlined, ignore, empty, block, p, heading
	/*	�������o���v�f�ihy�j�����ɏ�ʃ��x���̌��o��(hx)�������܂œ����[���ɂ��낦�܂��B
		���̂��߁A���o���̐[�����L�����Ă����܂��B
		���ʃ��x���̌��o���̐[���͌����܂ŕs��ŁA�O�̏͐߂ł̐[���͉e�����܂���B 2008.08.15 aroka
	*/
	int						nHeadDepth[6+1];		// [0]�� �󂯂Ă���
	for(k=0;k<=6;k++){
		nHeadDepth[k] = -1;
	}
	for(nLineCount=0;nLineCount<m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		pLineBuf = (const unsigned char *)m_cDocLineMgr.GetLineStr(nLineCount,&nLineLen);
		if( !pLineBuf )
		{
			break;
		}
		for(i=0;i<nLineLen-1;i++)
		{
			pLine = &pLineBuf[i];
			// 2004.04.20 Moca �R�����g����������
			if( bCommentTag )
			{
				if( i < nLineLen - 3 && 0 == memcmp( "-->", pLine, 3 ) )
				{
					bCommentTag = false;
					i += 2;
					pLine += 2;
				}
				continue;
			}
			// 2004.04.20 Moca To Here
			if( *pLine!='<' || nDepth>=nMaxStack )
			{
				continue;
			}
			bEndTag = false;
			pLine++; i++;
			if( *pLine=='/')
			{
				pLine++; i++;
				bEndTag = true;
			}
			for(j=0;i+j<nLineLen && j<sizeof(szTitle)-1; )
			{
				// �^�O����؂�o��
				// �X�y�[�X�A�^�u�A�u_:-.�p���v�ȊO�̔��p�����A�P�����ڂ́u-.�����v�͔F�߂Ȃ��B
				if( (pLine[j]==' ' || pLine[j]=='\t') ||
					(pLine[j]<0x80 && !strchr("_:-.",pLine[j]) && !isalnum(pLine[j])) ||
					(j==0 &&( (pLine[j]>='0' && pLine[j]<='9') || pLine[j]=='-' || pLine[j]=='.' )) )
				{
					break;
				}
				int nCharSize = CMemory::GetSizeOfChar((char*)pLine, nLineLen-i, j);
				memcpy(szTitle + j, pLine + j, nCharSize);
				j += nCharSize;
			}
			if(j==0)
			{
				// 2004.04.20 Moca From Here �R�����g����������
				if( i < nLineLen - 3 && 0 == memcmp( "!--", pLine, 3 ) )
				{
					bCommentTag = true;
					i += 3;
					pLine += 3;
				}
				// 2004.04.20 Moca To Here
				continue;
			}
			szTitle[j] = '\0';
			/*	�^�O�̎�ނ��Ƃɏ�����ς���K�v�����邪�A
				�s�x��r����̂̓R�X�g�������̂ŁA�ŏ��ɕ��ނ��Ă����B 2008.08.15 aroka
				��r�̉񐔂��������߁A�������ɕϊ����Ă�����strcmp���g���B
			*/
			strcpy( szTag, szTitle );
			_strlwr( szTag );
			
			nLabelType = 'de';//default
			// �����v�f�i�����ڂ�ς��邽�߂̃^�O�j�͍\����͂��Ȃ��B
			if( !strcmp(szTag,"b") || !strcmp(szTag,"big") || !strcmp(szTag,"blink")
			 || !strcmp(szTag,"font") || !strcmp(szTag,"i") || !strcmp(szTag,"marquee")
			 || !strcmp(szTag,"nobr") || !strcmp(szTag,"s") || !strcmp(szTag,"small")
			 || !strcmp(szTag,"strike") || !strcmp(szTag,"tt") || !strcmp(szTag,"u")
			 || !strcmp(szTag,"bdo") || !strcmp(szTag,"sub") || !strcmp(szTag,"sup") )
			{
				nLabelType = 'in';//inlined
			}
			// �C�����C���e�L�X�g�v�f�i�e�L�X�g���C������^�O�j�͍\����͂��Ȃ�?
//			if( !strcmp(szTag,"abbr") || !strcmp(szTag,"acronym") || !strcmp(szTag,"dfn")
//			 || !strcmp(szTag,"em") || !strcmp(szTag,"strong") || !strcmp(szTag,"span")
//			 || !strcmp(szTag,"code") || !strcmp(szTag,"samp") || !strcmp(szTag,"kbd")
//			 || !strcmp(szTag,"var") || !strcmp(szTag,"cite") || !strcmp(szTag,"q") )
//			{
//				nLabelType = 'in';//inlined
//			}
			// ���r�v�f�iXHTML1.1�j�͍\����͂��Ȃ��B
			if( !strcmp(szTag,"rbc") || !strcmp(szTag,"rtc") || !strcmp(szTag,"ruby")
			 || !strcmp(szTag,"rb") || !strcmp(szTag,"rt") || !strcmp(szTag,"rp") )
			{
				nLabelType = 'in';//inlined
			}
			// ��v�f�i���e�������Ȃ��^�O�j�̂����\���Ɋ֌W�Ȃ����͍̂\����͂��Ȃ��B
			if( !strcmp(szTag,"br") || !strcmp(szTag,"base") || !strcmp(szTag,"basefont")
			 || !strcmp(szTag,"frame") )
			{
				nLabelType = 'ig';//empty
			}
			// ��v�f�i���e�������Ȃ��^�O�j�̂����\���Ɋ֌W������́B
			if( !strcmp(szTag,"area") || !strcmp(szTag,"hr") || !strcmp(szTag,"img")
			 || !strcmp(szTag,"input") || !strcmp(szTag,"link") || !strcmp(szTag,"meta")
			 || !strcmp(szTag,"param") )
			{
				nLabelType = 'em';//empty
			}
			if( !strcmp(szTag,"div") || !strcmp(szTag,"center")
			 || !strcmp(szTag,"address") || !strcmp(szTag,"blockquote")
			 || !strcmp(szTag,"noscript") || !strcmp(szTag,"noframes")
			 || !strcmp(szTag,"ol") || !strcmp(szTag,"ul") || !strcmp(szTag,"dl")
			 || !strcmp(szTag,"dir") || !strcmp(szTag,"menu")
			 || !strcmp(szTag,"pre") || !strcmp(szTag,"table")
			 || !strcmp(szTag,"form") || !strcmp(szTag,"fieldset") || !strcmp(szTag,"isindex") )
			{
				nLabelType = 'bl';//block
			}
			if( !strcmp(szTag,"p") )
			{
				nLabelType = 'pa';//paragraph
			}
			if( (szTag[0]=='h') && ('1'<=szTitle[1]&&szTitle[1]<='6') ){
				nLabelType = 'hd';//heading
			}

			// 2009.08.08 syat �u/>�v�ŏI���^�O�̔���̂��߁A�I���^�O�������J�n�^�O�����̌�ɂ����B
			//                  �i�J�n�^�O�����̒��ŁAbEndTag��true�ɂ��Ă��鏊������B�j

			if( ! bEndTag ) // �J�n�^�O
			{
				if( nLabelType!='in' && nLabelType!='ig' ){
					// p�̒��Ńu���b�N�v�f��������A�����I��p�����B 2008.09.07 aroka
					if( bParaTag ){
						if( nLabelType=='hd' || nLabelType=='pa' || nLabelType=='bl' ){
							nDepth--;
						}
					}
					if( nLabelType=='hd' ){
						if( nHeadDepth[szTitle[1]-'0'] != -1 ) // �����o��:���o
						{
							nDepth = nHeadDepth[szTitle[1]-'0'];
							for(k=szTitle[1]-'0';k<=6;k++){
								nHeadDepth[k] = -1;
							}
							nHeadDepth[szTitle[1]-'0'] = nDepth;
							bParaTag = false;
						}
					}
					if( nLabelType=='pa' ){
						bParaTag = true;
					}
					if( nLabelType=='bl' ){
						bParaTag = false;
					}

					int		nPosX;
					int		nPosY;

					m_cLayoutMgr.CaretPos_Phys2Log(
						i,
						nLineCount,
						&nPosX,
						&nPosY
					);

					if( nLabelType!='em' ){
						// �I���^�O�Ȃ��������S�Ẵ^�O�炵�����̂𔻒�
						strcpy(pszStack[nDepth],szTitle);
						k	=	j;
						if(j<sizeof(szTitle)-3)
						{
							for(;i+j<nLineLen;j++)
							{
								if( pLine[j]=='/' && pLine[j+1]=='>' )
								{
									bEndTag = true;
									break;
								}
								else if( pLine[j]=='>' )
								{
									break;
								}
							}
							if(!bEndTag)
							{
								szTitle[k++]	=	' ';
								for(j-=k-1;i+j+k<nLineLen && k<sizeof(szTitle)-1;k++)
								{
									if( pLine[j+k]=='<' || pLine[j+k]=='\r' || pLine[j+k]=='\n' )
									{
										break;
									}
									szTitle[k] = pLine[j+k];
								}
								j += k-1;
							}
						}
						szTitle[k]	=	'\0';
						pcFuncInfoArr->AppendData( nLineCount+1, nPosY+1, szTitle, 0, nDepth++ );
					}
					else
					{
						for(;i+j<nLineLen && j<sizeof(szTitle)-1;j++)
						{
							if( pLine[j]=='>' )
							{
								break;
							}
							szTitle[j] = pLine[j];
						}
						szTitle[j]	=	'\0';
						pcFuncInfoArr->AppendData(nLineCount+1,nPosY+1,szTitle,0,nDepth);
					}
				}
			}
			if( bEndTag ) // �I���^�O
			{
				int nDepthOrg = nDepth; // 2004.04.20 Moca �ǉ�
				while(nDepth>0)
				{
					nDepth--;
					if(!_stricmp(pszStack[nDepth],szTitle))
					{
						break;
					}
				}
				// 2004.04.20 Moca �c���[���ƈ�v���Ȃ��Ƃ��́A���̏I���^�O�͖���
				if( nDepth == 0 )
				{
					if(_stricmp(pszStack[nDepth],szTitle))
					{
						nDepth = nDepthOrg;
					}
				}else{
					if( nLabelType=='hd' ){	//	���o���̏I���
						nHeadDepth[szTitle[1]-'0'] = nDepth;
						nDepth++;
					}
					if( nLabelType=='pa' ){
						bParaTag = false;
					}
				}
			}
			i	+=	j;
		}
	}
}

/*! TeX �A�E�g���C�����

	@author naoh
	@date 2003.07.21 naoh �V�K�쐬
	@date 2005.01.03 naoh �u�}�v�Ȃǂ�"}"���܂ޕ����ɑ΂���C���Aprosper��slide�ɑΉ�
*/
void CEditDoc::MakeTopicList_tex(CFuncInfoArr* pcFuncInfoArr)
{
	const char*	pLine;
	int						nLineLen;
	int						nLineCount;
	int						i;
	int						j;
	int						k;

	const int nMaxStack = 8;	//	�l�X�g�̍Ő[
	int nDepth = 0;				//	���܂̃A�C�e���̐[����\�����l�B
	char szTag[32], szTitle[256];			//	�ꎞ�̈�
	int thisSection=0, lastSection = 0;	// ���݂̃Z�N�V������ނƈ�O�̃Z�N�V�������
	int stackSection[nMaxStack];		// �e�[���ł̃Z�N�V�����̔ԍ�
	int nStartTitlePos;					// \section{dddd} �� dddd �̕����̎n�܂�ԍ�
	int bNoNumber;						// * �t�̏ꍇ�̓Z�N�V�����ԍ���t���Ȃ�

	// ��s����
	for(nLineCount=0;nLineCount<m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		pLine	=	(const char *)m_cDocLineMgr.GetLineStr(nLineCount,&nLineLen);
		if(!pLine) break;
		// �ꕶ������
		for(i=0;i<nLineLen-1;i++)
		{
			if(pLine[i] == '%' && !(i>0 && _IS_SJIS_1(pLine[i-1])) ) break;	// �R�����g�Ȃ�ȍ~�͂���Ȃ�
			if(pLine[i] != '\\' 
				&& !(i>0 && _IS_SJIS_1(pLine[i-1]))	// �u\�v�̑O�̕�����SJIS��1�o�C�g�ڂȂ玟�̕�����
				|| nDepth>=nMaxStack) continue;	// �u\�v���Ȃ��Ȃ玟�̕�����
			++i;
			// ���������u\�v�ȍ~�̕�����`�F�b�N
			for(j=0;i+j<nLineLen && j<sizeof(szTag)-1;j++)
			{
				if(pLine[i+j] == '{' && !(i+j>0 && _IS_SJIS_1((unsigned char)pLine[i+j-1])) ) {	// SJIS1�`�F�b�N
					bNoNumber = (pLine[i+j-1] == '*');
					nStartTitlePos = j+i+1;
					break;
				}
				szTag[j] = pLine[i+j];
			}
			if(j==0) continue;
			if(bNoNumber){
				szTag[j-1] = '\0';
			}else{
				szTag[j]   = '\0';
			}
//			MessageBox(NULL, szTitle, "", MB_OK);

			thisSection = 0;
			if(!strcmp(szTag,"subsubsection")) thisSection = 4;
			else if(!strcmp(szTag,"subsection")) thisSection = 3;
			else if(!strcmp(szTag,"section")) thisSection = 2;
			else if(!strcmp(szTag,"chapter")) thisSection = 1;
			else if(!strcmp(szTag,"begin")) {		// begin�Ȃ� prosper��slide�̉\�����l��
				// �����{slide}{}�܂œǂ݂Ƃ��Ă���
				if(strstr(pLine, "{slide}")){
					k=0;
					for(j=nStartTitlePos+1;i+j<nLineLen && j<sizeof(szTag)-1;j++)
					{
						if(pLine[i+j] == '{' && !(i+j>0 && _IS_SJIS_1((unsigned char)pLine[i+j-1])) ) {	// SJIS1�`�F�b�N
							nStartTitlePos = j+i+1;
							break;
						}
						szTag[k++]	=	pLine[i+j];
					}
					szTag[k] = '\0';
					thisSection = 1;
				}
			}

			if( thisSection > 0)
			{
				// section�̒��g�擾
				for(k=0;nStartTitlePos+k<nLineLen && k<sizeof(szTitle)-1;k++)
				{
					if(_IS_SJIS_1((unsigned char)pLine[k+nStartTitlePos])) {
						szTitle[k] = pLine[k+nStartTitlePos];
						k++;	// ���̓`�F�b�N�s�v
					} else if(pLine[k+nStartTitlePos] == '}') {
						break;
					}
					szTitle[k] = pLine[k+nStartTitlePos];
				}
				szTitle[k] = '\0';

				int		nPosX;
				int		nPosY;
				TCHAR tmpstr[256];
				TCHAR secstr[4];

				m_cLayoutMgr.CaretPos_Phys2Log(
					i,
					nLineCount,
					&nPosX,
					&nPosY
				);

				int sabunSection = thisSection - lastSection;
				if(lastSection == 0){
					nDepth = 0;
					stackSection[0] = 1;
				}else{
					nDepth += sabunSection;
					if(sabunSection > 0){
						if(nDepth >= nMaxStack) nDepth=nMaxStack-1;
						stackSection[nDepth] = 1;
					}else{
						if(nDepth < 0) nDepth=0;
						++stackSection[nDepth];
					}
				}
				tmpstr[0] = '\0';
				if(!bNoNumber){
					for(k=0; k<=nDepth; k++){
						sprintf(secstr, "%d.", stackSection[k]);
						strcat(tmpstr, secstr);
					}
					strcat(tmpstr, " ");
				}
				strcat(tmpstr, szTitle);
				pcFuncInfoArr->AppendData(nLineCount+1,nPosY+1, tmpstr, 0, nDepth);
				if(!bNoNumber) lastSection = thisSection;
			}
			i	+=	j;
		}
	}
}




/* �A�N�e�B�u�ȃy�C����ݒ� */
void  CEditDoc::SetActivePane( int nIndex )
{
	/* �A�N�e�B�u�ȃr���[��؂�ւ��� */
	int nOldIndex = m_nActivePaneIndex;
	m_nActivePaneIndex = nIndex;

	// �t�H�[�J�X���ړ�����	// 2007.10.16 ryoji
	m_cEditViewArr[nOldIndex].m_cUnderLine.CaretUnderLineOFF(TRUE);	//	2002/05/11 YAZAKI
	if( ::GetActiveWindow() == m_pcEditWnd->m_hWnd
		&& ::GetFocus() != m_cEditViewArr[m_nActivePaneIndex].m_hWnd )
	{
		// ::SetFocus()�Ńt�H�[�J�X��؂�ւ���
		::SetFocus( m_cEditViewArr[m_nActivePaneIndex].m_hWnd );
	}else{
		// 2010.04.08 ryoji
		// �N������ɃG�f�B�b�g�{�b�N�X�Ƀt�H�[�J�X�̂���_�C�A���O��\������ꍇ�ɃL�����b�g����������̏C��
		// �i��: -GREPDLG�I�v�V�����ŋN�������GREP�_�C�A���O�̃L�����b�g�������Ă���j
		// ���̖����C������̂��߁A�����I�Ȑ؂�ւ����������̂̓A�N�e�B�u�y�C�����ւ��Ƃ������ɂ����D
		if( m_nActivePaneIndex != nOldIndex ){
			// �A�N�e�B�u�łȂ��Ƃ���::SetFocus()����ƃA�N�e�B�u�ɂȂ��Ă��܂�
			// �i�s���Ȃ���ɂȂ�j�̂œ����I�ɐ؂�ւ��邾���ɂ���
			m_cEditViewArr[nOldIndex].OnKillFocus();
			m_cEditViewArr[m_nActivePaneIndex].OnSetFocus();
		}
	}

	m_cEditViewArr[m_nActivePaneIndex].RedrawAll();	/* �t�H�[�J�X�ړ����̍ĕ`�� */

	m_cSplitterWnd.SetActivePane( nIndex );

	if( NULL != m_cDlgFind.m_hWnd ){		/* �u�����v�_�C�A���O */
		/* ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX */
		m_cDlgFind.ChangeView( (LPARAM)&m_cEditViewArr[m_nActivePaneIndex] );
	}
	if( NULL != m_cDlgReplace.m_hWnd ){	/* �u�u���v�_�C�A���O */
		/* ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX */
		m_cDlgReplace.ChangeView( (LPARAM)&m_cEditViewArr[m_nActivePaneIndex] );
	}
	if( NULL != m_cHokanMgr.m_hWnd ){	/* �u���͕⊮�v�_�C�A���O */
		m_cHokanMgr.Hide();
		/* ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX */
		m_cHokanMgr.ChangeView( (LPARAM)&m_cEditViewArr[m_nActivePaneIndex] );
	}
	if( NULL != m_cDlgFuncList.m_hWnd ){	/* �u�A�E�g���C���v�_�C�A���O */ // 20060201 aroka
		/* ���[�h���X���F���݈ʒu�\���̑ΏۂƂȂ�r���[�̕ύX */
		m_cDlgFuncList.ChangeView( (LPARAM)&m_cEditViewArr[m_nActivePaneIndex] );
	}

	//	2002/05/08 YAZAKI OnKillFocus()��OnSetFocus()�ŁA�A���_�[���C���𐧌䂷��悤�ɂ����B
	//	2001/06/20 Start by asa-o:	�A�N�e�B�u�łȂ��y�C���̃J�[�\���A���_�[�o�[���\��
	//	m_cEditViewArr[m_nActivePaneIndex].CaretUnderLineON(TRUE);
	//	m_cEditViewArr[m_nActivePaneIndex^1].CaretUnderLineOFF(TRUE);
	//	m_cEditViewArr[m_nActivePaneIndex^2].CaretUnderLineOFF(TRUE);
	//	m_cEditViewArr[(m_nActivePaneIndex^2)^1].CaretUnderLineOFF(TRUE);
	//	2001/06/20 End

	return;
}



/* �A�N�e�B�u�ȃy�C�����擾 */
int CEditDoc::GetActivePane( void )
{
	return m_nActivePaneIndex;
}



/** ���ׂẴy�C���̕`��X�C�b�`��ݒ肷��

	@param bDraw [in] �`��X�C�b�`�̐ݒ�l

	@date 2008.06.08 ryoji �V�K�쐬
*/
void CEditDoc::SetDrawSwitchOfAllViews( BOOL bDraw )
{
	int i;
	CEditView* pView;

	for( i = 0; i < GetAllViewCount(); i++ ){
		pView = &m_cEditViewArr[i];
		pView->m_bDrawSWITCH = bDraw;
	}
}

/** ���ׂẴy�C����Redraw����

	�X�N���[���o�[�̏�ԍX�V�̓p�����[�^�Ńt���O���� or �ʊ֐��ɂ����ق��������H

	@param pViewExclude [in] Redraw���珜�O����r���[

	@date 2007.07.22 ryoji �X�N���[���o�[�̏�ԍX�V��ǉ�
	@date 2008.06.08 ryoji pViewExclude �p�����[�^�ǉ�
*/
void CEditDoc::RedrawAllViews( CEditView* pViewExclude )
{
	int i;
	CEditView* pView;

	for( i = 0; i < GetAllViewCount(); i++ ){
		pView = &m_cEditViewArr[i];
		if( pView == pViewExclude )
			continue;
		if( i == m_nActivePaneIndex ){
			pView->RedrawAll();
		}else{
			pView->Redraw();
			pView->AdjustScrollBars();
		}
	}
}

void CEditDoc::Views_Redraw()
{
	//�A�N�e�B�u�ȊO���ĕ`�悵�Ă���c
	for( int v = 0; v < GetAllViewCount(); ++v ){
		if( m_nActivePaneIndex != v ){
			m_cEditViewArr[v].Redraw();
		}
	}
	//�A�N�e�B�u���ĕ`��
	ActiveView().Redraw();
}


/* ���ׂẴy�C���ŁA�s�ԍ��\���ɕK�v�ȕ����Đݒ肷��i�K�v�Ȃ�ĕ`�悷��j */
BOOL CEditDoc::DetectWidthOfLineNumberAreaAllPane( BOOL bRedraw )
{
	if( 1 == GetAllViewCount() ){
		return m_cEditViewArr[m_nActivePaneIndex].DetectWidthOfLineNumberArea( bRedraw );
	}
	// �ȉ�2,4��������

	if ( m_cEditViewArr[m_nActivePaneIndex].DetectWidthOfLineNumberArea( bRedraw ) ){
		/* ActivePane�Ōv�Z������A�Đݒ�E�ĕ`�悪�K�v�Ɣ������� */
		if ( m_cSplitterWnd.GetAllSplitCols() == 2 ){
			m_cEditViewArr[m_nActivePaneIndex^1].DetectWidthOfLineNumberArea( bRedraw );
		}
		else {
			//	�\������Ă��Ȃ��̂ōĕ`�悵�Ȃ�
			m_cEditViewArr[m_nActivePaneIndex^1].DetectWidthOfLineNumberArea( FALSE );
		}
		if ( m_cSplitterWnd.GetAllSplitRows() == 2 ){
			m_cEditViewArr[m_nActivePaneIndex^2].DetectWidthOfLineNumberArea( bRedraw );
			if ( m_cSplitterWnd.GetAllSplitCols() == 2 ){
				m_cEditViewArr[(m_nActivePaneIndex^1)^2].DetectWidthOfLineNumberArea( bRedraw );
			}
		}
		else {
			m_cEditViewArr[m_nActivePaneIndex^2].DetectWidthOfLineNumberArea( FALSE );
			m_cEditViewArr[(m_nActivePaneIndex^1)^2].DetectWidthOfLineNumberArea( FALSE );
		}
		return TRUE;
	}
	return FALSE;
}

/** �E�[�Ő܂�Ԃ�
	@param nViewColNum	[in] �E�[�Ő܂�Ԃ��y�C���̔ԍ�
	@retval �܂�Ԃ���ύX�������ǂ���
	@date 2008.06.08 ryoji �V�K�쐬
*/
BOOL CEditDoc::WrapWindowWidth( int nPane )
{
	// �E�[�Ő܂�Ԃ�
	int nWidth = m_cEditViewArr[nPane].ViewColNumToWrapColNum( m_cEditViewArr[nPane].m_nViewColNum );
	if( m_cLayoutMgr.GetMaxLineSize() != nWidth ){
		ChangeLayoutParam( false, m_cLayoutMgr.GetTabSpace(), nWidth );
		return TRUE;
	}
	return FALSE;
}

/** �܂�Ԃ����@�֘A�̍X�V
	@retval ��ʍX�V�������ǂ���
	@date 2008.06.10 ryoji �V�K�쐬
*/
BOOL CEditDoc::UpdateTextWrap( void )
{
	// ���̊֐��̓R�}���h���s���Ƃɏ����̍ŏI�i�K�ŗ��p����
	// �i�A���h�D�o�^���S�r���[�X�V�̃^�C�~���O�j
	if( m_nTextWrapMethodCur == WRAP_WINDOW_WIDTH ){
		BOOL bWrap = WrapWindowWidth( 0 );	// �E�[�Ő܂�Ԃ�
		if( bWrap ){
			// WrapWindowWidth() �Œǉ������X�V���[�W�����ŉ�ʍX�V����
			for( int i = 0; i < GetAllViewCount(); i++ ){
				::UpdateWindow( m_cEditViewArr[i].m_hWnd );
			}
		}
		return bWrap;	// ��ʍX�V���܂�Ԃ��ύX
	}
	return FALSE;	// ��ʍX�V���Ȃ�����
}

/*! �R�}���h�R�[�h�ɂ�鏈���U�蕪��

	@param[in] nCommand MAKELONG( �R�}���h�R�[�h�C���M�����ʎq )

	@date 2006.05.19 genta ���16bit�ɑ��M���̎��ʎq������悤�ɕύX
	@date 2007.06.20 ryoji �O���[�v���ŏ��񂷂�悤�ɕύX
*/
BOOL CEditDoc::HandleCommand( int nCommand )
{
	int				i;
	int				j;
	int				nGroup;
	int				nRowNum;
	int				nPane;
	HWND			hwndWork;
	EditNode*		pEditNodeArr;

	//	May. 19, 2006 genta ���16bit�ɑ��M���̎��ʎq������悤�ɕύX�����̂�
	//	����16�r�b�g�݂̂����o��
	switch( LOWORD( nCommand )){
	case F_PREVWINDOW:	//�O�̃E�B���h�E
		nPane = m_cSplitterWnd.GetPrevPane();
		if( -1 != nPane ){
			SetActivePane( nPane );
		}else{
			/* ���݊J���Ă���ҏW���̃��X�g�𓾂� */
			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
			if(  nRowNum > 0 ){
				/* �����̃E�B���h�E�𒲂ׂ� */
				nGroup = 0;
				for( i = 0; i < nRowNum; ++i )
				{
					if( m_hwndParent == pEditNodeArr[i].m_hWnd )
					{
						nGroup = pEditNodeArr[i].m_nGroup;
						break;
					}
				}
				if( i < nRowNum )
				{
					// �O�̃E�B���h�E
					for( j = i - 1; j >= 0; --j )
					{
						if( nGroup == pEditNodeArr[j].m_nGroup )
							break;
					}
					if( j < 0 )
					{
						for( j = nRowNum - 1; j > i; --j )
						{
							if( nGroup == pEditNodeArr[j].m_nGroup )
								break;
						}
					}
					/* �O�̃E�B���h�E���A�N�e�B�u�ɂ��� */
					hwndWork = pEditNodeArr[j].m_hWnd;
					/* �A�N�e�B�u�ɂ��� */
					ActivateFrameWindow( hwndWork );
					/* �Ō�̃y�C�����A�N�e�B�u�ɂ��� */
					::PostMessage( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 1 );
				}
				delete [] pEditNodeArr;
			}
		}
		return TRUE;
	case F_NEXTWINDOW:	//���̃E�B���h�E
		nPane = m_cSplitterWnd.GetNextPane();
		if( -1 != nPane ){
			SetActivePane( nPane );
		}else{
			/* ���݊J���Ă���ҏW���̃��X�g�𓾂� */
			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
			if(  nRowNum > 0 ){
				/* �����̃E�B���h�E�𒲂ׂ� */
				nGroup = 0;
				for( i = 0; i < nRowNum; ++i )
				{
					if( m_hwndParent == pEditNodeArr[i].m_hWnd )
					{
						nGroup = pEditNodeArr[i].m_nGroup;
						break;
					}
				}
				if( i < nRowNum )
				{
					// ���̃E�B���h�E
					for( j = i + 1; j < nRowNum; ++j )
					{
						if( nGroup == pEditNodeArr[j].m_nGroup )
							break;
					}
					if( j >= nRowNum )
					{
						for( j = 0; j < i; ++j )
						{
							if( nGroup == pEditNodeArr[j].m_nGroup )
								break;
						}
					}
					/* ���̃E�B���h�E���A�N�e�B�u�ɂ��� */
					hwndWork = pEditNodeArr[j].m_hWnd;
					/* �A�N�e�B�u�ɂ��� */
					ActivateFrameWindow( hwndWork );
					/* �ŏ��̃y�C�����A�N�e�B�u�ɂ��� */
					::PostMessage( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 0 );
				}
				delete [] pEditNodeArr;
			}
		}
		return TRUE;

	default:
		return m_cEditViewArr[m_nActivePaneIndex].HandleCommand( nCommand, TRUE, 0, 0, 0, 0 );
	}
}

/*!
	���C�A�E�g�̕ύX�ɐ旧���āC�S�Ă�View�̍��W�𕨗����W�ɕϊ����ĕۑ�����D

	@return �f�[�^��ۑ������z��ւ̃|�C���^

	@note �擾�����l�̓��C�A�E�g�ύX���CEditDoc::RestorePhysPosOfAllView�֓n���D
	�n���Y���ƃ��������[�N�ƂȂ�D

	@date 2005.08.11 genta �V�K�쐬
*/
int* CEditDoc::SavePhysPosOfAllView(void)
{
	const int NUM_OF_VIEW = GetAllViewCount();
	const int NUM_OF_POS = 5;
	const int XY = 2;
	
	int* posary = new int[ NUM_OF_VIEW * NUM_OF_POS * XY ];
	
	for( int i = 0; i < NUM_OF_VIEW; ++i ){
		m_cLayoutMgr.CaretPos_Log2Phys(
			m_cEditViewArr[i].m_nCaretPosX,
			m_cEditViewArr[i].m_nCaretPosY,
			&posary[i * ( NUM_OF_POS * XY ) + 0 * XY + 0 ],
			&posary[i * ( NUM_OF_POS * XY ) + 0 * XY + 1 ]
		);
		if( m_cEditViewArr[i].m_nSelectLineBgnFrom >= 0 ){
			m_cLayoutMgr.CaretPos_Log2Phys(
				m_cEditViewArr[i].m_nSelectColmBgnFrom,
				m_cEditViewArr[i].m_nSelectLineBgnFrom,
				&posary[i * ( NUM_OF_POS * XY ) + 1 * XY + 0 ],
				&posary[i * ( NUM_OF_POS * XY ) + 1 * XY + 1 ]
			);
		}
		if( m_cEditViewArr[i].m_nSelectLineBgnTo >= 0 ){
			m_cLayoutMgr.CaretPos_Log2Phys(
				m_cEditViewArr[i].m_nSelectColmBgnTo,
				m_cEditViewArr[i].m_nSelectLineBgnTo,
				&posary[i * ( NUM_OF_POS * XY ) + 2 * XY + 0 ],
				&posary[i * ( NUM_OF_POS * XY ) + 2 * XY + 1 ]
			);
		}
		if( m_cEditViewArr[i].m_nSelectLineFrom >= 0 ){
			m_cLayoutMgr.CaretPos_Log2Phys(
				m_cEditViewArr[i].m_nSelectColmFrom,
				m_cEditViewArr[i].m_nSelectLineFrom,
				&posary[i * ( NUM_OF_POS * XY ) + 3 * XY + 0 ],
				&posary[i * ( NUM_OF_POS * XY ) + 3 * XY + 1 ]
			);
		}
		if( m_cEditViewArr[i].m_nSelectLineTo >= 0 ){
			m_cLayoutMgr.CaretPos_Log2Phys(
				m_cEditViewArr[i].m_nSelectColmTo,
				m_cEditViewArr[i].m_nSelectLineTo,
				&posary[i * ( NUM_OF_POS * XY ) + 4 * XY + 0 ],
				&posary[i * ( NUM_OF_POS * XY ) + 4 * XY + 1 ]
			);
		}
	}
	return posary;
}

/*!	���W�̕���

	CEditDoc::SavePhysPosOfAllView�ŕۑ������f�[�^�����ɍ��W�l���Čv�Z����D

	@date 2005.08.11 genta �V�K�쐬
*/
void CEditDoc::RestorePhysPosOfAllView( int* posary )
{
	const int NUM_OF_VIEW = GetAllViewCount();
	const int NUM_OF_POS = 5;
	const int XY = 2;

	for( int i = 0; i < NUM_OF_VIEW; ++i ){
		int		nPosX;
		int		nPosY;
		m_cLayoutMgr.CaretPos_Phys2Log(
			posary[i * ( NUM_OF_POS * XY ) + 0 * XY + 0 ],
			posary[i * ( NUM_OF_POS * XY ) + 0 * XY + 1 ],
			&nPosX,
			&nPosY
		);
		m_cEditViewArr[i].MoveCursor( nPosX, nPosY, TRUE );
		m_cEditViewArr[i].m_nCaretPosX_Prev = m_cEditViewArr[i].m_nCaretPosX;

		if( m_cEditViewArr[i].m_nSelectLineBgnFrom >= 0 ){
			m_cLayoutMgr.CaretPos_Phys2Log(
				posary[i * ( NUM_OF_POS * XY ) + 1 * XY + 0 ],
				posary[i * ( NUM_OF_POS * XY ) + 1 * XY + 1 ],
				&m_cEditViewArr[i].m_nSelectColmBgnFrom,
				&m_cEditViewArr[i].m_nSelectLineBgnFrom
			);
		}
		if( m_cEditViewArr[i].m_nSelectLineBgnTo >= 0 ){
			m_cLayoutMgr.CaretPos_Phys2Log(
				posary[i * ( NUM_OF_POS * XY ) + 2 * XY + 0 ],
				posary[i * ( NUM_OF_POS * XY ) + 2 * XY + 1 ],
				&m_cEditViewArr[i].m_nSelectColmBgnTo,
				&m_cEditViewArr[i].m_nSelectLineBgnTo
			);
		}
		if( m_cEditViewArr[i].m_nSelectLineFrom >= 0 ){
			m_cLayoutMgr.CaretPos_Phys2Log(
				posary[i * ( NUM_OF_POS * XY ) + 3 * XY + 0 ],
				posary[i * ( NUM_OF_POS * XY ) + 3 * XY + 1 ],
				&m_cEditViewArr[i].m_nSelectColmFrom,
				&m_cEditViewArr[i].m_nSelectLineFrom
			);
		}
		if( m_cEditViewArr[i].m_nSelectLineTo >= 0 ){
			m_cLayoutMgr.CaretPos_Phys2Log(
				posary[i * ( NUM_OF_POS * XY ) + 4 * XY + 0 ],
				posary[i * ( NUM_OF_POS * XY ) + 4 * XY + 1 ],
				&m_cEditViewArr[i].m_nSelectColmTo,
				&m_cEditViewArr[i].m_nSelectLineTo
			);
		}
	}
	delete[] posary;
}

/*! �r���[�ɐݒ�ύX�𔽉f������

	@date 2004.06.09 Moca ���C�A�E�g�č\�z����Progress Bar��\������D
	@date 2008.05.30 nasukoji	�e�L�X�g�̐܂�Ԃ����@�̕ύX������ǉ�
*/
void CEditDoc::OnChangeSetting( void )
{
//	return;
	int			i;
	HWND		hwndProgress = NULL;

	CEditWnd*	pCEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta

	//pCEditWnd->m_CFuncKeyWnd.Timer_ONOFF( FALSE ); // 20060126 aroka

	if( NULL != pCEditWnd ){
		hwndProgress = pCEditWnd->m_hwndProgressBar;
		//	Status Bar���\������Ă��Ȃ��Ƃ���m_hwndProgressBar == NULL
	}

	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_SHOW );
	}

	/* �t�@�C���̔r�����[�h�ύX */
	if( m_nFileShareModeOld != m_pShareData->m_Common.m_nFileShareMode ){
		/* �t�@�C���̔r�����b�N���� */
		DoFileUnLock();
		/* �t�@�C���̔r�����b�N */
		DoFileLock();
	}
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();
	CShareData::getInstance()->TransformFileName_MakeCache();
	int doctype = CShareData::getInstance()->GetDocumentType( GetFilePath() );
	SetDocumentType( doctype, false );

	int* posSaveAry = SavePhysPosOfAllView();

	/* ���C�A�E�g���̍쐬 */
//	STypeConfig& ref = GetDocumentAttribute();
	// 2008.06.07 nasukoji	�܂�Ԃ����@�̒ǉ��ɑΉ�
	STypeConfig ref = GetDocumentAttribute();

	// �܂�Ԃ����@�̈ꎞ�ݒ�ƃ^�C�v�ʐݒ肪��v������ꎞ�ݒ�K�p���͉���
	if( m_nTextWrapMethodCur == ref.m_nTextWrapMethod )
		m_bTextWrapMethodCurTemp = false;		// �ꎞ�ݒ�K�p��������

	// �ꎞ�ݒ�K�p���łȂ���ΐ܂�Ԃ����@�ύX
	if( !m_bTextWrapMethodCurTemp )
		m_nTextWrapMethodCur = ref.m_nTextWrapMethod;	// �܂�Ԃ����@

	// �w�茅�Ő܂�Ԃ��F�^�C�v�ʐݒ���g�p
	// �E�[�Ő܂�Ԃ��F���Ɍ��݂̐܂�Ԃ������g�p
	// ��L�ȊO�FMAXLINESIZE���g�p
	if( m_nTextWrapMethodCur != WRAP_SETTING_WIDTH ){
		if( m_nTextWrapMethodCur == WRAP_WINDOW_WIDTH )
			ref.m_nMaxLineSize = m_cLayoutMgr.GetMaxLineSize();	// ���݂̐܂�Ԃ���
		else
			ref.m_nMaxLineSize = MAXLINESIZE;
	}

	m_cLayoutMgr.SetLayoutInfo(
		TRUE,
		hwndProgress,
		ref
	); /* ���C�A�E�g���̕ύX */

	// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�Ȃ�e�L�X�g�ő啝���Z�o�A����ȊO�͕ϐ����N���A
	if( m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP )
		m_cLayoutMgr.CalculateTextWidth();		// �e�L�X�g�ő啝���Z�o����
	else
		m_cLayoutMgr.ClearLayoutLineWidth();	// �e�s�̃��C�A�E�g�s���̋L�����N���A����

	/* �r���[�ɐݒ�ύX�𔽉f������ */
	for( i = 0; i < GetAllViewCount(); ++i ){
		m_cEditViewArr[i].OnChangeSetting();
	}
	RestorePhysPosOfAllView( posSaveAry );
	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
}




/* �ҏW�t�@�C�������i�[ */
void CEditDoc::SetFileInfo( EditInfo* pfi )
{
	int		nX;
	int		nY;

	strcpy( pfi->m_szPath, GetFilePath() );
	pfi->m_nViewTopLine = m_cEditViewArr[m_nActivePaneIndex].m_nViewTopLine;	/* �\����̈�ԏ�̍s(0�J�n) */
	pfi->m_nViewLeftCol = m_cEditViewArr[m_nActivePaneIndex].m_nViewLeftCol;	/* �\����̈�ԍ��̌�(0�J�n) */
	//	pfi->m_nCaretPosX = m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX;	/* �r���[���[����̃J�[�\�����ʒu(�O�J�n) */
	//	pfi->m_nCaretPosY = m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosY;	/* �r���[��[����̃J�[�\���s�ʒu(�O�J�n) */

	/*
	  �J�[�\���ʒu�ϊ�
	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	  ��
	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	m_cLayoutMgr.CaretPos_Log2Phys(
		m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX,	/* �r���[���[����̃J�[�\�����ʒu(�O�J�n) */
		m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosY,	/* �r���[��[����̃J�[�\���s�ʒu(�O�J�n) */
		&nX,
		&nY
	);
	pfi->m_nX = nX;		/* �J�[�\�� �����ʒu(�s������̃o�C�g��) */
	pfi->m_nY = nY;		/* �J�[�\�� �����ʒu(�܂�Ԃ������s�ʒu) */


	pfi->m_bIsModified = IsModified() ? TRUE : FALSE;			/* �ύX�t���O */
	pfi->m_nCharCode = m_nCharCode;				/* �����R�[�h��� */
//	pfi->m_bPLSQL = m_cDlgJump.m_bPLSQL,		/* �s�W�����v�� PL/SQL���[�h�� */
//	pfi->m_nPLSQL_E1 = m_cDlgJump.m_nPLSQL_E1;	/* �s�W�����v�� PL/SQL���[�h�̂Ƃ��̊�_ */

	pfi->m_bIsGrep = m_bGrepMode;
	strcpy( pfi->m_szGrepKey, m_szGrepKey );

	//�f�o�b�O���j�^(�A�E�g�v�b�g�E�C���h�E)
	pfi->m_bIsDebug = m_bDebugMode;

	return;

}


// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
#if 0
/* �^�O�W�����v���ȂǎQ�ƌ��̏���ێ����� */
void CEditDoc::SetReferer( HWND hwndReferer, int nRefererX, int nRefererLine )
{
	m_hwndReferer	= hwndReferer;	/* �Q�ƌ��E�B���h�E */
	m_nRefererX		= nRefererX;	/* �Q�ƌ�  �s������̃o�C�g�ʒu�� */
	m_nRefererLine	= nRefererLine;	/* �Q�ƌ��s  �܂�Ԃ������̕����s�ʒu */
	return;
}
#endif



/*! �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F �� �ۑ����s

	@retval TRUE: �I�����ėǂ� / FALSE: �I�����Ȃ�
*/
BOOL CEditDoc::OnFileClose( void )
{
	int			nRet;
	int			nBool;
	HWND		hwndMainFrame;
	hwndMainFrame = ::GetParent( m_hWnd );

	//	Mar. 30, 2003 genta �T�u���[�`���ɂ܂Ƃ߂�
	AddToMRU();

	if( m_bGrepRunning ){		/* Grep������ */
		/* �A�N�e�B�u�ɂ��� */
		ActivateFrameWindow( hwndMainFrame );	//@@@ 2003.06.25 MIK
		::MYMESSAGEBOX(
			hwndMainFrame,
			MB_OK | MB_ICONINFORMATION | MB_TOPMOST,
			GSTR_APPNAME,
			"Grep�̏������ł��B\n"
		);
		return FALSE;
	}


	/* �e�L�X�g���ύX����Ă���ꍇ */
	if( IsModified()
	&& FALSE == m_bDebugMode	/* �f�o�b�O���j�^���[�h�̂Ƃ��͕ۑ��m�F���Ȃ� */
//	&& FALSE == m_bReadOnly		/* �ǂݎ���p���[�h */
	){
		if( TRUE == m_bGrepMode ){	/* Grep���[�h�̂Ƃ� */
			/* Grep���[�h�ŕۑ��m�F���邩 */
			if( FALSE == m_pShareData->m_Common.m_bGrepExitConfirm ){
				return TRUE;
			}
		}
		/* �E�B���h�E���A�N�e�B�u�ɂ��� */
		/* �A�N�e�B�u�ɂ��� */
		ActivateFrameWindow( hwndMainFrame );
		if( m_bReadOnly ){	/* �ǂݎ���p���[�h */
			ConfirmBeep();
			nRet = ::MYMESSAGEBOX(
				hwndMainFrame,
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				GSTR_APPNAME,
				"%s\n�͕ύX����Ă��܂��B ����O�ɕۑ����܂����H\n\n�ǂݎ���p�ŊJ���Ă���̂ŁA���O��t���ĕۑ�����΂����Ǝv���܂��B\n",
				IsFilePathAvailable() ? GetFilePath() : "�i����j"
			);
			switch( nRet ){
			case IDYES:
//				if( IsFilePathAvailable() ){
//					nBool = HandleCommand( F_FILESAVE );
//				}else{
					//nBool = HandleCommand( F_FILESAVEAS_DIALOG );
					nBool = FileSaveAs_Dialog();	// 2006.12.30 ryoji
//				}
				return nBool;
			case IDNO:
				return TRUE;
			case IDCANCEL:
			default:
				return FALSE;
			}
		}else{
			ConfirmBeep();
			nRet = ::MYMESSAGEBOX(
				hwndMainFrame,
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				GSTR_APPNAME,
				"%s\n�͕ύX����Ă��܂��B ����O�ɕۑ����܂����H",
				IsFilePathAvailable() ? GetFilePath() : "�i����j"
			);
			switch( nRet ){
			case IDYES:
				if( IsFilePathAvailable() ){
					//nBool = HandleCommand( F_FILESAVE );
					nBool = FileSave();	// 2006.12.30 ryoji
				}else{
					//nBool = HandleCommand( F_FILESAVEAS_DIALOG );
					nBool = FileSaveAs_Dialog();	// 2006.12.30 ryoji
				}
				return nBool;
			case IDNO:
				return TRUE;
			case IDCANCEL:
			default:
				return FALSE;
			}
		}
	}else{
		return TRUE;
	}
}


/* �����f�[�^�̃N���A */
void CEditDoc::Init( void )
{
//	int types;

	m_bReadOnly = FALSE;	/* �ǂݎ���p���[�h */
	strcpy( m_szGrepKey, "" );
	m_bGrepMode = FALSE;	/* Grep���[�h */
	m_eWatchUpdate = WU_QUERY; // Dec. 4, 2002 genta �X�V�Ď����@

	// 2005.06.24 Moca �o�O�C��
	//	�A�E�g�v�b�g�E�B���h�E�Łu����(����)�v���s���Ă��A�E�g�v�b�g�E�B���h�E�̂܂�
	if( m_bDebugMode ){
		m_pcEditWnd->SetDebugModeOFF();
	}

//	Sep. 10, 2002 genta
//	�A�C�R���ݒ�̓t�@�C�����ݒ�ƈ�̉��̂��߂�������͍폜

	/* �t�@�C���̔r�����b�N���� */
	DoFileUnLock();

	/* �t�@�C���̔r�����䃂�[�h */
	m_nFileShareModeOld = 0;


	/*�A���h�D�E���h�D�o�b�t�@�̃N���A */
	/* �S�v�f�̃N���A */
	m_cOpeBuf.ClearAll();

	/* �e�L�X�g�f�[�^�̃N���A */
	m_cDocLineMgr.Empty();
	m_cDocLineMgr.Init();

	/* ���ݕҏW���̃t�@�C���̃p�X */
	//	Sep. 10, 2002 genta
	//	�A�C�R���������ɏ����������
	SetFilePath( "" );

	/* ���ݕҏW���̃t�@�C���̃^�C���X�^���v */
	m_FileTime.dwLowDateTime = 0;
	m_FileTime.dwHighDateTime = 0;


	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();
	int doctype = CShareData::getInstance()->GetDocumentType( GetFilePath() );
	SetDocumentType( doctype, true );

	/* ���C�A�E�g�Ǘ����̏����� */
	/* ���C�A�E�g���̕ύX */
	STypeConfig& ref = GetDocumentAttribute();
	m_cLayoutMgr.SetLayoutInfo(
		TRUE,
		NULL,/*hwndProgress*/
		ref
	);


	/* �ύX�t���O */
	SetModified(false,false);	//	Jan. 22, 2002 genta

	/* �����R�[�h��� */
	m_nCharCode = 0;
	m_bBomExist = FALSE;	//	Jul. 26, 2003 ryoji

	//	May 12, 2000
	m_cNewLineCode.SetType( EOL_CRLF );
	
	//	Oct. 2, 2005 genta �}�����[�h
	SetInsMode( m_pShareData->m_Common.m_bIsINSMode != FALSE );

	return;
}

/* �S�r���[�̏������F�t�@�C���I�[�v��/�N���[�Y�����ɁA�r���[������������ */
void CEditDoc::InitAllView( void )
{
	int		i;

	m_nCommandExecNum = 0;	/* �R�}���h���s�� */

	// 2008.05.30 nasukoji	�e�L�X�g�̐܂�Ԃ����@��������
	m_nTextWrapMethodCur = GetDocumentAttribute().m_nTextWrapMethod;	// �܂�Ԃ����@
	m_bTextWrapMethodCurTemp = false;									// �ꎞ�ݒ�K�p��������

	// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�Ȃ�e�L�X�g�ő啝���Z�o�A����ȊO�͕ϐ����N���A
	if( m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP )
		m_cLayoutMgr.CalculateTextWidth();		// �e�L�X�g�ő啝���Z�o����
	else
		m_cLayoutMgr.ClearLayoutLineWidth();	// �e�s�̃��C�A�E�g�s���̋L�����N���A����

	/* �擪�փJ�[�\�����ړ� */
	for( i = 0; i < GetAllViewCount(); ++i ){
		//	Apr. 1, 2001 genta
		// �ړ������̏���
		m_cEditViewArr[i].m_cHistory->Flush();

		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		m_cEditViewArr[i].DisableSelectArea( FALSE );

		m_cEditViewArr[i].OnChangeSetting();
		m_cEditViewArr[i].MoveCursor( 0, 0, TRUE );
		m_cEditViewArr[i].m_nCaretPosX_Prev = 0;
	}

	return;
}


/*
	�����w���B2�ڈȍ~�̃r���[�����
	@param nViewCount  �����̃r���[���܂߂��r���[�̍��v�v����
*/
bool CEditDoc::CreateEditViewBySplit(int nViewCount )
{
	const int MAX_EDITVIEW = sizeof(m_cEditViewArr)/sizeof(m_cEditViewArr[0]);
	if( MAX_EDITVIEW < nViewCount ){
		return false;
	}
	if( GetAllViewCount() < nViewCount ){
		int i;
		for( i = GetAllViewCount(); i < nViewCount; i++ ){
			m_cEditViewArr[i].Create( m_hInstance, m_cSplitterWnd.m_hWnd, this, i, FALSE );
		}
		m_nEditViewCount = nViewCount;

		HWND hWndArr[MAX_EDITVIEW+1];
		for( i = 0; i < nViewCount; i++ ){
			hWndArr[i] = m_cEditViewArr[i].m_hWnd;
		}
		hWndArr[nViewCount] = NULL;

		m_cSplitterWnd.SetChildWndArr( hWndArr );
	}
	return true;
}


/* �t�@�C���̃^�C���X�^���v�̃`�F�b�N���� */
void CEditDoc::CheckFileTimeStamp( void )
{
	HWND		hwndActive;
	BOOL		bUpdate;
	bUpdate = FALSE;
	if( m_pShareData->m_Common.m_bCheckFileTimeStamp	/* �X�V�̊Ď� */
	 // Dec. 4, 2002 genta
	 && m_eWatchUpdate != WU_NONE
	 && m_pShareData->m_Common.m_nFileShareMode == 0	/* �t�@�C���̔r�����䃂�[�h */
	 && NULL != ( hwndActive = ::GetActiveWindow() )	/* �A�N�e�B�u? */
	 && hwndActive == m_hwndParent
	 && IsFilePathAvailable()
	 && ( m_FileTime.dwLowDateTime != 0 || m_FileTime.dwHighDateTime != 0 ) 	/* ���ݕҏW���̃t�@�C���̃^�C���X�^���v */

	){
		/* �t�@�C���X�^���v���`�F�b�N���� */

		// 2005.10.20 ryoji FindFirstFile���g���悤�ɕύX�i�t�@�C�������b�N����Ă��Ă��^�C���X�^���v�擾�\�j
		FILETIME ftime;
		if( GetLastWriteTimestamp( GetFilePath(), ftime )){
			if( 0 != ::CompareFileTime( &m_FileTime, &ftime ) )	//	Aug. 13, 2003 wmlhq �^�C���X�^���v���Â��ύX����Ă���ꍇ�����o�ΏۂƂ���
			{
				bUpdate = TRUE;
				m_FileTime = ftime;
			}
		}
	}

	//	From Here Dec. 4, 2002 genta
	if( bUpdate ){
		switch( m_eWatchUpdate ){
		case WU_NOTIFY:
			{
				char szText[40];
				//	���ݎ����̎擾
				SYSTEMTIME st;
				FILETIME lft;
				if( ::FileTimeToLocalFileTime( &m_FileTime, &lft ) &&
					::FileTimeToSystemTime( &lft, &st )){
					// nothing to do
				}
				else {
					//	�t�@�C�������̕ϊ��Ɏ��s�����ꍇ��
					//	���ݎ����ł��܂���
					::GetLocalTime( &st );
				}
				wsprintf( szText, "���t�@�C���X�V %02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond );
				m_pcEditWnd->SendStatusMessage( szText );
			}	
			break;
		default:
			{
				m_eWatchUpdate = WU_NONE; // �X�V�Ď��̗}��

				CDlgFileUpdateQuery dlg( GetFilePath(), IsModified() );
				int result = dlg.DoModal( m_hInstance, m_hWnd, IDD_FILEUPDATEQUERY, 0 );

				switch( result ){
				case 1:	// �ēǍ�
					/* ����t�@�C���̍ăI�[�v�� */
					ReloadCurrentFile( m_nCharCode, m_bReadOnly );
					m_eWatchUpdate = WU_QUERY;
					break;
				case 2:	// �Ȍ�ʒm���b�Z�[�W�̂�
					m_eWatchUpdate = WU_NOTIFY;
					break;
				case 3:	// �Ȍ�X�V���Ď����Ȃ�
					m_eWatchUpdate = WU_NONE;
					break;
				case 0:	// CLOSE
				default:
					m_eWatchUpdate = WU_QUERY;
					break;
				}
			}
			break;
		}
	}
	//	To Here Dec. 4, 2002 genta
	return;
}





/*! ����t�@�C���̍ăI�[�v�� */
void CEditDoc::ReloadCurrentFile(
	BOOL	nCharCode,		/*!< [in] �����R�[�h��� */
	BOOL	bReadOnly		/*!< [in] �ǂݎ���p���[�h */
)
{
	if( -1 == _access( GetFilePath(), 0 ) ){
		/* �t�@�C�������݂��Ȃ� */
		//	Jul. 26, 2003 ryoji BOM��W���ݒ��
		m_nCharCode = nCharCode;
		switch( m_nCharCode ){
		case CODE_UNICODE:
		case CODE_UNICODEBE:
			m_bBomExist = TRUE;
			break;
		case CODE_UTF8:
		default:
			m_bBomExist = FALSE;
			break;
		}
		return;
	}


	BOOL	bOpened;
	char	szFilePath[MAX_PATH];
	int		nViewTopLine;
	int		nViewLeftCol;
	int		nCaretPosX;
	int		nCaretPosY;
	nViewTopLine = m_cEditViewArr[m_nActivePaneIndex].m_nViewTopLine;	/* �\����̈�ԏ�̍s(0�J�n) */
	nViewLeftCol = m_cEditViewArr[m_nActivePaneIndex].m_nViewLeftCol;	/* �\����̈�ԍ��̌�(0�J�n) */
	nCaretPosX = m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX;
	nCaretPosY = m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosY;

	strcpy( szFilePath, GetFilePath() );

	// Mar. 30, 2003 genta �u�b�N�}�[�N�ۑ��̂���MRU�֓o�^
	AddToMRU();

	/* �����f�[�^�̃N���A */
	Init();

	/* �S�r���[�̏����� */
	InitAllView();

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	SetParentCaption();

	/* �t�@�C���ǂݍ��� */
	FileRead(
		szFilePath,
		&bOpened,
		nCharCode,	/* �����R�[�h�������� */
		bReadOnly,	/* �ǂݎ���p�� */
		FALSE		/* �����R�[�h�ύX���̊m�F�����邩�ǂ��� */
	);

	// ���C�A�E�g�s�P�ʂ̃J�[�\���ʒu����
	// �������ł̓I�v�V�����̃J�[�\���ʒu�����i�����s�P�ʁj���w�肳��Ă��Ȃ��ꍇ�ł���������
	// 2007.08.23 ryoji �\���̈敜��
	if( nCaretPosY < m_cLayoutMgr.GetLineCount() ){
		m_cEditViewArr[m_nActivePaneIndex].m_nViewTopLine = nViewTopLine;
		m_cEditViewArr[m_nActivePaneIndex].m_nViewLeftCol = nViewLeftCol;
	}
	m_cEditViewArr[m_nActivePaneIndex].MoveCursorProperly( nCaretPosX, nCaretPosY, TRUE );	// 2007.08.23 ryoji MoveCursor()->MoveCursorProperly()
	m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX_Prev = m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX;

	// 2006.09.01 ryoji �I�[�v���㎩�����s�}�N�������s����
	RunAutoMacro( m_pShareData->m_nMacroOnOpened );
}

//	From Here Nov. 20, 2000 genta
/*!	IME��Ԃ̐ݒ�
	
	@param mode [in] IME�̃��[�h
	
	@date Nov 20, 2000 genta
*/
void CEditDoc::SetImeMode( int mode )
{
	DWORD	conv, sent;
	HIMC	hIme;

	hIme = ImmGetContext( m_hwndParent );

	//	�ŉ��ʃr�b�g��IME���g��On/Off����
	if( ( mode & 3 ) == 2 ){
		ImmSetOpenStatus( hIme, FALSE );
	}
	if( ( mode >> 2 ) > 0 ){
		ImmGetConversionStatus( hIme, &conv, &sent );

		switch( mode >> 2 ){
		case 1:	//	FullShape
			conv |= IME_CMODE_FULLSHAPE;
			conv &= ~IME_CMODE_NOCONVERSION;
			break;
		case 2:	//	FullShape & Hiragana
			conv |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE;
			conv &= ~( IME_CMODE_KATAKANA | IME_CMODE_NOCONVERSION );
			break;
		case 3:	//	FullShape & Katakana
			conv |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE | IME_CMODE_KATAKANA;
			conv &= ~IME_CMODE_NOCONVERSION;
			break;
		case 4: //	Non-Conversion
			conv |= IME_CMODE_NOCONVERSION;
			break;
		}
		ImmSetConversionStatus( hIme, conv, sent );
	}
	if( ( mode & 3 ) == 1 ){
		ImmSetOpenStatus( hIme, TRUE );
	}
	ImmReleaseContext( m_hwndParent, hIme );
}
//	To Here Nov. 20, 2000 genta


/*!	$x�̓W�J

	���ꕶ���͈ȉ��̒ʂ�
	@li $  $���g
	@li A  �A�v����
	@li F  �J���Ă���t�@�C���̃t���p�X�B���O���Ȃ����(����)�B
	@li f  �J���Ă���t�@�C���̖��O�i�t�@�C����+�g���q�̂݁j
	@li g  �J���Ă���t�@�C���̖��O�i�g���q�����j
	@li /  �J���Ă���t�@�C���̖��O�i�t���p�X�B�p�X�̋�؂肪/�j
	@li N  �J���Ă���t�@�C���̖��O(�ȈՕ\��)
	@li C  ���ݑI�𒆂̃e�L�X�g
	@li x  ���݂̕������ʒu(�擪����̃o�C�g��1�J�n)
	@li y  ���݂̕����s�ʒu(1�J�n)
	@li d  ���݂̓��t(���ʐݒ�̓��t����)
	@li t  ���݂̎���(���ʐݒ�̎�������)
	@li p  ���݂̃y�[�W
	@li P  ���y�[�W
	@li D  �t�@�C���̃^�C���X�^���v(���ʐݒ�̓��t����)
	@li T  �t�@�C���̃^�C���X�^���v(���ʐݒ�̎�������)
	@li V  �G�f�B�^�̃o�[�W����������
	@li h  Grep�����L�[�̐擪32byte
	@li S  �T�N���G�f�B�^�̃t���p�X
	@li I  ini�t�@�C���̃t���p�X
	@li M  ���ݎ��s���Ă���}�N���t�@�C���p�X

	@date 2003.04.03 genta strncpy_ex�����ɂ��for���̍팸
	@date 2005.09.15 FILE ���ꕶ��S, M�ǉ�
	@date 2007.09.21 kobake ���ꕶ��A(�A�v����)��ǉ�
	@date 2008.05.05 novice GetModuleHandle(NULL)��NULL�ɕύX
*/
void CEditDoc::ExpandParameter(const char* pszSource, char* pszBuffer, int nBufferLen)
{
	
	// Apr. 03, 2003 genta �Œ蕶������܂Ƃ߂�
	static const char PRINT_PREVIEW_ONLY[] = "(����v���r���[�ł̂ݎg�p�ł��܂�)";
	const int PRINT_PREVIEW_ONLY_LEN = sizeof( PRINT_PREVIEW_ONLY ) - 1;
	static const char NO_TITLE[] = "(����)";
	const int NO_TITLE_LEN = sizeof( NO_TITLE ) - 1;
	static const char NOT_SAVED[] = "(�ۑ�����Ă��܂���)";
	const int NOT_SAVED_LEN = sizeof( NOT_SAVED ) - 1;

	const char *p, *r;	//	p�F�ړI�̃o�b�t�@�Br�F��Ɨp�̃|�C���^�B
	char *q, *q_max;
	for( p = pszSource, q = pszBuffer, q_max = pszBuffer + nBufferLen; *p != '\0' && q < q_max;){
		if( *p != '$' ){
			*q++ = *p++;
			continue;
		}
		switch( *(++p) ){
		case '$':	//	 $$ -> $
			*q++ = *p++;
			break;
		case 'A':	//�A�v����
			q = strncpy_ex( q, q_max - q, GSTR_APPNAME, _tcslen(GSTR_APPNAME) );
			++p;
			break;
		case 'F':	//	�J���Ă���t�@�C���̖��O�i�t���p�X�j
			if ( !IsFilePathAvailable() ){
				q = strncpy_ex( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				r = GetFilePath();
				q = strncpy_ex( q, q_max - q, r, strlen( r ));
				++p;
			}
			break;
		case 'f':	//	�J���Ă���t�@�C���̖��O�i�t�@�C����+�g���q�̂݁j
			// Oct. 28, 2001 genta
			//	�t�@�C�����݂̂�n���o�[�W����
			//	�|�C���^�𖖔���
			if ( ! IsFilePathAvailable() ){
				q = strncpy_ex( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				r = GetFileName(); // 2002.10.13 Moca �t�@�C����(�p�X�Ȃ�)���擾�B���{��Ή�
				//	����\\�������ɂ����Ă����̌��ɂ�\0������̂ŃA�N�Z�X�ᔽ�ɂ͂Ȃ�Ȃ��B
				q = strncpy_ex( q, q_max - q, r, strlen( r ));
				++p;
			}
			break;
		case 'g':	//	�J���Ă���t�@�C���̖��O�i�g���q�������t�@�C�����̂݁j
			//	From Here Sep. 16, 2002 genta
			if ( ! IsFilePathAvailable() ){
				q = strncpy_ex( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				//	�|�C���^�𖖔���
				const char *dot_position, *end_of_path;
				r = GetFileName(); // 2002.10.13 Moca �t�@�C����(�p�X�Ȃ�)���擾�B���{��Ή�
				end_of_path = dot_position =
					r + strlen( r );
				//	��납��.��T��
				for( --dot_position ; dot_position > r && *dot_position != '.'
					; --dot_position )
					;
				//	r�Ɠ����ꏊ�܂ōs���Ă��܂�����.����������
				if( dot_position == r )
					dot_position = end_of_path;

				q = strncpy_ex( q, q_max - q, r, dot_position - r );
				++p;
			}
			break;
			//	To Here Sep. 16, 2002 genta
		case '/':	//	�J���Ă���t�@�C���̖��O�i�t���p�X�B�p�X�̋�؂肪/�j
			// Oct. 28, 2001 genta
			if ( !IsFilePathAvailable() ){
				q = strncpy_ex( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				//	�p�X�̋�؂�Ƃ���'/'���g���o�[�W����
				for( r = GetFilePath(); *r != '\0' && q < q_max; ++r, ++q ){
					if( *r == '\\' )
						*q = '/';
					else
						*q = *r;
				}
				++p;
			}
			break;
		//	From Here 2003/06/21 Moca
		case 'N':
			if( !IsFilePathAvailable() ){
				q = strncpy_ex( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			}
			else {
				char szText[1024];
				CShareData::getInstance()->GetTransformFileNameFast( GetFilePath(), szText, 1023 );
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			break;
		//	To Here 2003/06/21 Moca
		//	From Here Jan. 15, 2002 hor
		case 'C':	//	���ݑI�𒆂̃e�L�X�g
			{
				CMemory cmemCurText;
				m_cEditViewArr[m_nActivePaneIndex].GetCurrentTextForSearch( cmemCurText );
				q = strncpy_ex( q, q_max - q, cmemCurText.GetStringPtr(), cmemCurText.GetStringLength());
				++p;
			}
		//	To Here Jan. 15, 2002 hor
			break;
		//	From Here 2002/12/04 Moca
		case 'x':	//	���݂̕������ʒu(�擪����̃o�C�g��1�J�n)
			{
				char szText[11];
				_itot( m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX_PHY + 1, szText, 10 );
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			break;
		case 'y':	//	���݂̕����s�ʒu(1�J�n)
			{
				char szText[11];
				_itot( m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosY_PHY + 1, szText, 10 );
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			break;
		//	To Here 2002/12/04 Moca
		case 'd':	//	���ʐݒ�̓��t����
			{
				char szText[1024];
				SYSTEMTIME systime;
				::GetLocalTime( &systime );
				CShareData::getInstance()->MyGetDateFormat( systime, szText, sizeof( szText ) - 1 );
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			break;
		case 't':	//	���ʐݒ�̎�������
			{
				char szText[1024];
				SYSTEMTIME systime;
				::GetLocalTime( &systime );
				CShareData::getInstance()->MyGetTimeFormat( systime, szText, sizeof( szText ) - 1 );
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			break;
		case 'p':	//	���݂̃y�[�W
			{
				CEditWnd*	pcEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta
				if (pcEditWnd->m_pPrintPreview){
					char szText[1024];
					itoa(pcEditWnd->m_pPrintPreview->GetCurPageNum() + 1, szText, 10);
					q = strncpy_ex( q, q_max - q, szText, strlen(szText));
					++p;
				}
				else {
					q = strncpy_ex( q, q_max - q, PRINT_PREVIEW_ONLY, PRINT_PREVIEW_ONLY_LEN );
					++p;
				}
			}
			break;
		case 'P':	//	���y�[�W
			{
				CEditWnd*	pcEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta
				if (pcEditWnd->m_pPrintPreview){
					char szText[1024];
					itoa(pcEditWnd->m_pPrintPreview->GetAllPageNum(), szText, 10);
					q = strncpy_ex( q, q_max - q, szText, strlen(szText));
					++p;
				}
				else {
					q = strncpy_ex( q, q_max - q, PRINT_PREVIEW_ONLY, PRINT_PREVIEW_ONLY_LEN );
					++p;
				}
			}
			break;
		case 'D':	//	�^�C���X�^���v
			if (m_FileTime.dwLowDateTime){
				FILETIME	FileTime;
				SYSTEMTIME	systimeL;
				::FileTimeToLocalFileTime( &m_FileTime, &FileTime );
				::FileTimeToSystemTime( &FileTime, &systimeL );
				char szText[1024];
				CShareData::getInstance()->MyGetDateFormat( systimeL, szText, sizeof( szText ) - 1 );
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			else {
				q = strncpy_ex( q, q_max - q, NOT_SAVED, NOT_SAVED_LEN );
				++p;
			}
			break;
		case 'T':	//	�^�C���X�^���v
			if (m_FileTime.dwLowDateTime){
				FILETIME	FileTime;
				SYSTEMTIME	systimeL;
				::FileTimeToLocalFileTime( &m_FileTime, &FileTime );
				::FileTimeToSystemTime( &FileTime, &systimeL );
				char szText[1024];
				CShareData::getInstance()->MyGetTimeFormat( systimeL, szText, sizeof( szText ) - 1 );
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			else {
				q = strncpy_ex( q, q_max - q, NOT_SAVED, NOT_SAVED_LEN );
				++p;
			}
			break;
		case 'V':	// Apr. 4, 2003 genta
			// Version number
			{
				char buf[28]; // 6(�����܂�WORD�̍ő咷) * 4 + 4(�Œ蕔��)
				//	2004.05.13 Moca �o�[�W�����ԍ��́A�v���Z�X���ƂɎ擾����
				DWORD dwVersionMS, dwVersionLS;
				GetAppVersionInfo( NULL, VS_VERSION_INFO,
					&dwVersionMS, &dwVersionLS );
				int len = sprintf( buf, "%d.%d.%d.%d",
					HIWORD( dwVersionMS ),
					LOWORD( dwVersionMS ),
					HIWORD( dwVersionLS ),
					LOWORD( dwVersionLS )
				);
				q = strncpy_ex( q, q_max - q, buf, len );
				++p;
			}
			break;
		case 'h':	//	Apr. 4, 2003 genta
			//	Grep Key������ MAX 32����
			//	���g��SetParentCaption()���ڐA
			{
				CMemory		cmemDes;
				LimitStringLengthB( m_szGrepKey, lstrlen( m_szGrepKey ),
					(q_max - q > 32 ? 32 : q_max - q - 3), cmemDes );
				if( (int)lstrlen( m_szGrepKey ) > cmemDes.GetStringLength() ){
					cmemDes.AppendString( "...", 3 );
				}
				q = strncpy_ex( q, q_max - q, cmemDes.GetStringPtr(), cmemDes.GetStringLength());
				++p;
			}
			break;
		case 'S':	//	Sep. 15, 2005 FILE
			//	�T�N���G�f�B�^�̃t���p�X
			{
				char	szPath[_MAX_PATH + 1];

				::GetModuleFileName( NULL, szPath, sizeof(szPath) );
				q = strncpy_ex( q, q_max - q, szPath, strlen(szPath) );
				++p;
			}
			break;
		case 'I':	//	May. 19, 2007 ryoji
			//	ini�t�@�C���̃t���p�X
			{
				char	szPath[_MAX_PATH + 1];

				CShareData::getInstance()->GetIniFileName( szPath );
				q = strncpy_ex( q, q_max - q, szPath, strlen(szPath) );
				++p;
			}
			break;
		case 'M':	//	Sep. 15, 2005 FILE
			//	���ݎ��s���Ă���}�N���t�@�C���p�X�̎擾
			{
				// ���s���}�N���̃C���f�b�N�X�ԍ� (INVALID_MACRO_IDX:���� / STAND_KEYMACRO:�W���}�N��)
				switch( m_pcSMacroMgr->GetCurrentIdx() ){
				case INVALID_MACRO_IDX:
					break;
				case STAND_KEYMACRO:
					{
						char* pszMacroFilePath = CShareData::getInstance()->GetShareData()->m_szKeyMacroFileName;
						q = strncpy_ex( q, q_max - q, pszMacroFilePath, strlen(pszMacroFilePath) );
					}
					break;
				default:
					{
						char szMacroFilePath[_MAX_PATH * 2];
						int n = CShareData::getInstance()->GetMacroFilename( m_pcSMacroMgr->GetCurrentIdx(), szMacroFilePath, sizeof(szMacroFilePath) );
						if ( 0 < n ){
							q = strncpy_ex( q, q_max - q, szMacroFilePath, strlen(szMacroFilePath) );
						}
					}
					break;
				}
				++p;
			}
			break;
		//	Mar. 31, 2003 genta
		//	��������
		//	${cond:string1$:string2$:string3$}
		//	
		case '{':	// ��������
			{
				int cond;
				cond = ExParam_Evaluate( p + 1 );
				while( *p != '?' && *p != '\0' )
					++p;
				if( *p == '\0' )
					break;
				p = ExParam_SkipCond( p + 1, cond );
			}
			break;
		case ':':	// ��������̒���
			//	��������̖����܂�SKIP
			p = ExParam_SkipCond( p + 1, -1 );
			break;
		case '}':	// ��������̖���
			//	���ɂ��邱�Ƃ͂Ȃ�
			++p;
			break;
		default:
			*q++ = '$';
			*q++ = *p++;
			break;
		}
	}
	*q = '\0';
}

/*! @brief �����̓ǂݔ�΂�

	��������̍\�� ${cond:A0$:A1$:A2$:..$} �ɂ����āC
	�w�肵���ԍ��ɑΉ�����ʒu�̐擪�ւ̃|�C���^��Ԃ��D
	�w��ԍ��ɑΉ����镨���������$}�̎��̃|�C���^��Ԃ��D

	${���o�ꂵ���ꍇ�ɂ̓l�X�g�ƍl����$}�܂œǂݔ�΂��D

	@param pszSource [in] �X�L�������J�n���镶����̐擪�Dcond:�̎��̃A�h���X��n���D
	@param part [in] �ړ�����ԍ����ǂݔ�΂�$:�̐��D-1��^����ƍŌ�܂œǂݔ�΂��D

	@return �ړ���̃|�C���^�D�Y���̈�̐擪�����邢��$}�̒���D

	@author genta
	@date 2003.03.31 genta �쐬
*/
const char* CEditDoc::ExParam_SkipCond(const char* pszSource, int part)
{
	if( part == 0 )
		return pszSource;
	
	int nest = 0;	// ����q�̃��x��
	bool next = true;	// �p���t���O
	const char *p;
	for( p = pszSource; next && *p != '\0'; ++p ) {
		if( *p == '$' && p[1] != '\0' ){ // $�������Ȃ疳��
			switch( *(++p)){
			case '{':	// ����q�̊J�n
				++nest;
				break;
			case '}':
				if( nest == 0 ){
					//	�I���|�C���g�ɒB����
					next = false; 
				}
				else {
					//	�l�X�g���x����������
					--nest;
				}
				break;
			case ':':
				if( nest == 0 && --part == 0){ // ����q�łȂ��ꍇ�̂�
					//	�ړI�̃|�C���g
					next = false;
				}
				break;
			}
		}
	}
	return p;
}

/*!	@brief �����̕]��

	@param pCond [in] ������ʐ擪�D'?'�܂ł������ƌ��Ȃ��ĕ]������
	@return �]���̒l

	@note
	�|�C���^�̓ǂݔ�΂���Ƃ͍s��Ȃ��̂ŁC'?'�܂ł̓ǂݔ�΂���
	�Ăяo�����ŕʓr�s���K�v������D

	@author genta
	@date 2003.03.31 genta �쐬

*/
int CEditDoc::ExParam_Evaluate( const char* pCond )
{
	switch( *pCond ){
	case 'R': // �ǂݎ���p
		if( m_bReadOnly ){	/* �ǂݎ���p���[�h */
			return 0;
		}else
		if( 0 != m_nFileShareModeOld && /* �t�@�C���̔r�����䃂�[�h */
			NULL == m_hLockedFile		/* ���b�N���Ă��Ȃ� */
		){
			return 1;
		}else{
			return 2;
		}
	case 'w': // Grep���[�h/Output Mode
		if( m_bGrepMode ){
			return 0;
		}else if( m_bDebugMode ){
			return 1;
		}else {
			return 2;
		}
	case 'M': // �L�[�{�[�h�}�N���̋L�^��
		if( TRUE == m_pShareData->m_bRecordingKeyMacro &&
		m_pShareData->m_hwndRecordingKeyMacro == m_hwndParent ){ /* �E�B���h�E */
			return 0;
		}else {
			return 1;
		}
	case 'U': // �X�V
		if( IsModified()){
			return 0;
		}
		else {
			return 1;
		}
	case 'I': // �A�C�R��������Ă��邩
		if( ::IsIconic( m_hwndParent )){
			return 0;
		} else {
 			return 1;
 		}
	default:
		return 0;
	}
	return 0;
}

/*!	@brief �}�N���������s

	@param type [in] �������s�}�N���ԍ�
	@return

	@author ryoji
	@date 2006.09.01 ryoji �쐬
	@date 2007.07.20 genta HandleCommand�ɒǉ�����n���D
		�������s�}�N���Ŕ��s�����R�}���h�̓L�[�}�N���ɕۑ����Ȃ�
*/
void CEditDoc::RunAutoMacro( int idx, const char *pszSaveFilePath )
{
	static bool bRunning = false;

	if( bRunning )
		return;	// �ē�����s�͂��Ȃ�

	bRunning = true;
	if( m_pcSMacroMgr->IsEnabled(idx) ){
		if( !( ::GetAsyncKeyState(VK_SHIFT) & 0x8000 ) ){	// Shift �L�[��������Ă��Ȃ���Ύ��s
			if( NULL != pszSaveFilePath )
				strcpy( m_szSaveFilePath, pszSaveFilePath );
			//	2007.07.20 genta �������s�}�N���Ŕ��s�����R�}���h�̓L�[�}�N���ɕۑ����Ȃ�
			HandleCommand(( F_USERMACRO_0 + idx ) | FA_NONRECORD );
			m_szSaveFilePath[0] = '\0';
		}
	}
	bRunning = false;
}

/*[EOF]*/