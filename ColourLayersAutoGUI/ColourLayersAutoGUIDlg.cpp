
// ColourLayersAutoGUIDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "ColourLayersAutoGUI.h"
#include "ColourLayersAutoGUIDlg.h"
#include "afxdialogex.h"
#include "DialogParams.h"
#include "io.h"
#include <thread> 
#include <dataarea.h>
#include "CLWorker.h"
#include <fstream>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CLParams Params;
CLParams ParamsBackup;

bool bLexemesSet;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:

	//afx_msg void OnBnClickedOk();

};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)

END_MESSAGE_MAP()


// CColourLayersAutoGUIDlg dialog



CColourLayersAutoGUIDlg::CColourLayersAutoGUIDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_COLOURLAYERSAUTOGUI_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CColourLayersAutoGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, TimeBox1);
	DDX_Control(pDX, IDC_EDIT2, CoordBox1);
	DDX_Control(pDX, IDC_EDIT3, CoordBox2);
	DDX_Control(pDX, IDC_EDIT4, CoordBox3);
	DDX_Control(pDX, IDC_EDIT5, CoordBox4);
	DDX_Control(pDX, IDC_LIST1, MainList);

	DDX_Control(pDX, IDC_CHECK1, LexemesSetBox);
	DDX_Control(pDX, IDC_PROGRESS1, ProgressBar);
}

BEGIN_MESSAGE_MAP(CColourLayersAutoGUIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CColourLayersAutoGUIDlg::OnBnClickedButton1)
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BUTTON2, &CColourLayersAutoGUIDlg::OnBnClickedButton2)
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_MESSAGE(WM_UPDATE_CONTROL, &CColourLayersAutoGUIDlg::OnUpdateControl)
	ON_MESSAGE(WM_UPDATE_PROGRESS, &CColourLayersAutoGUIDlg::OnUpdateProgress)
	ON_MESSAGE(WM_SHOW_MESSAGEBOX, &CColourLayersAutoGUIDlg::OnShowMessageBox)
	ON_LBN_SELCHANGE(IDC_LIST1, &CColourLayersAutoGUIDlg::OnLbnSelchangeList1)
	ON_BN_CLICKED(IDC_BUTTON_PARAMS, &CColourLayersAutoGUIDlg::OnBnClickedButtonParams)
	ON_BN_CLICKED(IDC_BUTTON_LE, &CColourLayersAutoGUIDlg::OnBnClickedButtonLoadEt)
	ON_BN_CLICKED(IDC_BUTTON_LL, &CColourLayersAutoGUIDlg::OnBnClickedButtonLoadLe)
	ON_BN_CLICKED(IDC_CHECK1, &CColourLayersAutoGUIDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDCANCEL, &CColourLayersAutoGUIDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_SR, &CColourLayersAutoGUIDlg::OnBnClickedButtonSr)
	ON_BN_CLICKED(IDC_BUTTON_PC, &CColourLayersAutoGUIDlg::OnBnClickedButtonPc)
	ON_BN_CLICKED(IDC_BUTTON_FC, &CColourLayersAutoGUIDlg::OnBnClickedButtonFc)
END_MESSAGE_MAP()


// CColourLayersAutoGUIDlg message handlers

LRESULT CColourLayersAutoGUIDlg::OnUpdateControl(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 1)
	{
		char* info = (char*)lParam;

		if (!strcmp(info, "Algo_4"))
		{
			//Не хочу его добавлять в список
			delete[] info;
			return 0;
		}
		
		MainList.AddString(info);

		LoadCustomImage(info);


		delete[] info;
	}
	return 0;
}

LRESULT CColourLayersAutoGUIDlg::OnShowMessageBox(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 1)
	{
		char* info = (char*)lParam;

		MessageBox(info, "Error info");

		ReleaseButtons();
	}
	return 0;
}

