#include "stdafx.h"
#include "CViewSelect.h"
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "mem/CMemoryIterator.h"
#include "doc/CLayout.h"
#include "window/CEditWnd.h"

CViewSelect::CViewSelect(CEditView* pcEditView)
: m_pcEditView(pcEditView)
{
	m_bSelectingLock   = false;	// �I����Ԃ̃��b�N
	m_bBeginSelect     = false;		// �͈͑I��
	m_bBeginBoxSelect  = false;	// ��`�͈͑I��
	m_bBeginLineSelect = false;	// �s�P�ʑI��
	m_bBeginWordSelect = false;	// �P��P�ʑI��

	m_sSelectBgn.Clear(-1); // �͈͑I��(���_)
	m_sSelect   .Clear(-1); // �͈͑I��
	m_sSelectOld.Clear(0);  // �͈͑I��(Old)
}

void CViewSelect::CopySelectStatus(CViewSelect* pSelect) const
{
	pSelect->m_bSelectingLock		= m_bSelectingLock;		/* �I����Ԃ̃��b�N */
	pSelect->m_bBeginSelect			= m_bBeginSelect;		/* �͈͑I�� */
	pSelect->m_bBeginBoxSelect		= m_bBeginBoxSelect;	/* ��`�͈͑I�� */

	pSelect->m_sSelectBgn			= m_sSelectBgn;			//�͈͑I��(���_)
	pSelect->m_sSelect				= m_sSelect;			//�͈͑I��
	pSelect->m_sSelectOld			= m_sSelectOld;			//�͈͑I��

	pSelect->m_ptMouseRollPosOld	= m_ptMouseRollPosOld;	// �}�E�X�͈͑I��O��ʒu(XY���W)
}

//! ���݂̃J�[�\���ʒu����I�����J�n����
void CViewSelect::BeginSelectArea()
{
	const CEditView* pView=GetEditView();

	m_sSelectBgn.Set(pView->GetCaret().GetCaretLayoutPos()); //�͈͑I��(���_)
	m_sSelect.   Set(pView->GetCaret().GetCaretLayoutPos()); //�͈͑I��
}


// ���݂̑I��͈͂��I����Ԃɖ߂�
void CViewSelect::DisableSelectArea( bool bDraw )
{
	const CEditView* pView=GetEditView();
	CEditView* pView2=GetEditView();

	m_sSelectOld = m_sSelect;		//�͈͑I��(Old)
	m_sSelect.Clear(-1);

	if( bDraw ){
		DrawSelectArea();
		m_bDrawSelectArea = false;	// 02/12/13 ai
	}

	m_bSelectingLock	 = false;	// �I����Ԃ̃��b�N
	m_sSelectOld.Clear(0);			// �͈͑I��(Old)
	m_bBeginBoxSelect = false;		// ��`�͈͑I��
	m_bBeginLineSelect = false;		// �s�P�ʑI��
	m_bBeginWordSelect = false;		// �P��P�ʑI��

	// 2002.02.16 hor ���O�̃J�[�\���ʒu�����Z�b�g
	pView2->GetCaret().m_nCaretPosX_Prev=pView->GetCaret().GetCaretLayoutPos().GetX();

	// �J�[�\���s�A���_�[���C����ON
	pView2->GetCaret().m_cUnderLine.CaretUnderLineON( bDraw );
}



// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
void CViewSelect::ChangeSelectAreaByCurrentCursor( const CLayoutPoint& ptCaretPos )
{
	m_sSelectOld=m_sSelect; // �͈͑I��(Old)

	//	2002/04/08 YAZAKI �R�[�h�̏d����r��
	ChangeSelectAreaByCurrentCursorTEST(
		ptCaretPos,
		&m_sSelect
	);

	// �I��̈�̕`��
	DrawSelectArea();
}


// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX(�e�X�g�̂�)
void CViewSelect::ChangeSelectAreaByCurrentCursorTEST(
	const CLayoutPoint& ptCaretPos,
	CLayoutRange* pSelect
)
{
	const CEditView* pView=GetEditView();
	CEditView* pView2=GetEditView();

	if(m_sSelectBgn.GetFrom()==m_sSelectBgn.GetTo()){
		if( ptCaretPos==m_sSelectBgn.GetFrom() ){
			// �I������
			pSelect->Clear(-1);
		}
		else if( PointCompare(ptCaretPos,m_sSelectBgn.GetFrom() ) < 0 ){ //�L�����b�g�ʒu��m_sSelectBgn��from��菬����������
			 pSelect->SetFrom(ptCaretPos);
			 pSelect->SetTo(m_sSelectBgn.GetFrom());
		}
		else{
			pSelect->SetFrom(m_sSelectBgn.GetFrom());
			pSelect->SetTo(ptCaretPos);
		}
	}
	else{
		// �펞�I��͈͈͓͂̔�
		// �L�����b�g�ʒu�� m_sSelectBgn �� from�ȏ�ŁAto��菬�����ꍇ
		if( PointCompare(ptCaretPos,m_sSelectBgn.GetFrom()) >= 0 && PointCompare(ptCaretPos,m_sSelectBgn.GetTo()) < 0 ){
			pSelect->SetFrom(m_sSelectBgn.GetFrom());
			if ( ptCaretPos==m_sSelectBgn.GetFrom() ){
				pSelect->SetTo(m_sSelectBgn.GetTo());
			}
			else {
				pSelect->SetTo(ptCaretPos);
			}
		}
		//�L�����b�g�ʒu��m_sSelectBgn��from��菬����������
		else if( PointCompare(ptCaretPos,m_sSelectBgn.GetFrom()) < 0 ){
			// �펞�I��͈͂̑O����
			pSelect->SetFrom(ptCaretPos);
			pSelect->SetTo(m_sSelectBgn.GetTo());
		}
		else{
			// �펞�I��͈͂̌�����
			pSelect->SetFrom(m_sSelectBgn.GetFrom());
			pSelect->SetTo(ptCaretPos);
		}
	}
}



