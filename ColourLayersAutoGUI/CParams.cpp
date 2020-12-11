#include "pch.h"
#include "CParams.h"
#include "iostream"
#include "io.h"

int CParams::readData(char *pSection)
{
	char config[MAX_PATH];

	strcpy_s(config, ".\\ColourLayersAuto.ini");

	_fullpath(config, config, MAX_PATH);

	if (_access(config, 0) != 0)
	{
		printf("No INI file\n");
		return 1;
	}

	char section[20];

	if (pSection == nullptr)
	{
		sprintf_s(section, "all");
	}
	else
	{
		strcpy_s(section, 20, pSection);
	}

	char szBuffer[1024];
	GetPrivateProfileString(section, "imagesPath", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	strcpy_s(imagesPath, MAX_PATH, szBuffer);

	

	GetPrivateProfileString(section, "etalonPath", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	strcpy_s(etalonPath, MAX_PATH, szBuffer);

	GetPrivateProfileString(section, "lexemesPath", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	strcpy_s(lexemesPath, MAX_PATH, szBuffer);

	GetPrivateProfileString(section, "CLSavingPath", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	strcpy_s(CLSavingPath, MAX_PATH, szBuffer);




	return 0;
}

int CParams::readAdditionalData(char* pSection)
{
	char config[MAX_PATH];

	strcpy_s(config, ".\\additionalParams.ini");

	_fullpath(config, config, MAX_PATH);

	if (_access(config, 0) != 0)
	{
		return 1;
	}

	char section[20];

	if (pSection == nullptr)
	{
		sprintf_s(section, "all");
	}
	else
	{
		strcpy_s(section, 20, pSection);
	}
	
	char szBuffer[1024];
	GetPrivateProfileString(section, "Radius", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Radius = atoi(szBuffer);


	GetPrivateProfileString(section, "Radius2", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Radius2 = atoi(szBuffer);

	GetPrivateProfileString(section, "L_Coef_SQUARE", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	L_Coef_SQUARE = atof(szBuffer);


	GetPrivateProfileString(section, "tooLowPixelCount", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	tooLowPixelCount = atoi(szBuffer);

	GetPrivateProfileString(section, "pCoef", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	pCoef = atof(szBuffer);

	GetPrivateProfileString(section, "startingRadiusCoef", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	startingRadiusCoef = atof(szBuffer);

	GetPrivateProfileString(section, "L_Radius_Coef", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	L_Radius_Coef = atof(szBuffer);

	GetPrivateProfileString(section, "LDirectionCoef", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	LDirectionCoef = atof(szBuffer);

	GetPrivateProfileString(section, "Mx_Div_Coef", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	Mx_Div_Coef = atof(szBuffer);

	GetPrivateProfileString(section, "M_Mul_Coef", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	M_Mul_Coef = atof(szBuffer);

	GetPrivateProfileString(section, "LAB_Merge_Limit", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	LAB_Merge_Limit = atof(szBuffer);
	
	GetPrivateProfileString(section, "L_Merge_Limit", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	L_Merge_Limit = atof(szBuffer);

	GetPrivateProfileString(section, "AB_Merge_Limit", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	AB_Merge_Limit = atof(szBuffer);

	GetPrivateProfileString(section, "AB_Merge_Check_Limit", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	AB_Merge_Check_Limit = atof(szBuffer);

	GetPrivateProfileString(section, "L_Coef_Clusters_Radius", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	L_Coef_Clusters_Radius = atof(szBuffer);

	GetPrivateProfileString(section, "L_Coef_Nearest_Draw", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	L_Coef_Nearest_Draw = atof(szBuffer);

	GetPrivateProfileString(section, "redrawLimit", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	redrawLimit = atof(szBuffer);

	GetPrivateProfileString(section, "L_Coef_Redraw", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	L_Coef_Redraw = atof(szBuffer);

	GetPrivateProfileString(section, "saveImages", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	saveImages = szBuffer[0] != '0' && szBuffer[0] != 0;

	GetPrivateProfileString(section, "correctBackGroundPixels", nullptr, szBuffer, sizeof(szBuffer) - 1, config);
	correctBackGroundPixels = szBuffer[0] != '0' && szBuffer[0] != 0;
	
	
	return 0;
}