#include "pch.h"
#include "ColourLayersAutoGUIDlg.h"

#define KEY_ARROW_TOP		38
#define KEY_ARROW_BOTTOM	40
#define KEY_ARROW_LEFT		37
#define KEY_ARROW_RIGHT		39

#define KEY_SHIFT			16
#define KEY_CTRL			17
#define KEY_E				69
#define KEY_R				82


void CColourLayersAutoGUIDlg::OnKeyDown(UINT nChar)
{

	bool bReDrawImage = false;
	switch (nChar)
	{
	case KEY_E:
	{
		if (nImagesCount)
		{
			CurrentImage = &Images[iEtalonIndex];
			
			if ((ImagesMask[iCurrentImageIndex] & IMAGE_ID_MAIN) == false)
				ReturnOriginalImagePositions();

			iCurrentImageIndex = iEtalonIndex;
			bReDrawImage = true;
		}
		break;
	}
	case KEY_R:
	{
		if (nImagesCount)
		{
			CurrentImage = &Images[iResultIndex];
			
			if ((ImagesMask[iCurrentImageIndex] & IMAGE_ID_MAIN) == false)
				ReturnOriginalImagePositions();

			iCurrentImageIndex = iResultIndex;
			bReDrawImage = true;
		}
		break;
	}
	case KEY_CTRL:
	{
		bCtrlPressed = true;
		break;
	}
	case KEY_SHIFT:
	{
		bShiftPressed = true;
	}
	default:
		break;
	}

	if (bReDrawImage == true)
	{
		PaintImage();
	}
	

}

void CColourLayersAutoGUIDlg::OnKeyUp(UINT nChar)
{
	switch (nChar)
	{
	case KEY_CTRL:
	{
		bCtrlPressed = false;
		break;
	}
	case KEY_SHIFT:
	{
		bShiftPressed = false;
	}
	default:
		break;
	}
}

BOOL CColourLayersAutoGUIDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		OnKeyDown(pMsg->wParam);
	}
	if (pMsg->message == WM_KEYUP)
	{
		OnKeyUp(pMsg->wParam);
	}

	return CDialog::PreTranslateMessage(pMsg);
}

