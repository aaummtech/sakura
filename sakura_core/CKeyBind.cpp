/*!	@file
	@brief �L�[���蓖�ĂɊւ���N���X

	@author Norio Nakatani
	@date 1998/03/25 �V�K�쐬
	@date 1998/05/16 �N���X���Ƀf�[�^�������Ȃ��悤�ɕύX
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, jepro, genta
	Copyright (C) 2002, YAZAKI, aroka
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CKeyBind.h"
#include "Debug.h"
#include "CSMacroMgr.h"// 2002/2/10 aroka
#include "CFuncLookup.h"
#include "Keycode.h"// 2002/2/10 aroka
#include "CMemory.h"// 2002/2/10 aroka


CKeyBind::CKeyBind()
{
	return;
}


CKeyBind::~CKeyBind()
{
	return;
}




/*! Windows �A�N�Z�����[�^�̍쐬
	@date 2007.02.22 ryoji �f�t�H���g�@�\���蓖�ĂɊւ��鏈����ǉ�
*/
HACCEL CKeyBind::CreateAccerelator(
		int			nKeyNameArrNum,
		KEYDATA*	pKeyNameArr
)
{
	ACCEL*	pAccelArr;
	int		nAccelArrNum;
	HACCEL	hAccel;
	int		i, j, k;

	/* �@�\�����蓖�Ă��Ă���L�[�̐����J�E���g */
	nAccelArrNum = 0;
	for( i = 0; i < nKeyNameArrNum; ++i ){
		if( 0 != pKeyNameArr[i].m_nKeyCode ){
			for( j = 0; j < 8; ++j ){
				if( 0 != GetFuncCodeAt( pKeyNameArr[i], j ) ){
					nAccelArrNum++;
				}
			}
		}
	}
//	nAccelArrNum = nKeyNameArrNum * 8;


	if( nAccelArrNum <= 0 ){
		/* �@�\���蓖�Ă��[�� */
		return NULL;
	}
	pAccelArr = new ACCEL[nAccelArrNum];
	k = 0;
	for( i = 0; i < nKeyNameArrNum; ++i ){
		if( 0 != pKeyNameArr[i].m_nKeyCode ){
			for( j = 0; j < 8; ++j ){
				if( 0 != GetFuncCodeAt( pKeyNameArr[i], j ) ){
					pAccelArr[k].fVirt = FNOINVERT | FVIRTKEY;;
					pAccelArr[k].key = pKeyNameArr[i].m_nKeyCode;
					pAccelArr[k].cmd = pKeyNameArr[i].m_nKeyCode | (((WORD)j)<<8) ;
					if( j & _SHIFT ){
						pAccelArr[k].fVirt |= FSHIFT;
					}
					if( j & _CTRL ){
						pAccelArr[k].fVirt |= FCONTROL;
					}
					if( j & _ALT ){
						pAccelArr[k].fVirt |= FALT;
					}
					k++;
				}
			}
		}
	}
	hAccel = ::CreateAcceleratorTable( pAccelArr, nAccelArrNum );
	delete [] pAccelArr;
	return hAccel;
}






/*! �A�N���Z���[�^���ʎq�ɑΉ�����R�}���h���ʎq��Ԃ��D
	�Ή�����A�N���Z���[�^���ʎq���Ȃ��ꍇ�܂��͋@�\�����蓖�Ă̏ꍇ��0��Ԃ��D

	@date 2007.02.22 ryoji �f�t�H���g�@�\���蓖�ĂɊւ��鏈����ǉ�
*/
int CKeyBind::GetFuncCode(
		WORD		nAccelCmd,
		int			nKeyNameArrNum,
		KEYDATA*	pKeyNameArr,
		BOOL		bGetDefFuncCode /* = TRUE */
)
{
	int i;
	int nCmd = (int)( nAccelCmd & 0x00ff );
	int nSts = (int)( ( nAccelCmd & 0xff00 ) >> 8 );
	for( i = 0; i < nKeyNameArrNum; ++i ){
		if( nCmd == pKeyNameArr[i].m_nKeyCode ){
			return GetFuncCodeAt( pKeyNameArr[i], nSts, bGetDefFuncCode );
		}
	}
	return 0;
}






