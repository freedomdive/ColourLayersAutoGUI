#include "pch.h"
#include "ColourLayersAutoGUIDlg.h"


BOOL CColourLayersAutoGUIDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (ImagesMask[iCurrentImageIndex] & IMAGE_ID_SHOW_FULL)
		return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
	
	// TODO: Add your message handler code here and/or call default
	if (pt.x >= WindowImageShowRect.left && pt.x <= WindowImageShowRect.right && pt.y >= WindowImageShowRect.top && pt.y <= WindowImageShowRect.bottom)
	{

		if (bCtrlPressed)
		{

			pt.x -= WindowImageShowRect.left;
			pt.y -= WindowImageShowRect.top;
			
			CurrentCursorCoefX = (double)pt.x / (WindowImageShowRect.right - WindowImageShowRect.left);
			CurrentCursorCoefY = (double)pt.y / (WindowImageShowRect.bottom - WindowImageShowRect.top);

			if (zDelta < 0)
				fScaleCoefX /= ReScaleFactor;
			else
				fScaleCoefX *= ReScaleFactor;
			if (fScaleCoefX < ScaleMinRange)
				fScaleCoefX = ScaleMinRange;
			if (fScaleCoefX > ScaleMaxRange)
				fScaleCoefX = ScaleMaxRange;


			ReCalcCurrentInImageShifts();

			PaintImage();
		}
		else
		{
			if (zDelta > 0)
			{
				CurrentInImageShiftTop -= 2.0 * 250 / fScaleCoefX;
			}
			else
			{
				CurrentInImageShiftTop += 2.0 * 250 / fScaleCoefX;
			}

			PaintImage();
		}

	}

	


	
	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}
void CColourLayersAutoGUIDlg::OnRButtonDown(UINT shift, CPoint pt)
{
	if (CurrentImage->IsNull() == false && pt.x >= WindowImageShowRect.left && pt.x <= WindowImageShowRect.right && pt.y >= WindowImageShowRect.top && pt.y <= WindowImageShowRect.bottom)
	{
		if (bShiftPressed)
		{
			UCHAR R, G, B;

			POINT ImageCoords = GetCurrentImageCoords(pt);
			
			GetCurrentImageColour(ImageCoords, R, G, B);

			AddForbiddenColour(R, G, B);
		}
	}
	
}

void CColourLayersAutoGUIDlg::OnLButtonDown(UINT shift, CPoint pt)
{
	if (CurrentImage->IsNull() == false && pt.x >= WindowImageShowRect.left && pt.x <= WindowImageShowRect.right && pt.y >= WindowImageShowRect.top && pt.y <= WindowImageShowRect.bottom)
	{
		if (bShiftPressed)
		{
			UCHAR R, G, B;

			POINT ImageCoords = GetCurrentImageCoords(pt);
			
			GetCurrentImageColour(ImageCoords, R, G, B);

			AddPresetColour(R, G, B);
		}
	}
	if (ImagesMask[iCurrentImageIndex] & IMAGE_ID_SHOW_FULL)
		return;
	OldCoords = pt;
	bDragging = true;
}


void CColourLayersAutoGUIDlg::OnLButtonUp(UINT shift, CPoint pt)
{
	bDragging = false;
}



void CColourLayersAutoGUIDlg::OnMouseMove(UINT nFlags, CPoint pt)
{
	
	if (CurrentImage->IsNull() == false && pt.x >= WindowImageShowRect.left && pt.x <= WindowImageShowRect.right && pt.y >= WindowImageShowRect.top && pt.y <= WindowImageShowRect.bottom)
	{
		
		UCHAR R = 0, G = 0, B = 0;


		POINT ImageCoords = GetCurrentImageCoords(pt);
		
		GetCurrentImageColour(ImageCoords, R, G, B);

		UCHAR ROld = 0, GOld = 0, BOld = 0;

		GetLastPixelColours(ROld, GOld, BOld);

		SetCurrentPixelCoords(ImageCoords);
		
		if (R != ROld || G != GOld || B != BOld)
			SetCurrentPixelColours(R, G, B);

		
		
	}

	

	
	if (ImagesMask[iCurrentImageIndex] & IMAGE_ID_SHOW_FULL)
		return;
	
	if (bDragging && !bCtrlPressed && !bShiftPressed)
	{
		CurrentInImageShiftLeft -= 2.0 * (pt.x - OldCoords.x) / fScaleCoefX;
		CurrentInImageShiftTop -= 2.0 * (pt.y - OldCoords.y) / fScaleCoefX;
		OldCoords = pt;
		PaintImage();

	}
	
	CDialog::OnMouseMove(nFlags, pt);

}