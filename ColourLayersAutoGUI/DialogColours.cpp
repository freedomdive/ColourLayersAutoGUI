// DialogColours.cpp : implementation file
//

#include "pch.h"
#include "ColourLayersAutoGUI.h"
#include "DialogColours.h"
#include "afxdialogex.h"
#include "CLWorker.h"


extern CLParams Params;

// DialogColours dialog

IMPLEMENT_DYNAMIC(DialogColours, CDialogEx)

DialogColours::DialogColours(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_COLOURS_DIALOG, pParent)
{
	nState = STATE_PRESET;
}

DialogColours::~DialogColours()
{
}

void DialogColours::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, ListBoxColours);
}


BEGIN_MESSAGE_MAP(DialogColours, CDialogEx)
	ON_BN_CLICKED(ID_CLEAR, &DialogColours::OnBnClickedClear)
	ON_BN_CLICKED(ID_DELETE, &DialogColours::OnBnClickedDelete)
	ON_WM_PAINT()
	ON_LBN_SELCHANGE(IDC_LIST1, &DialogColours::OnLbnSelchangeList1)
END_MESSAGE_MAP()


// DialogColours message handlers


BOOL DialogColours::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (nState == STATE_PRESET)
		SetWindowText("Preset Colours");

	if (nState == STATE_FORBIDDEN)
		SetWindowText("Forbidden Colours");

	FillListBox();
	
	return TRUE;
}

void DialogColours::FillListBox()
{
	PRGB_STRUCT local;
	int nColoursCount;

	GetColours(local, nColoursCount);
	
	for (int i = 0; i < nColoursCount; i++)
	{
		char buffer[25];

		sprintf_s(buffer, "(%d, %d, %d)", local[i].R, local[i].G, local[i].B);
		ListBoxColours.AddString(buffer);
	}

	if (nColoursCount)
	{
		ListBoxColours.SetCurSel(0);
		//FillColour();
	}
}


void DialogColours::OnBnClickedClear()
{
	//Удалить все цвета
	ListBoxColours.ResetContent();

	SetColours(nullptr, 0);
	
}

void DialogColours::GetColours(PRGB_STRUCT& local, int& nColoursCount)
{
	if (nState == STATE_PRESET)
	{
		local = Params.pPresetColours;
		nColoursCount = Params.nPresetColoursCount;
		
	}
	
	if (nState == STATE_FORBIDDEN)
	{
		local = Params.pForbiddenColours;
		nColoursCount = Params.nForbiddenColoursCount;
	}
}

void DialogColours::SetColours(PRGB_STRUCT local, int nColoursCount)
{
	if (nState == STATE_PRESET)
	{
		PRGB_STRUCT tmp = nullptr;
		if (nColoursCount)
		{
			tmp = new RGB_STRUCT[nColoursCount];

			memcpy(tmp, local, nColoursCount * sizeof(RGB_STRUCT));
		}
		
		delete[] local;

		Params.pPresetColours = tmp;
		
		Params.nPresetColoursCount = nColoursCount;
	}

	if (nState == STATE_FORBIDDEN)
	{
		PRGB_STRUCT tmp = nullptr;
		if (nColoursCount)
		{
			tmp = new RGB_STRUCT[nColoursCount];

			memcpy(tmp, local, nColoursCount * sizeof(RGB_STRUCT));
		}
		
		delete[] local;

		Params.pForbiddenColours = tmp;
		
		Params.nForbiddenColoursCount = nColoursCount;
	}
}

void DialogColours::OnBnClickedDelete()
{
	//Удалить цвет

	
	PRGB_STRUCT local;
	int nColoursCount;
	
	GetColours(local, nColoursCount);

	if (nColoursCount == 0)
	{
		return;
	}


	int nDeletingColourIndex = ListBoxColours.GetCurSel();


	if (nColoursCount > 1)
	{
		PRGB_STRUCT newLocal;

		int nCountLocal = 0;

		newLocal = new RGB_STRUCT[nColoursCount - 1];
		
		for (int i = 0; i < nColoursCount; i++)
		{
			if (i != nDeletingColourIndex)
			{
				newLocal[nCountLocal].R = local[i].R;
				newLocal[nCountLocal].G = local[i].G;
				newLocal[nCountLocal].B = local[i].B;
				nCountLocal++;
			}
		}

		ListBoxColours.DeleteString(nDeletingColourIndex);

		ListBoxColours.SetCurSel(max(nDeletingColourIndex - 1, 0));
		
		SetColours(newLocal, nColoursCount - 1);
		OnPaint();
	}
	else
	{
		ListBoxColours.DeleteString(nDeletingColourIndex);

		SetColours(nullptr, 0);

		OnPaint();
	}

	GotoDlgCtrl(GetDlgItem(IDC_LIST1));
	
}

void DialogColours::FillColour()
{

	PRGB_STRUCT local;

	int nColoursCount;
	
	GetColours(local, nColoursCount);

	if (!nColoursCount)
		return;
	
	const UCHAR uDrawingR = local[ListBoxColours.GetCurSel()].R;
	const UCHAR uDrawingG = local[ListBoxColours.GetCurSel()].G;
	const UCHAR uDrawingB = local[ListBoxColours.GetCurSel()].B;
	
	RECT WindowImageShowRect = {238, 15, 471, 355};

	CImage Image;
	Image.Create(WindowImageShowRect.right - WindowImageShowRect.left, WindowImageShowRect.bottom - WindowImageShowRect.top, 3 * 8);

	const int Offset = Image.GetPitch();

	PUCHAR pData = (PUCHAR)Image.GetBits();
	
	for (int y = 0; y < WindowImageShowRect.bottom - WindowImageShowRect.top; y++)
	{
		int iShift = y * Offset;
		for (int x = 0; x < WindowImageShowRect.right - WindowImageShowRect.left; x++)
		{
			pData[iShift + 0] = uDrawingB;
			pData[iShift + 1] = uDrawingG;
			pData[iShift + 2] = uDrawingR;
			
			iShift += 3;
		}
	}

	Image.BitBlt(GetDC()->m_hDC, WindowImageShowRect.left, WindowImageShowRect.top);

	Image.Destroy();
}

void DialogColours::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting


	}
	else
	{
		CDialogEx::OnPaint();
		FillColour();
	}
}


void DialogColours::OnLbnSelchangeList1()
{
	// TODO: Add your control notification handler code here
	OnPaint();
}
