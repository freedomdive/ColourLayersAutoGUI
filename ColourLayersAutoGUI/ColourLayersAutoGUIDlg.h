
// ColourLayersAutoGUIDlg.h : header file
//

#pragma once

#define WM_UPDATE_CONTROL    WM_APP + 0x10
#define WM_SHOW_MESSAGEBOX   WM_APP + 0x11
#define WM_UPDATE_PROGRESS   WM_APP + 0x12


#define MAX_IMAGES_COUNT 512

#define IMAGE_ID_NONE			0
#define IMAGE_ID_SHOW_FULL		1	//ѕринудительно скалировать изображение
#define IMAGE_ID_MAIN			2	//Ќе сбрасывать сдвиги у изображений такого типа

#define IMAGE_TYPE_NONE				0
#define IMAGE_TYPE_SAMPLE			1
#define IMAGE_TYPE_ALGO				2
#define IMAGE_TYPE_ARTEFACTS		3
#define IMAGE_TYPE_COLOUR1			4
#define IMAGE_TYPE_COLOUR2			5
#define IMAGE_TYPE_COLOUR3			6
#define IMAGE_TYPE_HISTO_1			7
#define IMAGE_TYPE_HISTO_2			8
#define IMAGE_TYPE_LAB_F_1			9
#define IMAGE_TYPE_LAB_F_2			10
#define IMAGE_TYPE_LAB_F_RAD_1		11
#define IMAGE_TYPE_LAB_F_RAD_2		12
#define IMAGE_TYPE_RESULT			13



#include "CLWorker.h"
#include "DialogParams.h"
#include "DialogColours.h"


// CColourLayersAutoGUIDlg dialog
class CColourLayersAutoGUIDlg : public CDialogEx
{
// Construction
public:
	CColourLayersAutoGUIDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAIN_DIALOG };
#endif

	
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	

	void ReadLastParams();
	
	void WriteLastParams();
	
// Implementation
protected:
	void Initialize();

	void InitDefaultParams();
	
	void ReturnOriginalImagePositions(bool bSeperate = false, bool bShow = true);
	
	LRESULT OnUpdateControl(WPARAM wParam, LPARAM lParam);

	LRESULT OnShowMessageBox(WPARAM wParam, LPARAM lParam);

	LRESULT OnUpdateProgress(WPARAM wParam, LPARAM lParam);
	
	void LoadCustomImage(char* info);

	void LoadCustomImageInner(char* info, bool bShow, bool bResult, bool bFullScreen, bool bMain, UCHAR uImageType);
	
	//void ColourLayersMain(LPVOID pvParam);
	
	double ReScaleFactor;

	double ScaleMinRange;

	double ScaleMaxRange;
	
	int CurrentInImageShiftLeft;
	int CurrentInImageShiftTop;

	int InImageShiftRightMax;
	int InImageShiftBottomMax;

	char EtalonPath[MAX_PATH];
	char LexemesPath[MAX_PATH];

	//ќтображаемый RECT в координатах окна
	RECT WindowImageShowRect;

	//ќтображаемый RECT в координатах изображени€(часть изображени€ что видим)
	RECT ImageShowRect;

	bool bCtrlPressed;
	bool bShiftPressed;

	bool bDragging;

	char sLastPreset[10];
	bool bLastLexemesState;

	char CLAMainConfig[MAX_PATH];
	
	POINT OldCoords;
	
	double fScaleCoefX;
	double fScaleCoefY;
	double fScaleCoefOldX;
	double fScaleCoefOldY;
	
	CImage *CurrentImage;

	CImage NullImage;

	int iEtalonIndex;
	int iResultIndex;
	
	CImage Images[MAX_IMAGES_COUNT];
	UCHAR ImagesMask[MAX_IMAGES_COUNT];
	UCHAR ImagesType[MAX_IMAGES_COUNT];

	
	int iCurrentImageIndex;
	int nImagesCount;

	HWND* phObjectHandle;
	
	char ResultPath[MAX_PATH];

	
	double CurrentCursorCoefX;
	double CurrentCursorCoefY;

	
	DialogParams dlgParams;
	DialogColours dlgColours;

	void AddPresetColour(UCHAR R, UCHAR G, UCHAR B);

	void AddForbiddenColour(UCHAR R, UCHAR G, UCHAR B);
	
	void ReCalcCurrentInImageShifts();

	POINT GetCurrentImageCoords(POINT pt);
	
	void GetCurrentImageColour(POINT pt, UCHAR &R, UCHAR &G, UCHAR &B);

	void SetCurrentPixelCoords(POINT pt);
	
	void SetCurrentPixelColours(UCHAR R, UCHAR G, UCHAR B);

	void GetLastPixelColours(UCHAR& R, UCHAR& G, UCHAR& B);
	
	HICON m_hIcon;
	void PaintImage();

	void CalcStartScaleCoef(bool bSeperate = false);
	
	void ReCalcImageShowRect(bool bShow = true);

	void CalcMaxInImageShifts();

	void ReCalcValidInImageShifts();

	void KillFocus();

	void LockButtons();

	void ReleaseButtons();
	
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	void ScaleImage(CImage& Image, int left, int top, double fCoefX, double fCoefY);
	BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnLButtonUp(UINT, CPoint);
	afx_msg void OnLButtonDown(UINT, CPoint);
	afx_msg void OnRButtonDown(UINT, CPoint);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar);
	afx_msg void OnKeyUp(UINT nChar);
	
	afx_msg void OnBnClickedButton2();
	CEdit TimeBox1;
	CEdit CoordBox1;
	CEdit CoordBox2;

	CEdit CoordBox3;
	CEdit CoordBox4;
	
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	CListBox MainList;

	afx_msg void OnLbnSelchangeList1();
	afx_msg void OnBnClickedButtonParams();

	afx_msg void OnBnClickedButtonLoadEt();
	afx_msg void OnBnClickedButtonLoadLe();
	afx_msg void OnBnClickedCheck1();
	CButton LexemesSetBox;
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonSr();
	afx_msg void OnBnClickedButtonPc();
	afx_msg void OnBnClickedButtonFc();
	CProgressCtrl ProgressBar;
};
