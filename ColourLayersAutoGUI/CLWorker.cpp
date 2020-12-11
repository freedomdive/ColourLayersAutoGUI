#include "pch.h"
#include "CLWorker.h"
#include <iostream>
#include <direct.h>

//Инициализация объекта worker
Result* CLCreateWorker(DataArea * DaEtalon, DataArea * DaLexemes, char * inputPath)
{
	auto worker = new CLWorker();

	char savingPath[260];
	char LABTable[260];


	uint k = 0;
	for (uint i = 0; i < strlen(inputPath); i++)
	{
		if (inputPath[i] != ';')
		{
			LABTable[k] = inputPath[i];
			k++;
		}
		else
			break;
	}

	LABTable[k] = 0;
	k++;
	for (uint i = k; i < strlen(inputPath); i++)
	{
		savingPath[i - k] = inputPath[i];
	}

	savingPath[strlen(inputPath) - k] = 0;

	worker->setImagesPath(savingPath);

	if (worker->setEtalonDataArea(DaEtalon) == 1)
	{
		return new Result("Etalon Plants != 3");
	}
	if (worker->setEtalonDataArea(DaEtalon) == 2)
	{
		return new Result("Etalon StartAddress is nullptr");
	}
	if (worker->setLexemesDataArea(DaLexemes) == 1)
	{
		return new Result("Lexems Plants != 1");
	}
	if (worker->setLexemesDataArea(DaLexemes) == 2)
	{
		return new Result("Lexems StartAddress is nullptr");
	}
	if (worker->checkCorrectness() == false)
	{
		return new Result("Forbidden Lexems and Etalon sizes");
	}

	if (worker->readLABTable(LABTable) == false)
	{
		return new Result("Failed to read LAB Table, no way to analyze without LAB");
	}

	worker->RGB2LABEtalon();

	return new Result(worker, sizeof * worker);
}

