#include "pch.h"
#include "ColourLayersAutoGUIDlg.h"
#include "resource.h"


/*
 * Ф-ия берёт CurrentImage, выделяет временную область, перегоняет в неё CurrentImage с учётом сдвигов и коэффициента скалирования и отправляет в текущий DC
 */
void CColourLayersAutoGUIDlg::ScaleImage(CImage& Image, int left, int top,  double fCoefX, double fCoefY)
{
	if (Image.IsNull())
		return;

	
	PUCHAR pIn = (PUCHAR)Image.GetBits();

	int Width = Image.GetWidth();
	int Height = Image.GetHeight();
	int Offset = Image.GetPitch();
	int Plants = Image.GetBPP() / 8;
	
	int NewWidth = WindowImageShowRect.right - WindowImageShowRect.left;
	int NewHeight = WindowImageShowRect.bottom - WindowImageShowRect.top;
	

	CImage Image2;
	Image2.Create(NewWidth, NewHeight, Plants * 8);

	int OffsetNew = Image2.GetPitch();
	
	PUCHAR pOut = (PUCHAR)Image2.GetBits();


	
	double fy0 = top;

	int shiftOutStart = 0;

	for (int y = 0; y < NewHeight; y++)
	{
		int y0 = static_cast<int>(fy0 + 0.5);

		int shiftOut = shiftOutStart;

		double x0Start = left;

		for (int x = 0; x < NewWidth; x++)
		{
			int x0 = static_cast<int>(x0Start);

			if (y0 < 0 || y0 >= Height || x0 < 0 || x0 >= Width)
			{
				for (int iPlant = 0; iPlant < Plants; iPlant++)
				{
					pOut[shiftOut + iPlant] = 250;
				}
			}
			else
			{
				const int iShift = y0 * Offset + x0 * Plants;
				for (int iPlant = 0; iPlant < Plants; iPlant++)
				{
					pOut[shiftOut + iPlant] = pIn[iShift + iPlant];
				}
			}

			shiftOut += 3;
			x0Start += fCoefX;
		}

		fy0 += fCoefY;
		shiftOutStart += OffsetNew;
	}


	Image2.BitBlt(GetDC()->m_hDC, WindowImageShowRect.left, WindowImageShowRect.top);

	Image2.Destroy();

}

void CColourLayersAutoGUIDlg::ReCalcImageShowRect(bool bShow)
{
	int width = CurrentImage->GetWidth();
	int height = CurrentImage->GetHeight();

	int WindowWidth = WindowImageShowRect.right - WindowImageShowRect.left;
	int WindowHeight = WindowImageShowRect.bottom - WindowImageShowRect.top;

	double fScaleCoefX = 2.0 / ((double)width / WindowWidth);
	double fScaleCoefY = 2.0 / ((double)height / WindowHeight);
	

	ImageShowRect.right = fScaleCoefX * width / this->fScaleCoefX;
	ImageShowRect.bottom = fScaleCoefY * height / this->fScaleCoefX;

	ImageShowRect.left = CurrentInImageShiftLeft;
	ImageShowRect.top = CurrentInImageShiftTop;

	ImageShowRect.right += CurrentInImageShiftLeft;
	ImageShowRect.bottom += CurrentInImageShiftTop;

	if (bShow)
	{
		char text[10];

		sprintf_s(text, "%d", ImageShowRect.left);

		CoordBox1.SetWindowText(text);

		sprintf_s(text, "%d", ImageShowRect.top);

		CoordBox2.SetWindowText(text);

		sprintf_s(text, "%d", ImageShowRect.right);

		CoordBox3.SetWindowText(text);

		sprintf_s(text, "%d", ImageShowRect.bottom);

		CoordBox4.SetWindowText(text);
	}
}

void CColourLayersAutoGUIDlg::CalcStartScaleCoef(bool bSeperate)
{
	if (bSeperate == false)
	{
		const double StartCoef = max((double)CurrentImage->GetWidth() / (WindowImageShowRect.right - WindowImageShowRect.left), (double)CurrentImage->GetHeight() / (WindowImageShowRect.bottom - WindowImageShowRect.top));

		fScaleCoefX = 2.0 / StartCoef;
	}
	else
	{
		const double StartCoefX = (double)CurrentImage->GetWidth() / (WindowImageShowRect.right - WindowImageShowRect.left);
		const double StartCoefY = (double)CurrentImage->GetHeight() / (WindowImageShowRect.bottom - WindowImageShowRect.top);

		fScaleCoefX = 2.0 / StartCoefX;
		fScaleCoefY = 2.0 / StartCoefY;
	}
}

