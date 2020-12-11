#include "pch.h"
#include "ColourLayersAutoGUIDlg.h"
#include "CParams.h"
#include "CLWorker.h"

extern CLParams Params;
extern CLParams ParamsBackup;

extern bool bLexemesSet;

void ListAddText(HWND* phObjectHandle, char* info)
{
	if (info)
	{
		char* newPChar = new char[MAX_PATH];
		strcpy_s(newPChar, MAX_PATH, info);
		::PostMessage(*phObjectHandle, WM_UPDATE_CONTROL, 1, (LONG_PTR)newPChar);

	}
}
void ProgressSetStep(HWND* phObjectHandle, char* info)
{
	if (info)
	{
		char* newPChar = new char[10];
		strcpy_s(newPChar, 10, info);
		::PostMessage(*phObjectHandle, WM_UPDATE_PROGRESS, 1, (LONG_PTR)newPChar);
	}
}


void ShowMessageBox(HWND* phObjectHandle, char* info)
{
	if (info)
		::PostMessage(*phObjectHandle, WM_SHOW_MESSAGEBOX, 1, (LONG_PTR)info);
}

UINT ColourLayersMain(LPVOID pvParam)
{
	HWND* phObjectHandle = static_cast<HWND*>(pvParam);
	
	char imagesPath[MAX_PATH];
	char etalonPath[MAX_PATH];
	char lexemesPath[MAX_PATH];

	strcpy_s(imagesPath, ".\\Images\\");
	strcpy_s(etalonPath, "Sample.png");
	strcpy_s(lexemesPath, "SampleLexem.png");
	
	char etalonFullPath1[MAX_PATH];
	char lexemesFullPath1[MAX_PATH];

	sprintf_s(etalonFullPath1, "%s%s", imagesPath, etalonPath);
	sprintf_s(lexemesFullPath1, "%s%s", imagesPath, lexemesPath);

	
	DataArea_t et1;
	DataAreaReadAlloc(&et1, etalonFullPath1);

	DataArea_t lexemes1;

	DataAreaReadAlloc(&lexemes1, lexemesFullPath1);
	
	DataArea et2;
	DataArea lexemes2;

	et2.Cols = et1.Cols;
	et2.Rows = et1.Rows;
	et2.StartAddress = et1.StartAddress;
	et2.Offset = et1.Offset;
	et2.Plants = et1.Plants;


	if (bLexemesSet == false || lexemes1.StartAddress == nullptr)
	{
		lexemes2.Cols = et1.Cols;
		lexemes2.Rows = et1.Rows;
		lexemes2.StartAddress = new UCHAR[et1.Cols * et1.Rows];
		memset(lexemes2.StartAddress, UCHAR(1), et1.Cols * et1.Rows);
		lexemes2.Offset = et1.Cols;
		lexemes2.Plants = 1;
	}
	else
	{
		lexemes2.Cols = lexemes1.Cols;
		lexemes2.Rows = lexemes1.Rows;
		lexemes2.StartAddress = lexemes1.StartAddress;
		lexemes2.Offset = lexemes1.Offset;
		lexemes2.Plants = lexemes1.Plants;
	}



	
	if (et1.StartAddress == nullptr)
	{
		ShowMessageBox(phObjectHandle, "Etalon Image did not set");

		if (bLexemesSet == false || lexemes1.StartAddress == nullptr)
			delete[] lexemes2.StartAddress;
		
		return 0;
	}
	


	CLData* InData = nullptr;
	CLData* OutData = nullptr;


	char inputPath[500];
	sprintf_s(inputPath, 500, ".\\table.lab;%s", imagesPath);

	ListAddText(phObjectHandle, "Sample");
	
	Result* result = CLCreateWorker(&et2, &lexemes2, inputPath);

	if (result->error_length != 0)
	{
		ShowMessageBox(phObjectHandle, result->error);
		return 0;
	}

	CLWorker* worker = static_cast<CLWorker*>(result->data);
	InData = nullptr;
	OutData = nullptr;

	for (int i = 1; i <= 5; i++)
	{

		result = CLExecuteStep(worker, i, Params, InData);
		if (result->error_length > 0)
		{
			ShowMessageBox(phObjectHandle, result->error);
			break;
		}
		OutData = static_cast<CLData*>(result->data);


		for (int k = 0; k < OutData->imagesCount; k++)
		{
			if (OutData->DBGImages[k])
				ListAddText(phObjectHandle, OutData->DBGImages[k]->info);
		}

		if (i == 5)
		{
			ListAddText(phObjectHandle, "Result");
		}

		char tmp[10];

		sprintf_s(tmp, "%d", i);
		
		ProgressSetStep(phObjectHandle, tmp);

		CLDataFree(new Result(InData, sizeof * InData));

		InData = OutData;
	}

	CLDataFree(new Result(InData, sizeof * InData));
	
	CLDeleteWorker(worker);

	if (bLexemesSet == false || lexemes1.StartAddress == nullptr)
	{
		delete[] lexemes2.StartAddress;
	}
	else
	{
		DataAreaDestroy(&lexemes1);
	}

	if (et1.StartAddress)
	{
		DataAreaDestroy(&et1);
	}

	return 0;
	
	
}