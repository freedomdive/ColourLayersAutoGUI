// DialogParams.cpp : implementation file
//

#include "pch.h"
#include "ColourLayersAutoGUI.h"
#include "DialogParams.h"
#include "afxdialogex.h"
#include "CLWorker.h"

extern CLParams Params;

// DialogParams dialog

IMPLEMENT_DYNAMIC(DialogParams, CDialogEx)

DialogParams::DialogParams(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PARAMS, pParent)
{
	sprintf_s(ParamsPath, ".\\Params\\additionalParams.ini");
	sprintf_s(PresetPath, ".\\Params\\presetColours.txt");
	sprintf_s(ForbiddenPath, ".\\Params\\forbiddenColours.txt");


	iCurrentParamsListIndex = 0;

	iCurrentParamsListIndexToShow = 0;
	
	memset(sCurrentSection, 0, sizeof(sCurrentSection));

	memset(sCurrentSectionToShow, 0, sizeof(sCurrentSectionToShow));
}

DialogParams::~DialogParams()
{
}

BOOL DialogParams::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	bool bEqual = iCurrentParamsListIndex == iCurrentParamsListIndexToShow;
	
	iCurrentParamsListIndex = iCurrentParamsListIndexToShow;

	memcpy(sCurrentSection, sCurrentSectionToShow, sizeof(sCurrentSectionToShow));
	
	static bool bFirstTime = true;

	auto ret = FillPresetList(bFirstTime, bEqual);

	bFirstTime = false;
	
	return ret;
}


void DialogParams::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, PresetsList);
	DDX_Control(pDX, IDC_PARAM6, cBGPCheckBox);
}



BEGIN_MESSAGE_MAP(DialogParams, CDialogEx)
	ON_LBN_SELCHANGE(IDC_LIST1, &DialogParams::OnLbnSelchangeList1)
	ON_EN_KILLFOCUS(IDC_PARAM1, &DialogParams::OnEnKillfocusParam1)
	ON_EN_KILLFOCUS(IDC_PARAM2, &DialogParams::OnEnKillfocusParam2)
	ON_EN_KILLFOCUS(IDC_PARAM3, &DialogParams::OnEnKillfocusParam3)
	ON_EN_KILLFOCUS(IDC_PARAM4, &DialogParams::OnEnKillfocusParam4)
	ON_EN_KILLFOCUS(IDC_PARAM5, &DialogParams::OnEnKillfocusParam5)
	ON_BN_CLICKED(IDC_PARAM6, &DialogParams::OnBnClickedCBGP)
	ON_BN_CLICKED(IDC_SAVE, &DialogParams::OnBnClickedSave)
	ON_BN_CLICKED(IDCANCEL, &DialogParams::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &DialogParams::OnBnClickedOk)
	ON_BN_CLICKED(ID_REVERT, &DialogParams::OnBnClickedRevert)
	ON_BN_CLICKED(IDC_MODIFY, &DialogParams::OnBnClickedModify)
END_MESSAGE_MAP()


// DialogParams message handlers








void DialogParams::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	//RestoreParams();
	
	CDialogEx::OnCancel();
}


void DialogParams::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here

	iCurrentParamsListIndexToShow = iCurrentParamsListIndex;

	memcpy(sCurrentSectionToShow, sCurrentSection, sizeof(sCurrentSectionToShow));
	
	CDialogEx::OnOK();
}


void DialogParams::OnBnClickedRevert()
{
	RestoreParams();
	ShowParams();
}


void DialogParams::OnBnClickedModify()
{
	system("start notepad.exe .\\Params\\additionalParams.ini");

	
}