LRESULT CColourLayersAutoGUIDlg::OnUpdateProgress(WPARAM wParam, LPARAM lParam)
{

	if (wParam == 1)
	{
		char* info = (char*)lParam;
		
		int nStep = atoi(info);

		delete[] info;
		
		ProgressBar.SetPos(nStep);
	}
	return 0;
}

void CColourLayersAutoGUIDlg::ReadLastParams()
{
	if (_access(CLAMainConfig, 0) == 0)
	{
		char szBuffer[1024];
		GetPrivateProfileString("all", "LastPreset", "all", szBuffer, sizeof(szBuffer) - 1, CLAMainConfig);

		strcpy_s(sLastPreset, szBuffer);
		
		GetPrivateProfileString("all", "LastLexemesState", "0", szBuffer, sizeof(szBuffer) - 1, CLAMainConfig);

		bLastLexemesState = szBuffer[0] != '0' && szBuffer[0] != 0;
		
	}
	else
	{

		ifstream f(dlgParams.ParamsPath);
		string s;

		while (f >> s)
		{
			if (s.length() >= 3 && s[0] == '[' && s[s.length() - 1] == ']')
			{
				s = s.substr(1, s.length() - 2);
				break;
			}
		}

		f.close();
		
		strcpy_s(sLastPreset, s.c_str());
		bLastLexemesState = false;
	}
}

void CColourLayersAutoGUIDlg::WriteLastParams()
{
	if (_access(CLAMainConfig, 0) == 0)
	{
		char szBuffer[1024];

		strcpy_s(szBuffer, sLastPreset);
		
		WritePrivateProfileString("all", "LastPreset", szBuffer, CLAMainConfig);

		szBuffer[1] = 0;

		szBuffer[0] = bLastLexemesState ? '1' : '0';
		
		WritePrivateProfileString("all", "LastLexemesState", szBuffer, CLAMainConfig);

		bLastLexemesState = szBuffer[0] != '0' && szBuffer[0] != 0;

	}
}

void CColourLayersAutoGUIDlg::InitDefaultParams()
{
	Params.Radius = 3;

	Params.Radius2 = 4;

	Params.L_Coef_SQUARE = 0.06;

	Params.pCoef = 0.084;
	Params.startingRadiusCoef = 1.65;
	Params.L_Radius_Coef = 2;
	Params.LDirectionCoef = 2;
	Params.Mx_Div_Coef = 3;
	Params.M_Mul_Coef = sqrt(3);

	//Params.LAB_Merge_Limit = 1.6;
	Params.AB_Merge_Check_Limit = 6;
	Params.L_Coef_Clusters_Radius = 0.15;
	Params.L_Coef_Nearest_Draw = 0.15;

	Params.additionalInfo = false;
	Params.saveImages = true;

	Params.correctBackGroundPixels = false;

	Params.L_Merge_Limit = 1.6;
	Params.AB_Merge_Limit = 1.6;
	Params.tooLowPixelCount = 7;
	Params.redrawLimit = 5;
	Params.L_Coef_Redraw = 0.06;


	Params.pPresetColours = nullptr;
	Params.nPresetColoursCount = 0;
	Params.pForbiddenColours = nullptr;
	Params.nForbiddenColoursCount = 0;
}