/*!
	@param hInstance [in] �C���X�^���X�n���h��
	@param nKeyNameArrNum [in] 
	@param pKeyNameArr [out] 
	@param cMemList
	@param pcFuncLookup [in] �@�\�ԍ������O�̑Ή������
	@param bGetDefFuncCode [in] ON:�f�t�H���g�@�\���蓖�Ă��g��/OFF:�g��Ȃ�

	@return �@�\�����蓖�Ă��Ă���L�[�X�g���[�N�̐�
	
	@date Oct. 31, 2001 genta ���I�ȋ@�\���ɑΉ����邽�߈����ǉ�
	@date 2007.02.22 ryoji �f�t�H���g�@�\���蓖�ĂɊւ��鏈����ǉ�
*/
int CKeyBind::CreateKeyBindList(
		HINSTANCE	hInstance,
		int			nKeyNameArrNum,
		KEYDATA*	pKeyNameArr,
		CMemory&	cMemList,
		CFuncLookup* pcFuncLookup,
		BOOL		bGetDefFuncCode /* = TRUE */
)
{
	int		i;
	int		j;
	int		nValidKeys;
	char	pszStr[256];
	char	szFuncName[256];
	char	szFuncNameJapanese[256];

	nValidKeys = 0;
//	cMemList = "";
//	cMemList.SetData( "", strlen( "" ) );
	cMemList.SetString( "" );
	char*	pszSHIFT = "Shift+";
	char*	pszCTRL = "Ctrl+";
	char*	pszALT = "Alt+";
//	char*	pszEQUAL = " = ";
	char*	pszTAB = "\t";

//	char*	pszCR = "\n";	//Feb. 17, 2001 JEPRO \n=0x0a=LF���s���R�[�h�ɂȂ��Ă��܂��̂�
	char*	pszCR = "\r\n";	//\r=0x0d=CR��ǉ�


	cMemList.AppendString( "�L�[\t�@�\��\t�֐���\t�@�\�ԍ�\t�L�[�}�N���L�^��/�s��" );
	cMemList.AppendString( pszCR );
	cMemList.AppendString( "-----\t-----\t-----\t-----\t-----" );
	cMemList.AppendString( pszCR );

	for( j = 0; j < 8; ++j ){
		for( i = 0; i < nKeyNameArrNum; ++i ){
			int iFunc = GetFuncCodeAt( pKeyNameArr[i], j, bGetDefFuncCode );

			if( 0 != iFunc ){
				nValidKeys++;
				if( j & _SHIFT ){
					cMemList.AppendString( pszSHIFT );
				}
				if( j & _CTRL ){
					cMemList.AppendString( pszCTRL );
				}
				if( j & _ALT ){
					cMemList.AppendString( pszALT );
				}
				cMemList.AppendString( pKeyNameArr[i].m_szKeyName );
				//	Oct. 31, 2001 genta 
				if( !pcFuncLookup->Funccode2Name(
					iFunc,
					szFuncNameJapanese, 255 )){
					strcpy( szFuncNameJapanese, "---���O����`����Ă��Ȃ�-----" );
				}
				strcpy( szFuncName, ""/*"---unknown()--"*/ );

//				/* �@�\�����{�� */
//				::LoadString(
//					hInstance,
//					pKeyNameArr[i].m_nFuncCodeArr[j],
//					 szFuncNameJapanese, 255
//				);
				cMemList.AppendString( pszTAB );
				cMemList.AppendString( szFuncNameJapanese );

				/* �@�\ID���֐����C�@�\�����{�� */
				//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
//				CMacro::GetFuncInfoByID(
				CSMacroMgr::GetFuncInfoByID(
					hInstance,
					iFunc,
					szFuncName,
					szFuncNameJapanese
				);

				/* �֐��� */
				cMemList.AppendString( pszTAB );
				cMemList.AppendString( szFuncName );

				/* �@�\�ԍ� */
				cMemList.AppendString( pszTAB );
				wsprintf( pszStr, "%d", iFunc );
				cMemList.AppendString( pszStr );

				/* �L�[�}�N���ɋL�^�\�ȋ@�\���ǂ����𒲂ׂ� */
				cMemList.AppendString( pszTAB );
				//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
//				if( CMacro::CanFuncIsKeyMacro( pKeyNameArr[i].m_nFuncCodeArr[j] ) ){
				if( CSMacroMgr::CanFuncIsKeyMacro( iFunc ) ){
					cMemList.AppendString( "��" );
				}else{
					cMemList.AppendString( "�~" );
				}



				cMemList.AppendString( pszCR );
			}
		}
	}
//	delete [] pszStr;
	return nValidKeys;
}

