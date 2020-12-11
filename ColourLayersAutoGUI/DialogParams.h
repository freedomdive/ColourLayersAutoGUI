#pragma once


// DialogParams dialog

class DialogParams : public CDialogEx
{
	DECLARE_DYNAMIC(DialogParams)

public:
	DialogParams(CWnd* pParent = nullptr);   // standard constructor
	virtual ~DialogParams();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PARAMS };
#endif

	void FillParamsDlg(char* section, bool bInternal = false);

	char config[MAX_PATH];

	char sLastPreset[10];

	void SetLastPreset(char* sLastPreset) { strcpy_s(this->sLastPreset, sLastPreset); }
	
	char ParamsPath[MAX_PATH];
	char PresetPath[MAX_PATH];
	char ForbiddenPath[MAX_PATH];

	char sCurrentSection[20];

	char sCurrentSectionToShow[20];

	int iCurrentParamsListIndex;

	int iCurrentParamsListIndexToShow;

	void BackupParams();

	void RestoreParams();
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();
	
	void ShowParams();

	
	void SetDlgItemFloat(int nID, double fValue);
	
	
	BOOL FillPresetList(bool bFirstTime, bool bEqual);
	
	DECLARE_MESSAGE_MAP()
public:
	CListBox PresetsList;
	afx_msg void OnBnClickedCBGP();
	CButton cBGPCheckBox;
	afx_msg void OnLbnSelchangeList1();
	afx_msg void OnEnKillfocusParam1();
	afx_msg void OnEnKillfocusParam2();
	afx_msg void OnEnKillfocusParam3();
	afx_msg void OnEnKillfocusParam4();
	afx_msg void OnEnKillfocusParam5();
	afx_msg void OnBnClickedSave();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedRevert();
	afx_msg void OnBnClickedModify();
};