//Выполнение шагов алгоритма
Result * CLExecuteStep(void * pWorker, int stepNumber, CLParams & A, CLData * Data)
{
	if (stepNumber < 1 || stepNumber > STEP_COUNT)
	{
		//Сообщение об ошибке
		return new Result("Wrong stepNumber");
	}

	auto worker = static_cast<CLWorker *>(pWorker);

	//Установка параметров
	
	worker->SetInnerRadius(A.Radius);
	worker->SetOutterRadius(A.Radius2);
	worker->SetLCoef(A.L_Coef_SQUARE);
	worker->SetPCoef(A.pCoef);
	worker->SetStartingRadiusCoef(A.startingRadiusCoef);
	worker->SetLRadiusCoef(A.L_Radius_Coef);
	worker->SetLDirectionCoef(A.LDirectionCoef);
	worker->SetMxDivCoef(A.Mx_Div_Coef);
	worker->SetMMulCoef(A.M_Mul_Coef);
	worker->SetLMergeLimit(A.L_Merge_Limit);
	worker->SetABMergeLimit(A.AB_Merge_Limit);
	worker->SetABMergeCheckLimit(A.AB_Merge_Check_Limit);
	worker->SetLCoefClustersRadius(A.L_Coef_Clusters_Radius);
	worker->SetLCoefNearestDraw(A.L_Coef_Nearest_Draw);


	worker->SetTooLowPixelCount(A.tooLowPixelCount);
	worker->SetRedrawLimit(A.redrawLimit);
	worker->SetLCoefRedraw(A.L_Coef_Redraw);

	worker->SetAdditionalInfo(A.additionalInfo);
	worker->SetSaveImages(A.saveImages);
	worker->SetCorrectBackGroundPixels(A.correctBackGroundPixels);

	worker->SetPresetColours(A.pPresetColours, A.nPresetColoursCount);
	worker->SetForbiddenColours(A.pForbiddenColours, A.nForbiddenColoursCount);

	//Корректировка параметров
	worker->correctParamsValues();

	CLData * retData = new CLData();

	worker->Data = retData;
	worker->info = "";

	if (stepNumber == 1)
	{
		worker->attempt = 1;

		worker->firstStepClustersSize = 0;

		worker->clearArtefacts();

		worker->calcLABStatistics();

		worker->calcClusters();

		worker->printClustersInfo();


		worker->info += "Clusters size = " + to_string(worker->vectorClusters.size()) + "\n";

		worker->info += "1st step clusters:\n";

		worker->analyzeClusters(worker->calcStatistics());

		retData->imagesCount = 4 + worker->vectorClusters.size() * (worker->vectorClusters.size() - 1) / 2;
		retData->DBGImages = new DataArea *[retData->imagesCount];

		retData->DBGImages[0] = new DataArea();
		*retData->DBGImages[0] = *worker->DaClearArtefacts;
		retData->DBGImages[0]->StartAddress = new UCHAR[worker->width * worker->height * 3];
		memcpy(retData->DBGImages[0]->StartAddress, worker->DaClearArtefacts->StartAddress, worker->width * worker->height * 3);
		delete[] worker->DaClearArtefacts->StartAddress;
		delete worker->DaClearArtefacts;
		

		worker->flushInfo(&retData->DBGImages[0]->info, "artefacts");


		worker->drawFoundColours();
		worker->drawLABStatistics();
		worker->drawLABRadiuses();
		worker->drawHistoBwAllColours();

		retData->clustersCount = worker->vectorClusters.size();

		retData->clusters = new pixelInfo[retData->clustersCount];

		for (int i = 0; i < retData->clustersCount; i++)
		{
			retData->clusters[i] = worker->vectorClusters[i];
		}

		worker->vectorClusters.resize(0);

		worker->flushInfo();

		return new Result(retData, sizeof * retData);
	}
	if (stepNumber == 2)
	{
		worker->attempt = 1;

		if (Data != nullptr)
		{
			if (Data->clustersCount == 0)
			{
				return new Result("clustersCount is 0");
			}
			if (Data->clusters == nullptr)
			{
				return new Result("clusters is nullptr");
			}
			if (Data->artefacts == nullptr)
			{
				return new Result("artefacts is nullptr");
			}
			retData->clusters = new pixelInfo[Data->clustersCount];
			memcpy(retData->clusters, Data->clusters, Data->clustersCount * sizeof(pixelInfo));
			retData->clustersCount = Data->clustersCount;
			retData->artefacts = new DataArea();
			*retData->artefacts = *Data->artefacts;
			retData->artefacts->StartAddress = new UCHAR[worker->width * worker->height];
			memcpy(retData->artefacts->StartAddress, Data->artefacts->StartAddress, worker->width * worker->height);
		}
		else
		{
			return new Result("Input Data is nullptr");
		}
		
		retData->imagesCount = 4;
		retData->DBGImages = new DataArea *[retData->imagesCount];

		worker->drawColourLayers(0);

		worker->extendColourLayersWaveAlgorithm(1);
		worker->extendColourLayersWaveAlgorithm(2);
		worker->extendColourLayersWaveAlgorithm(3);

		worker->flushInfo();

		return new Result(retData, sizeof * retData);
	}

	if (stepNumber == 3)
	{
		worker->attempt = 2;

		if (Data != nullptr)
		{
			if (Data->clustersCount == 0)
			{
				return new Result("clustersCount is 0");
			}
			if (Data->clusters == nullptr)
			{
				return new Result("clusters is nullptr");
			}
			if (Data->artefacts == nullptr)
			{
				return new Result("artefacts is nullptr");
			}
			if (Data->colourLayersIndex == nullptr)
			{
				return new Result("colourLayersIndex is nullptr");
			}
			if (Data->DaColourLayers == nullptr)
			{
				return new Result("DaColourLayers is nullptr");
			}
			if (Data->DaColourLayers->StartAddress == nullptr)
			{
				return new Result("DaColourLayers StartAddress is nullptr");
			}

			retData->clusters = new pixelInfo[Data->clustersCount];
			memcpy(retData->clusters, Data->clusters, Data->clustersCount * sizeof(pixelInfo));
			retData->clustersCount = Data->clustersCount;
			retData->DaColourLayers = new DataArea();
			*retData->DaColourLayers = *Data->DaColourLayers;
			retData->DaColourLayers->StartAddress = new UCHAR[worker->width * worker->height * 3];
			memcpy(retData->DaColourLayers->StartAddress, Data->DaColourLayers->StartAddress, worker->width * worker->height * 3 * sizeof(UCHAR));
			retData->colourLayersIndex = new DataArea();
			*retData->colourLayersIndex = worker->DaLexemes;
			retData->colourLayersIndex->StartAddress = new UCHAR[worker->width * worker->height];
			memcpy(retData->colourLayersIndex->StartAddress, Data->colourLayersIndex->StartAddress, worker->width * worker->height * sizeof(UCHAR));
			retData->artefacts = new DataArea();
			*retData->artefacts = *Data->artefacts;
			retData->artefacts->StartAddress = new UCHAR[worker->width * worker->height];
			memcpy(retData->artefacts->StartAddress, Data->artefacts->StartAddress, worker->width * worker->height);
		}
		else
		{
			return new Result("Input Data is nullptr");
		}

		worker->calcLABStatistics();

		worker->saveClustersInfo();

		worker->calcClusters();

		if (worker->checkClustersCorrectness() == false)
		{
			worker->flushInfo();
			
			retData->imagesCount = 0;
			
			return new Result(retData, sizeof * retData);
			//return new Result("Empty clusters, nothing to analyze");
		}
		
		worker->info += "2nd step clusters:\n";

		worker->analyzeClusters(worker->calcStatistics());
		
		retData->imagesCount = worker->vectorClusters.size() ? 3 + worker->vectorClusters.size() * (worker->vectorClusters.size() - 1) / 2 : 0;
		retData->DBGImages = new DataArea *[retData->imagesCount];

		worker->drawFoundColours();
		worker->drawLABStatistics();
		worker->drawLABRadiuses();
		worker->drawHistoBwAllColours();

		worker->combineClustersInfo();

		retData->clustersCount = worker->vectorClusters.size();

		retData->clusters = new pixelInfo[retData->clustersCount];

		for (int i = 0; i < retData->clustersCount; i++)
		{
			retData->clusters[i] = worker->vectorClusters[i];
		}

		worker->vectorClusters.resize(0);

		worker->flushInfo();

		return new Result(retData, sizeof * retData);
	}
	if (stepNumber == 4)
	{
		worker->attempt = 2;

		if (Data != nullptr)
		{
			if (Data->clustersCount == 0)
			{
				return new Result("clustersCount is 0");
			}
			if (Data->clusters == nullptr)
			{
				return new Result("clusters is nullptr");
			}
			if (Data->artefacts == nullptr)
			{
				return new Result("artefacts is nullptr");
			}
			if (Data->colourLayersIndex == nullptr)
			{
				return new Result("colourLayersIndex is nullptr");
			}
			if (Data->DaColourLayers == nullptr)
			{
				return new Result("DaColourLayers is nullptr");
			}
			if (Data->DaColourLayers->StartAddress == nullptr)
			{
				return new Result("DaColourLayers StartAddress is nullptr");
			}
			retData->clusters = new pixelInfo[Data->clustersCount];
			memcpy(retData->clusters, Data->clusters, Data->clustersCount * sizeof(pixelInfo));
			retData->clustersCount = Data->clustersCount;
			retData->DaColourLayers = new DataArea();
			*retData->DaColourLayers = *Data->DaColourLayers;
			retData->DaColourLayers->StartAddress = new UCHAR[worker->width * worker->height * 3];
			memcpy(retData->DaColourLayers->StartAddress, Data->DaColourLayers->StartAddress, worker->width * worker->height * 3 * sizeof(UCHAR));
			retData->colourLayersIndex = new DataArea();
			*retData->colourLayersIndex = worker->DaLexemes;
			retData->colourLayersIndex->StartAddress = new UCHAR[worker->width * worker->height];
			memcpy(retData->colourLayersIndex->StartAddress, Data->colourLayersIndex->StartAddress, worker->width * worker->height * sizeof(UCHAR));
			retData->artefacts = new DataArea();
			*retData->artefacts = *Data->artefacts;
			retData->artefacts->StartAddress = new UCHAR[worker->width * worker->height];
			memcpy(retData->artefacts->StartAddress, Data->artefacts->StartAddress, worker->width * worker->height);
		}
		else
		{
			return new Result("Input Data is nullptr");
		}

		retData->imagesCount = 7;
		retData->DBGImages = new DataArea *[retData->imagesCount];

		worker->drawColourLayers(0);

		worker->extendColourLayersWaveAlgorithm(1);
		worker->extendColourLayersWaveAlgorithm(2);

		for (int i = 0; i < worker->Data->clustersCount; i++)
		{
			for (int j = 0; j < 6; j++)
			{
				worker->Data->clusters[i].clusterRadius[j] *= 6;
			}
		}


		worker->drawColourLayers(3);

		worker->extendColourLayersWaveAlgorithm(4);
		worker->extendColourLayersWaveAlgorithm(5);

		worker->extendColourLayersNearestColour();

		worker->flushInfo();

		return new Result(retData, sizeof * retData);
	}
	if (stepNumber == 5)
	{
		worker->attempt = 2;

		if (Data != nullptr)
		{
			if (Data->clustersCount == 0)
			{
				return new Result("clustersCount is 0");
			}
			if (Data->clusters == nullptr)
			{
				return new Result("clusters is nullptr");
			}
			if (Data->artefacts == nullptr)
			{
				return new Result("artefacts is nullptr");
			}
			if (Data->colourLayersIndex == nullptr)
			{
				return new Result("colourLayersIndex is nullptr");
			}
			if (Data->DaColourLayers == nullptr)
			{
				return new Result("DaColourLayers is nullptr");
			}
			if (Data->DaColourLayers->StartAddress == nullptr)
			{
				return new Result("DaColourLayers StartAddress is nullptr");
			}
			retData->clusters = new pixelInfo[Data->clustersCount];
			memcpy(retData->clusters, Data->clusters, Data->clustersCount * sizeof(pixelInfo));
			retData->clustersCount = Data->clustersCount;
			retData->DaColourLayers = new DataArea();
			*retData->DaColourLayers = *Data->DaColourLayers;
			retData->DaColourLayers->StartAddress = new UCHAR[worker->width * worker->height * 3];
			memcpy(retData->DaColourLayers->StartAddress, Data->DaColourLayers->StartAddress, worker->width * worker->height * 3 * sizeof(UCHAR));
			retData->colourLayersIndex = new DataArea();
			*retData->colourLayersIndex = worker->DaLexemes;
			retData->colourLayersIndex->StartAddress = new UCHAR[worker->width * worker->height];
			memcpy(retData->colourLayersIndex->StartAddress, Data->colourLayersIndex->StartAddress, worker->width * worker->height * sizeof(UCHAR));
			retData->artefacts = new DataArea();
			*retData->artefacts = *Data->artefacts;
			retData->artefacts->StartAddress = new UCHAR[worker->width * worker->height];
			memcpy(retData->artefacts->StartAddress, Data->artefacts->StartAddress, worker->width * worker->height);
		}
		else
		{
			return new Result("Input Data is nullptr");
		}

		retData->imagesCount = 5;
		retData->DBGImages = new DataArea *[retData->imagesCount];

		retData->DBGImages[0] = nullptr;
		retData->DBGImages[1] = nullptr;

		if (worker->redrawLimit > 0)
			worker->correctColourLayers();

		worker->artefactsCheck();

		worker->formIndexedColourLayers();

		worker->drawFoundColours(true);

		char ColoursFullPath[MAX_PATH];
		sprintf_s(ColoursFullPath, "%sSampleCL_Algo_4.png", worker->imagesPath);

		//if (worker->saveImages == true)
			//DataAreaWrite(worker->Data->DaColourLayers, ColoursFullPath);

		retData->DBGImages[2] = new DataArea();

		*retData->DBGImages[2] = *worker->Data->DaColourLayers;

		retData->DBGImages[2]->StartAddress = new UCHAR[worker->width * worker->height * 3];
		
		memcpy(retData->DBGImages[2]->StartAddress, worker->Data->DaColourLayers->StartAddress, worker->width * worker->height * 3);

		sprintf_s(ColoursFullPath, "%sSampleCL_Algo_3_4.png", worker->imagesPath);
		if (worker->saveImages == true)
			DataAreaWrite(worker->Data->DaColourLayers, ColoursFullPath);
		
		worker->flushInfo(&retData->DBGImages[2]->info, "Algo_3_4");

		worker->flushInfo();

		
		return new Result(retData, sizeof * retData);
	}

	return new Result("Unexpected exception, probably something with Instruction Poiner");
}


