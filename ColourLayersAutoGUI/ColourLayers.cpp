#include "pch.h"
#include "ColourLayers.h"
#include "io.h"
#include <map>
#include <iostream>

#include <algorithm>
#include <ctime>
#include <direct.h>
using namespace std;

#define FORBIDDEN 254
#define UNDEFINED 255

int DataAreaWrite(DataArea * pDa, char *pszFileName)
{
	DataArea_t s;
	s.Cols = pDa->Cols;
	s.Rows = pDa->Rows;
	s.Plants = pDa->Plants;
	s.Offset = pDa->Offset;
	s.StartAddress = pDa->StartAddress;
	s.PixBit = 8;
	s.PixOnPlant = 1;
	s.PixSize = 1;

	return DataAreaWrite(&s, pszFileName);
}

CLWorker::CLWorker()
{
	memset(&DaEtalon, 0, sizeof(DataArea_t));
	memset(&DaLexemes, 0, sizeof(DataArea_t));
	//memset(&DaColourLayers, 0, sizeof(DataArea_t));
	width = height = 0;

	pLabTable = nullptr;
	labBuffer = nullptr;

	LAB_SPACE = nullptr;
	AB_SPACE = nullptr;
	C = nullptr;
	LAB_SPACE_E = nullptr;

	PresetColours = nullptr;
	PresetColoursCount = 0;
	ForbiddenColours = nullptr;
	ForbiddenColoursCount = 0;


	//Установка параметров по-умолчанию

	Radius = 3;
	Square = (2 * Radius + 1) * (2 * Radius + 1);

	Radius2 = 4;
	Square2 = (2 * Radius2 + 1) * (2 * Radius2 + 1);
	
	RadiusE = Radius + 1;
	Radius2E = RadiusE + 1;
	SquareE = (2 * RadiusE + 1) * (2 * RadiusE + 1);
	Square2E = (2 * Radius2E + 1) * (2 * Radius2E + 1);

	L_Coef_SQUARE = 0.06;

	tooLowPixelCount = 3;

	pCoef = 0.084;
	startingRadiusCoef = 1.65;
	L_Radius_Coef = 2;
	LDirectionCoef = 2;
	Mx_Div_Coef = 3;
	M_Mul_Coef = sqrt(3);
	L_Merge_Limit = 1.5;
	AB_Merge_Limit = 1.5;
	AB_Merge_Check_Limit = 6;
	L_Coef_Clusters_Radius = 0.15;
	L_Coef_Nearest_Draw = 0.15;
	redrawLimit = 5;
	L_Coef_Redraw = 0.06;

	attempt = 0;
	firstStepClustersSize = 0;
}

//DataArea с эталоном и файлом лексем будут подаваться извне. Корректные Plants - 3 и 1.
int CLWorker::setEtalonDataArea(DataArea * p)
{
	DaEtalon = *p;
	if (DaEtalon.StartAddress == nullptr)
	{
		return 2;
	}
	if (DaEtalon.Plants != 3)
	{
		return 1;
	}
	width = DaEtalon.Cols;
	height = DaEtalon.Rows;
	return 0;
}

int CLWorker::setLexemesDataArea(DataArea * p)
{
	DaLexemes = *p;
	if (DaLexemes.StartAddress == nullptr)
	{
		return 2;
	}
	if (DaLexemes.Plants != 1)
	{
		return 1;
	}
	return 0;
}

bool CLWorker::checkCorrectness() const
{
	if (DaEtalon.Cols != DaLexemes.Cols || DaEtalon.Rows != DaLexemes.Rows || DaEtalon.Cols == 0 || DaEtalon.Rows == 0 || DaLexemes.Cols == 0 || DaLexemes.Rows == 0)
		return false;

	return true;
}



double CLWorker::length(LAB_STRUCT_f x1, LAB_STRUCT_f x2)
{
	return sqrt(L_Coef_SQUARE * (x1.L - x2.L) * (x1.L - x2.L) + (x1.A - x2.A) * (x1.A - x2.A) + (x1.B - x2.B) * (x1.B - x2.B));
}

double CLWorker::length(LAB_STRUCT_f x1, LAB_STRUCT x2)
{
	return sqrt(L_Coef_SQUARE * (x1.L - x2.L) * (x1.L - x2.L) + (x1.A - x2.A) * (x1.A - x2.A) + (x1.B - x2.B) * (x1.B - x2.B));
}

double CLWorker::length(LAB_STRUCT x1, LAB_STRUCT x2)
{
	return sqrt(L_Coef_SQUARE * (x1.L - x2.L) * (x1.L - x2.L) + (x1.A - x2.A) * (x1.A - x2.A) + (x1.B - x2.B) * (x1.B - x2.B));
}



void CLWorker::setImagesPath(char* imagesPath)
{
	strcpy_s(this->imagesPath, MAX_PATH, imagesPath);
}




//Функции работы с суммами
void CLWorker::calcSumMatrix()
{
	int HorizSum = 0;

	delete[] C;
	C = new MATRIX[_1D];

	for (int i = 0; i < _1D; i++)
	{
		for (int j = 0; j < _1D; j++)
		{
			memset(C[i][j], 0, sizeof(int) * _1D);
		}
	}



	int * p = LAB_SPACE;

	auto B = static_cast<int(*)[_1D]>(malloc(_1D * _1D * sizeof(int)));

	for (int i = 0; i < _1D; i++)
	{
		memset(B[i], 0, sizeof(int) * _1D);
	}

	for (int x = 0; x < _1D; x++)
	{
		HorizSum += *p;
		C[0][0][x] = HorizSum;
		p++;
	}

	for (int y = 1; y < _1D; y++)
	{
		HorizSum = 0;
		for (int x = 0; x < _1D; x++)
		{
			HorizSum += *p;
			C[0][y][x] = C[0][y - 1][x] + HorizSum;
			p++;
		}
	}

	

	for (int z = 1; z < _1D; z++)
	{
		HorizSum = 0;
		for (int x = 0; x < _1D; x++)
		{
			HorizSum += *p;
			C[z][0][x] = C[z - 1][0][x] + HorizSum;
			B[0][x] = HorizSum;
			p++;
		}
		for (int y = 1; y < _1D; y++)
		{
			HorizSum = 0;
			for (int x = 0; x < _1D; x++)
			{
				HorizSum += *p;
				
				C[z][y][x] = C[z - 1][y][x] + B[y - 1][x] + HorizSum;
				
				B[y][x] = B[y - 1][x] + HorizSum;
				p++;
			}
		}

	}
	free(B);
}


int CLWorker::sumMatrix(UCHAR L, UCHAR A, UCHAR B) const
{
	if (L < Radius + 1 || L > 255 - Radius || A < Radius + 1 || A > 255 - Radius || B < Radius + 1 || B > 255 - Radius)
		return 0;
	return C[(L + Radius)][A + Radius][B + Radius]
		- C[L - Radius - 1][A + Radius][B + Radius]
		- C[L + Radius][A - Radius - 1][B + Radius]
		- C[L + Radius][A + Radius][B - Radius - 1]
		+ C[L - Radius - 1][A + Radius][B - Radius - 1]
		+ C[L + Radius][A - Radius - 1][B - Radius - 1]
		+ C[L - Radius - 1][A - Radius - 1][B + Radius]
		- C[L - Radius - 1][A - Radius - 1][B - Radius - 1];

}

int CLWorker::sumMatrix2(UCHAR L, UCHAR A, UCHAR B) const
{
	if (L < Radius2 + 1 || L > 255 - Radius2 || A < Radius2 + 1 || A > 255 - Radius2 || B < Radius2 + 1 || B > 255 - Radius2)
		return 0;
	return C[(L + Radius2)][A + Radius2][B + Radius2]
		- C[L - Radius2 - 1][A + Radius2][B + Radius2]
		- C[L + Radius2][A - Radius2 - 1][B + Radius2]
		- C[L + Radius2][A + Radius2][B - Radius2 - 1]
		+ C[L - Radius2 - 1][A + Radius2][B - Radius2 - 1]
		+ C[L + Radius2][A - Radius2 - 1][B - Radius2 - 1]
		+ C[L - Radius2 - 1][A - Radius2 - 1][B + Radius2]
		- C[L - Radius2 - 1][A - Radius2 - 1][B - Radius2 - 1];

}

int CLWorker::sumMatrixE(UCHAR L, UCHAR A, UCHAR B) const
{
	if (L < RadiusE + 1 || L > 255 - RadiusE || A < RadiusE + 1 || A > 255 - RadiusE || B < RadiusE + 1 || B > 255 - RadiusE)
		return 0;
	return C[(L + RadiusE)][A + RadiusE][B + RadiusE]
		- C[L - RadiusE - 1][A + RadiusE][B + RadiusE]
		- C[L + RadiusE][A - RadiusE - 1][B + RadiusE]
		- C[L + RadiusE][A + RadiusE][B - RadiusE - 1]
		+ C[L - RadiusE - 1][A + RadiusE][B - RadiusE - 1]
		+ C[L + RadiusE][A - RadiusE - 1][B - RadiusE - 1]
		+ C[L - RadiusE - 1][A - RadiusE - 1][B + RadiusE]
		- C[L - RadiusE - 1][A - RadiusE - 1][B - RadiusE - 1];

}

int CLWorker::sumMatrix2E(UCHAR L, UCHAR A, UCHAR B) const
{
	if (L < Radius2E + 1 || L > 255 - Radius2E || A < Radius2E + 1 || A > 255 - Radius2E || B < Radius2E + 1 || B > 255 - Radius2E)
		return 0;
	return C[(L + Radius2E)][A + Radius2E][B + Radius2E]
		- C[L - Radius2E - 1][A + Radius2E][B + Radius2E]
		- C[L + Radius2E][A - Radius2E - 1][B + Radius2E]
		- C[L + Radius2E][A + Radius2E][B - Radius2E - 1]
		+ C[L - Radius2E - 1][A + Radius2E][B - Radius2E - 1]
		+ C[L + Radius2E][A - Radius2E - 1][B - Radius2E - 1]
		+ C[L - Radius2E - 1][A - Radius2E - 1][B + Radius2E]
		- C[L - Radius2E - 1][A - Radius2E - 1][B - Radius2E - 1];

}


void CLWorker::calcLABStatistics()
{
	cout << "attempt " << attempt << endl;

	delete[] AB_SPACE;
	delete[] LAB_SPACE;
	delete[] LAB_SPACE_E;

	LAB_SPACE = new int[_3D];
	memset(LAB_SPACE, 0, sizeof(int) * (_3D));
	LAB_SPACE_E = new int[_3D];
	memset(LAB_SPACE_E, 0, sizeof(int) * (_3D));

	AB_SPACE = new int[_2D];
	memset(AB_SPACE, 0, sizeof(int) * (_2D));

	int shift = 1 + width;
	int shift3 = shift * 3;
	for (int y = 1; y < height - 1; y++)
	{
		for (int x = 1; x < width - 1; x++)
		{
			if ((attempt == 1 && DaLexemes.StartAddress[shift] > 0 && Data->artefacts->StartAddress[shift] == 0) || (attempt == 2 && Data->colourLayersIndex->StartAddress[shift] == UNDEFINED && Data->artefacts->StartAddress[shift] == 0))
			{
				int addWeight = 1;

				LAB_SPACE[labBuffer[shift].L * (1 << 16) + labBuffer[shift].A * (1 << 8) + labBuffer[shift].B] += addWeight;
				AB_SPACE[labBuffer[shift].A * (1 << 8) + labBuffer[shift].B] += addWeight;
				
			}
			shift++;
			shift3 += 3;
		}
		shift += 2;
		shift3 += 6;
	}

	calcSumMatrix();


	bool found = false;

	shift = 0;
	for (int l = 0; l < _1D; l++)
	{
		for (int a = 0; a < _1D; a++)
		{
			for (int b = 0; b < _1D; b++)
			{
				int sumElement = static_cast<int>((2 * static_cast<double>(sumMatrix(l, a, b)) - static_cast<double>(sumMatrix2(l, a, b))) / Square2);

				sumElement > 0 ? LAB_SPACE[shift] = sumElement, found = true : LAB_SPACE[shift] = 0;

				sumElement = static_cast<int>((2 * static_cast<double>(sumMatrixE(l, a, b)) - static_cast<double>(sumMatrix2E(l, a, b))) / Square2E);

				sumElement > 0 ? LAB_SPACE_E[shift] = sumElement : LAB_SPACE_E[shift] = 0;

				shift++;
			}
		}
	}

	if (found == false)
	{
		SetOutterRadius(this->Radius);
		shift = 0;
		for (int l = 0; l < _1D; l++)
		{
			for (int a = 0; a < _1D; a++)
			{
				for (int b = 0; b < _1D; b++)
				{
					int sumElement = static_cast<int>((2 * static_cast<double>(sumMatrix(l, a, b)) - static_cast<double>(sumMatrix2(l, a, b))) / Square2);

					sumElement > 0 ? LAB_SPACE[shift] = sumElement : LAB_SPACE[shift] = 0;

					shift++;
				}
			}
		}
	}


	memset(AB_SPACE, 0, sizeof(int) * _2D);

	for (int a = 0; a < _1D; a++)
	{
		for (int b = 0; b < _1D; b++)
		{
			AB_SPACE[a * (1 << 8) + b] = 0;
			for (int l = 1; l < _1D; l++)
			{
				int value = LAB_SPACE[l * (1 << 16) + a * (1 << 8) + b];

				AB_SPACE[a * (1 << 8) + b] += value;

			}
		}
	}
}

