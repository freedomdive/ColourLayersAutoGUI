#pragma once

#define STATE_PRESET	0
#define STATE_FORBIDDEN	1
#include "CLWorker.h"

// DialogColours dialog

class DialogColours : public CDialogEx
{
	DECLARE_DYNAMIC(DialogColours)

public:
	DialogColours(CWnd* pParent = nullptr);   // standard constructor
	virtual ~DialogColours();

	void SetPreset() { nState = STATE_PRESET; }
	void SetForbidden() { nState = STATE_FORBIDDEN; }

	void FillListBox();
	
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COLOURS_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();

	void GetColours(PRGB_STRUCT& local, int& nColours);

	void SetColours(PRGB_STRUCT local, int nColours);

	void FillColour();
	
	int nState;
	
	DECLARE_MESSAGE_MAP()
public:
	CListBox ListBoxColours;
	afx_msg void OnBnClickedClear();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnPaint();
	afx_msg void OnLbnSelchangeList1();
};