/*! �I��̈�̕`��

	@date 2006.10.01 Moca �d���R�[�h�폜�D��`�����P�D
*/
void CViewSelect::DrawSelectArea() const
{
	const CEditView* pView=GetEditView();

	if( !pView->GetDrawSwitch() ){
		return;
	}

	CLayoutRange sRangeA;

	CLayoutInt			nLineNum;

	m_bDrawSelectArea = true;

	// 2006.10.01 Moca �d���R�[�h����
	HDC         hdc = pView->GetDC();
	HBRUSH      hBrush = ::CreateSolidBrush( SELECTEDAREA_RGB );
	HBRUSH      hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
	int         nROP_Old = ::SetROP2( hdc, SELECTEDAREA_ROP2 );

//	MYTRACE_A( "DrawSelectArea()  m_bBeginBoxSelect=%ls\n", m_bBeginBoxSelect?"TRUE":"FALSE" );
	if( IsBoxSelecting() ){		// ��`�͈͑I��
		// 2001.12.21 hor ��`�G���A��EOF������ꍇ�ARGN_XOR�Ō��������
		// EOF�ȍ~�̃G���A�����]���Ă��܂��̂ŁA���̏ꍇ��Redraw���g��
		// 2002.02.16 hor �������}�~���邽��EOF�ȍ~�̃G���A�����]�����������x���]���Č��ɖ߂����Ƃɂ���
		//if((GetTextArea().GetViewTopLine()+m_nViewRowNum+1>=m_pcEditDoc->m_cLayoutMgr.GetLineCount()) &&
		//   (m_sSelect.GetTo().y+1 >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ||
		//	m_sSelectOld.GetTo().y+1 >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ) ) {
		//	Redraw();
		//	return;
		//}

		const int nCharWidth = pView->GetTextMetrics().GetHankakuDx();
		const int nCharHeight = pView->GetTextMetrics().GetHankakuDy();


		// 2�_��Ίp�Ƃ����`�����߂�
		CLayoutRect  rcOld;
		TwoPointToRect(
			&rcOld,
			m_sSelectOld.GetFrom(),	// �͈͑I���J�n
			m_sSelectOld.GetTo()	// �͈͑I���I��
		);
		rcOld.left   = t_max(rcOld.left  , pView->GetTextArea().GetViewLeftCol()  );
		rcOld.right  = t_max(rcOld.right , pView->GetTextArea().GetViewLeftCol()  );
		rcOld.right  = t_min(rcOld.right , pView->GetTextArea().GetRightCol() + 1 );
		rcOld.top    = t_max(rcOld.top   , pView->GetTextArea().GetViewTopLine()  );
		rcOld.bottom = t_min(rcOld.bottom, pView->GetTextArea().GetBottomLine()   );

		RECT rcOld2;
		rcOld2.left		= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)rcOld.left  * nCharWidth;
		rcOld2.right	= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)rcOld.right * nCharWidth;
		rcOld2.top		= (Int)( rcOld.top - pView->GetTextArea().GetViewTopLine() ) * nCharHeight + pView->GetTextArea().GetAreaTop();
		rcOld2.bottom	= (Int)( rcOld.bottom + 1 - pView->GetTextArea().GetViewTopLine() ) * nCharHeight + pView->GetTextArea().GetAreaTop();
		HRGN hrgnOld = ::CreateRectRgnIndirect( &rcOld2 );

		// 2�_��Ίp�Ƃ����`�����߂�
		CLayoutRect  rcNew;
		TwoPointToRect(
			&rcNew,
			m_sSelect.GetFrom(),	// �͈͑I���J�n
			m_sSelect.GetTo()		// �͈͑I���I��
		);
		rcNew.left   = t_max(rcNew.left  , pView->GetTextArea().GetViewLeftCol() );
		rcNew.right  = t_max(rcNew.right , pView->GetTextArea().GetViewLeftCol() );
		rcNew.right  = t_min(rcNew.right , pView->GetTextArea().GetRightCol() + 1);
		rcNew.top    = t_max(rcNew.top   , pView->GetTextArea().GetViewTopLine() );
		rcNew.bottom = t_min(rcNew.bottom, pView->GetTextArea().GetBottomLine()  );

		RECT rcNew2;
		rcNew2.left		= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)rcNew.left  * nCharWidth;
		rcNew2.right	= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)rcNew.right * nCharWidth;
		rcNew2.top		= (Int)(rcNew.top - pView->GetTextArea().GetViewTopLine()) * nCharHeight + pView->GetTextArea().GetAreaTop();
		rcNew2.bottom	= (Int)(rcNew.bottom + 1 - pView->GetTextArea().GetViewTopLine()) * nCharHeight + pView->GetTextArea().GetAreaTop();

		HRGN hrgnNew = ::CreateRectRgnIndirect( &rcNew2 );

		// ��`���B
		// ::CombineRgn()�̌��ʂ��󂯎�邽�߂ɁA�K���ȃ��[�W���������
		HRGN hrgnDraw = ::CreateRectRgnIndirect( &rcNew2 );
		{
			// ���I����`�ƐV�I����`�̃��[�W������������� �d�Ȃ肠�������������������܂�
			if( NULLREGION != ::CombineRgn( hrgnDraw, hrgnOld, hrgnNew, RGN_XOR ) ){

				// 2002.02.16 hor
				// ������̃G���A��EOF���܂܂��ꍇ��EOF�ȍ~�̕������������܂�
				// 2006.10.01 Moca ���[�\�[�X���[�N���C��������A�`�����悤�ɂȂ������߁A
				// �}���邽�߂� EOF�ȍ~�����[�W��������폜����1�x�̍��ɂ���

				// 2006.10.01 Moca Start EOF�ʒu�v�Z��GetEndLayoutPos�ɏ��������B
				CLayoutPoint ptLast;
				pView->m_pcEditDoc->m_cLayoutMgr.GetEndLayoutPos( &ptLast );
				// 2006.10.01 Moca End
				if(m_sSelect.GetFrom().y>=ptLast.y || m_sSelect.GetTo().y>=ptLast.y ||
					m_sSelectOld.GetFrom().y>=ptLast.y || m_sSelectOld.GetTo().y>=ptLast.y){
					//	Jan. 24, 2004 genta nLastLen�͕������Ȃ̂ŕϊ��K�v
					//	�ŏI�s��TAB�������Ă���Ɣ��]�͈͂��s������D
					//	2006.10.01 Moca GetEndLayoutPos�ŏ������邽��ColumnToIndex�͕s�v�ɁB
					RECT rcNew;
					rcNew.left   = pView->GetTextArea().GetAreaLeft() + (Int)(pView->GetTextArea().GetViewLeftCol() + ptLast.x) * nCharWidth;
					rcNew.right  = pView->GetTextArea().GetAreaRight();
					rcNew.top    = (Int)(ptLast.y - pView->GetTextArea().GetViewTopLine()) * nCharHeight + pView->GetTextArea().GetAreaTop();
					rcNew.bottom = rcNew.top + nCharHeight;
					
					// 2006.10.01 Moca GDI(���[�W����)���\�[�X���[�N�C��
					HRGN hrgnEOFNew = ::CreateRectRgnIndirect( &rcNew );
					::CombineRgn( hrgnDraw, hrgnDraw, hrgnEOFNew, RGN_DIFF );
					::DeleteObject( hrgnEOFNew );
				}
				::PaintRgn( hdc, hrgnDraw );
			}
		}

		//////////////////////////////////////////
		// �f�o�b�O�p ���[�W������`�̃_���v
