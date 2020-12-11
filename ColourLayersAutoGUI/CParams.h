#pragma once
#include <windows.h>

class CParams
{
public:
	char imagesPath[MAX_PATH];
	char etalonPath[MAX_PATH];
	char lexemesPath[MAX_PATH];
	char CLSavingPath[MAX_PATH];

	int Radius;
	int Radius2;

	double L_Coef_SQUARE;
	int tooLowPixelCount;
	double pCoef;
	double startingRadiusCoef;
	double L_Radius_Coef;
	double LDirectionCoef;
	double Mx_Div_Coef;
	double M_Mul_Coef;
	double LAB_Merge_Limit;
	double L_Merge_Limit;
	double AB_Merge_Limit;
	double AB_Merge_Check_Limit;
	double L_Coef_Clusters_Radius;
	double L_Coef_Nearest_Draw;
	double redrawLimit;
	double L_Coef_Redraw;

	bool saveImages;

	bool correctBackGroundPixels;
	
	int readData(char *pSection = nullptr);
	int readAdditionalData(char* pSection = nullptr);
};