/** �@�\�ɑΉ�����L�[���̃T�[�`(�⏕�֐�)

	�^����ꂽ�V�t�g��Ԃɑ΂��āC�w�肳�ꂽ�͈͂̃L�[�G���A����
	���Y�@�\�ɑΉ�����L�[�����邩�𒲂ׁC����������
	�Ή�����L�[��������Z�b�g����D
	
	�֐�����o��Ƃ��ɂ͌����J�n�ʒu(nKeyNameArrBegin)��
	���ɏ�������index��ݒ肷��D

	@param[in,out] nKeyNameArrBegin �����J�nINDEX (�I�����ɂ͎���̊J�nINDEX�ɏ�����������)
	@param[in] nKeyNameArrBegin �����I��INDEX + 1
	@param[in] pKeyNameArr �L�[�z��
	@param[in] nShiftState �V�t�g���
	@param[out] cMemList �L�[������ݒ��
	@param[in]	nFuncId �����Ώۋ@�\ID
	@param[in]	bGetDefFuncCode �W���@�\���擾���邩�ǂ���
*/
bool CKeyBind::GetKeyStrSub(
		int&		nKeyNameArrBegin,
		int			nKeyNameArrEnd,
		KEYDATA*	pKeyNameArr,
		int			nShiftState,
		CMemory&	cMemList,
		int			nFuncId,
		BOOL		bGetDefFuncCode /* = TRUE */
)
{
	const char pszSHIFT[] = "Shift+";
	const char pszCTRL[] = "Ctrl+";
	const char pszALT[] = "Alt+";

	int i;
	for( i = nKeyNameArrBegin; i < nKeyNameArrEnd; ++i ){
		if( nFuncId == GetFuncCodeAt( pKeyNameArr[i], nShiftState, bGetDefFuncCode ) ){
			if( nShiftState & _SHIFT ){
				cMemList.AppendString( pszSHIFT );
			}
			if( nShiftState & _CTRL ){
				cMemList.AppendString( pszCTRL );
			}
			if( nShiftState & _ALT ){
				cMemList.AppendString( pszALT );
			}
			cMemList.AppendString( pKeyNameArr[i].m_szKeyName );
			nKeyNameArrBegin = i + 1;
			return true;
		}
	}
	nKeyNameArrBegin = i;
	return false;
}


/** �@�\�ɑΉ�����L�[���̎擾

	@date 2007.02.22 ryoji �f�t�H���g�@�\���蓖�ĂɊւ��鏈����ǉ�
	@date 2007.11.04 genta �}�E�X�N���b�N���L�[���蓖�Ă̗D��x���グ��
	@date 2007.11.04 genta ���ʋ@�\�̃T�u���[�`����
*/
int CKeyBind::GetKeyStr(
		HINSTANCE	hInstance,
		int			nKeyNameArrNum,
		KEYDATA*	pKeyNameArr,
		CMemory&	cMemList,
		int			nFuncId,
		BOOL		bGetDefFuncCode /* = TRUE */
)
{
	int		i;
	int		j;
	char*	pszSHIFT = "Shift+";
	char*	pszCTRL = "Ctrl+";
	char*	pszALT = "Alt+";
	cMemList.SetString( "" );

	//	��ɃL�[�����𒲍�����
	for( j = 0; j < 8; ++j ){
		for( i = MOUSEFUNCTION_KEYBEGIN; i < nKeyNameArrNum; /* 1�������Ă͂����Ȃ� */ ){
			if( GetKeyStrSub( i, nKeyNameArrNum, pKeyNameArr, j, cMemList, nFuncId, bGetDefFuncCode )){
				return 1;
			}
		}
	}

	//	��Ƀ}�E�X�����𒲍�����
	for( j = 0; j < 8; ++j ){
		for( i = 0; i < MOUSEFUNCTION_KEYBEGIN; /* 1�������Ă͂����Ȃ� */ ){
			if( GetKeyStrSub( i, nKeyNameArrNum, pKeyNameArr, j, cMemList, nFuncId, bGetDefFuncCode )){
				return 1;
			}
		}
	}
	return 0;
}


/** �@�\�ɑΉ�����L�[���̎擾(����)

	@date 2007.02.22 ryoji �f�t�H���g�@�\���蓖�ĂɊւ��鏈����ǉ�
	@date 2007.11.04 genta ���ʋ@�\�̃T�u���[�`����
*/
int CKeyBind::GetKeyStrList(
		HINSTANCE	hInstance,
		int			nKeyNameArrNum,
		KEYDATA*	pKeyNameArr,
		CMemory***	pppcMemList,
		int			nFuncId,
		BOOL		bGetDefFuncCode /* = TRUE */
)
{
	int		i;
	int		j;
	char*	pszSHIFT = "Shift+";
	char*	pszCTRL = "Ctrl+";
	char*	pszALT = "Alt+";
	int		nAssignedKeysNum;

	nAssignedKeysNum = 0;
	if( 0 == nFuncId ){
		return 0;
	}
	for( j = 0; j < 8; ++j ){
		for( i = 0; i < nKeyNameArrNum; ++i ){
			if( nFuncId == GetFuncCodeAt( pKeyNameArr[i], j, bGetDefFuncCode ) ){
				nAssignedKeysNum++;
			}
		}
	}
	if( 0 == nAssignedKeysNum ){
		return 0;
	}
	(*pppcMemList) = new CMemory*[nAssignedKeysNum + 1];
	for( i = 0; i < nAssignedKeysNum; ++i ){
		(*pppcMemList)[i] = new CMemory;
	}
	(*pppcMemList)[i] = NULL;


	nAssignedKeysNum = 0;
	for( j = 0; j < 8; ++j ){
		for( i = 0; i < nKeyNameArrNum; /* 1�������Ă͂����Ȃ� */ ){
			//	2007.11.04 genta ���ʋ@�\�̃T�u���[�`����
			if( GetKeyStrSub( i, nKeyNameArrNum, pKeyNameArr, j,
					*((*pppcMemList)[nAssignedKeysNum]), nFuncId, bGetDefFuncCode )){
				nAssignedKeysNum++;
			}
		}
	}
	return nAssignedKeysNum;
}


