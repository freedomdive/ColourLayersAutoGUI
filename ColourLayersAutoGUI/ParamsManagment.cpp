#include "pch.h"
#include <fstream>
#include "DialogParams.h"
#include "io.h"
#include "CLWorker.h"
#include "resource.h"
using namespace std;

extern CLParams Params;
extern CLParams ParamsBackup;

void DialogParams::FillParamsDlg(char * section, bool bInternal)
{
	if (config[0] == 0)
	{
		return;
	}
	
	//CLParams Params;

	
	char szBuffer[1024];
	GetPrivateProfileString(section, "Radius", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Params.Radius = atoi(szBuffer);


	GetPrivateProfileString(section, "Radius2", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Params.Radius2 = atoi(szBuffer);

	GetPrivateProfileString(section, "L_Coef_SQUARE", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Params.L_Coef_SQUARE = atof(szBuffer);

	
	GetPrivateProfileString(section, "pCoef", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Params.pCoef = atof(szBuffer);

	GetPrivateProfileString(section, "startingRadiusCoef", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Params.startingRadiusCoef = atof(szBuffer);

	GetPrivateProfileString(section, "L_Radius_Coef", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Params.L_Radius_Coef = atof(szBuffer);

	GetPrivateProfileString(section, "LDirectionCoef", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Params.LDirectionCoef = atof(szBuffer);

	GetPrivateProfileString(section, "Mx_Div_Coef", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Params.Mx_Div_Coef = atof(szBuffer);

	GetPrivateProfileString(section, "M_Mul_Coef", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Params.M_Mul_Coef = atof(szBuffer);

	
	GetPrivateProfileString(section, "AB_Merge_Check_Limit", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Params.AB_Merge_Check_Limit = atof(szBuffer);

	GetPrivateProfileString(section, "L_Coef_Clusters_Radius", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Params.L_Coef_Clusters_Radius = atof(szBuffer);

	GetPrivateProfileString(section, "L_Coef_Nearest_Draw", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Params.L_Coef_Nearest_Draw = atof(szBuffer);


	//Здесь не используем
	Params.additionalInfo = false;
	//Изображения считываются этим GUI
	Params.saveImages = true;

	GetPrivateProfileString(section, "L_Merge_Limit", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Params.L_Merge_Limit = atof(szBuffer);

	GetPrivateProfileString(section, "AB_Merge_Limit", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Params.AB_Merge_Limit = atof(szBuffer);

	GetPrivateProfileString(section, "tooLowPixelCount", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Params.tooLowPixelCount = atoi(szBuffer);

	GetPrivateProfileString(section, "redrawLimit", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Params.redrawLimit = atof(szBuffer);

	GetPrivateProfileString(section, "L_Coef_Redraw", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Params.L_Coef_Redraw = atof(szBuffer);

	GetPrivateProfileString(section, "correctBackGroundPixels", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Params.correctBackGroundPixels = szBuffer[0] != '0' && szBuffer[0] != 0;


	if (!bInternal)
		ShowParams();
	
}

BOOL DialogParams::FillPresetList(bool bFirstTime, bool bEqual)
{
	ifstream f(ParamsPath);
	string s;

	int iCurrentParamsListIndexLocal = 0;

	bool bFoundLastIndex = false;
	
	while (f >> s)
	{
		if (s.length() >= 3 && s[0] == '[' && s[s.length() - 1] == ']')
		{
			s = s.substr(1, s.length() - 2);

			if (bFoundLastIndex == false)
			{
				if (strcmp(sLastPreset, s.c_str()))
				{
					iCurrentParamsListIndexLocal++;

				}
				else
				{
					bFoundLastIndex = true;
				}
			}
			PresetsList.AddString(s.c_str());
		}
	}

	f.close();

	if (bFirstTime)
	{
		iCurrentParamsListIndex = iCurrentParamsListIndexToShow = iCurrentParamsListIndexLocal;
	}

	if (PresetsList.GetCount())
		PresetsList.SetCurSel(iCurrentParamsListIndex);

	char buffer[20];

	PresetsList.GetText(iCurrentParamsListIndex, buffer);
	
	if (bFirstTime)
	{
		strcpy_s(sCurrentSection, buffer);

		strcpy_s(sCurrentSectionToShow, buffer);
		
		FillParamsDlg(sCurrentSection);

		BackupParams();
	}
	else
	{
		if (bEqual == false)
		{
			FillParamsDlg(sCurrentSection);
		}
		else
		{
			ShowParams();
		}
	}
	
	
	return TRUE;
}

void DialogParams::SetDlgItemFloat(int nID, double fValue)
{
	char szBuffer[10];
	
	if (fValue == (int)fValue)
	{
		sprintf_s(szBuffer, "%.0f", fValue);
	}
	else
	{
		sprintf_s(szBuffer, "%.2f", fValue);

		int k = 0;

		for (k = 0; k < 10; k++)
		{
			if (szBuffer[k] == '.')
				break;
		}

		if (k < 8)
		{
			if (szBuffer[k + 2] == '0')
			{
				szBuffer[k + 2] = 0;
			}
		}

	}

	SetDlgItemText(nID, szBuffer);
}

void DialogParams::OnEnKillfocusParam1()
{
	char szBuffer[10];
	
	GetDlgItemText(IDC_PARAM1, szBuffer, 10);
	
 	Params.L_Merge_Limit = atof(szBuffer);

	

	SetDlgItemFloat(IDC_PARAM1, Params.L_Merge_Limit);
}

void DialogParams::OnEnKillfocusParam2()
{
	char szBuffer[10];

	GetDlgItemText(IDC_PARAM2, szBuffer, 10);

	Params.AB_Merge_Limit = atof(szBuffer);


	SetDlgItemFloat(IDC_PARAM2, Params.AB_Merge_Limit);
}


void DialogParams::OnEnKillfocusParam3()
{
	char szBuffer[10];

	GetDlgItemText(IDC_PARAM3, szBuffer, 10);

	Params.tooLowPixelCount = atoi(szBuffer);

	sprintf_s(szBuffer, "%d", Params.tooLowPixelCount);

	SetDlgItemText(IDC_PARAM3, szBuffer);

}


void DialogParams::OnEnKillfocusParam4()
{
	char szBuffer[10];

	GetDlgItemText(IDC_PARAM4, szBuffer, 10);

	Params.redrawLimit = atof(szBuffer);

	SetDlgItemFloat(IDC_PARAM4, Params.redrawLimit);
}


void DialogParams::OnEnKillfocusParam5()
{
	char szBuffer[10];

	GetDlgItemText(IDC_PARAM5, szBuffer, 10);

	Params.L_Coef_Redraw = atof(szBuffer);

	SetDlgItemFloat(IDC_PARAM5, Params.L_Coef_Redraw);
}

void DialogParams::OnBnClickedCBGP()
{

	Params.correctBackGroundPixels = cBGPCheckBox.GetCheck();
}


void DialogParams::OnLbnSelchangeList1()
{
	char buffer[20];

	PresetsList.GetText(PresetsList.GetCurSel(), buffer);

	iCurrentParamsListIndex = PresetsList.GetCurSel();
	FillParamsDlg(buffer);

	BackupParams();
	
	strcpy_s(sCurrentSection, buffer);
}

void DialogParams::OnBnClickedSave()
{
	char szBuffer[1024];

	sprintf_s(szBuffer, "%.2f", Params.L_Merge_Limit);
	
	WritePrivateProfileString(sCurrentSection, "L_Merge_Limit", szBuffer, config);

	sprintf_s(szBuffer, "%.2f", Params.AB_Merge_Limit);
	
	WritePrivateProfileString(sCurrentSection, "AB_Merge_Limit", szBuffer, config);

	sprintf_s(szBuffer, "%d", Params.tooLowPixelCount);
	
	WritePrivateProfileString(sCurrentSection, "tooLowPixelCount", szBuffer, config);

	sprintf_s(szBuffer, "%.2f", Params.redrawLimit);
	
	WritePrivateProfileString(sCurrentSection, "redrawLimit", szBuffer, config);

	sprintf_s(szBuffer, "%.2f", Params.L_Coef_Redraw);
	
	WritePrivateProfileString(sCurrentSection, "L_Coef_Redraw", szBuffer, config);

	sprintf_s(szBuffer, "%d", Params.correctBackGroundPixels);
	
	WritePrivateProfileString(sCurrentSection, "correctBackGroundPixels", szBuffer, config);

	
}


void DialogParams::ShowParams()
{
	char szBuffer[10];
	
	SetDlgItemFloat(IDC_PARAM1, Params.L_Merge_Limit);

	SetDlgItemFloat(IDC_PARAM2, Params.AB_Merge_Limit);


	sprintf_s(szBuffer, "%d", Params.tooLowPixelCount);
	
	SetDlgItemText(IDC_PARAM3, szBuffer);

	SetDlgItemFloat(IDC_PARAM4, Params.redrawLimit);

	SetDlgItemFloat(IDC_PARAM5, Params.L_Coef_Redraw);

	cBGPCheckBox.SetCheck(Params.correctBackGroundPixels);
}

void DialogParams::BackupParams()
{
	//Не нужно бэкапить цвета
	//memcpy(&ParamsBackup, &Params, sizeof(Params));

	ParamsBackup.Radius = Params.Radius;
	ParamsBackup.Radius2 = Params.Radius2;
	ParamsBackup.L_Coef_SQUARE = Params.L_Coef_SQUARE;
	ParamsBackup.tooLowPixelCount = Params.tooLowPixelCount;
	ParamsBackup.pCoef = Params.pCoef;
	ParamsBackup.startingRadiusCoef = Params.startingRadiusCoef;
	ParamsBackup.L_Radius_Coef = Params.L_Radius_Coef;
	ParamsBackup.LDirectionCoef = Params.LDirectionCoef;
	ParamsBackup.Mx_Div_Coef = Params.Mx_Div_Coef;
	ParamsBackup.M_Mul_Coef = Params.M_Mul_Coef;
	ParamsBackup.L_Merge_Limit = Params.L_Merge_Limit;
	ParamsBackup.AB_Merge_Limit = Params.AB_Merge_Limit;
	ParamsBackup.AB_Merge_Check_Limit = Params.AB_Merge_Check_Limit;
	ParamsBackup.L_Coef_Clusters_Radius = Params.L_Coef_Clusters_Radius;
	ParamsBackup.L_Coef_Nearest_Draw = Params.L_Coef_Nearest_Draw;
	ParamsBackup.redrawLimit = Params.redrawLimit;
	ParamsBackup.L_Coef_Redraw = Params.L_Coef_Redraw;
	ParamsBackup.correctBackGroundPixels = Params.correctBackGroundPixels;

	ParamsBackup.additionalInfo = Params.additionalInfo;
	ParamsBackup.saveImages = Params.saveImages;
	
}

void DialogParams::RestoreParams()
{
	//memcpy(&Params, &ParamsBackup, sizeof(Params));

	Params.Radius = ParamsBackup.Radius;
	Params.Radius2 = ParamsBackup.Radius2;
	Params.L_Coef_SQUARE = ParamsBackup.L_Coef_SQUARE;
	Params.tooLowPixelCount = ParamsBackup.tooLowPixelCount;
	Params.pCoef = ParamsBackup.pCoef;
	Params.startingRadiusCoef = ParamsBackup.startingRadiusCoef;
	Params.L_Radius_Coef = ParamsBackup.L_Radius_Coef;
	Params.LDirectionCoef = ParamsBackup.LDirectionCoef;
	Params.Mx_Div_Coef = ParamsBackup.Mx_Div_Coef;
	Params.M_Mul_Coef = ParamsBackup.M_Mul_Coef;
	Params.L_Merge_Limit = ParamsBackup.L_Merge_Limit;
	Params.AB_Merge_Limit = ParamsBackup.AB_Merge_Limit;
	Params.AB_Merge_Check_Limit = ParamsBackup.AB_Merge_Check_Limit;
	Params.L_Coef_Clusters_Radius = ParamsBackup.L_Coef_Clusters_Radius;
	Params.L_Coef_Nearest_Draw = ParamsBackup.L_Coef_Nearest_Draw;
	Params.redrawLimit = ParamsBackup.redrawLimit;
	Params.L_Coef_Redraw = ParamsBackup.L_Coef_Redraw;
	Params.correctBackGroundPixels = ParamsBackup.correctBackGroundPixels;

	Params.additionalInfo = ParamsBackup.additionalInfo;
	Params.saveImages = ParamsBackup.saveImages;
}