//@@		TraceRgn( hrgnDraw );


		if( NULL != hrgnDraw ){
			::DeleteObject( hrgnDraw );
		}
		if( NULL != hrgnNew ){
			::DeleteObject( hrgnNew );
		}
		if( NULL != hrgnOld ){
			::DeleteObject( hrgnOld );
		}
	}else{

		// ���ݕ`�悳��Ă���͈͂Ǝn�_������
		if( m_sSelect == m_sSelectOld ){
			// �͈͂�����Ɋg�傳�ꂽ
			if( PointCompare(m_sSelect.GetTo(),m_sSelectOld.GetTo()) > 0 ){
				sRangeA.SetFrom(m_sSelectOld.GetTo());
				sRangeA.SetTo  (m_sSelect.GetTo());
			}
			else{
				sRangeA.SetFrom(m_sSelect.GetTo());
				sRangeA.SetTo  (m_sSelectOld.GetTo());
			}
			for( nLineNum = sRangeA.GetFrom().GetY2(); nLineNum <= sRangeA.GetTo().GetY2(); ++nLineNum ){
				if( nLineNum >= pView->GetTextArea().GetViewTopLine() && nLineNum <= pView->GetTextArea().GetBottomLine() + 1 ){
					DrawSelectAreaLine(	hdc, nLineNum, sRangeA);
				}
			}
		}
		else if( m_sSelect.GetTo() == m_sSelectOld.GetTo() ){
			// �͈͂��O���Ɋg�傳�ꂽ
			if(PointCompare(m_sSelect.GetFrom(),m_sSelectOld.GetFrom()) < 0){
				sRangeA.SetFrom(m_sSelect.GetFrom());
				sRangeA.SetTo  (m_sSelectOld.GetFrom());
			}
			else{
				sRangeA.SetFrom(m_sSelectOld.GetFrom());
				sRangeA.SetTo  (m_sSelect.GetFrom());
			}
			for( nLineNum = sRangeA.GetFrom().GetY2(); nLineNum <= sRangeA.GetTo().GetY2(); ++nLineNum ){
				if( nLineNum >= pView->GetTextArea().GetViewTopLine() && nLineNum <= pView->GetTextArea().GetBottomLine() + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, sRangeA );
				}
			}
		}
		else{
			sRangeA = m_sSelectOld;
			for( nLineNum = sRangeA.GetFrom().GetY2(); nLineNum <= sRangeA.GetTo().GetY2(); ++nLineNum ){
				if( nLineNum >= pView->GetTextArea().GetViewTopLine() && nLineNum <= pView->GetTextArea().GetBottomLine() + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, sRangeA );
				}
			}
			sRangeA = m_sSelect;
			for( nLineNum = sRangeA.GetFrom().GetY2(); nLineNum <= sRangeA.GetTo().GetY2(); ++nLineNum ){
				if( nLineNum >= pView->GetTextArea().GetViewTopLine() && nLineNum <= pView->GetTextArea().GetBottomLine() + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, sRangeA );
				}
			}
		}
	}
	// 2006.10.01 Moca �d���R�[�h����
	::SetROP2( hdc, nROP_Old );
	::SelectObject( hdc, hBrushOld );
	::DeleteObject( hBrush );
	pView->ReleaseDC( hdc );
	//	Jul. 9, 2005 genta �I��̈�̏���\��
	PrintSelectionInfoMsg();
}