void CColourLayersAutoGUIDlg::Initialize()
{
	bDragging = false;

	bLexemesSet = true;

	
	memset(EtalonPath, 0, sizeof(EtalonPath));
	memset(LexemesPath, 0, sizeof(LexemesPath));
	memset(ResultPath, 0, sizeof(ResultPath));

	_fullpath(CLAMainConfig, ".\\Params\\ColourLayersAuto.ini", MAX_PATH);
	
	InitDefaultParams();

	SetDlgItemInt(IDC_STATIC_PC, Params.nPresetColoursCount);
	SetDlgItemInt(IDC_STATIC_FC, Params.nForbiddenColoursCount);
	
	iEtalonIndex = 0;
	iResultIndex = 0;
	
	iCurrentImageIndex = 0;
	nImagesCount = 0;

	memset(ImagesMask, 0, sizeof(ImagesMask));
	memset(ImagesType, 0, sizeof(ImagesType));
	
	fScaleCoefX = fScaleCoefY = 1.0;
	fScaleCoefOldX = fScaleCoefOldY = 1.0;

	bCtrlPressed = false;
	bShiftPressed = false;
	
	CurrentInImageShiftLeft = 0;
	CurrentInImageShiftTop = 0;

	NullImage.Load(const_cast<char*>(".\\Images\\Sample.png"));
	CurrentImage = &NullImage;

	phObjectHandle = new HWND;
	
	RECT ClientRECT;

	GetClientRect(&ClientRECT);

	CWnd* pWnd = GetDlgItem(IDC_LIST1);

	RECT ListRECT;
	pWnd->GetWindowRect(&ListRECT);

	RECT LowestButtonRECT;

	pWnd = GetDlgItem(IDC_BUTTON2);

	pWnd->GetWindowRect(&LowestButtonRECT);
	
	WindowImageShowRect.left = ListRECT.right + 5;
	WindowImageShowRect.top = LowestButtonRECT.bottom + 10;
	WindowImageShowRect.right = ClientRECT.right - 50;
	WindowImageShowRect.bottom = ClientRECT.bottom - 50;

	::SetWindowPos(GetDlgItem(IDC_PROGRESS1)->m_hWnd, HWND_TOP,
		WindowImageShowRect.left, WindowImageShowRect.bottom + 5, WindowImageShowRect.right - WindowImageShowRect.left,
		25, NULL);
	
	::SetWindowPos(GetDlgItem(IDC_LIST1)->m_hWnd, HWND_TOP,
		LowestButtonRECT.left, WindowImageShowRect.top, ListRECT.right - LowestButtonRECT.left,
		WindowImageShowRect.bottom - WindowImageShowRect.top, NULL);



	if (CurrentImage->IsNull() == false)
		ReturnOriginalImagePositions();

	ReadLastParams();

	dlgParams.SetLastPreset(sLastPreset);
	
	LexemesSetBox.SetCheck(bLastLexemesState);

	bLexemesSet = !LexemesSetBox.GetCheck();
	
	if (bLastLexemesState == true)
	{
		GetDlgItem(IDC_BUTTON_LL)->EnableWindow(FALSE);
	}

	SetDlgItemText(IDC_STATIC_PARAMS, sLastPreset);


	_fullpath(dlgParams.config, dlgParams.ParamsPath, MAX_PATH);

	if (_access(dlgParams.config, 0) != 0)
	{
		MessageBox("No Ini File .\\Params\\additionalParams.ini, accepting default Params");
		return;
	}

	char buffer[20];

	strcpy_s(buffer, sLastPreset);
	
	dlgParams.FillParamsDlg(buffer, true);

	KillFocus();


	ProgressBar.SetRange(0, 5);

	ProgressBar.SetPos(0);

	


	
}