void CLWorker::calcClusters()
{

	vector<pixelInfo> lv;

	int sumCount = 0;

	for (int l = 0; l < 256; l++)
	{
		for (int a = 0; a < 256; a++)
		{
			for (int b = 0; b < 256; b++)
			{
				int count = LAB_SPACE[l * (1 << 16) + a * (1 << 8) + b];
				if (count > 0)
					lv.push_back({ { static_cast<float>(l), static_cast<float>(a), static_cast<float>(b) }, {0, 0, 0}, {0, 0, 0, 0, 0, 0}, count });
				sumCount += count;
			}
		}
	}


	sort(lv.begin(), lv.end(), [](const pixelInfo &x1, const pixelInfo &x2)
	{
		return x1.count > x2.count;
	});


	vectorClusters.resize(0);

	if (attempt == 1)
	{
		for (uint i = 0; i < PresetColoursCount; i++)
		{
			vectorClusters.push_back({ PresetColours[i], {0, 0, 0}, {0, 0, 0, 0, 0, 0}, lv[0].count });
		}
	}

	for (uint i = 0; i < ForbiddenColoursCount; i++)
	{
		vectorClusters.push_back({ ForbiddenColours[i], {0, 0, 0}, {0, 0, 0, 0, 0, 0}, lv[0].count });
	}

	if (lv.size() == 0)
		return;

	highestCluster = lv[0];

	bool bUnique = true;
	
	for (auto it : vectorClusters)
	{
		if (it.LAB.L == highestCluster.LAB.L && it.LAB.A == highestCluster.LAB.A && it.LAB.B == highestCluster.LAB.B)
		{
			bUnique = false;
		}
	}

	if (bUnique)
		vectorClusters.push_back(lv[0]);

	for (uint i = 0; i < lv.size(); i++)
	{
		if (lv[i].count < tooLowPixelCount)
			break;
		bool join = false;
		for (uint j = 0; j < vectorClusters.size(); j++)
		{
			double length1 = length(lv[i].LAB, vectorClusters[j].LAB);
			int c1 = vectorClusters[j].count;
			int expectedWeight = static_cast<int>(c1 - length1 * c1 * pCoef);


			if (lv[i].count < expectedWeight || length1 < 2)
			{
				join = true;
				break;
			}
		}
		if (join == false)
		{
			vectorClusters.push_back(lv[i]);
		}
	}

}





void CLWorker::clearArtefacts()
{
	DaClearArtefacts = new DataArea();

	DaClearArtefacts->Cols = width;
	DaClearArtefacts->Rows = height;
	DaClearArtefacts->Plants = 3;
	DaClearArtefacts->Offset = DaClearArtefacts->Cols * DaClearArtefacts->Plants;

	DaClearArtefacts->StartAddress = new UCHAR[DaClearArtefacts->Cols * DaClearArtefacts->Rows * DaClearArtefacts->Plants];

	Data->artefacts = new DataArea();
	Data->artefacts->StartAddress = new UCHAR[width * height];
	memset(Data->artefacts->StartAddress, 0, width * height);
	memset(DaClearArtefacts->StartAddress, 0, DaClearArtefacts->Cols * DaClearArtefacts->Rows * DaClearArtefacts->Plants);


	int L_Limit = 7;

	int shift = 1 + width;
	int shift3 = 3 * shift;
	for (int y = 1; y < height - 1; y++)
	{
		for (int x = 1; x < width - 1; x++)
		{

			DaClearArtefacts->StartAddress[shift3 + 0] = 186;
			DaClearArtefacts->StartAddress[shift3 + 1] = 54;
			DaClearArtefacts->StartAddress[shift3 + 2] = 166;
			
			if (abs(static_cast<int>(labBuffer[shift].L) - static_cast<int>(labBuffer[shift - 1].L)) >= L_Limit && abs(static_cast<int>(labBuffer[shift].L) - static_cast<int>(labBuffer[shift + 1].L)) >= L_Limit)
			{
				if (!(static_cast<int>(labBuffer[shift + 1].L) > static_cast<int>(labBuffer[shift].L) && static_cast<int>(labBuffer[shift - 1].L) > static_cast<int>(labBuffer[shift].L)))
				{
					DaClearArtefacts->StartAddress[shift3 + 0] = 255;
					DaClearArtefacts->StartAddress[shift3 + 1] = 255;
					DaClearArtefacts->StartAddress[shift3 + 2] = 255;
					Data->artefacts->StartAddress[shift] = 1;
				}
			}
			

			if (abs(static_cast<int>(labBuffer[shift].L) - static_cast<int>(labBuffer[shift - width].L)) >= L_Limit && abs(static_cast<int>(labBuffer[shift].L) - static_cast<int>(labBuffer[shift + width].L)) >= L_Limit)
			{
				if (!(static_cast<int>(labBuffer[shift + width].L) > static_cast<int>(labBuffer[shift].L) && static_cast<int>(labBuffer[shift - width].L) > static_cast<int>(labBuffer[shift].L)))
				{
					DaClearArtefacts->StartAddress[shift3 + 0] = 255;
					DaClearArtefacts->StartAddress[shift3 + 1] = 255;
					DaClearArtefacts->StartAddress[shift3 + 2] = 255;
					Data->artefacts->StartAddress[shift] = 1;
				}
			}
			

			if (DaLexemes.StartAddress[shift] > 0 && (DaLexemes.StartAddress[shift + 1]  == 0 || DaLexemes.StartAddress[shift - 1] == 0 || DaLexemes.StartAddress[shift + width] == 0 || DaLexemes.StartAddress[shift - width] == 0
				|| DaLexemes.StartAddress[shift + 1 + width] == 0 || DaLexemes.StartAddress[shift - 1 + width] == 0 || DaLexemes.StartAddress[shift + 1 - width] == 0 || DaLexemes.StartAddress[shift -1 - width] == 0))
			{
				DaClearArtefacts->StartAddress[shift3 + 0] = 255;
				DaClearArtefacts->StartAddress[shift3 + 1] = 255;
				DaClearArtefacts->StartAddress[shift3 + 2] = 255;
				Data->artefacts->StartAddress[shift] = 1;
			}

			if (DaLexemes.StartAddress[shift] == 0)
			{
				DaClearArtefacts->StartAddress[shift3 + 0] = 0;
				DaClearArtefacts->StartAddress[shift3 + 1] = 0;
				DaClearArtefacts->StartAddress[shift3 + 2] = 0;
			}


			shift++;
			shift3 += 3;
		}
		shift += 2;
		shift3 += 6;
	}

	if (correctBackGroundPixels == true)
	{

		LAB_STRUCT_f middleBackGround = { 0, 0, 0 };
		int count = 0;
		shift = 0;
		shift3 = 0;
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{

				if (DaLexemes.StartAddress[shift] == 0 && DaEtalon.StartAddress[shift3] != 0)
				{
					middleBackGround.L += labBuffer[shift].L;
					middleBackGround.A += labBuffer[shift].A;
					middleBackGround.B += labBuffer[shift].B;
					count++;
				}

				shift++;
				shift3 += 3;
			}

		}

		if (count > 0)
		{
			middleBackGround.L /= count;
			middleBackGround.A /= count;
			middleBackGround.B /= count;
		}
		else
		{
			middleBackGround = { 255, 128, 128 };
		}

		int Lim = 30;

		shift = 2 + width * 2;
		shift3 = 3 * shift;
		for (int y = 2; y < height - 2; y++)
		{
			for (int x = 2; x < width - 2; x++)
			{

				if (DaLexemes.StartAddress[shift] > 0)
				{
					if (Data->artefacts->StartAddress[shift] == 0 && abs((int)middleBackGround.L - (int)labBuffer[shift].L) <= Lim && abs((int)middleBackGround.A - (int)labBuffer[shift].A) <= Lim && abs((int)middleBackGround.B - (int)labBuffer[shift].B) <= Lim)
					{
						if ((labBuffer[shift].L > labBuffer[shift + 1].L || labBuffer[shift].L > labBuffer[shift + 2].L) && (labBuffer[shift].L > labBuffer[shift - 1].L || labBuffer[shift].L > labBuffer[shift - 2].L) ||
							(labBuffer[shift].L > labBuffer[shift + width].L || labBuffer[shift].L > labBuffer[shift + 2 * width].L) && (labBuffer[shift].L > labBuffer[shift - width].L || labBuffer[shift].L > labBuffer[shift - 2 * width].L))
						{
							DaClearArtefacts->StartAddress[shift3 + 0] = 0;
							DaClearArtefacts->StartAddress[shift3 + 1] = 255;
							DaClearArtefacts->StartAddress[shift3 + 2] = 0;
							Data->artefacts->StartAddress[shift] = 1;
						}
					}

				}

				shift += 1;
				shift3 += 3;
			}
			shift += 4;
			shift3 += 12;
		}
	}

	char ColoursFullPath[MAX_PATH];
	sprintf_s(ColoursFullPath, "%sSample_Algo_Missing.png", imagesPath);

	if (saveImages == true)
		DataAreaWrite(DaClearArtefacts, ColoursFullPath);

}

void CLWorker::printClustersInfo()
{
	for (auto s : vectorClusters)
	{
		info += to_string(lround(s.LAB.L)) + " " + to_string(lround(s.LAB.A)) + " " + to_string(lround(s.LAB.B)) + " - " + to_string(s.count) + "\n";
	}
}

template <typename T>
void CLWorker::setParamValue(T * param, double left, double right)
{
	if (*param < left)
		*param = left;
	
	if (*param > right)
		*param = right;
}

void CLWorker::correctParamsValues()
{
	setParamValue(&Radius, 1, 10);
	setParamValue(&Radius2, Radius, 10);
	setParamValue(&L_Coef_SQUARE, 0, 1);
	setParamValue(&tooLowPixelCount, 0, 1000);
	setParamValue(&pCoef, 0.01, 1);
	setParamValue(&startingRadiusCoef, 0.1, 20);
	setParamValue(&L_Radius_Coef, 0.1, 20);
	setParamValue(&LDirectionCoef, 0.1, 20);
	setParamValue(&Mx_Div_Coef, 0.1, 20);
	setParamValue(&M_Mul_Coef, 0.1, 20);
	setParamValue(&L_Merge_Limit, 0.1, 20);
	setParamValue(&AB_Merge_Limit, 0.1, 20);
	setParamValue(&AB_Merge_Check_Limit, 1, 20);
	setParamValue(&L_Coef_Clusters_Radius, 0, 1);
	setParamValue(&L_Coef_Nearest_Draw, 0, 1);
	setParamValue(&redrawLimit, 0, 100);
	setParamValue(&L_Coef_Redraw, 0, 1);
}


int CLWorker::formIndexedColourLayers()
{
	map<int, int> colours;
	int shift = 0;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (DaLexemes.StartAddress[shift] > 0 && Data->colourLayersIndex->StartAddress[shift] < FORBIDDEN)
				colours[Data->colourLayersIndex->StartAddress[shift]] = 0;
			shift++;
		}
	}

	int index = 1;

	vectorClusters.resize(0);
	for (uint i = 0; i < colours.size(); i++)
	{
		colours[i] = index++;
		vectorClusters.push_back(Data->clusters[colours[i] - 1]);
	}

	if (Data->clustersCount != vectorClusters.size())
	{
		Data->clustersCount = vectorClusters.size();
		delete[] Data->clusters;
		Data->clusters = new pixelInfo[Data->clustersCount];

		for (int i = 0; i < Data->clustersCount; i++)
		{
			Data->clusters[i] = vectorClusters[i];
		}
	}

	vectorClusters.resize(0);


	cout << "Result colours " << colours.size() << endl;

	Data->DBGImages[3] = new DataArea();

	*Data->DBGImages[3] = DaLexemes;

	flushInfo(&Data->DBGImages[3]->info, "Algo_4");

	DataArea * s = Data->DBGImages[3];

	s->Cols = width;
	s->Rows = height;
	s->Plants = 1;
	s->Offset = s->Cols * s->Plants;
	/*s->PixBit = 8;
	s->PixOnPlant = 1;
	s->PixSize = 1;*/

	s->StartAddress = new UCHAR[width * height];


	shift = 0;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (DaLexemes.StartAddress[shift] == 0)
			{
				s->StartAddress[shift] = 0;
			}
			else
			{
				s->StartAddress[shift] = colours[Data->colourLayersIndex->StartAddress[shift]];
			}
			shift++;
		}
	}

	
	char ColoursFullPath[MAX_PATH];
	char ColoursFullPathColours[MAX_PATH];
	sprintf_s(ColoursFullPath, "%sSampleCL_Algo_CL.png", imagesPath);
	sprintf_s(ColoursFullPathColours, "%sSampleCL_Algo_CLResult.png", imagesPath);

	DataAreaWrite(s, ColoursFullPath);


	DataArea sImage = *s;

	sImage.StartAddress = new UCHAR[3 * width * height];
	sImage.Plants = 3;
	sImage.Offset *= 3;
	shift = 0;
	int shift3 = 0;

	memset(sImage.StartAddress, 0, 3 * width * height);
	
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (DaLexemes.StartAddress[shift] != 0)
			{
				sImage.StartAddress[shift3] = Data->clusters[Data->colourLayersIndex->StartAddress[shift]].RGB.R;
				sImage.StartAddress[shift3 + 1] = Data->clusters[Data->colourLayersIndex->StartAddress[shift]].RGB.G;
				sImage.StartAddress[shift3 + 2] = Data->clusters[Data->colourLayersIndex->StartAddress[shift]].RGB.B;
				
			}

			shift3 += 3;
			shift++;
		}

	}

	DataAreaWrite(&sImage, ColoursFullPathColours);

	delete[] sImage.StartAddress;
	
	
	

	return colours.size();
}