/*! �I��̈�̒��̎w��s�̕`��

	�����s�ɓn��I��͈͂̂����CnLineNum�Ŏw�肳�ꂽ1�s��������`�悷��D
	�I��͈͂͌Œ肳�ꂽ�܂�nLineNum�݂̂��K�v�s���ω����Ȃ���Ăт������D

	@date 2006.03.29 Moca 3000��������P�p�D
*/
void CViewSelect::DrawSelectAreaLine(
	HDC					hdc,		//!< [in] �`��̈��Device Context Handle
	CLayoutInt			nLineNum,	//!< [in] �`��Ώۍs(���C�A�E�g�s)
	const CLayoutRange&	sRange		//!< [in] �I��͈�(���C�A�E�g�P��)
) const
{
	const CEditView* pView=GetEditView();

	RECT			rcClip;
	CLayoutInt		nSelectFrom;	// �`��s�̑I���J�n���ʒu
	CLayoutInt		nSelectTo;		// �`��s�̑I���J�n�I���ʒu

	if( sRange.IsLineOne() ){
		nSelectFrom = sRange.GetFrom().x;
		nSelectTo	= sRange.GetTo().x;
	}
	else{
		// 2006.03.29 Moca �s���܂ł̒��������߂�ʒu���ォ�炱���Ɉړ�
		CLayoutInt nPosX = CLayoutInt(0);
		const CLayout* pcLayout = pView->m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );
		CMemoryIterator it( pcLayout, pView->m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
		while( !it.end() ){
			it.scanNext();
			if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
				nPosX ++;
				break;
			}
			// 2006.03.28 Moca ��ʊO�܂ŋ��߂���ł��؂�
			if( it.getColumn() >pView->GetTextArea().GetRightCol() ){
				break;
			}
			it.addDelta();
		}
		nPosX += it.getColumn();
		
		if( nLineNum == sRange.GetFrom().y ){
			nSelectFrom = sRange.GetFrom().x;
			nSelectTo	= nPosX;
		}
		else if( nLineNum == sRange.GetTo().y ){
			nSelectFrom = pcLayout ? pcLayout->GetIndent() : CLayoutInt(0);
			nSelectTo	= sRange.GetTo().x;
		}
		else{
			nSelectFrom = pcLayout ? pcLayout->GetIndent() : CLayoutInt(0);
			nSelectTo	= nPosX;
		}
		// 2006.05.24 Moca�t���[�J�[�\���I��(�I���J�n/�I���s)��
		// To < From �ɂȂ邱�Ƃ�����B�K�� From < To �ɂȂ�悤�ɓ���ւ���B
		if( nSelectTo < nSelectFrom ){
			t_swap(nSelectFrom, nSelectTo);
		}
	}
	
	// 2006.03.28 Moca �E�B���h�E�����傫���Ɛ��������]���Ȃ������C��
	if( nSelectFrom < pView->GetTextArea().GetViewLeftCol() ){
		nSelectFrom = pView->GetTextArea().GetViewLeftCol();
	}
	int		nLineHeight = pView->GetTextMetrics().GetHankakuDy();
	int		nCharWidth = pView->GetTextMetrics().GetHankakuDx();
	rcClip.left		= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)nSelectFrom * nCharWidth;
	rcClip.right	= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)nSelectTo   * nCharWidth;
	rcClip.top		= (Int)(nLineNum - pView->GetTextArea().GetViewTopLine()) * nLineHeight + pView->GetTextArea().GetAreaTop();
	rcClip.bottom	= rcClip.top + nLineHeight;
	if( rcClip.right > pView->GetTextArea().GetAreaRight() ){
		rcClip.right = pView->GetTextArea().GetAreaRight();
	}
	//	�K�v�ȂƂ������B
	if ( rcClip.right != rcClip.left ){
		pView->GetCaret().m_cUnderLine.CaretUnderLineOFF( TRUE );
		
		// 2006.03.28 Moca �\������̂ݏ�������
		if( nSelectFrom <=pView->GetTextArea().GetRightCol() && pView->GetTextArea().GetViewLeftCol() < nSelectTo ){
			HRGN hrgnDraw = ::CreateRectRgn( rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );
			::PaintRgn( hdc, hrgnDraw );
			::DeleteObject( hrgnDraw );
		}
	}
}



