#pragma once
#include <vector>
#include "DaRW.h"
#include <queue>
#include <string>
using namespace std;

#define MAX_PATH 260

#define LAB_INDEX(r, g, b) (((((int)(r))>>1)<<14) + ((((int)(g))>>1)<<7) + (((int)(b))>>1))

#define uint unsigned int

#define _3D (1 << 24)
#define _2D (1 << 16)
#define _1D (1 << 8)



typedef int MATRIX[_1D][_1D];

typedef double SIX_RADIUS[6];

typedef struct
{
	UCHAR R;
	UCHAR G;
	UCHAR B;
} RGB_STRUCT, *PRGB_STRUCT;

typedef struct
{
	UCHAR L;
	UCHAR A;
	UCHAR B;
} LAB_STRUCT, *PLAB_STRUCT;

typedef struct
{
	float L;
	float A;
	float B;
} LAB_STRUCT_f, *PLAB_STRUCT_f;


typedef LAB_STRUCT LAB_TABLE[(1 << 7) * (1 << 7) * (1 << 7)];

typedef struct
{
	LAB_STRUCT_f LAB;
	RGB_STRUCT RGB;
	SIX_RADIUS clusterRadius;
	int count;

} pixelInfo;

struct DataArea
{
	unsigned char* StartAddress;	// Указатель на область памяти с пикселями
	unsigned Cols;					// Количество пикселей в строке
	unsigned Rows;					// Количество пикселей по вертикали
	unsigned Offset;				// Длина строки (c учетом плоскостей) в байтах
	unsigned short Plants;			// Количество плоскостей

	char * info;
};

int DataAreaWrite(DataArea * pDa, char *pszFileName);

//Класс, содержащий результаты расчёта на каждом шаге
struct CLData
{
	pixelInfo * clusters;
	int clustersCount;
	DataArea * colourLayersIndex;
	DataArea * DaColourLayers;
	DataArea * artefacts;

	DataArea ** DBGImages;
	int imagesCount;
	char * info;

	CLData()
	{
		clusters = nullptr;
		clustersCount = 0;
		colourLayersIndex = nullptr;
		DaColourLayers = nullptr;
		artefacts = nullptr;

		DBGImages = nullptr;
		imagesCount = 0;
		info = nullptr;
	}
};

//Внутренний класс для объекта worker. Содержится в его data
class CLWorker
{
public:
	CLWorker();
	~CLWorker();

	void setImagesPath(char *);

	//Установка эталона
	int setEtalonDataArea(DataArea * p);
	//Установка лексем
	int setLexemesDataArea(DataArea * p);
	//Проверка согласованности эталона и лексем
	bool checkCorrectness() const;

	DataArea * getColourLayerDataArea();

	//Считывание таблицы RGB->LAB, чтобы не вычислять эту функцию самостоятельно
	bool readLABTable(char * tablename);


	//Установка обязательных цветов
	void SetPresetColours(PRGB_STRUCT Colours, int count);

	//Установка запретных цветов
	void SetForbiddenColours(PRGB_STRUCT Colours, int count);
	

	//Интерфейс для изменения параметров:

	//Основные параметры (на мой субъективный взгляд ими можно обойтись)
	void SetLABMergeLimit(double LAB_Merge_Limit) { this->L_Merge_Limit = LAB_Merge_Limit; this->AB_Merge_Limit = LAB_Merge_Limit; }
	void SetTooLowPixelCount(int tooLowPixelCount) { this->tooLowPixelCount = tooLowPixelCount; }
	void SetRedrawLimit(double redrawLimit) { this->redrawLimit = redrawLimit; }
	void SetLCoefRedraw(double L_Coef_Redraw) { this->L_Coef_Redraw = L_Coef_Redraw; }

