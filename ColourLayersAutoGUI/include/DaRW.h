#pragma once

#ifdef DARW_EXPORTS
#define DARW_API __declspec(dllexport)
#else
#define DARW_API __declspec(dllimport)
#endif

#include "dataarea.h"

//����������
#define DARW_OK			0
#define DARW_NOFILE		1
#define DARW_BADARG		2
#define DARW_RWERROR	3
#define DARW_MEMERROR	4


//������ ������� �� �����
DARW_API int DataAreaRead(PDataArea_t pDa, char *pszFileName, int nMaxSize);
//������ ������� �� �����
//������� ����� ������������� �������� � ��������� ��������
//� ����� ������ ����� ������������� ����� ����� �������� ��� ��� DataAreaDestroy
DARW_API int DataAreaReadAlloc(PDataArea_t pDa, char *pszFileName);
//������ ������� � ����
DARW_API int DataAreaWrite(PDataArea_t pDa, char *pszFileName);
//������������ �������, ���������� ������ DataAreaReadAlloc
DARW_API void DataAreaDestroy(PDataArea_t pDa);