BOOL CColourLayersAutoGUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	
	
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	ShowWindow(SW_MAXIMIZE);

	Initialize();

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CColourLayersAutoGUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CColourLayersAutoGUIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		
		CDialogEx::OnPaint();
		PaintImage();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CColourLayersAutoGUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CColourLayersAutoGUIDlg::OnBnClickedButton1()
{

	if (nImagesCount)
	{

		switch (ImagesType[iCurrentImageIndex])
		{
		case IMAGE_TYPE_NONE:
			MessageBox("Default Image", "Description");

			break;
		case IMAGE_TYPE_SAMPLE:
			MessageBox("Эталон", "Description");

			break;
		case IMAGE_TYPE_ALGO:
			MessageBox("Промежуточный этап работы алгоритма\nКрасные пиксели - неопределившиеся\nСиние пиксели - не просматриваемые в данный момент\n", "Description");
			
			break;
		case IMAGE_TYPE_ARTEFACTS:
			MessageBox("Маска артефактных пикселей\nПоиск будет вестить на фиолетовых пикселях", "Description");

			break;
		case IMAGE_TYPE_COLOUR1:
			MessageBox("Найденные цвета на первом шаге", "Description");

			break;
		case IMAGE_TYPE_COLOUR2:
			MessageBox("Найденные цвета на втором шаге", "Description");

			break;
		case IMAGE_TYPE_COLOUR3:
			MessageBox("Найденные цвета", "Description");

			break;
		case IMAGE_TYPE_HISTO_1:
			MessageBox("Гистограмма в направлении между найденными цветами на первом шаге", "Description");

			break;
		case IMAGE_TYPE_HISTO_2:
			MessageBox("Гистограмма в направлении между найденными цветами на втором шаге", "Description");

			break;
		case IMAGE_TYPE_LAB_F_1:
			MessageBox("Гистограмма пространства LAB в проекции на AB\nЧем темнее пиксель, тем больше пикселей на изображении с таким цветом\nСам цвет подсвечен\nПервый шаг", "Description");

			break;
		case IMAGE_TYPE_LAB_F_2:
			MessageBox("Гистограмма пространства LAB в проекции на AB\nЧем темнее пиксель, тем больше пикселей на изображении с таким цветом\nСам цвет подсвечен\nВторой шаг", "Description");

			break;
		case IMAGE_TYPE_LAB_F_RAD_1:
			MessageBox("Гистограмма пространства LAB в проекции на AB\nЧем темнее пиксель, тем больше пикселей на изображении с таким цветом\nЦветом подсвечены кластеры цветов\nЦентры кластеров выделены красным цветом\nПервый шаг", "Description");

			break;
		case IMAGE_TYPE_LAB_F_RAD_2:
			MessageBox("Гистограмма пространства LAB в проекции на AB\nЧем темнее пиксель, тем больше пикселей на изображении с таким цветом\nЦветом подсвечены кластеры цветов\nЦентры кластеров выделены красным цветом\nВторой шаг", "Description");

			break;
		case IMAGE_TYPE_RESULT:
			MessageBox("Итоговое изображение с раскраской пикселей в те цвета, к которым они отнеслись\nКнопка сохранения сохранит изображения в формате для Visir-ов.", "Description");

			break;
		default:

			break;
		}
	}
	
}


extern UINT ColourLayersMain(LPVOID pvParam);



void CColourLayersAutoGUIDlg::OnBnClickedButton2()
{
	CurrentImage = &NullImage;
	
	ProgressBar.SetPos(0);

	for (int i = 0; i < nImagesCount; i++)
	{
		if (Images[i].IsNull() == false)
		{
			Images[i].Destroy();
			ImagesMask[i] = 0;
			ImagesType[i] = 0;
		}
	}

	MainList.ResetContent();

	iCurrentImageIndex = 0;
	nImagesCount = 0;


	*phObjectHandle = GetSafeHwnd();

	LockButtons();
	
	AfxBeginThread(ColourLayersMain, phObjectHandle);

	

	KillFocus();
}

void CColourLayersAutoGUIDlg::OnLbnSelchangeList1()
{
	int iCurrentImageIndexOld = iCurrentImageIndex;
	
	iCurrentImageIndex = MainList.GetCurSel();

	CurrentImage = &Images[iCurrentImageIndex];

	if (!((ImagesMask[iCurrentImageIndexOld] & IMAGE_ID_MAIN) && (ImagesMask[iCurrentImageIndex] & IMAGE_ID_MAIN)))
		ReturnOriginalImagePositions(ImagesMask[iCurrentImageIndex] & IMAGE_ID_SHOW_FULL);
	

	PaintImage();
}

void CColourLayersAutoGUIDlg::KillFocus()
{
	GotoDlgCtrl(GetDlgItem(IDC_LIST1));
}