/*!	�I��͈͏�񃁃b�Z�[�W�̕\��

	@author genta
	@date 2005.07.09 genta �V�K�쐬
	@date 2006.06.06 ryoji �I��͈͂̍s�����݂��Ȃ��ꍇ�̑΍��ǉ�
*/
void CViewSelect::PrintSelectionInfoMsg() const
{
	const CEditView* pView=GetEditView();

	//	�o�͂���Ȃ��Ȃ�v�Z���ȗ�
	if( ! pView->m_pcEditDoc->m_pcEditWnd->m_cStatusBar.SendStatusMessage2IsEffective() )
		return;

	if( ! IsTextSelected() ){
		pView->m_pcEditDoc->m_pcEditWnd->m_cStatusBar.SendStatusMessage2( _T("") );
		return;
	}

	TCHAR msg[128];
	//	From here 2006.06.06 ryoji �I��͈͂̍s�����݂��Ȃ��ꍇ�̑΍�
	CLayoutInt nLineCount = pView->m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	if( m_sSelect.GetFrom().y >= nLineCount ){	// �擪�s�����݂��Ȃ�
		pView->m_pcEditDoc->m_pcEditWnd->m_cStatusBar.SendStatusMessage2( _T("") );
		return;
	}

	CLayoutInt select_line;
	if( m_sSelect.GetTo().y >= nLineCount ){	// �ŏI�s�����݂��Ȃ�
		select_line = nLineCount - m_sSelect.GetFrom().y + 1;
	}
	else {
		select_line = m_sSelect.GetTo().y - m_sSelect.GetFrom().y + 1;
	}
	
	//	To here 2006.06.06 ryoji �I��͈͂̍s�����݂��Ȃ��ꍇ�̑΍�
	if( IsBoxSelecting() ){
		//	��`�̏ꍇ�͕��ƍ��������ł��܂���
		CLayoutInt select_col = m_sSelect.GetFrom().x - m_sSelect.GetTo().x;
		if( select_col < 0 ){
			select_col = -select_col;
		}
		auto_sprintf( msg, _T("%d Columns * %d lines selected."),
			select_col, select_line );
			
	}
	else {
		//	�ʏ�̑I���ł͑I��͈͂̒��g�𐔂���
		int select_sum = 0;	//	�o�C�g�����v
		const wchar_t *pLine;	//	�f�[�^���󂯎��
		CLogicInt	nLineLen;		//	�s�̒���
		const CLayout*	pcLayout;

		//	1�s��
		pLine = pView->m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_sSelect.GetFrom().GetY2(), &nLineLen, &pcLayout );
		if( pLine ){
			//	1�s�����I������Ă���ꍇ
			if( m_sSelect.IsLineOne() ){
				select_sum =
					pView->LineColmnToIndex( pcLayout, m_sSelect.GetTo().GetX2() )
					- pView->LineColmnToIndex( pcLayout, m_sSelect.GetFrom().GetX2() );
			}
			else {	//	2�s�ȏ�I������Ă���ꍇ
				select_sum =
					pcLayout->GetLengthWithoutEOL()
					+ pcLayout->GetLayoutEol().GetLen()
					- pView->LineColmnToIndex( pcLayout, m_sSelect.GetFrom().GetX2() );

				//	GetSelectedData�Ǝ��Ă��邪�C�擪�s�ƍŏI�s�͔r�����Ă���
				//	Aug. 16, 2005 aroka nLineNum��for�ȍ~�ł��g����̂�for�̑O�Ő錾����
				//	VC .NET�ȍ~�ł�Microsoft�g����L���ɂ����W�������VC6�Ɠ������Ƃɒ���
				CLayoutInt nLineNum;
				for( nLineNum = m_sSelect.GetFrom().GetY2() + CLayoutInt(1);
					nLineNum < m_sSelect.GetTo().GetY2(); ++nLineNum ){
					pLine = pView->m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
					//	2006.06.06 ryoji �w��s�̃f�[�^�����݂��Ȃ��ꍇ�̑΍�
					if( NULL == pLine )
						break;
					select_sum += pcLayout->GetLengthWithoutEOL() + pcLayout->GetLayoutEol().GetLen();
				}

				//	�ŏI�s�̏���
				pLine = pView->m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
				if( pLine ){
					int last_line_chars = pView->LineColmnToIndex( pcLayout, m_sSelect.GetTo().GetX2() );
					select_sum += last_line_chars;
					if( last_line_chars == 0 ){
						//	�ŏI�s�̐擪�ɃL�����b�g������ꍇ��
						//	���̍s���s���Ɋ܂߂Ȃ�
						--select_line;
					}
				}
				else
				{
					//	�ŏI�s����s�Ȃ�
					//	���̍s���s���Ɋ܂߂Ȃ�
					--select_line;
				}
			}
		}

#ifdef _DEBUG
		auto_sprintf( msg, _T("%d chars (%d lines) selected. [%d:%d]-[%d:%d]"),
			select_sum, select_line,
			m_sSelect.GetFrom().x, m_sSelect.GetFrom().y,
			m_sSelect.GetTo().x, m_sSelect.GetTo().y );
#else
		auto_sprintf( msg, _T("%d chars (%d lines) selected."), select_sum, select_line );
#endif
	}
	pView->m_pcEditDoc->m_pcEditWnd->m_cStatusBar.SendStatusMessage2( msg );
}