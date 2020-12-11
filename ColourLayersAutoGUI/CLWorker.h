#pragma once
#include "ColourLayers.h"
#include "Result.h"

#define STEP_COUNT 5

typedef struct
{
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
	double L_Merge_Limit;
	double AB_Merge_Limit;
	//double LAB_Merge_Limit;
	double AB_Merge_Check_Limit;
	double L_Coef_Clusters_Radius;
	double L_Coef_Nearest_Draw;
	double redrawLimit;
	double L_Coef_Redraw;
	bool correctBackGroundPixels;

	bool additionalInfo;
	bool saveImages;

	PRGB_STRUCT pPresetColours;
	int nPresetColoursCount;
	PRGB_STRUCT pForbiddenColours;
	int nForbiddenColoursCount;

} CLParams;


//Ёкспортируемые функции:

Result * CLCreateWorker(DataArea * DaEtalon, DataArea * DaLexemes, char * inputPath);

Result * CLExecuteStep(void * pWorker, int stepNumber, CLParams & A, CLData * Data);

void CLDeleteWorker(void * pWorker);

void CLDataFree(Result * pData);