void CColourLayersAutoGUIDlg::OnBnClickedButtonParams()
{
	INT_PTR nResponse = dlgParams.DoModal();

	if (nResponse == IDOK)
	{
		SetDlgItemText(IDC_STATIC_PARAMS, dlgParams.sCurrentSection);
		strcpy_s(sLastPreset, dlgParams.sCurrentSection);
	}
	else if (nResponse == IDCANCEL)
	{
		dlgParams.RestoreParams();
	}
	
}


void CColourLayersAutoGUIDlg::OnBnClickedButtonLoadEt()
{

	static char BASED_CODE szFilter[] = "Images |*.png; *.bmp; *.jpg; *.jpeg|All Files | *.*|| ";
	
	CFileDialog FileDlg(TRUE, nullptr, nullptr, 4 | 2, szFilter);

	if (FileDlg.DoModal() == IDOK)
	{
		strcpy_s(EtalonPath, FileDlg.GetPathName());

		if (EtalonPath[0])
		{
			char imagesPath[MAX_PATH];
			char etalonPath[MAX_PATH];

			strcpy_s(imagesPath, ".\\Images\\");
			strcpy_s(etalonPath, "Sample.png");

			char etalonFullPath1[MAX_PATH];

			sprintf_s(etalonFullPath1, "%s%s", imagesPath, etalonPath);

			CopyFile(EtalonPath, etalonFullPath1, FALSE);
			
			if (!NullImage.IsNull())
			{
				NullImage.Destroy();
			}
			NullImage.Load(const_cast<char*>(".\\Images\\Sample.png"));

			CurrentImage = &NullImage;

			if (CurrentImage->IsNull() == false)
				ReturnOriginalImagePositions();
			
			PaintImage();
		}
	}
	else
	{
		memset(EtalonPath, 0, sizeof(EtalonPath));
	}

}


void CColourLayersAutoGUIDlg::OnBnClickedButtonLoadLe()
{

	static char BASED_CODE szFilter[] = "Images |*.png; *.bmp; *.jpg; *.jpeg|All Files | *.*|| ";
	
	CFileDialog FileDlg(TRUE, nullptr, nullptr, 4 | 2, szFilter);

	if (FileDlg.DoModal() == IDOK)
	{
		strcpy_s(LexemesPath, FileDlg.GetPathName());

		if (LexemesPath[0])
		{
			char imagesPath[MAX_PATH];
			char lexemesPath[MAX_PATH];

			strcpy_s(imagesPath, ".\\Images\\");
			strcpy_s(lexemesPath, "SampleLexem.png");

			char lexemesFullPath1[MAX_PATH];

			sprintf_s(lexemesFullPath1, "%s%s", imagesPath, lexemesPath);

			CopyFile(LexemesPath, lexemesFullPath1, FALSE);
		
		}
		
	}
	else
	{
		memset(LexemesPath, 0, sizeof(LexemesPath));
	}
}


void CColourLayersAutoGUIDlg::OnBnClickedCheck1()
{
	bLexemesSet = !LexemesSetBox.GetCheck();

	if (bLexemesSet)
	{
		GetDlgItem(IDC_BUTTON_LL)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_LL)->EnableWindow(FALSE);
	}

	bLastLexemesState = !bLexemesSet;
	
}


void CColourLayersAutoGUIDlg::OnBnClickedCancel()
{
	
	WriteLastParams();
	
	CDialogEx::OnCancel();
}




void CColourLayersAutoGUIDlg::OnBnClickedButtonSr()
{
	CFileDialog FileDlg(FALSE, "png", "SampleCL", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, nullptr);

	if (FileDlg.DoModal() == IDOK)
	{
		bool Result = CopyFile(".\\Images\\SampleCL_Algo_CL.png", FileDlg.GetPathName(), FALSE);
		if (Result == false)
			MessageBox("No File .\\Images\\SampleCL_Algo_CL.png\nTry to recalc Algo");
	}
}