void CLWorker::artefactsCheck()
{
	if (Data->clustersCount - firstStepClustersSize == 1)
	{
		UCHAR currentIndex = firstStepClustersSize;
		int * c = new int[firstStepClustersSize];
		memset(c, 0, sizeof(int) * firstStepClustersSize);

		int shift = 1 + width;
		int shift3 = shift * 3;

		for (int y = 1; y < height - 1; y++)
		{
			for (int x = 1; x < width - 1; x++)
			{
				if (Data->colourLayersIndex->StartAddress[shift] == currentIndex)
				{
					UCHAR nextIndex;
					nextIndex = Data->colourLayersIndex->StartAddress[shift + 1];
					if (nextIndex < FORBIDDEN && nextIndex < currentIndex)
					{
						c[nextIndex]++;
					}
					nextIndex = Data->colourLayersIndex->StartAddress[shift - 1];
					if (nextIndex < FORBIDDEN && nextIndex < currentIndex)
					{
						c[nextIndex]++;
					}
					nextIndex = Data->colourLayersIndex->StartAddress[shift + width];
					if (nextIndex < FORBIDDEN && nextIndex < currentIndex)
					{
						c[nextIndex]++;
					}
					nextIndex = Data->colourLayersIndex->StartAddress[shift - width];
					if (nextIndex < FORBIDDEN && nextIndex < currentIndex)
					{
						c[nextIndex]++;
					}
				}
				shift++;
				shift3 += 3;
			}
			shift += 2;
			shift3 += 6;
		}

		int count = 0;
		int index = 0;
		double ABLength = 10000;
		double LABLength = 0;
		int minIndex = 0;
		for (int i = 0; i < currentIndex; i++)
		{
			if (c[i] > 0)
			{

				count++;
				index = i;
				double currentLength = sqrt((Data->clusters[i].LAB.A - Data->clusters[firstStepClustersSize].LAB.A) * (Data->clusters[i].LAB.A - Data->clusters[firstStepClustersSize].LAB.A) + (Data->clusters[i].LAB.B - Data->clusters[firstStepClustersSize].LAB.B) * (Data->clusters[i].LAB.B - Data->clusters[firstStepClustersSize].LAB.B));
				
				if (currentLength < ABLength)
				{
					ABLength = currentLength;
					LABLength = length(Data->clusters[i].LAB, Data->clusters[firstStepClustersSize].LAB);
					minIndex = i;
				}
			}
		}

		if ((count == 1 && firstStepClustersSize > 1) || (ABLength <= 5 && LABLength <= 13))
		{
			shift = 1 + width;
			shift3 = shift * 3;

			if (ABLength <= 5)
				index = minIndex;

			for (int y = 1; y < height - 1; y++)
			{
				for (int x = 1; x < width - 1; x++)
				{
					if (Data->colourLayersIndex->StartAddress[shift] == currentIndex)
					{
						putPixel(shift3, index);

						Data->colourLayersIndex->StartAddress[shift] = index;
					}

					shift++;
					shift3 += 3;
				}
				shift += 2;
				shift3 += 6;
			}
		}

		delete[] c;
	}
}


int CLWorker::nearestColour(LAB_STRUCT colour)
{
	double minLength = 10000;
	int minIndex = -1;

	for (int i = 0; i < Data->clustersCount; i++)
	{
		double length = sqrt(0.2 * 0.2 * (colour.L - Data->clusters[i].LAB.L) * (colour.L - Data->clusters[i].LAB.L) + (colour.A - Data->clusters[i].LAB.A) * (colour.A - Data->clusters[i].LAB.A) + (colour.B - Data->clusters[i].LAB.B) * (colour.B - Data->clusters[i].LAB.B));
		if (length < minLength)
		{
			minLength = length;
			minIndex = i;
		}
	}

	return minIndex;
}

int CLWorker::nearestColour(LAB_STRUCT_f colour)
{
	double minLength = 10000;
	int minIndex = -1;

	for (int i = 0; i < Data->clustersCount; i++)
	{
		double length = sqrt(0.2 * 0.2 * (colour.L - Data->clusters[i].LAB.L) * (colour.L - Data->clusters[i].LAB.L) + (colour.A - Data->clusters[i].LAB.A) * (colour.A - Data->clusters[i].LAB.A) + (colour.B - Data->clusters[i].LAB.B) * (colour.B - Data->clusters[i].LAB.B));
		if (length < minLength)
		{
			minLength = length;
			minIndex = i;
		}
	}

	return minIndex;
}

double CLWorker::lengthCoef(double coef, LAB_STRUCT_f X, LAB_STRUCT_f Y)
{
	return sqrt(coef  * (X.L - Y.L) * (X.L - Y.L) + (X.A - Y.A) * (X.A - Y.A) + (X.B - Y.B) * (X.B - Y.B));
}

double CLWorker::lengthCoef(double coef, LAB_STRUCT X, LAB_STRUCT Y)
{
	return sqrt(coef  * (X.L - Y.L) * (X.L - Y.L) + (X.A - Y.A) * (X.A - Y.A) + (X.B - Y.B) * (X.B - Y.B));
}

double CLWorker::lengthCoef(double coef, LAB_STRUCT_f X, LAB_STRUCT Y)
{
	return sqrt(coef  * (X.L - Y.L) * (X.L - Y.L) + (X.A - Y.A) * (X.A - Y.A) + (X.B - Y.B) * (X.B - Y.B));
}

void CLWorker::analyzeSquare(int foreignX, int foreignY, int InnerWidth, int OutterWidth)
{
	const int startX = foreignX;
	const int endX = startX + InnerWidth;
	const int startY = foreignY;
	const int endY = startY + InnerWidth;

	const int startXBig = startX - OutterWidth;
	const int endXBig = endX + OutterWidth;
	const int startYBig = startY - OutterWidth;
	const int endYBig = endY + OutterWidth;


	bool here = false;
	int watchingX = 839;
	int watchingY = 369;
	if (startX <= watchingX && watchingX <= endX)
	{
		if (startY <= watchingY && watchingY <= endY)
		{
			here = true;
		}
	}

	

	bool bDraw = false;
	bool print = bDraw;

	if (here == false && bDraw == true)
	{
		return;
	}

	if (startXBig < 0)
		return;
	if (startYBig < 0)
		return;
	if (endXBig >= width)
		return;
	if (endYBig >= height)
		return;

	for (int ly = startYBig; ly < endYBig; ly++)
	{
		int shift = ly * width + startXBig;
		for (int lx = startXBig; lx < endXBig; lx++)
		{
			cclMask[shift] = false;
			shift++;
		}
	}

	const int k = Data->clustersCount;
	auto c = new int[k];
	memset(c, 0, sizeof(int) * k);

	LAB_STRUCT_f sumPixels = {0, 0, 0};
	int pixelsCount = 0;

	auto colours = new LAB_STRUCT_f[k];
	auto middleColours = new LAB_STRUCT_f[k];
	auto middleNonColours = new LAB_STRUCT_f[k];
	auto ncolours = new int[k];

	auto length_C_RC = new double[k];
	auto length_RC_NC = new double[k];

	auto partPixels = new double[k];
	auto partPixelsBig = new double[k];

	for (int i = 0; i < k; i++)
	{
		colours[i].L = 0;
		colours[i].A = 0;
		colours[i].B = 0;
		middleColours[i].L = 0;
		middleColours[i].A = 0;
		middleColours[i].B = 0;
		middleNonColours[i].L = 0;
		middleNonColours[i].A = 0;
		middleNonColours[i].B = 0;
		ncolours[i] = 0;
		length_C_RC[i] = 0;
		length_RC_NC[i] = 0;
		partPixels[i] = 0;
	}

	for (int y = startY; y < endY; y++)
	{
		int shift = y * width + startX;
		for (int x = startX; x < endX; x++)
		{
			if (DaLexemes.StartAddress[shift] > 0)
			{
				c[Data->colourLayersIndex->StartAddress[shift]]++;
				pixelsCount++;
				colours[Data->colourLayersIndex->StartAddress[shift]].L += labBuffer[shift].L;
				colours[Data->colourLayersIndex->StartAddress[shift]].A += labBuffer[shift].A;
				colours[Data->colourLayersIndex->StartAddress[shift]].B += labBuffer[shift].B;
				sumPixels.L += labBuffer[shift].L;
				sumPixels.A += labBuffer[shift].A;
				sumPixels.B += labBuffer[shift].B;
			}

			shift++;
		}
	}
	auto cBig = new int[k];
	memset(cBig, 0, sizeof(int) * k);
	int pixelsCountBig = 0;
	for (int y = startYBig; y < endYBig; y++)
	{
		int shift = y * width + startXBig;
		for (int x = startXBig; x < endXBig; x++)
		{
			if (DaLexemes.StartAddress[shift] > 0)
			{
				cBig[Data->colourLayersIndex->StartAddress[shift]]++;
				pixelsCountBig++;
			}
			shift++;
		}

	}

	int differentColours = 0;
	for (int i = 0; i < k; i++)
	{
		if (c[i] > 0)
		{
			differentColours++;
			partPixels[i] = (double)(c[i]) / pixelsCount;
			partPixelsBig[i] = (double)(cBig[i]) / pixelsCountBig;
			middleColours[i].L = colours[i].L / c[i];
			middleColours[i].A = colours[i].A / c[i];
			middleColours[i].B = colours[i].B / c[i];
			middleNonColours[i].L = (sumPixels.L - colours[i].L) / (pixelsCount - c[i]);
			middleNonColours[i].A = (sumPixels.A - colours[i].A) / (pixelsCount - c[i]);
			middleNonColours[i].B = (sumPixels.B - colours[i].B) / (pixelsCount - c[i]);
			ncolours[i] = nearestColour(middleNonColours[i]);
			LAB_STRUCT_f tmp1 = { middleColours[i].L, middleColours[i].A, middleColours[i].B };
			LAB_STRUCT_f tmp2 = { middleNonColours[i].L, middleNonColours[i].A, middleNonColours[i].B };
			length_C_RC[i] = lengthCoef(L_Coef_Redraw, tmp1, tmp2);
		}
	}
	if (differentColours <= 1)
	{
		delete[] cBig;
		delete[] c;
		delete[] colours;
		delete[] middleColours;
		delete[] middleNonColours;
		delete[] ncolours;

		delete[] length_C_RC;
		delete[] length_RC_NC;

		delete[] partPixels;
		delete[] partPixelsBig;
		return;
	}
	const LAB_STRUCT_f tmp = { sumPixels.L / pixelsCount , sumPixels.A / pixelsCount , sumPixels.B / pixelsCount };
	int middleColourIndex = nearestColour(tmp);



	queue<int> q;
	vector<int> coords;
	int count = 0;
	for (int y = startYBig; y < endYBig; y++)
	{
		int shift = y * width + startXBig;
		for (int x = startXBig; x < endXBig; x++)
		{
			if (DaLexemes.StartAddress[shift] > 0 && cclMask[shift] == false)
			{
				count++;


				q.push(shift);
				coords.push_back(shift);

				bool bIsAtSmall = false;
				bool bIsOut = false;

				

				cclMask[shift] = true;
				const int currentIndex = Data->colourLayersIndex->StartAddress[shift];
				while (!q.empty())
				{
					const int lShift = q.front();
					q.pop();

					const int localX = lShift % width;
					const int localY = lShift / width;

					
					if (bIsAtSmall == false)
					{
						if (localX >= startX && localX < endX && localY >= startY && localY < endY)
						{
							bIsAtSmall = true;
						}
					}

					
					for (int j = -1; j <= 1; j++)
					{
						for (int i = -1; i <= 1; i++)
						{
							int newShift = lShift + i + j * width;

							if (isValid(newShift) == false)
								continue;

							bool bIsValid = isValid(newShift, startXBig, endXBig, startYBig, endYBig);

							if (bIsValid == false && DaLexemes.StartAddress[newShift] > 0 && Data->colourLayersIndex->StartAddress[newShift] == currentIndex)
							{
								bIsOut = true;
							}

							if (bIsValid && DaLexemes.StartAddress[newShift] > 0 && Data->colourLayersIndex->StartAddress[newShift] == currentIndex && cclMask[newShift] == false)
							{
								q.push(newShift);
								coords.push_back(newShift);
								cclMask[newShift] = true;
							}
						}
					}

				}

				LAB_STRUCT_f middleColourNonArtefacts = { 0, 0, 0 };
				LAB_STRUCT_f middleColour = { 0, 0, 0 };
				int countNonArtefacts = 0;

				for (auto it : coords)
				{	
					middleColour.L += labBuffer[it].L;
					middleColour.A += labBuffer[it].A;
					middleColour.B += labBuffer[it].B;
					if (Data->artefacts->StartAddress[it] == 0)
					{
						middleColourNonArtefacts.L += labBuffer[it].L;
						middleColourNonArtefacts.A += labBuffer[it].A;
						middleColourNonArtefacts.B += labBuffer[it].B;
						countNonArtefacts++;
					}
				}
				
				if (countNonArtefacts > 0)
				{
					middleColour.L = middleColourNonArtefacts.L / countNonArtefacts;
					middleColour.A = middleColourNonArtefacts.A / countNonArtefacts;
					middleColour.B = middleColourNonArtefacts.B / countNonArtefacts;
				}
				else
				{
					middleColour.L /= coords.size();
					middleColour.A /= coords.size();
					middleColour.B /= coords.size();
				}
				
				LAB_STRUCT_f tmp1 = { middleColour.L, middleColour.A, middleColour.B };
				LAB_STRUCT_f tmp2 = { middleNonColours[currentIndex].L, middleNonColours[currentIndex].A, middleNonColours[currentIndex].B };
				double length = lengthCoef(L_Coef_Redraw, tmp1, tmp2);


				if (bIsAtSmall == true && bIsOut == false && print == true)
				{
					cout << count << " - " << coords.size() << " - " << lround(middleColour.L) << " " << lround(middleColour.A) << " " << lround(middleColour.B) << " - ";
					cout << currentIndex << " - " << middleColourIndex << " - " << Data->clusters[currentIndex].LAB.L << " " << Data->clusters[currentIndex].LAB.A << " " << Data->clusters[currentIndex].LAB.B << endl;
					cout << 1.0 * coords.size() / pixelsCount  << " " << partPixels[currentIndex] << " " << partPixelsBig[currentIndex]  << endl;
					cout << middleNonColours[currentIndex].L << " " << middleNonColours[currentIndex].A << " " << middleNonColours[currentIndex].B << endl;
				}
				

				int nearestIndex = nearestColour(middleColour);

				int reDrawingIndex;
				bNearestOrMiddle == false ? reDrawingIndex = nearestIndex : reDrawingIndex = middleColourIndex;

				//Перекраска по размеру
				if (partPixels[currentIndex] < 0.019 && partPixelsBig[currentIndex] < 0.03 && bIsAtSmall == true && bIsOut == false && fabs(middleColour.L - middleNonColours[currentIndex].L) <= 50)
				{
					// Это неправильный цвет, который никто не поддержал.
					// Его надо раскрасить ближайшим наиболее близким цветом.
					map <int, bool> mapNearestColours;
					for (auto it : coords)
					{
						for (int j = -1; j <= 1; j++)
						{
							for (int i = -1; i <= 1; i++)
							{
								int newShift = it + i + j * width;
								if (Data->colourLayersIndex->StartAddress[newShift] < FORBIDDEN && Data->colourLayersIndex->StartAddress[newShift] != currentIndex)
								{
									mapNearestColours[Data->colourLayersIndex->StartAddress[newShift]] = true;
								}
							}
						}
					}
						
					double minLength = 10000;
					int minLengthIndex = 0;
					for (auto m : mapNearestColours)
					{
						double length = lengthCoef(L_Coef_Redraw, tmp1, Data->clusters[m.first].LAB);
						if (length < minLength)
						{
							minLength = length;
							minLengthIndex = m.first;
						}
					}
					if (minLength == 10000)
					{
						minLengthIndex = nearestIndex;
					}

					for (auto it : coords)
					{
						int it3 = 3 * it;
						Data->DaColourLayers->StartAddress[it3 + 0] = Data->clusters[minLengthIndex].RGB.R;
						Data->DaColourLayers->StartAddress[it3 + 1] = Data->clusters[minLengthIndex].RGB.G;
						Data->DaColourLayers->StartAddress[it3 + 2] = Data->clusters[minLengthIndex].RGB.B;
						
						DaColourLayersWithRedraw.StartAddress[it3 + 0] = 0;
						DaColourLayersWithRedraw.StartAddress[it3 + 1] = 255;
						DaColourLayersWithRedraw.StartAddress[it3 + 2] = 0;
						
						Data->colourLayersIndex->StartAddress[it] = minLengthIndex;
					}
				}
				else // Доверяем больше перекраске по размеру
				{
					//Перекраска по цвету
					if ((length < redrawLimit || coords.size() <= 3) && bIsAtSmall == true && bIsOut == false && partPixels[currentIndex] < 0.75)
					{
						for (auto it : coords)
						{
							int it3 = 3 * it;
							Data->DaColourLayers->StartAddress[it3 + 0] = Data->clusters[reDrawingIndex].RGB.R;
							Data->DaColourLayers->StartAddress[it3 + 1] = Data->clusters[reDrawingIndex].RGB.G;
							Data->DaColourLayers->StartAddress[it3 + 2] = Data->clusters[reDrawingIndex].RGB.B;

							DaColourLayersWithRedraw.StartAddress[it3 + 0] = 255;
							DaColourLayersWithRedraw.StartAddress[it3 + 1] = 255;
							DaColourLayersWithRedraw.StartAddress[it3 + 2] = 0;

							Data->colourLayersIndex->StartAddress[it] = reDrawingIndex;
						}
					}
				}

			}

			shift++;
			coords.resize(0);
		}
	}

	

	if (bDraw == true)
	{
		int y = startY;
		int shift3 = 3 * y * width + 3 * startX;
		for (int x = startX; x <= endX; x++)
		{
			DaColourLayersWithRedraw.StartAddress[shift3 + 0] = 255;
			DaColourLayersWithRedraw.StartAddress[shift3 + 1] = 0;
			DaColourLayersWithRedraw.StartAddress[shift3 + 2] = 0;
			shift3 += 3;
		}
		y = endY;
		shift3 = 3 * y * width + 3 * startX;
		for (int x = startX; x <= endX; x++)
		{
			DaColourLayersWithRedraw.StartAddress[shift3 + 0] = 255;
			DaColourLayersWithRedraw.StartAddress[shift3 + 1] = 0;
			DaColourLayersWithRedraw.StartAddress[shift3 + 2] = 0;
			shift3 += 3;
		}

		int x = startX;
		shift3 = (startY * width + x) * 3;
		for (int y = startY; y <= endY; y++)
		{

			DaColourLayersWithRedraw.StartAddress[shift3 + 0] = 255;
			DaColourLayersWithRedraw.StartAddress[shift3 + 1] = 0;
			DaColourLayersWithRedraw.StartAddress[shift3 + 2] = 0;
			shift3 += 3 * width;
		}

		x = endX;
		shift3 = (startY * width + x) * 3;
		for (int y = startY; y <= endY; y++)
		{

			DaColourLayersWithRedraw.StartAddress[shift3 + 0] = 255;
			DaColourLayersWithRedraw.StartAddress[shift3 + 1] = 0;
			DaColourLayersWithRedraw.StartAddress[shift3 + 2] = 0;
			shift3 += 3 * width;
		}


		y = startYBig;
		shift3 = 3 * y * width + 3 * startXBig;
		for (int x = startXBig; x <= endXBig; x++)
		{
			DaColourLayersWithRedraw.StartAddress[shift3 + 0] = 0;
			DaColourLayersWithRedraw.StartAddress[shift3 + 1] = 0;
			DaColourLayersWithRedraw.StartAddress[shift3 + 2] = 255;
			shift3 += 3;
		}
		y = endYBig;
		shift3 = 3 * y * width + 3 * startXBig;
		for (int x = startXBig; x <= endXBig; x++)
		{
			DaColourLayersWithRedraw.StartAddress[shift3 + 0] = 0;
			DaColourLayersWithRedraw.StartAddress[shift3 + 1] = 0;
			DaColourLayersWithRedraw.StartAddress[shift3 + 2] = 255;
			shift3 += 3;
		}

		x = startXBig;
		shift3 = (startYBig * width + x) * 3;
		for (int y = startYBig; y <= endYBig; y++)
		{

			DaColourLayersWithRedraw.StartAddress[shift3 + 0] = 0;
			DaColourLayersWithRedraw.StartAddress[shift3 + 1] = 0;
			DaColourLayersWithRedraw.StartAddress[shift3 + 2] = 255;
			shift3 += 3 * width;
		}

		x = endXBig;
		shift3 = (startYBig * width + x) * 3;
		for (int y = startYBig; y <= endYBig; y++)
		{

			DaColourLayersWithRedraw.StartAddress[shift3 + 0] = 0;
			DaColourLayersWithRedraw.StartAddress[shift3 + 1] = 0;
			DaColourLayersWithRedraw.StartAddress[shift3 + 2] = 255;
			shift3 += 3 * width;
		}
	}

	delete[] cBig;
	delete[] c;
	delete[] colours;
	delete[] middleColours;
	delete[] middleNonColours;
	delete[] ncolours;

	delete[] length_C_RC;
	delete[] length_RC_NC;

	delete[] partPixels;
	delete[] partPixelsBig;
}