	//Дополнительные параметры
	void SetInnerRadius(int Radius) { this->Radius = Radius; this->Square = (2 * Radius + 1) * (2 * Radius + 1); this->RadiusE = Radius + 1; this->SquareE = (2 * this->RadiusE + 1) * (this->RadiusE + 1); this->Radius2E = this->RadiusE + 1; this->Square2E = (2 * this->Radius2E + 1) * (this->Radius2E + 1); }
	void SetOutterRadius(int Radius) { this->Radius2 = Radius; Square2 = (2 * Radius + 1) * (2 * Radius + 1); }
	void SetLCoef(double L_Coef) { this->L_Coef_SQUARE = L_Coef; }
	void SetPCoef(double pCoef) { this->pCoef = pCoef; }
	void SetStartingRadiusCoef(double startingRadiusCoef) { this->startingRadiusCoef = startingRadiusCoef; }
	void SetLRadiusCoef(double L_Radius_Coef) { this->L_Radius_Coef = L_Radius_Coef; }
	void SetLDirectionCoef(double LDirectionCoef) { this->LDirectionCoef = LDirectionCoef; }
	void SetMxDivCoef(double Mx_Div_Coef) { this->Mx_Div_Coef = Mx_Div_Coef; }
	void SetMMulCoef(double M_Mul_Coef) { this->M_Mul_Coef = M_Mul_Coef; }
	void SetLMergeLimit(double L_Merge_Limit) { this->L_Merge_Limit = L_Merge_Limit; }
	void SetABMergeLimit(double AB_Merge_Limit) { this->AB_Merge_Limit = AB_Merge_Limit; }
	void SetABMergeCheckLimit(double AB_Merge_Check_Limit) { this->AB_Merge_Check_Limit = AB_Merge_Check_Limit; }
	void SetLCoefClustersRadius(double L_Coef_Clusters_Radius) { this->L_Coef_Clusters_Radius = L_Coef_Clusters_Radius; }
	void SetLCoefNearestDraw(double L_Coef_Nearest_Draw) { this->L_Coef_Nearest_Draw = L_Coef_Nearest_Draw; }
	void SetAdditionalInfo(bool additionalInfo) { this->additionalInfo = additionalInfo; }
	void SetSaveImages(bool saveImages) { this->saveImages = saveImages; }
	void SetCorrectBackGroundPixels(bool correctBackGroundPixels) { this->correctBackGroundPixels = correctBackGroundPixels;}

//private:
	//Расстояния между точкам в пространстве LAB
	double length(LAB_STRUCT_f x1, LAB_STRUCT_f x2);
	double length(LAB_STRUCT_f x1, LAB_STRUCT x2);
	double length(LAB_STRUCT x1, LAB_STRUCT x2);

	//Переводы RGB <-> LAB
	LAB_STRUCT RGB2LAB(RGB_STRUCT c);
	RGB_STRUCT LAB2RGB(LAB_STRUCT lab);
	RGB_STRUCT LAB2RGB_f(LAB_STRUCT_f lab);

	//Перевод LAB -> XYZ
	void LAB2XYZ(double dL, double dA, double dB, double* pdX, double* pdY, double* pdZ) const;
	//Перевод XYZ -> RGB
	void XYZ2RGB(double dX, double dY, double dZ, double* pdR, double* pdG, double* pdB);
	
	//Перевод всех точек эталона из RGB в LAB (В labBuffer)
	void RGB2LABEtalon();
	
	//Закраска точек эталона, которые находятся в эллипсоидах у clusters относительно радиусов clusterRadius
	void drawColourLayers(int number);

	//Отправка неопределившихся пикселей в ближайший опорный цвет
	void extendColourLayersNearestColour();

	//Закраски соседних пикселей
	void extendColourLayersWaveAlgorithm(int number);

	//Перекраска на 5 шаге
	void correctColourLayers();

	//Внутренний функционал перекраски компонент пикселей
	void analyzeSquare(int foreignX, int foreignY, int InnerWidth, int OutterWidth);

	//Определение индекса ближайшего опорного цвета
	int nearestColour(LAB_STRUCT colour);
	int nearestColour(LAB_STRUCT_f colour);

	//Длины между цветами с учётом коррекции по L
	double lengthCoef(double coef, LAB_STRUCT_f X, LAB_STRUCT_f Y);
	double lengthCoef(double coef, LAB_STRUCT X, LAB_STRUCT Y);
	double lengthCoef(double coef, LAB_STRUCT_f X, LAB_STRUCT Y);

	//Выдача инфорации по кластерам
	void printClustersInfo();

	//Формирование итогового изображения цветовых слоёв в нужном формате
	int formIndexedColourLayers();

	//Подсчёт частот пространств LAB и AB для более удобной индексации
	void calcLABStatistics();