void CColourLayersAutoGUIDlg::OnBnClickedButtonPc()
{
	// Пресетные цвета

	dlgColours.SetPreset();
	
	INT_PTR nResponse = dlgColours.DoModal();

	if (nResponse == IDOK)
	{
		
	}
	else if (nResponse == IDCANCEL)
	{
		
	}

	SetDlgItemInt(IDC_STATIC_PC, Params.nPresetColoursCount);
}


void CColourLayersAutoGUIDlg::OnBnClickedButtonFc()
{
	// Запретные цвета

	dlgColours.SetForbidden();

	INT_PTR nResponse = dlgColours.DoModal();

	if (nResponse == IDOK)
	{

	}
	else if (nResponse == IDCANCEL)
	{

	}

	SetDlgItemInt(IDC_STATIC_FC, Params.nForbiddenColoursCount);
}

void CColourLayersAutoGUIDlg::GetLastPixelColours(UCHAR& R, UCHAR& G, UCHAR& B)
{
	R = GetDlgItemInt(IDC_COLOUR_R);
	G = GetDlgItemInt(IDC_COLOUR_G);
	B = GetDlgItemInt(IDC_COLOUR_B);
}

void CColourLayersAutoGUIDlg::SetCurrentPixelCoords(POINT pt)
{
	SetDlgItemInt(IDC_COORD_X, pt.x);
	SetDlgItemInt(IDC_COORD_Y, pt.y);
}

void CColourLayersAutoGUIDlg::SetCurrentPixelColours(UCHAR R, UCHAR G, UCHAR B)
{
	SetDlgItemInt(IDC_COLOUR_R, R);
	SetDlgItemInt(IDC_COLOUR_G, G);
	SetDlgItemInt(IDC_COLOUR_B, B);
}

void CColourLayersAutoGUIDlg::AddPresetColour(UCHAR R, UCHAR G, UCHAR B)
{
	PRGB_STRUCT tmp = new RGB_STRUCT[Params.nPresetColoursCount + 1];
	
	if (Params.pPresetColours)
	{
		memcpy(tmp, Params.pPresetColours, sizeof(RGB_STRUCT) * Params.nPresetColoursCount);
		delete[] Params.pPresetColours;
	}

	tmp[Params.nPresetColoursCount].R = R;
	tmp[Params.nPresetColoursCount].G = G;
	tmp[Params.nPresetColoursCount].B = B;

	Params.pPresetColours = tmp;
	
	Params.nPresetColoursCount++;

	SetDlgItemInt(IDC_STATIC_PC, Params.nPresetColoursCount);
}

void CColourLayersAutoGUIDlg::AddForbiddenColour(UCHAR R, UCHAR G, UCHAR B)
{

	PRGB_STRUCT tmp = new RGB_STRUCT[Params.nForbiddenColoursCount + 1];

	if (Params.pForbiddenColours)
	{
		memcpy(tmp, Params.pForbiddenColours, sizeof(RGB_STRUCT) * Params.nForbiddenColoursCount);
		delete[] Params.pForbiddenColours;
	}

	tmp[Params.nForbiddenColoursCount].R = R;
	tmp[Params.nForbiddenColoursCount].G = G;
	tmp[Params.nForbiddenColoursCount].B = B;

	Params.pForbiddenColours = tmp;

	Params.nForbiddenColoursCount++;

	SetDlgItemInt(IDC_STATIC_FC, Params.nForbiddenColoursCount);
}

void CColourLayersAutoGUIDlg::LockButtons()
{
	GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_PARAMS)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_LE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_LL)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK1)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_SR)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_PC)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_FC)->EnableWindow(FALSE);
}

void CColourLayersAutoGUIDlg::ReleaseButtons()
{
	GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_PARAMS)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_LE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_LL)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHECK1)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_SR)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_PC)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_FC)->EnableWindow(TRUE);

	if (LexemesSetBox.GetCheck())
	{
		GetDlgItem(IDC_BUTTON_LL)->EnableWindow(FALSE);
	}
}