void CLWorker::correctColourLayers()
{
	cclMask = new bool[width * height];
	memset(cclMask, 0, width * height);
	//Разбиваем сетку на 100x100

	DaColourLayersWithRedraw = *Data->DaColourLayers;

	DaColourLayersWithRedraw.StartAddress = new UCHAR[DaColourLayersWithRedraw.Cols * DaColourLayersWithRedraw.Rows * DaColourLayersWithRedraw.Plants];

	memcpy(DaColourLayersWithRedraw.StartAddress, Data->DaColourLayers->StartAddress, DaColourLayersWithRedraw.Cols * DaColourLayersWithRedraw.Rows * DaColourLayersWithRedraw.Plants);

	bNearestOrMiddle = false;


	int InnerWidth = 50;
	int OutterWidth = 25;

	bNearestOrMiddle = false;
	for (int y = 0; y < height; y += InnerWidth)
	{
		for (int x = 0; x < width; x += InnerWidth)
		{
			analyzeSquare(x, y, InnerWidth, OutterWidth);
		}
	}

	bNearestOrMiddle = true;
	for (int y = 25; y < height; y+= InnerWidth)
	{
		for (int x = 25; x < width; x+= InnerWidth)
		{
			analyzeSquare(x, y, InnerWidth, OutterWidth);
		}
	}

	char ColoursFullPath[MAX_PATH];
	sprintf_s(ColoursFullPath, "%sSampleCL_Algo_3_2.png", imagesPath);

	if (saveImages == true)
		DataAreaWrite(&DaColourLayersWithRedraw, ColoursFullPath);

	Data->DBGImages[0] = new DataArea();

	*Data->DBGImages[0] = DaColourLayersWithRedraw;
	Data->DBGImages[0]->StartAddress = new UCHAR[width * height * 3];
	memcpy(Data->DBGImages[0]->StartAddress, DaColourLayersWithRedraw.StartAddress, width * height * 3);

	flushInfo(&Data->DBGImages[0]->info, "Algo_3_2");

	sprintf_s(ColoursFullPath, "%sSampleCL_Algo_3_3.png", imagesPath);

	if (saveImages == true)
		DataAreaWrite(Data->DaColourLayers, ColoursFullPath);

	Data->DBGImages[1] = new DataArea();

	*Data->DBGImages[1] = *Data->DaColourLayers;
	Data->DBGImages[1]->StartAddress = new UCHAR[width * height * 3];
	memcpy(Data->DBGImages[1]->StartAddress, Data->DaColourLayers->StartAddress, width * height * 3);

	flushInfo(&Data->DBGImages[1]->info, "Algo_3_3");

	delete[] DaColourLayersWithRedraw.StartAddress;
	delete[] cclMask;
}

void CLWorker::SetPresetColours(PRGB_STRUCT Colours, int count)
{
	if (count <= 0)
		return;

	delete[] PresetColours;
	PresetColours = new LAB_STRUCT_f[count];
	for (int i = 0; i < count; i++)
	{
		LAB_STRUCT tmp = RGB2LAB(Colours[i]);
		PresetColours[i].L = static_cast<float>(tmp.L);
		PresetColours[i].A = static_cast<float>(tmp.A);
		PresetColours[i].B = static_cast<float>(tmp.B);
	}
	PresetColoursCount = count;
}

void CLWorker::SetForbiddenColours(PRGB_STRUCT Colours, int count)
{
	if (count <= 0)
		return;
	delete[] ForbiddenColours;
	ForbiddenColours = new LAB_STRUCT_f[count];
	for (int i = 0; i < count; i++)
	{
		LAB_STRUCT tmp = RGB2LAB(Colours[i]);
		ForbiddenColours[i].L = static_cast<float>(tmp.L);
		ForbiddenColours[i].A = static_cast<float>(tmp.A);
		ForbiddenColours[i].B = static_cast<float>(tmp.B);
	}
	ForbiddenColoursCount = count;
}

void CLWorker::mergeClusters(uint i, uint j)
{

	int maxWeight = 0;
	LAB_STRUCT_f maxWeightLAB = {0, 0, 0};

	for (int l = lround(min(vectorClusters[i].LAB.L, vectorClusters[j].LAB.L)); l <= lround(max(vectorClusters[i].LAB.L, vectorClusters[j].LAB.L)); l++)
	for (int a = lround(min(vectorClusters[i].LAB.A, vectorClusters[j].LAB.A)); a <= lround(max(vectorClusters[i].LAB.A, vectorClusters[j].LAB.A)); a++)
	for (int b = lround(min(vectorClusters[i].LAB.B, vectorClusters[j].LAB.B)); b <= lround(max(vectorClusters[i].LAB.B, vectorClusters[j].LAB.B)); b++)
	{
		int currentWeight = LAB_SPACE_E[(l << 16) + (a << 8) + b];
		if (maxWeight < currentWeight)
		{
			maxWeight = currentWeight;
			maxWeightLAB = { static_cast<float>(l), static_cast<float>(a), static_cast<float>(b) };
		}
	}

	if (attempt == 1 && i >= PresetColoursCount)
	{
		vectorClusters[i].LAB.L = maxWeightLAB.L;
		vectorClusters[i].LAB.A = maxWeightLAB.A;
		vectorClusters[i].LAB.B = maxWeightLAB.B;
	}

	vectorClusters[i].count = max(vectorClusters[i].count, vectorClusters[j].count);


	for (int index = 0; index < 6; index++)
		vectorClusters[i].clusterRadius[index] = max(vectorClusters[i].clusterRadius[index], vectorClusters[j].clusterRadius[index]);

	vectorClusters.erase(vectorClusters.begin() + j);

}

void CLWorker::putPixel(int shift3, int index)
{
	Data->DaColourLayers->StartAddress[shift3 + 0] = Data->clusters[index].RGB.R;
	Data->DaColourLayers->StartAddress[shift3 + 1] = Data->clusters[index].RGB.G;
	Data->DaColourLayers->StartAddress[shift3 + 2] = Data->clusters[index].RGB.B;
}