	//Формирование первичных опорных цветов
	void calcClusters();

	double calcStatistics();

	double getDoubleLABValue(double dL, double dA, double dB);

	//Формирование гистограмм между цветами
	void drawHistoBwAllColours();

	void drawHistoBwColours(int i, int j);

	//Формирование вспомогательной области артефактов
	void clearArtefacts();

	//Вспомогательная область для преобразований
	static double XYZ2SRGB_D65[9];

	//Формирование вспомогательных изображений с найденными цветами
	void drawFoundColours(bool bFinal = false);

	//Формирование вспомогательных изображений с найденными кластерами
	void drawLABStatistics();

	//Формирование вспомогательных изображений с найденными кластерами с подсветкой цветов кластеров
	void drawLABRadiuses();

	void calcSumMatrix();

	template <typename T>
	void setParamValue(T * param, double left, double right);

	//Корректировка значений параметров
	void correctParamsValues();

	void artefactsCheck();

	//Сохранение кластеров, найденных на первом прогоне алгоритма
	void saveClustersInfo();

	//Проверка найденных кластеров на пустоту. Если на втором шаге кластеров нет, то принудительно закрасить в цвета из первого шага
	bool checkClustersCorrectness();

	//Объединение кластеров из первого и второго шага
	void combineClusters();

	//Объединение кластеров из первого и второго шага
	void combineClustersInfo();

	int sumMatrix(UCHAR L, UCHAR A, UCHAR B) const;
	int sumMatrixE(UCHAR L, UCHAR A, UCHAR B) const;
 	int sumMatrix2(UCHAR L, UCHAR A, UCHAR B) const;
	int sumMatrix2E(UCHAR L, UCHAR A, UCHAR B) const;

	
	//Попытка объединения кластеров
	int correctClustersRadius();

	//Бинарный поиск точки между кластерами с указанными линейными радиусами
	LAB_STRUCT_f findDot(double radX, double radY, double radZ, LAB_STRUCT_f Xi, LAB_STRUCT_f Xj);

	//Попала ли точка в эллипс
	bool isDotInEllips(LAB_STRUCT_f X, LAB_STRUCT_f Y, int clusterIndex, bool L);
	bool isDotInEllips(LAB_STRUCT_f X, LAB_STRUCT Y, int clusterIndex, bool L);

	//Проверка невыхода сдвига за границы зоны
	bool isValid(int shift, int leftX = 0, int rightX = 0, int leftY = 0, int rightY = 0);

	void putPixel(int shift3, int index);

	//Основной анализ кластеров
	void analyzeClusters(double vectorAreaLength);

	//Слияние кластеров
	void mergeClusters(uint i, uint j);

	char imagesPath[MAX_PATH];

	LAB_TABLE * pLabTable;

	DataArea DaEtalon;
	DataArea DaLexemes;
	

	DataArea DaColourLayersWithRedraw;

	DataArea * DaClearArtefacts;

	PLAB_STRUCT labBuffer;

	int width;
	int height;

	int *LAB_SPACE;
	int *AB_SPACE;

	int *LAB_SPACE_E;

	vector<pixelInfo> vectorClusters;

	MATRIX * C;
	

	bool * cclMask;

	int attempt;

	bool bNearestOrMiddle;

	int firstStepClustersSize;

	pixelInfo highestCluster;

	vector<pixelInfo> clustersFirst;


	SIX_RADIUS * clusterRadiusFirst;

	void flushInfo();
	void flushInfo(char ** pszInfo, string stringInfo);

	PLAB_STRUCT_f PresetColours;
	uint PresetColoursCount;
	PLAB_STRUCT_f ForbiddenColours;
	uint ForbiddenColoursCount;

	//Параметры:
	int Radius;
	int Radius2;
	int Square;
	int Square2;

	int RadiusE;
	int Radius2E;
	int SquareE;
	int Square2E;

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
	double AB_Merge_Check_Limit;
	double L_Coef_Clusters_Radius;
	double L_Coef_Nearest_Draw;
	double redrawLimit;
	double L_Coef_Redraw;
	bool additionalInfo;
	bool saveImages;
	bool correctBackGroundPixels;

	CLData * Data;
	string info;
};