void CColourLayersAutoGUIDlg::ReCalcCurrentInImageShifts()
{
	if (fScaleCoefX == fScaleCoefOldX)
		return;


	double c = fScaleCoefX / fScaleCoefOldX;

	int wOld = ImageShowRect.right - ImageShowRect.left;
	int hOld = ImageShowRect.bottom - ImageShowRect.top;


	int wNew = wOld / c;
	int hNew = hOld / c;

	CurrentInImageShiftLeft += CurrentCursorCoefX * (wOld - wNew);
	CurrentInImageShiftTop += CurrentCursorCoefY * (hOld - hNew);
		
	
}

void CColourLayersAutoGUIDlg::CalcMaxInImageShifts()
{
	InImageShiftRightMax = ImageShowRect.right - ImageShowRect.left;
	InImageShiftBottomMax = ImageShowRect.bottom - ImageShowRect.top;
}

void CColourLayersAutoGUIDlg::ReCalcValidInImageShifts()
{
	if (CurrentInImageShiftLeft < 0)
		CurrentInImageShiftLeft = 0;

	if (CurrentInImageShiftTop < 0)
		CurrentInImageShiftTop = 0;

	double c = fScaleCoefX / fScaleCoefOldX;

	int NewWidth = (double)(ImageShowRect.right - ImageShowRect.left) / c;

	int NewHeight = (double)(ImageShowRect.bottom - ImageShowRect.top) / c;
	
	if (CurrentInImageShiftLeft + NewWidth > InImageShiftRightMax)
	{
		CurrentInImageShiftLeft -= CurrentInImageShiftLeft + NewWidth - InImageShiftRightMax;
	}

	if (CurrentInImageShiftTop + NewHeight > InImageShiftBottomMax)
	{
		CurrentInImageShiftTop -= CurrentInImageShiftTop + NewHeight - InImageShiftBottomMax;
	}

	
}

/*
 * Основной обработчик отрисовки окна
 */
void CColourLayersAutoGUIDlg::PaintImage()
{
	if (CurrentImage->IsNull() == true)
	{
		return;
	}
	
	ReCalcValidInImageShifts();
	
	auto s = clock();
	
	ScaleImage(*CurrentImage, CurrentInImageShiftLeft, CurrentInImageShiftTop, 2.0 / fScaleCoefX, ImagesMask[iCurrentImageIndex] & IMAGE_ID_SHOW_FULL ? 2.0 / fScaleCoefY : 2.0 / fScaleCoefX);

	
	auto time = clock() - s;

	fScaleCoefOldX = fScaleCoefX;
	
	ReCalcImageShowRect();

	
	
	char text[10];

	sprintf_s(text, "%d", time);

	TimeBox1.SetWindowText(text);
}



void CColourLayersAutoGUIDlg::LoadCustomImage(char* info)
{
	char imagesPath[MAX_PATH];
	char etalonPath[MAX_PATH];
	char lexemesPath[MAX_PATH];

	strcpy_s(imagesPath, ".\\Images\\");
	strcpy_s(etalonPath, "Sample.png");
	strcpy_s(lexemesPath, "SampleLexem.png");

	bool bShow = false;
	bool bFullScreen = false;
	bool bResult = false;
	bool bMain = false;
	
	char buffer[MAX_PATH];


	UCHAR uImageType = IMAGE_TYPE_NONE;

	
	if (!strcmp(info, "Sample"))
	{		
		sprintf_s(buffer, "%s%s", imagesPath, etalonPath);
		bShow = true;
		bMain = true;
		uImageType = IMAGE_TYPE_SAMPLE;
	}
	
	if (!strcmp(info, "artefacts"))
	{
		sprintf_s(buffer, "%s%s", imagesPath, "Sample_Algo_Missing.png");
		bShow = true;
		bMain = true;
		uImageType = IMAGE_TYPE_ARTEFACTS;
	}

	if (!strcmp(info, "Colours_1"))
	{
		sprintf_s(buffer, "%s%s", imagesPath, "Colours_1.png");
		bFullScreen = true;
		uImageType = IMAGE_TYPE_COLOUR1;
	}

	if (!strcmp(info, "Colours_2"))
	{
		sprintf_s(buffer, "%s%s", imagesPath, "Colours_2.png");
		bFullScreen = true;
		uImageType = IMAGE_TYPE_COLOUR2;
	}

	if (!strcmp(info, "Colours_3"))
	{
		sprintf_s(buffer, "%s%s", imagesPath, "Colours_3.png");
		bFullScreen = true;
		uImageType = IMAGE_TYPE_COLOUR3;
	}

	if (info[0] == 'L')
	{
		sprintf_s(buffer, "%s%s.png", imagesPath, info);
		if (!strcmp(info, "LAB_frequancy_1"))
			uImageType = IMAGE_TYPE_LAB_F_1;
		if (!strcmp(info, "LAB_frequancy_2"))
			uImageType = IMAGE_TYPE_LAB_F_2;
		if (!strcmp(info, "LAB_frequancy_Rad_1"))
			uImageType = IMAGE_TYPE_LAB_F_RAD_1;
		if (!strcmp(info, "LAB_frequancy_Rad_2"))
			uImageType = IMAGE_TYPE_LAB_F_RAD_2;

	}

	if (info[0] == 'H')
	{
		sprintf_s(buffer, "%sHisto_%c\\%s.png", imagesPath, info[6], info);
		bFullScreen = true;
		if (info[6] == '1')
			uImageType = IMAGE_TYPE_HISTO_1;
		else
			uImageType = IMAGE_TYPE_HISTO_2;
	}

	if (info[0] == 'A')
	{
		sprintf_s(buffer, "%sSampleCL_%s.png", imagesPath, info);
		bShow = true;
		bMain = true;
		uImageType = IMAGE_TYPE_ALGO;
	}

	if (!strcmp(info, "Result"))
	{
		sprintf_s(buffer, "%s%s", imagesPath, "SampleCL_Algo_CLResult.png");
		bShow = true;
		bResult = true;
		bMain = true;
		uImageType = IMAGE_TYPE_RESULT;
	}
	
	LoadCustomImageInner(buffer, bShow, bResult, bFullScreen, bMain, uImageType);
}