bool CLWorker::isValid(int shift, int leftX, int rightX, int leftY, int rightY)
{
	int y = shift / width;
	int x = shift % width;
	if (rightX == 0)
		rightX = width;
	if (rightY == 0)
		rightY = height;
	return (x  >= leftX && x < rightX && y >= leftY && y < rightY);
}

void CLWorker::extendColourLayersWaveAlgorithm(int number)
{
	int k = Data->clustersCount;
	auto q = new queue <int>[k];

	auto frontNumber = new int[width * height];
	fill(frontNumber, frontNumber + width * height, -1);
	
	auto zeroFrontColour = new LAB_STRUCT[width * height];
	memset(zeroFrontColour, 0, width * height * sizeof(LAB_STRUCT));

	int shift = 1 + width;
	for (int y = 1; y < height - 1; y++)
	{
		for (int x = 1; x < width - 1; x++)
		{
			UCHAR index = Data->colourLayersIndex->StartAddress[shift];
			if (index < FORBIDDEN)
			{
				bool bMinusOneIndex = false;
				for (int j = -1; j <= 1; j++)
				{
					for (int i = -1; i <= 1; i++)
					{
						UCHAR newIndex = Data->colourLayersIndex->StartAddress[shift + i + j * width];
						if (newIndex == UNDEFINED)
						{
							bMinusOneIndex = true;
							break;
						}
					}
					if (bMinusOneIndex == true)
						break;
				}
				if (bMinusOneIndex == true)
				{
					q[index].push(shift);
					frontNumber[shift] = 0;
					zeroFrontColour[shift] = labBuffer[shift];
				}
			}
			shift++;
		}
		shift += 2;
	}

	

	auto mask = new bool[width * height];
	memset(mask, 0, width * height);

	const double A = 5.9;
	const double K = 2;
	const double a = 2 * K / A - 1;
	const double neirboughsLimit = 1;

	double Coef = 0.15378;

	int limitAdd = 0;
	
	bool bThereIsNotEmptyQueue = true;

	while (bThereIsNotEmptyQueue == true)
	{
		bThereIsNotEmptyQueue = false;
		for (int i = 0; i < k; i++)
		{
			queue<int> qNew;
			while (!q[i].empty())
			{
				int lShift = q[i].front();
				q[i].pop();


				for (int y = -1; y <= 1; y++)
				{
					for (int x = -1; x <= 1; x++)
					{
						int newShift = lShift + x + y * width;

						if (isValid(newShift) && (Data->colourLayersIndex->StartAddress[newShift] == UNDEFINED || Data->colourLayersIndex->StartAddress[newShift] == FORBIDDEN && attempt == 2 && number >= 3) && mask[newShift] == false)
						{

							double limit = K / (frontNumber[lShift] + 1 + a) + A * 0.75 + limitAdd;



							if ((lengthCoef(Coef, zeroFrontColour[lShift], labBuffer[newShift]) <= limit || lengthCoef(Coef, labBuffer[newShift], labBuffer[lShift]) <= neirboughsLimit) && (attempt == 2 && number >= 4 || Data->artefacts->StartAddress[newShift] == 0 && (Data->artefacts->StartAddress[lShift] == 0 || attempt == 2 && number >= 3)))
							{
								if (Data->colourLayersIndex->StartAddress[newShift] != FORBIDDEN)
								{
									putPixel(3 * newShift, i);
									
									Data->colourLayersIndex->StartAddress[newShift] = i;
								}
								mask[newShift] = true;
								if (Data->artefacts->StartAddress[newShift] == 0 || attempt == 2 && number >= 3)
									qNew.push(newShift);
								frontNumber[newShift] = frontNumber[lShift] + 1;
								zeroFrontColour[newShift] = zeroFrontColour[lShift];
							}
							else
							{
								if ((Data->artefacts->StartAddress[newShift] == 1 && Data->artefacts->StartAddress[lShift] == 0) || attempt == 2 && number >= 4)
								{
									if (Data->colourLayersIndex->StartAddress[newShift] != FORBIDDEN)
									{
										putPixel(3 * newShift, i);
										
										Data->colourLayersIndex->StartAddress[newShift] = i;
									}
									mask[newShift] = true;
									if (attempt == 2 && number >= 4)
										qNew.push(newShift);
								}
							}
						}
					}
				}
			}

			bThereIsNotEmptyQueue = bThereIsNotEmptyQueue || !qNew.empty();
			q[i] = qNew;
		}

	}

	char DaColourLayersSavingPath[MAX_PATH];
	sprintf_s(DaColourLayersSavingPath, MAX_PATH, "%sSampleCL_Algo_%d_%d.png", imagesPath, attempt, number);

	if (saveImages == true)
		DataAreaWrite(Data->DaColourLayers, DaColourLayersSavingPath);
	
	Data->DBGImages[number] = new DataArea();

	*Data->DBGImages[number] = *Data->DaColourLayers;
	Data->DBGImages[number]->StartAddress = new UCHAR[width * height * 3];
	memcpy(Data->DBGImages[number]->StartAddress, Data->DaColourLayers->StartAddress, width * height * 3);
	flushInfo(&Data->DBGImages[number]->info, "Algo_" + to_string(attempt) + "_" + to_string(number));


	delete[] frontNumber;
	delete[] mask;
	delete[] q;
	delete[] zeroFrontColour;
}

void CLWorker::extendColourLayersNearestColour()
{
	if (Data->clustersCount == 0)
		return;
	int shift = 1 + width;
	const uint k = Data->clustersCount;
	const auto lengths = new double[k];


	//Отдача Красных пикселей из SampleCL_Algo_2_3.png к ближайшему пикселю.
	for (int y = 1; y < height - 1; y++)
	{
		for (int x = 1; x < width - 1; x++)
		{
			if (Data->colourLayersIndex->StartAddress[shift] == UNDEFINED && Data->artefacts->StartAddress[shift] == 0)
			{
				double minLength = 100000;
				int minIndex = 0;
				for (int i = 0; i < Data->clustersCount; i++)
				{
					lengths[i] = sqrt(L_Coef_Nearest_Draw * (labBuffer[shift].L - Data->clusters[i].LAB.L) * (labBuffer[shift].L - Data->clusters[i].LAB.L) + (labBuffer[shift].A - Data->clusters[i].LAB.A) * (labBuffer[shift].A - Data->clusters[i].LAB.A) + (labBuffer[shift].B - Data->clusters[i].LAB.B) * (labBuffer[shift].B - Data->clusters[i].LAB.B));
				}

				for (int i = 0; i < Data->clustersCount; i++)
				{
					if (lengths[i] < minLength)
					{
						minLength = lengths[i];
						minIndex = i;
					}
				}

				putPixel(3 * shift, minIndex);
				Data->colourLayersIndex->StartAddress[shift] = minIndex;
			}
			shift++;
		}
		shift += 2;
	}


	const auto c = new int[k];
	memset(c, 0, k * sizeof(int));
	const int depth = 2;
	shift = 2 + 2 * width;
	for (int y = 2; y < height - 2; y++)
	{
		for (int x = 2; x < width - 2; x++)
		{
			memset(c, 0, k * sizeof(int));
			if (Data->colourLayersIndex->StartAddress[shift] == UNDEFINED && Data->artefacts->StartAddress[shift] == 1)
			{
				for (int h = -depth; h <= depth; h++)
				{
					for (int w = -depth; w <= depth; w++)
					{
						int newShift = shift + w + h * width;


						if (Data->artefacts->StartAddress[newShift] == 0 && Data->colourLayersIndex->StartAddress[newShift] < FORBIDDEN)
						{
							c[Data->colourLayersIndex->StartAddress[newShift]]++;
						}

						double minLength = 100000;
						uint minIndex = 0;
						for (int i = 0; i < Data->clustersCount; i++)
						{
							if (c[i] > 1)
								lengths[i] = sqrt(L_Coef_Nearest_Draw * (labBuffer[shift].L - Data->clusters[i].LAB.L) * (labBuffer[shift].L - Data->clusters[i].LAB.L) + (labBuffer[shift].A - Data->clusters[i].LAB.A) * (labBuffer[shift].A - Data->clusters[i].LAB.A) + (labBuffer[shift].B - Data->clusters[i].LAB.B) * (labBuffer[shift].B - Data->clusters[i].LAB.B));
						}

						for (int i = 0; i < Data->clustersCount; i++)
						{
							if (lengths[i] < minLength && c[i] > 1)
							{
								minLength = lengths[i];
								minIndex = i;
							}
						}

						if (minLength < 100000)
						{
							putPixel(3 * shift, minIndex);
							Data->colourLayersIndex->StartAddress[shift] = minIndex;
						}

					}
				}
			}
			shift++;
		}
		shift += 4;
	}
	shift = 1 + width;
	for (int y = 1; y < height - 1; y++)
	{
		for (int x = 1; x < width - 1; x++)
		{
			if (Data->colourLayersIndex->StartAddress[shift] == UNDEFINED && Data->artefacts->StartAddress[shift] == 1)
			{
				double minLength = 100000;
				int minIndex = 0;
				for (int i = 0; i < Data->clustersCount; i++)
				{
					lengths[i] = sqrt(L_Coef_Nearest_Draw * (labBuffer[shift].L - Data->clusters[i].LAB.L) * (labBuffer[shift].L - Data->clusters[i].LAB.L) + (labBuffer[shift].A - Data->clusters[i].LAB.A) * (labBuffer[shift].A - Data->clusters[i].LAB.A) + (labBuffer[shift].B - Data->clusters[i].LAB.B) * (labBuffer[shift].B - Data->clusters[i].LAB.B));
				}

				for (int i = 0; i < Data->clustersCount; i++)
				{
					if (lengths[i] < minLength)
					{
						minLength = lengths[i];
						minIndex = i;
					}
				}

				putPixel(3 * shift, minIndex);
				Data->colourLayersIndex->StartAddress[shift] = minIndex;
			}
			shift++;
		}
		shift += 2;
	}

	char DaColourLayersSavingPath[MAX_PATH];
	sprintf_s(DaColourLayersSavingPath, MAX_PATH, "%sSampleCL_Algo_3_1.png", imagesPath);

	if (saveImages == true)
		DataAreaWrite(Data->DaColourLayers, DaColourLayersSavingPath);

	if (Data->DBGImages)
	{
		Data->DBGImages[6] = new DataArea();

		*Data->DBGImages[6] = *Data->DaColourLayers;
		Data->DBGImages[6]->StartAddress = new UCHAR[width * height * 3];
		memcpy(Data->DBGImages[6]->StartAddress, Data->DaColourLayers->StartAddress, width * height * 3);

		flushInfo(&Data->DBGImages[6]->info, "Algo_3_1");
	}
	
	delete[] lengths;
	delete[] c;
}


double CLWorker::calcStatistics()
{
	int sumWeight = 0;

	for (int l = 0; l < 256; l++)
	{
		for (int a = 0; a < 256; a++)
		{
			for (int b = 0; b < 256; b++)
			{
				int weight = LAB_SPACE[(l << 16) + (a << 8) + b];
				if (weight > 0)
				{
					sumWeight += weight;
				}
			}
		}
	}

	double middleLWeight = 0;
	double middleAWeight = 0;
	double middleBWeight = 0;

	for (int l = 0; l < 256; l++)
	{
		for (int a = 0; a < 256; a++)
		{
			for (int b = 0; b < 256; b++)
			{
				int weight = LAB_SPACE[(l << 16) + (a << 8) + b];
				if (weight > 0)
				{
					double p = (double)weight / sumWeight;
					middleLWeight += static_cast<double>(l) * p;
					middleAWeight += static_cast<double>(a) * p;
					middleBWeight += static_cast<double>(b) * p;
				}
			}
		}
	}

	double dispL = 0;
	double dispA = 0;
	double dispB = 0;

	for (int l = 0; l < 256; l++)
	{
		for (int a = 0; a < 256; a++)
		{
			for (int b = 0; b < 256; b++)
			{
				int weight = LAB_SPACE[(l << 16) + (a << 8) + b];
				if (weight > 0)
				{
					double p = (double)weight / sumWeight;

					dispL += (static_cast<double>(l) - middleLWeight) * (static_cast<double>(l) - middleLWeight) * p;
					dispA += (static_cast<double>(a) - middleAWeight) * (static_cast<double>(a) - middleAWeight) * p;
					dispB += (static_cast<double>(b) - middleBWeight) * (static_cast<double>(b) - middleBWeight) * p;
				}
			}
		}
	}

	return sqrt(L_Coef_SQUARE * 6 * sqrt(dispL) + 6 * sqrt(dispA) + 6 * sqrt(dispB));
}

double CLWorker::getDoubleLABValue(double dL, double dA, double dB)
{
	if (dL < 1)
		dL = 1;

	if (dA < 1)
		dA = 1;

	if (dB < 1)
		dB = 1;

	if (dL > 254)
		dL = 254;

	if (dA > 254)
		dA = 254;

	int L_Left = static_cast<int>(dL);
	int A_Left = static_cast<int>(dA);
	int B_Left = static_cast<int>(dB);

	int L_Right = L_Left + 1;
	int A_Right = A_Left + 1;
	int B_Right = B_Left + 1;

	double deltaL = dL - L_Left;
	double deltaA = dA - A_Left;
	double deltaB = dB - B_Left;
	 
	return ((LAB_SPACE[(L_Left << 16) + (A_Left << 8) + B_Left] * (1 - deltaB) + LAB_SPACE[(L_Left << 16) + (A_Left << 8) + B_Right] * (deltaB)) * (1 - deltaA) + 
		   (LAB_SPACE[(L_Left << 16) + (A_Right << 8) + B_Left] * (1 - deltaB) + LAB_SPACE[(L_Left << 16) + (A_Right << 8) + B_Right] * (deltaB)) * (deltaA)) * (1 - deltaL) + 
		((LAB_SPACE[(L_Right << 16) + (A_Left << 8) + B_Left] * (1 - deltaB) + LAB_SPACE[(L_Right << 16) + (A_Left << 8) + B_Right] * (deltaB)) * (1 - deltaA) +
		(LAB_SPACE[(L_Right << 16) + (A_Right << 8) + B_Left] * (1 - deltaB) + LAB_SPACE[(L_Right << 16) + (A_Right << 8) + B_Right] * (deltaB)) * (deltaA)) * deltaL;
}