void CLDeleteWorker(void* pWorker)
{
	delete static_cast<CLWorker *>(pWorker);
}

void CLDataFree(Result * pData)
{
	CLData * Data = static_cast<CLData *>(pData->data);
	if (Data != nullptr)
	{
		delete[] Data->clusters;
		delete[] Data->info;
		Data->clusters = nullptr;
		Data->info = nullptr;
		if (Data->artefacts != nullptr)
		{
			delete[] Data->artefacts->StartAddress;
			Data->artefacts->StartAddress = nullptr;
		}
		delete Data->artefacts;
		Data->artefacts = nullptr;
		if (Data->colourLayersIndex != nullptr)
		{
			delete[] Data->colourLayersIndex->StartAddress;
			Data->colourLayersIndex->StartAddress = nullptr;
		}
		delete Data->colourLayersIndex;
		Data->colourLayersIndex = nullptr;
		if (Data->DaColourLayers != nullptr)
		{
			delete[] Data->DaColourLayers->StartAddress;
			Data->DaColourLayers->StartAddress = nullptr;
		}
		delete Data->DaColourLayers;
		Data->DaColourLayers = nullptr;
		if (Data->DBGImages != nullptr)
		{
			for (int i = 0; i < Data->imagesCount; i++)
			{
				if (Data->DBGImages[i] != nullptr)
				{
					delete[] Data->DBGImages[i]->StartAddress;
					Data->DBGImages[i]->StartAddress = nullptr;
				}
				delete Data->DBGImages[i];
				Data->DBGImages[i] = nullptr;
			}
		}
		delete[] Data->DBGImages;
		Data->DBGImages = nullptr;
	}
	delete Data; 
	Data = nullptr;
	delete pData;
}