void CColourLayersAutoGUIDlg::LoadCustomImageInner(char* info, bool bShow, bool bResult, bool bFullScreen, bool bMain, UCHAR uImageType)
{
	//sprintf_s(ImagesPath[nImagesCount], info);

	if (bFullScreen)
		ImagesMask[nImagesCount] |= IMAGE_ID_SHOW_FULL;

	if (bMain)
		ImagesMask[nImagesCount] |= IMAGE_ID_MAIN;

	ImagesType[nImagesCount] = uImageType;
	
	CurrentInImageShiftLeft = 0;
	CurrentInImageShiftTop = 0;

	Images[nImagesCount].Load(const_cast<char*>(info));
	CurrentImage = &Images[nImagesCount];
	
	ReturnOriginalImagePositions(ImagesMask[iCurrentImageIndex] & IMAGE_ID_SHOW_FULL, bShow);

	if (bShow)
	{
		PaintImage();
		iCurrentImageIndex = nImagesCount;
	}

	if (bResult == true)
	{
		iResultIndex = nImagesCount;
		ReleaseButtons();
	}
	
	nImagesCount++;

}

void CColourLayersAutoGUIDlg::ReturnOriginalImagePositions(bool bSeperate, bool bShow)
{
	CurrentCursorCoefX = 0;
	CurrentCursorCoefY = 0;


	CalcStartScaleCoef(bSeperate);

	fScaleCoefOldX = fScaleCoefX;
	fScaleCoefOldY = fScaleCoefY;

	ReScaleFactor = 1.2;

	ScaleMinRange = fScaleCoefX;

	ScaleMaxRange = 25;


	ReCalcImageShowRect(bShow);

	CalcMaxInImageShifts();
}

POINT CColourLayersAutoGUIDlg::GetCurrentImageCoords(POINT pt)
{
	double fCurrentScaleX = 2.0 / fScaleCoefX; //Текущее скалирование по X

	double fCurrentScaleY = ImagesMask[iCurrentImageIndex] & IMAGE_ID_SHOW_FULL ? 2.0 / fScaleCoefY : 2.0 / fScaleCoefX; //Текущее скалирование по Y

	pt.x -= WindowImageShowRect.left;
	pt.y -= WindowImageShowRect.top;

	return { (int)(fCurrentScaleX * pt.x + CurrentInImageShiftLeft + 0.5), (int)(fCurrentScaleY * pt.y + CurrentInImageShiftTop + 0.5) };
}

void CColourLayersAutoGUIDlg::GetCurrentImageColour(POINT pt, UCHAR& R, UCHAR& G, UCHAR& B)
{
	//pt в координатах изображения (после GetCurrentImageCoords)
	//WindowImageShowRect.left - сдвиг изображения в окне по X
	//WindowImageShowRect.top - сдвиг изображения в окне по Y
	//CurrentInImageShiftLeft - текущий сдвиг изображения в зоне по X
	//CurrentInImageShiftTop - текущий сдвиг изображения в зоне по Y

	

	PUCHAR pOut = (PUCHAR)CurrentImage->GetBits();

	int Offset = CurrentImage->GetPitch();
	int Plants = CurrentImage->GetBPP() / 8;

	const int iShift = pt.y * Offset + pt.x * Plants;

	if (iShift <= 0 && iShift > Offset * (CurrentImage->GetHeight() - 1))
	{
		B = pOut[iShift];
		G = pOut[iShift + 1];
		R = pOut[iShift + 2];
		
	}
	
	
}