void CLWorker::drawHistoBwAllColours()
{
	if (vectorClusters.empty() == false && vectorClusters[0].count > 0)
	{
		char GistoDir[MAX_PATH];
		sprintf_s(GistoDir, "%s\\Histo_%d", imagesPath, attempt);

		if (saveImages == true)
		{
			_mkdir(GistoDir);
		}

		for (uint i = 0; i < vectorClusters.size(); i++)
		{
			for (uint j = i + 1; j < vectorClusters.size(); j++)
			{
				drawHistoBwColours(i, j);
			}
		}

	}
}

void CLWorker::drawHistoBwColours(int i, int j)
{
	int L1 = lround(vectorClusters[i].LAB.L);
	int A1 = lround(vectorClusters[i].LAB.A);
	int B1 = lround(vectorClusters[i].LAB.B);

	int L2 = lround(vectorClusters[j].LAB.L);
	int A2 = lround(vectorClusters[j].LAB.A);
	int B2 = lround(vectorClusters[j].LAB.B);

	int MAXDiff = max(max(abs(L2 - L1), abs(A2 - A1)), abs(B2 - B1));

	int edgeEPS = lround(static_cast<double>(MAXDiff + 1) / 3);

	//cout << MAXDiff << endl;

	float stepL = static_cast<float>(L2 - L1) / MAXDiff;
	float stepA = static_cast<float>(A2 - A1) / MAXDiff;
	float stepB = static_cast<float>(B2 - B1) / MAXDiff;

	int expansion = 10;

	int DBGImagesIndex = 3;
	if (attempt == 1)
		DBGImagesIndex++;

	DBGImagesIndex += vectorClusters.size() * i - i * (i + 1)/2 + j - i - 1;

	Data->DBGImages[DBGImagesIndex] = new DataArea();

	DataArea * s = Data->DBGImages[DBGImagesIndex];
	
	s->Cols = (MAXDiff + 1 + 2 * edgeEPS) * 10;
	s->Rows = max(LAB_SPACE[(L1 << 16) + (A1 << 8) + B1], LAB_SPACE[(L2 << 16) + (A2 << 8) + B2]);
	s->Plants = 3;
	s->Offset = s->Cols * s->Plants;


	flushInfo(&s->info, "Histo_" + to_string(attempt) + "_" + to_string(i) + "_" + to_string(j));

	s->StartAddress = new UCHAR[s->Cols * s->Rows * s->Plants];

	fill(s->StartAddress, s->StartAddress + s->Cols * s->Rows * s->Plants, 255);

	for (int u = -edgeEPS; u <= MAXDiff + edgeEPS; u++)
	{
		uint value = lround(getDoubleLABValue(u * stepL + L1, u * stepA + A1, u * stepB + B1));

		int shift = s->Cols * (s->Rows - 1) * s->Plants + expansion * (u + edgeEPS) * s->Plants;

		LAB_STRUCT_f tmp = { u * stepL + L1, u * stepA + A1, u * stepB + B1 };

		bool iInEllips = isDotInEllips(vectorClusters[i].LAB, tmp, i, true);
		bool jInEllips = isDotInEllips(vectorClusters[j].LAB, tmp, j, true);

		for (uint y = 0; y < min(value, s->Rows); y++)
		{
			for (int e = 0; e < expansion; e++)
			{
				RGB_STRUCT Colour = LAB2RGB_f({ tmp.L, tmp.A, tmp.B });
				if (iInEllips)
				{
					if (y == 0 || y == min(value, s->Rows) - 1 || e == 0 || e == expansion - 1)
					{
						Colour = { 0, 0, 0 };
						if (u == 0 || u == MAXDiff)
						{
							Colour = { 255, 0, 0 };
						}
					}
				}
				if (jInEllips)
				{
					if (y == 0 || y == min(value, s->Rows) - 1 || e == 0 || e == expansion - 1)
					{
						Colour = { 0, 0, 0 };
						if (u == 0 || u == MAXDiff)
						{
							Colour = { 255, 0, 0 };
						}
					}
				}
				

				
				s->StartAddress[shift + 0] = Colour.R;
				s->StartAddress[shift + 1] = Colour.G;
				s->StartAddress[shift + 2] = Colour.B;
				shift += s->Plants;
			}
			shift -= expansion * s->Plants;
			shift -= s->Cols * s->Plants;
		}

		
	}

	char ColoursFullPath[MAX_PATH];
	sprintf_s(ColoursFullPath, "%s\\Histo_%d\\Histo_%d_%d_%d.png", imagesPath, attempt, attempt, i, j);

	if (saveImages == true)
		DataAreaWrite(s, ColoursFullPath);

}

void CLWorker::flushInfo()
{
	Data->info = new char[info.length() + 1];
	strcpy_s(Data->info, info.length() + 1, info.c_str());
}

void CLWorker::flushInfo(char ** pszInfo, string stringInfo)
{
	*pszInfo = new char[stringInfo.length() + 1];
	strcpy_s(*pszInfo, stringInfo.length() + 1, stringInfo.c_str());
}


LAB_STRUCT_f CLWorker::findDot(double radX, double radY, double radZ, LAB_STRUCT_f Xi, LAB_STRUCT_f Xj)
{
	
	float LeftX = Xi.L;
	float LeftY = Xi.A;
	float LeftZ = Xi.B;

	float RightX = Xj.L;
	float RightY = Xj.A;
	float RightZ = Xj.B;

	float SearchCoordX = 0;
	float SearchCoordY = 0;
	float SearchCoordZ = 0;

	int count = 0;
	while (true)
	{
		SearchCoordX = (LeftX + RightX) / 2;
		SearchCoordY = (LeftY + RightY) / 2;
		SearchCoordZ = (LeftZ + RightZ) / 2;

		double value = (SearchCoordX - Xi.L) * (SearchCoordX - Xi.L) / (radX * radX) + (SearchCoordY - Xi.A) * (SearchCoordY - Xi.A) / (radY * radY) +
			(SearchCoordZ - Xi.B) * (SearchCoordZ - Xi.B) / (radZ * radZ);

		if (SearchCoordX == Xi.L && SearchCoordY == Xi.A && SearchCoordZ == Xi.B)
			break;
		if (SearchCoordX == Xj.L && SearchCoordY == Xj.A && SearchCoordZ == Xj.B)
			break;

		if (value >= 0.99 && value <= 1.01)
			break;
		if (value > 1)
		{
			RightX = SearchCoordX;
			RightY = SearchCoordY;
			RightZ = SearchCoordZ;
		}
		else
		{
			LeftX = SearchCoordX;
			LeftY = SearchCoordY;
			LeftZ = SearchCoordZ;
		}
		

		count++;

		if (count == 100)
			break;
	}

	return { SearchCoordX , SearchCoordY, SearchCoordZ };
}


int CLWorker::correctClustersRadius()
{
	int clustersMerged = 0;
	// Нужно определить 6 радиусов каждого кластера. По 2 на каждое направление в LAB с + и -
	if (vectorClusters.size() == 1)
		return 0;


	for (uint i = 0; i < vectorClusters.size(); i++)
	{
		for (uint j = max(i + 1, PresetColoursCount + ForbiddenColoursCount); j < vectorClusters.size(); j++)
		{
			auto c_i = vectorClusters[i];
			auto c_j = vectorClusters[j];

			double clustersDistance = sqrt(L_Coef_Clusters_Radius * (c_i.LAB.L - c_j.LAB.L) * (c_i.LAB.L - c_j.LAB.L) + (c_i.LAB.A - c_j.LAB.A) * (c_i.LAB.A - c_j.LAB.A) + (c_i.LAB.B - c_j.LAB.B) * (c_i.LAB.B - c_j.LAB.B));

			if (additionalInfo == true)
			{
				info += "I_J Coords:\n";
				info += to_string(lround(c_i.LAB.L)) + " " + to_string(lround(c_i.LAB.A)) + " " + to_string(lround(c_i.LAB.B)) + "\n";
				info += to_string(lround(c_j.LAB.L)) + " " + to_string(lround(c_j.LAB.A)) + " " + to_string(lround(c_j.LAB.B)) + "\n";
			}
			//Знаки радиусов кластеров "0" - "-", "1" - "+"
			bool signLi = (c_j.LAB.L - c_i.LAB.L) > 0;
			bool signAi = (c_j.LAB.A - c_i.LAB.A) > 0;
			bool signBi = (c_j.LAB.B - c_i.LAB.B) > 0;

			bool signLj = !signLi;
			bool signAj = !signAi;
			bool signBj = !signBi;


			double RX_i = vectorClusters[i].clusterRadius[!signLi];
			double RY_i = vectorClusters[i].clusterRadius[!signAi + 2];
			double RZ_i = vectorClusters[i].clusterRadius[!signBi + 4];
			double RX_j = vectorClusters[j].clusterRadius[!signLj];
			double RY_j = vectorClusters[j].clusterRadius[!signAj + 2];
			double RZ_j = vectorClusters[j].clusterRadius[!signBj + 4];

		

			LAB_STRUCT_f m_i;
			LAB_STRUCT_f m_j;


			m_i = findDot(RX_i, RY_i, RZ_i, c_i.LAB, c_j.LAB);
			m_j = findDot(RX_j, RY_j, RZ_j, c_j.LAB, c_i.LAB);
			
			if (additionalInfo == true)
			{
				info += "m_i dot: " + to_string(m_i.L) + " " + to_string(m_i.A) + " " + to_string(m_i.B) + "\n";
				info += "m_j dot: " + to_string(m_j.L) + " " + to_string(m_j.A) + " " + to_string(m_j.B) + "\n";
			}
			//Радиусы кластеров вдоль направления между центрами кластеров.

			double iDirectionRadius = sqrt(L_Coef_Clusters_Radius * (m_i.L - c_i.LAB.L) * (m_i.L - c_i.LAB.L) + (m_i.A - c_i.LAB.A) * (m_i.A - c_i.LAB.A) + (m_i.B - c_i.LAB.B) * (m_i.B - c_i.LAB.B));
			double jDirectionRadius = sqrt(L_Coef_Clusters_Radius * (m_j.L - c_j.LAB.L) * (m_j.L - c_j.LAB.L) + (m_j.A - c_j.LAB.A) * (m_j.A - c_j.LAB.A) + (m_j.B - c_j.LAB.B) * (m_j.B - c_j.LAB.B));
			
			if (additionalInfo == true)
			{
				info += "DirectionRadius: " + to_string(iDirectionRadius) + " " + to_string(jDirectionRadius) + "\n";
				info += "clustersDistance: " + to_string(clustersDistance) + "\n";
			}
			//Если расстояние между центрами кластеров меньше их радиусов, то эти радиусы нужно раздвигать.
			if (clustersDistance < (iDirectionRadius + jDirectionRadius) || max(fabs(c_i.LAB.A - c_j.LAB.A), fabs(c_i.LAB.B - c_j.LAB.B)) <= AB_Merge_Check_Limit)
			{
				//Веса кластеров
				int iWeight = vectorClusters[i].count;
				int jWeight = vectorClusters[j].count;

				if (additionalInfo == true)
				{
					info += "Need to breed " + to_string(i) + " and " + to_string(j) + " clusters" + "\n";
				}



				//Первая корректировка радиусов в зависимости от весов кластеров. Сумма данных радиусов равна расстоянию между кластерами.
				double iR_Corrected = clustersDistance * iWeight / (iWeight + jWeight);
				double jR_Corrected = clustersDistance * jWeight / (iWeight + jWeight);


				if (additionalInfo == true)
				{
					info += "FirstStepRadiusCorrectness: " + to_string(iR_Corrected) + " " + to_string(jR_Corrected) + "\n";
				}

				int L_ = lround(c_i.LAB.L * jWeight / (iWeight + jWeight) + c_j.LAB.L * iWeight / (iWeight + jWeight));
				int A_ = lround(c_i.LAB.A * jWeight / (iWeight + jWeight) + c_j.LAB.A * iWeight / (iWeight + jWeight));
				int B_ = lround(c_i.LAB.B * jWeight / (iWeight + jWeight) + c_j.LAB.B * iWeight / (iWeight + jWeight));


				//Вес элемента посередине между кластерами
				int mWeight = LAB_SPACE[(L_ << 16) + (A_ << 8) + B_];

				if (additionalInfo == true)
				{
					info += "iWeight: " + to_string(iWeight) + "\n";
					info += "jWeight: " + to_string(jWeight) + "\n";
					info += "mWeight: " + to_string(mWeight) + "\n";
				}

				/*Вторая корректировка радиусов в зависимости от веса элемента посередине.
				 *Это функция биективно отображает радиусы в зависимости от веса элемента посередине
				 *[0, (iWeight + jWeight)/2] -> [Radius, 0]
				 */
				iR_Corrected = (-2 * iR_Corrected * mWeight) / (iWeight + jWeight) + iR_Corrected;
				jR_Corrected = (-2 * jR_Corrected * mWeight) / (iWeight + jWeight) + jR_Corrected;

				bool nullRadius = false;

				if (iR_Corrected <= 0)
				{
					iR_Corrected = 0.0001;
					nullRadius = true;
				}
				if (jR_Corrected <= 0)
				{
					jR_Corrected = 0.0001;
					nullRadius = true;
				}
				double iCoef = iDirectionRadius / iR_Corrected;
				double jCoef = jDirectionRadius / jR_Corrected;

				if (additionalInfo == true)
				{
					info += "YMENSHENIE V " + to_string(iCoef) + " RAZ\n";
					info += "YMENSHENIE V " + to_string(jCoef) + " RAZ\n";
					if (nullRadius)
					{
						info += "nullRadius\n";
					}
				}

				LAB_STRUCT_f m_i_Corrected = { 0, 0, 0 };

				LAB_STRUCT_f m_j_Corrected = { 0, 0, 0 };

				if (nullRadius == false)
				{
					m_i_Corrected = findDot(RX_i / iCoef, RY_i / iCoef, RZ_i / iCoef, c_i.LAB, c_j.LAB);
					m_j_Corrected = findDot(RX_j / jCoef, RY_j / jCoef, RZ_j / jCoef, c_j.LAB, c_i.LAB);
				}

				if (additionalInfo == true)
				{
					info += "m_i_Corrected: " + to_string(m_i_Corrected.L) + " " + to_string(m_i_Corrected.A) + " " + to_string(m_i_Corrected.B) + "\n";
				}

				if (additionalInfo == true)
				{
					info += "m_j_Corrected: " + to_string(m_j_Corrected.L) + " " + to_string(m_j_Corrected.A) + " " + to_string(m_j_Corrected.B) + "\n";
				}
				if (additionalInfo == true)
				{
					info += "SecondStepRadiusCorrectness: " + to_string(iR_Corrected) + " " + to_string(jR_Corrected) + "\n";
				}

				double Mx = fabs(m_i_Corrected.L - c_i.LAB.L) * M_Mul_Coef;
				double My = fabs(m_i_Corrected.A - c_i.LAB.A) * M_Mul_Coef;
				double Mz = fabs(m_i_Corrected.B - c_i.LAB.B) * M_Mul_Coef;

				Mx /= Mx_Div_Coef;

				if (additionalInfo == true)
				{
					info += "Mx: " + to_string(Mx) + " My: " + to_string(My) + " Mz: " + to_string(Mz) + "\n";
				}

				vectorClusters[i].clusterRadius[!signLi] = min(LDirectionCoef * (My + Mz)/2, vectorClusters[i].clusterRadius[!signLi]);
				vectorClusters[i].clusterRadius[!signAi + 2] = min((Mx + Mz) / 2, vectorClusters[i].clusterRadius[!signAi + 2]);
				vectorClusters[i].clusterRadius[!signBi + 4] = min((My + Mx) / 2, vectorClusters[i].clusterRadius[!signBi + 4]);

				Mx = fabs(m_j_Corrected.L - c_j.LAB.L) * M_Mul_Coef;
				My = fabs(m_j_Corrected.A - c_j.LAB.A) * M_Mul_Coef;
				Mz = fabs(m_j_Corrected.B - c_j.LAB.B) * M_Mul_Coef;

				Mx /= Mx_Div_Coef;

				if (additionalInfo == true)
				{
					info += "Mx: " + to_string(Mx) + " My: " + to_string(My) + " Mz: " + to_string(Mz) + "\n";
				}

				vectorClusters[j].clusterRadius[!signLj] = min(LDirectionCoef * (My + Mz) / 2, vectorClusters[j].clusterRadius[!signLj]);
				vectorClusters[j].clusterRadius[!signAj + 2] = min((Mx + Mz) / 2, vectorClusters[j].clusterRadius[!signAj + 2]);
				vectorClusters[j].clusterRadius[!signBj + 4] = min((My + Mx) / 2, vectorClusters[j].clusterRadius[!signBj + 4]);

				if (additionalInfo == true)
				{
					info += "Found new Clusters Radiuses:\n";
					info += to_string(vectorClusters[i].clusterRadius[!signLi]) + " " + to_string(vectorClusters[i].clusterRadius[!signAi + 2]) + " " + to_string(vectorClusters[i].clusterRadius[!signBi + 4]) + "\n";
					info += to_string(vectorClusters[j].clusterRadius[!signLj]) + " " + to_string(vectorClusters[j].clusterRadius[!signAj + 2]) + " " + to_string(vectorClusters[j].clusterRadius[!signBj + 4]) + "\n";
				}

				if ((min(vectorClusters[i].clusterRadius[!signLi], vectorClusters[j].clusterRadius[!signLj]) < L_Merge_Limit * LDirectionCoef || (min(vectorClusters[i].clusterRadius[!signAi + 2], vectorClusters[j].clusterRadius[!signAj + 2]) < AB_Merge_Limit) || min(vectorClusters[j].clusterRadius[!signBj + 4], vectorClusters[i].clusterRadius[!signBi + 4]) < AB_Merge_Limit))
				{

					if (additionalInfo == true)
					{
						info += "Merging " + to_string(i) + " and " + to_string(j) + " clusters\n";
					}
					mergeClusters(i, j);
					j--;

					clustersMerged++;

					if (additionalInfo == true)
					{
						info += "New " + to_string(i) + " LAB coords:\n";
						info += to_string(lround(vectorClusters[i].LAB.L)) + " " + to_string(lround(vectorClusters[i].LAB.A)) + " " + to_string(lround(vectorClusters[i].LAB.B)) + "\n";
					}

				}
				else
				{
					if (nullRadius == true)
					{

						if (additionalInfo == true)
						{
							info += "Merging " + to_string(i) + " and " + to_string(j) + " clusters\n";
						}
						mergeClusters(i, j);
						j--;
						clustersMerged++;

						if (additionalInfo == true)
						{
							info += "New " + to_string(i) + " LAB coords:\n";
							info += to_string(vectorClusters[i].LAB.L) + " " + to_string(vectorClusters[i].LAB.A) + " " + to_string(vectorClusters[i].LAB.B) + "\n";
						}
					}
				}
			}

			if (vectorClusters.size() == 1)
				return 0;
		}
	}
	return clustersMerged;
}