/*! ���j���[���x���̍쐬
	@date 2007.02.22 ryoji �f�t�H���g�@�\���蓖�ĂɊւ��鏈����ǉ�
*/
char* CKeyBind::GetMenuLabel(
		HINSTANCE	hInstance,
		int			nKeyNameArrNum,
		KEYDATA*	pKeyNameArr,
		int			nFuncId,
		char*		pszLabel,
		BOOL		bKeyStr,
		BOOL		bGetDefFuncCode /* = TRUE */
)
{
	CMemory		cMemList;
//	int			i;


	if( 0 == strlen( pszLabel ) ){
		strcpy( pszLabel, "-- undefined name --" );
		::LoadString( hInstance, nFuncId, pszLabel, 255 );
	}


	/* �@�\�ɑΉ�����L�[����ǉ����邩 */
	if( bKeyStr ){
		/* �@�\�ɑΉ�����L�[���̎擾 */
		if( ( IDM_SELWINDOW <= nFuncId && nFuncId <= IDM_SELWINDOW + 999 )
		 || ( IDM_SELMRU <= nFuncId && nFuncId <= IDM_SELMRU + 999 )
		 || ( IDM_SELOPENFOLDER <= nFuncId && nFuncId <= IDM_SELOPENFOLDER + 999 )
		 ){
		}else{
			strcat( pszLabel, "\t" );
		}
		if( GetKeyStr( hInstance, nKeyNameArrNum, pKeyNameArr, cMemList, nFuncId, bGetDefFuncCode ) ){
			strcat( pszLabel, cMemList.GetStringPtr() );
		}
	}
	return pszLabel;
}


/*! �L�[�̃f�t�H���g�@�\���擾����

	@param nKeyCode [in] �L�[�R�[�h
	@param nState [in] Shift,Ctrl,Alt�L�[���

	@return �@�\�ԍ�

	@date 2007.02.22 ryoji �V�K�쐬
*/
int CKeyBind::GetDefFuncCode( int nKeyCode, int nState )
{
	DLLSHAREDATA* pShareData = CShareData::getInstance()->GetShareData();
	if( pShareData == NULL )
		return 0;

	int nDefFuncCode = 0;
	if( nKeyCode == VK_F4 ){
		if( nState == _CTRL ){
			nDefFuncCode = F_FILECLOSE;	// ����(����)
			if( pShareData->m_Common.m_bDispTabWnd && !pShareData->m_Common.m_bDispTabWndMultiWin ){
				nDefFuncCode = F_WINCLOSE;	// ����
			}
		}else if( nState == _ALT ){
			nDefFuncCode = F_WINCLOSE;	// ����
			if( pShareData->m_Common.m_bDispTabWnd && !pShareData->m_Common.m_bDispTabWndMultiWin ){
				if( !pShareData->m_Common.m_bTab_CloseOneWin ){
					nDefFuncCode = F_GROUPCLOSE;	// �O���[�v�����	// 2007.06.20 ryoji
				}
			}
		}
	}
	return nDefFuncCode;
}


/*! ����̃L�[��񂩂�@�\�R�[�h���擾����

	@param KeyData [in] �L�[���
	@param nState [in] Shift,Ctrl,Alt�L�[���
	@param bGetDefFuncCode [in] �f�t�H���g�@�\���擾���邩�ǂ���

	@return �@�\�ԍ�

	@date 2007.03.07 ryoji �C�����C���֐�����ʏ�̊֐��ɕύX�iBCC�̍œK���o�O�΍�j
*/
int CKeyBind::GetFuncCodeAt( KEYDATA& KeyData, int nState, BOOL bGetDefFuncCode )
{
	if( 0 != KeyData.m_nFuncCodeArr[nState] )
		return KeyData.m_nFuncCodeArr[nState];
	if( bGetDefFuncCode )
		return GetDefFuncCode( KeyData.m_nKeyCode, nState );
	return 0;
};


/*[EOF]*/