bool CLWorker::isDotInEllips(LAB_STRUCT_f X, LAB_STRUCT_f Y, int clusterIndex, bool L = true)
{
	bool signLi = (Y.L - X.L) > 0;
	bool signAi = (Y.A - X.A) > 0;
	bool signBi = (Y.B - X.B) > 0;

	double RX_i;
	double RY_i;
	double RZ_i;

	if (vectorClusters.empty() == false)
	{
		RX_i = vectorClusters[clusterIndex].clusterRadius[!signLi];
		RY_i = vectorClusters[clusterIndex].clusterRadius[!signAi + 2];
		RZ_i = vectorClusters[clusterIndex].clusterRadius[!signBi + 4];
	}
	else
	{
		if (Data->clustersCount > 0)
		{
			RX_i = Data->clusters[clusterIndex].clusterRadius[!signLi];
			RY_i = Data->clusters[clusterIndex].clusterRadius[!signAi + 2];
			RZ_i = Data->clusters[clusterIndex].clusterRadius[!signBi + 4];
		}
	}
	if (L == true)
	{
		return (X.L - Y.L) *  (X.L - Y.L) / (0.15378 * RX_i * RX_i) + (X.A - Y.A) *  (X.A - Y.A) / (RY_i * RY_i) + (X.B - Y.B) * (X.B - Y.B) / (RZ_i * RZ_i) <= 1;
	}
	else
	{
		return (X.A - Y.A) *  (X.A - Y.A) / (RY_i * RY_i) + (X.B - Y.B) * (X.B - Y.B) / (RZ_i * RZ_i) <= 1;
	}

}

bool CLWorker::isDotInEllips(LAB_STRUCT_f X, LAB_STRUCT Y, int clusterIndex, bool L = true)
{
	LAB_STRUCT_f Y_f = {static_cast<float>(Y.L), static_cast<float>(Y.A) , static_cast<float>(Y.B)};
	return isDotInEllips(X, Y_f, clusterIndex, L);
}

void CLWorker::drawLABRadiuses()
{
	if (vectorClusters.empty() || vectorClusters[0].count == 0)
		return;

	int DBGImagesIndex = 1;
	if (attempt == 1)
		DBGImagesIndex++;

	Data->DBGImages[DBGImagesIndex + 1] = new DataArea();

	*Data->DBGImages[DBGImagesIndex + 1] = *Data->DBGImages[DBGImagesIndex];

	Data->DBGImages[DBGImagesIndex + 1]->StartAddress = new UCHAR[Data->DBGImages[DBGImagesIndex]->Cols * Data->DBGImages[DBGImagesIndex]->Rows * 3];

	memcpy(Data->DBGImages[DBGImagesIndex + 1]->StartAddress, Data->DBGImages[DBGImagesIndex]->StartAddress, Data->DBGImages[DBGImagesIndex]->Cols * Data->DBGImages[DBGImagesIndex]->Rows * 3);

	DataArea * s = Data->DBGImages[DBGImagesIndex + 1];

	int shift3 = 0;

	for (int a = 0; a < _1D; a++)
	{
		for (int b = 0; b < _1D; b++)
		{
			for (uint i = 0; i < vectorClusters.size(); i++)
			{
				LAB_STRUCT tmp = { 0, static_cast<UCHAR>(a), static_cast<UCHAR>(b) };
				if (isDotInEllips(vectorClusters[i].LAB, tmp, i, false))
				{
					s->StartAddress[shift3 + 0] = vectorClusters[i].RGB.R;
					s->StartAddress[shift3 + 1] = vectorClusters[i].RGB.G;
					s->StartAddress[shift3 + 2] = vectorClusters[i].RGB.B;

					break;
				}
			}

			shift3 += 3;
		}
	}

	for (auto f : vectorClusters)
	{
		int a = lround(f.LAB.A);
		int b = lround(f.LAB.B);

		int shift = 3 * (a * 256 + b);

		s->StartAddress[shift + 0] = 255;
		s->StartAddress[shift + 1] = 0;
		s->StartAddress[shift + 2] = 0;

	}

	char ColoursFullPathNew[MAX_PATH];
	sprintf_s(ColoursFullPathNew, "%sLAB_frequancy_Rad_%d.png", imagesPath, attempt);
	
	flushInfo(&s->info, "LAB_frequancy_Rad_" + to_string(attempt));

	if (saveImages == true)
		DataAreaWrite(s, ColoursFullPathNew);
}

void CLWorker::analyzeClusters(double vectorAreaLength)
{
	if (vectorClusters.empty() || vectorClusters[0].count == 0)
		return;

	int maxCount = 0;

	for (uint i = 0; i < vectorClusters.size(); i++)
	{
		if (vectorClusters[i].count > maxCount)
		{
			maxCount = vectorClusters[i].count;
		}
	}
	
	vectorAreaLength *= startingRadiusCoef;

	// Заполняем 6 радиусов каждого кластера линейно в зависимости от разброса точек и отноншения веса кластера к максимальному весу класера.
	for (uint i = 0; i < vectorClusters.size(); i++)
	{
		//Возможно, стоит это тоже вынести в качестве параметра
		fill(vectorClusters[i].clusterRadius, vectorClusters[i].clusterRadius + 6, vectorAreaLength * 0.5 * (static_cast<double>(vectorClusters[i].count) / maxCount + 1));

		vectorClusters[i].clusterRadius[0] *= L_Radius_Coef;
		vectorClusters[i].clusterRadius[1] *= L_Radius_Coef;

		
	}

	if (additionalInfo == true)
	{
		info += "CLUSTERS INFO BEFORE CORRECTION:\n";
		for (uint i = 0; i < vectorClusters.size(); i++)
		{
			for (int j = 0; j < 6; j++)
			{
				info += to_string(vectorClusters[i].clusterRadius[j]) + " ";
			}
			info += "\n";
		}
	}
	
	while (correctClustersRadius() > 0);

	uint PresetColoursCountLocal = PresetColoursCount;

	if (attempt != 1)
		PresetColoursCountLocal = 0;

	uint indexBorder = PresetColoursCountLocal + ForbiddenColoursCount;
	for (uint j = PresetColoursCountLocal; j < indexBorder; j++)
	{
		vectorClusters.erase(vectorClusters.begin() + j);

		j--;
		indexBorder--;
	}
	

	if (vectorClusters.empty() && attempt == 1)
	{
		vectorClusters.push_back(highestCluster);
		fill(vectorClusters[0].clusterRadius, vectorClusters[0].clusterRadius + 6, vectorAreaLength * 0.5 * (static_cast<double>(vectorClusters[0].count) / maxCount + 1));
	}


	if (additionalInfo == true && attempt == 1)
	{
		info += "CLUSTERS INFO AFTER CORRECTION:\n";
		for (uint i = 0; i < vectorClusters.size(); i++)
		{
			for (int j = 0; j < 6; j++)
			{
				info += to_string(Data->clusters[i].clusterRadius[j]) + " ";
			}
			info += "\n";
		}
	}

	printClustersInfo();

	cout << "Found " << vectorClusters.size() << " clusters" << endl;
	
	info += "Found " + to_string(vectorClusters.size()) + " clusters\n";
	
}

void CLWorker::drawLABStatistics()
{
	if (vectorClusters.empty() || vectorClusters[0].count == 0)
		return;

	int DBGImagesIndex = 1;
	if (attempt == 1)
		DBGImagesIndex++;

	Data->DBGImages[DBGImagesIndex] = new DataArea();

	DataArea * s = Data->DBGImages[DBGImagesIndex];

	s->Cols = 256;
	s->Rows = 256;
	s->Plants = 3;
	s->Offset = s->Cols * s->Plants;

	s->StartAddress = new UCHAR[s->Cols * s->Rows * s->Plants];

	int max_int = 0;

	int shift = 0;
	for (int a = 0; a < 256; a++)
	{
		for (int b = 0; b < 256; b++)
		{
			if (max_int < AB_SPACE[shift])
			{
				max_int = AB_SPACE[shift];
			}
			shift++;
		}
	}


	double coef = 255.0 / max_int;
	shift = 0;
	for (int a = 0; a < 256; a++)
	{
		for (int b = 0; b < 256; b++)
		{
			const UCHAR Colour = 255 - static_cast<UCHAR>(coef * AB_SPACE[a * (1 << 8) + b]);
			s->StartAddress[shift + 0] = Colour;
			s->StartAddress[shift + 1] = Colour;
			s->StartAddress[shift + 2] = Colour;
			shift+=3;
		}
	}

	for (uint i = 0; i < vectorClusters.size(); i++)
	{
		const auto v = vectorClusters[i];

		RGB_STRUCT rgb = LAB2RGB_f(v.LAB);
		int index = 3 * (lround(v.LAB.A) * (1 << 8) + lround(v.LAB.B));
		s->StartAddress[index + 0] = rgb.R;
		s->StartAddress[index + 1] = rgb.G;
		s->StartAddress[index + 2] = rgb.B;
	}

	char ColoursFullPath[MAX_PATH];
	sprintf_s(ColoursFullPath, "%sLAB_frequancy_%d.png", imagesPath, attempt);
	
	flushInfo(&s->info, "LAB_frequancy_" + to_string(attempt));

	if (saveImages == true)
		DataAreaWrite(s, ColoursFullPath);

}

void CLWorker::drawFoundColours(bool bFinal)
{
	if (bFinal == false && (vectorClusters.empty() || vectorClusters[0].count == 0))
		return;

	int DBGImagesIndex = 0;
	if (attempt == 1)
		DBGImagesIndex++;

	if (bFinal)
		DBGImagesIndex = 4;

	Data->DBGImages[DBGImagesIndex] = new DataArea();
	
	DataArea * s = Data->DBGImages[DBGImagesIndex];

	if (bFinal)
	{
		vectorClusters.resize(0);
		for (int i = 0; i < Data->clustersCount; i++)
		{
			vectorClusters.push_back(Data->clusters[i]);
		}
	}

	s->Cols = vectorClusters.size() * 100;
	s->Rows = 300;
	s->Plants = 3;
	s->Offset = s->Cols * s->Plants;

	for (uint i = 0; i < vectorClusters.size(); i++)
	{
		RGB_STRUCT rgb = LAB2RGB({ static_cast<UCHAR>(lround(vectorClusters[i].LAB.L)) , static_cast<UCHAR>(lround(vectorClusters[i].LAB.A)), static_cast<UCHAR>(lround(vectorClusters[i].LAB.B)) });
		vectorClusters[i].RGB = rgb;
	}


	s->StartAddress = new UCHAR[s->Offset * s->Rows];
	int shift3 = 0;
	for (uint y = 0; y < s->Rows; y++)
	{
		for (uint x = 0; x < s->Cols; x++)
		{
			int index = x / 100;
			s->StartAddress[shift3 + 0] = vectorClusters[index].RGB.R;
			s->StartAddress[shift3 + 1] = vectorClusters[index].RGB.G;
			s->StartAddress[shift3 + 2] = vectorClusters[index].RGB.B;
			shift3+=3;
		}
	}
	

	int nSavingIndex = attempt;

	if (bFinal)
	{
		nSavingIndex = 3;
		vectorClusters.resize(0);
	}
	char ColoursFullPath[MAX_PATH];
	sprintf_s(ColoursFullPath, "%sColours_%d.png", imagesPath, nSavingIndex);

	flushInfo(&s->info, "Colours_" + to_string(nSavingIndex));

	if (saveImages == true)
		DataAreaWrite(s, ColoursFullPath);

}


void CLWorker::drawColourLayers(int number)
{

	if (attempt == 1)
	{
		Data->DaColourLayers = new DataArea;
		*Data->DaColourLayers = DaEtalon;
		Data->DaColourLayers->StartAddress = new UCHAR[width * height * 3];

		Data->DaColourLayers->Plants = 3;
		Data->DaColourLayers->Offset = width * 3;
		Data->colourLayersIndex = new DataArea;
		*Data->colourLayersIndex = DaLexemes;
		Data->colourLayersIndex->StartAddress = new UCHAR[width * height];
		fill(Data->colourLayersIndex->StartAddress, Data->colourLayersIndex->StartAddress + width * height, FORBIDDEN);
		
	}

	int shift = 0;
	int shift3 = 0;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{

			if (Data->colourLayersIndex->StartAddress[shift] < FORBIDDEN)
			{
				shift++;
				shift3 += 3;
				continue;
			}

			bool found = false;

			if (DaLexemes.StartAddress[shift] != 0)
			{
				double minDistance = 10000;
				int minIndex = 0;
				for (int i = 0; i < Data->clustersCount; i++)
				{
					LAB_STRUCT tmp = { labBuffer[shift].L, labBuffer[shift].A, labBuffer[shift].B };

					if (isDotInEllips(Data->clusters[i].LAB, tmp, i, true))
					{
						const double distance = length(Data->clusters[i].LAB, tmp);

						if (minDistance > distance)
						{
							minDistance = distance;
							minIndex = i;
						}

						found = true;
					}

				}
				if (found == true)
				{
					if (!(Data->artefacts->StartAddress[shift] == 1))
					{
						Data->DaColourLayers->StartAddress[shift3 + 0] = Data->clusters[minIndex].RGB.R;
						Data->DaColourLayers->StartAddress[shift3 + 1] = Data->clusters[minIndex].RGB.G;
						Data->DaColourLayers->StartAddress[shift3 + 2] = Data->clusters[minIndex].RGB.B;
						Data->colourLayersIndex->StartAddress[shift] = minIndex;
					}
					else
						found = false;

				}
			}
			

			if (found == false)
			{
				Data->DaColourLayers->StartAddress[shift3 + 0] = 255;
				Data->DaColourLayers->StartAddress[shift3 + 1] = 0;
				Data->DaColourLayers->StartAddress[shift3 + 2] = 0;
				Data->colourLayersIndex->StartAddress[shift] = UNDEFINED;
				if (Data->artefacts->StartAddress[shift] == 1)
				{
					Data->DaColourLayers->StartAddress[shift3 + 0] = 0;
					Data->DaColourLayers->StartAddress[shift3 + 1] = 0;
					Data->DaColourLayers->StartAddress[shift3 + 2] = 255;
				}
			}

			if (DaLexemes.StartAddress[shift] == 0)
			{
				Data->DaColourLayers->StartAddress[shift3 + 0] = 0;
				Data->DaColourLayers->StartAddress[shift3 + 1] = 0;
				Data->DaColourLayers->StartAddress[shift3 + 2] = 0;
				Data->colourLayersIndex->StartAddress[shift] = FORBIDDEN;
			}

			shift++;
			shift3 += 3;
		}
	}

	char DaColourLayersSavingPath[MAX_PATH];
	sprintf_s(DaColourLayersSavingPath, MAX_PATH, "%sSampleCL_Algo_%d_%d.png", imagesPath, attempt, number);

	if (saveImages == true)
		DataAreaWrite(Data->DaColourLayers, DaColourLayersSavingPath);

	Data->DBGImages[number] = new DataArea();

	*Data->DBGImages[number] = *Data->DaColourLayers;
	Data->DBGImages[number]->StartAddress = new UCHAR[width * height * 3];
	memcpy(Data->DBGImages[number]->StartAddress, Data->DaColourLayers->StartAddress, width * height * 3);
	flushInfo(&Data->DBGImages[number]->info, "Algo_" + to_string(attempt) + "_" + to_string(number));

}

bool CLWorker::checkClustersCorrectness()
{
	if (vectorClusters.empty())
	{
		vectorClusters = clustersFirst;
		extendColourLayersNearestColour();
		return false;
	}
	return true;
}

void CLWorker::saveClustersInfo()
{

	clustersFirst.resize(Data->clustersCount);

	for (int i = 0; i < Data->clustersCount; i++)
	{
		clustersFirst[i] = Data->clusters[i];
	}

	clusterRadiusFirst = new SIX_RADIUS[Data->clustersCount];

	for (int i = 0; i < Data->clustersCount; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			clusterRadiusFirst[i][j] = Data->clusters[i].clusterRadius[j];
		}
	}
}


void CLWorker::combineClusters()
{
	auto tmp = vectorClusters;
	vectorClusters.resize(0);
	for (auto i : clustersFirst)
	{
		vectorClusters.push_back(i);
	}
	for (auto i : tmp)
	{
		vectorClusters.push_back(i);
	}

	firstStepClustersSize = clustersFirst.size();
}


void CLWorker::combineClustersInfo()
{
	combineClusters();

	clustersFirst.resize(0);

}

DataArea * CLWorker::getColourLayerDataArea()
{
	return Data->DaColourLayers;
}

CLWorker::~CLWorker()
{
	delete[] labBuffer;
	delete[] pLabTable;
	delete[] LAB_SPACE;
	delete[] LAB_SPACE_E;
	delete[] AB_SPACE;

	delete [] C;

	attempt = 0;
}

bool CLWorker::readLABTable(char * tablename)
{
	char fulltablename[MAX_PATH];
	_fullpath(fulltablename, tablename, _MAX_PATH);

	if (_access(fulltablename, 0))
		return false;
	
	FILE *f;
	fopen_s(&f, fulltablename, "rb");
	
	if (!f)
		return false;
	
	fseek(f, 0, SEEK_END);

	if (ftell(f) != sizeof(LAB_TABLE))
	{
		fclose(f);
		return false;
	}
	
	fseek(f, 0, SEEK_SET);
	
	pLabTable = new LAB_TABLE[1];
	
	fread(pLabTable, sizeof(LAB_TABLE), 1, f);
	
	fclose(f);
	return true;
}

LAB_STRUCT CLWorker::RGB2LAB(RGB_STRUCT c)
{
	LAB_STRUCT LAB;

	LAB = pLabTable[0][LAB_INDEX(c.R, c.G, c.B)];

	if (((c.R & 1) + (c.G & 1) + (c.B & 1) > 1) && (LAB.L < 255))
		LAB.L++;

	return LAB;
}

void CLWorker::RGB2LABEtalon()
{
	labBuffer = new LAB_STRUCT[width * height];
	
	int shift3 = 0;
	int shift = 0;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			labBuffer[shift] = RGB2LAB({DaEtalon.StartAddress[shift3 + 0], DaEtalon.StartAddress[shift3 + 1], DaEtalon.StartAddress[shift3 + 2] });
			shift3+=3;
			shift++;
		}
	}
}

RGB_STRUCT CLWorker::LAB2RGB_f(LAB_STRUCT_f lab_f)
{
	LAB_STRUCT lab;
	lab.L = static_cast<UCHAR>(lround(lab_f.L));
	lab.A = static_cast<UCHAR>(lround(lab_f.A));
	lab.B = static_cast<UCHAR>(lround(lab_f.B));
	return LAB2RGB(lab);
}

RGB_STRUCT CLWorker::LAB2RGB(LAB_STRUCT lab)
{
	RGB_STRUCT rgb;
	double dX, dY, dZ;
	double lr, lg, lb;
	LAB2XYZ(static_cast<double>(lab.L), static_cast<double>(lab.A), static_cast<double>(lab.B), &dX, &dY, &dZ);
	XYZ2RGB(dX, dY, dZ, &lr, &lg , &lb);
	rgb.R = lab.L = static_cast<UCHAR>(lround(255 * lr));
	rgb.G = lab.L = static_cast<UCHAR>(lround(255 * lg));
	rgb.B = lab.L = static_cast<UCHAR>(lround(255 * lb));

	return rgb;
}

void CLWorker::LAB2XYZ(double dL, double dA, double dB, double* pdX, double* pdY, double* pdZ) const
{

	dL *= 100.0 / 255.0;
	dA -= 128;
	dB -= 128;


	double fy, fx, fz;

	if (dL > 8.0)
		*pdY = pow((dL + 16) / 116.0, 3);
	else
		*pdY = dL / 903.3;

	fy = pow(*pdY, 0.3333333);

	fx = fy + dA / 500.0;
	if (fx > 0.206893)
		*pdX = 0.95047 * pow(fx, 3);
	else
		*pdX = 0.95047 * (fx - 16.0 / 116.0) / 7.787;

	fz = fy - dB / 200.0;
	if (fz > 0.206893)
		*pdZ = 1.08883 * pow(fz, 3);
	else
		*pdZ = 1.08883 * (fz - 16.0 / 116.0) / 7.787;
	
}

void CLWorker::XYZ2RGB(double dX, double dY, double dZ, double* pdR, double* pdG, double* pdB)
{

	double* pdRGB = nullptr;

	*pdR = dX * XYZ2SRGB_D65[0] + dY * XYZ2SRGB_D65[1] + dZ * XYZ2SRGB_D65[2];
	*pdG = dX * XYZ2SRGB_D65[3] + dY * XYZ2SRGB_D65[4] + dZ * XYZ2SRGB_D65[5];
	*pdB = dX * XYZ2SRGB_D65[6] + dY * XYZ2SRGB_D65[7] + dZ * XYZ2SRGB_D65[8];

	for (int i = 1; i <= 3; i++)
	{


		if (i == 1)pdRGB = pdR;
		if (i == 2)pdRGB = pdG;
		if (i == 3)pdRGB = pdB;

		if (*pdRGB < 0.0031308)
		{
			*pdRGB *= 12.92;
		}
		else
		{
			*pdRGB = 1.055 * pow(*pdRGB, 0.41666) - 0.055;
		}
	}

}

double CLWorker::XYZ2SRGB_D65[9] =
{
	3.2406, -1.5372,-0.4986,
	-0.9689, 1.8758, 0.0415,
	0.0557, -0.2040, 1.0570
};
