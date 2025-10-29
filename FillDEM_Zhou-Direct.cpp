#include <iostream>
#include <string>
#include <fstream>
#include <queue>
#include <algorithm>
#include "dem.h"
#include "Node.h"
#include "utils.h"
#include <time.h>
#include <list>
#include <stack>
#include <unordered_map>
using namespace std;

//�������������ͱ�����NodeVector �� Node �����������PriorityQueue ��һ�����ȶ��У�
//ʹ�� Node ���󲢰� Node::Greater ���򣨿����ǰ��߳�ֵ���򣩡�
typedef std::vector<Node> NodeVector;
typedef std::priority_queue<Node, NodeVector, Node::Greater> PriorityQueue;
// ��ʼ�����ȼ����еĺ�����
// ������һ��DEM����һ����־�����������У�׷�ٶ��к����ȶ��У��Լ�һ�����ڽ��ȼ���Ĳ�����
void InitPriorityQue_Direct(CDEM& dem, Flag& flag, queue<Node>& traceQueue, PriorityQueue& priorityQueue, int& percentFive)
{
	//��ȡDEM�Ŀ�Ⱥ͸߶ȣ���ʼ����ЧԪ�ؼ���������ʱ�ڵ������
	int width = dem.Get_NX();
	int height = dem.Get_NY();
	int validElementsCount = 0;
	Node tmpNode;
	int iRow, iCol;
	//push border cells into the PQ
	//����DEM��ÿ����Ԫ������Ƿ��Ƿ����ݵ�Ԫ����NoData����Ȼ�����8���ھӽ��д���
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			if (!dem.is_NoData(row, col))
			{
				validElementsCount++;
				for (int i = 0; i < 8; i++)
				{
					iRow = Get_rowTo(i, row);
					iCol = Get_colTo(i, col);
					//����ھ��Ǳ߽��������ݵ�Ԫ���򽫵�ǰ��Ԫ����Ϊ�߽絥Ԫ���������ȶ��С�
					if (!dem.is_InGrid(iRow, iCol) || dem.is_NoData(iRow, iCol))
					{
						tmpNode.col = col;
						tmpNode.row = row;
						tmpNode.spill = dem.asFloat(row, col);
						priorityQueue.push(tmpNode);

						flag.SetFlag(row, col);
						break;
					}
				}
			}
			else {
				flag.SetFlag(row, col);
			}
		}
	}
	//�������ڽ��ȸ��µ���ֵ
	percentFive = validElementsCount / 20;
}
//����׷�ٶ����еĽڵ㣬���������ȶ��кͼ�������
void ProcessTraceQue_Direct(CDEM& dem, Flag& flag, queue<Node>& traceQueue, PriorityQueue& priorityQueue, int& count, int percentFive)
{
	int iRow, iCol, i;
	float iSpill;//���ڻ�ȡָ������λ�õĸ߳�ֵ
	Node N, node, headNode;
	int width = dem.Get_NX();
	int height = dem.Get_NY();
	int total = 0, nPSC = 0;//��ʼ���ܴ���ڵ��������ȶ����������Ľڵ���
	bool bInPQ = false;//��ǵ�ǰ�ڵ��Ƿ��ѱ���ӵ����ȼ�����
	while (!traceQueue.empty())
	{
		node = traceQueue.front();
		traceQueue.pop();
		total++;
		if ((count + total) % percentFive == 0)
		{
			std::cout << "Progress" << (count + total) / percentFive * 5 << "%\r";
		}
		bInPQ = false;
		for (i = 0; i < 8; i++)
		{
			iRow = Get_rowTo(i, node.row);// ���ݵ�ǰ����ͽڵ�λ�ü���Ŀ����
			iCol = Get_colTo(i, node.col);// ���ݵ�ǰ����ͽڵ�λ�ü���Ŀ����
			// ���Ŀ��λ���Ѿ��������������
			if (flag.IsProcessedDirect(iRow, iCol)) continue;

			//���ڻ�ȡָ������λ�õĸ߳�ֵ
			iSpill = dem.asFloat(iRow, iCol);

			if (iSpill <= node.spill) {
				if (!bInPQ) {
					// make sure that node is pushed into PQ only once
					priorityQueue.push(node);
					bInPQ = true;
					nPSC++;
				}
				continue;
			}
			//otherwise
			//N is unprocessed and N is higher than C
			// �������Ŀ��λ�õ�����߶ȴ��ڵ�ǰ�ڵ������߶ȣ���  
			// ��Ŀ��λ����ӵ�׷�ٶ��У������Ϊ�Ѵ���
			N.col = iCol;
			N.row = iRow;
			N.spill = iSpill;
			traceQueue.push(N);
			flag.SetFlag(iRow, iCol);
		}
	}
	count += total - nPSC;
}

void ProcessPit_Direct(CDEM& dem, Flag& flag, queue<Node>& depressionQue, queue<Node>& traceQueue, PriorityQueue& priorityQueue, int& count, int percentFive)
{
	int iRow, iCol, i;
	float iSpill;
	Node N;
	Node node;
	int width = dem.Get_NX();
	int height = dem.Get_NY();
	while (!depressionQue.empty())
	{
		node = depressionQue.front();
		depressionQue.pop();
		count++;
		if (count % percentFive == 0)
		{
			std::cout << "Progress:" << count / percentFive * 5 << "%\r";
		}
		for (i = 0; i < 8; i++)
		{
			iRow = Get_rowTo(i, node.row);
			iCol = Get_colTo(i, node.col);

			if (flag.IsProcessedDirect(iRow, iCol)) continue;
			iSpill = dem.asFloat(iRow, iCol);
			if (iSpill > node.spill)
			{
				//slope cell
				N.row = iRow;
				N.col = iCol;
				N.spill = iSpill;
				flag.SetFlag(iRow, iCol);
				// ���µؽڵ���ӵ�׷�ٶ�����
				traceQueue.push(N);
				continue;
			}

			//depression cell
			// ���Ŀ��λ�õĸ߳�ֵС�ڻ���ڵ�ǰ�ڵ�ĸ߳�ֵ��˵���ǰ��ݻ�ƽ��  
			// ���Ŀ��λ��Ϊ�Ѵ���
			flag.SetFlag(iRow, iCol);
			dem.Set_Value(iRow, iCol, node.spill);
			N.row = iRow;
			N.col = iCol;
			N.spill = node.spill;
			// ������Ľڵ㣨������߳��Ѹı䣩������ӵ����ݶ���
			depressionQue.push(N);
		}
	}
}
//�����������ڶ�ȡDEM�ļ�������ݵأ�����������
void FillDEM_Zhou_Direct(const char* inputFile, const char* outputFilledPath)
{
	queue<Node> traceQueue;
	queue<Node> depressionQue;

	//read float-type DEM
	CDEM dem;
	double geoTransformArgs[6];
	std::cout << "Reading tiff files..." << endl;
	//��ȡGeoTIFF��ʽ��DEM�ļ�
	if (!readTIFF(inputFile, GDALDataType::GDT_Float32, dem, geoTransformArgs))
	{
		printf("Error occurred while reading GeoTIFF file!\n");
		return;
	}

	std::cout << "Finish reading DEM file." << endl;

	time_t timeStart, timeEnd;
	int width = dem.Get_NX();
	int height = dem.Get_NY();

	timeStart = time(NULL);
	std::cout << "Using the direction implementation of the proposed variant to fill DEM" << endl;

	Flag flag;
	if (!flag.Init(width, height)) {
		printf("Failed to allocate memory!\n");
		return;
	}

	PriorityQueue priorityQueue;
	int percentFive;
	int count = 0, potentialSpillCount = 0;
	int iRow, iCol, row, col;
	float iSpill, spill;

	//��ʼ�����ȶ���
	InitPriorityQue_Direct(dem, flag, traceQueue, priorityQueue, percentFive);
	while (!priorityQueue.empty())
	{
		Node tmpNode = priorityQueue.top();
		priorityQueue.pop();
		count++;
		if (count % percentFive == 0)
		{
			std::cout << "Progress:" << count / percentFive * 5 << "%\r";
		}
		row = tmpNode.row;
		col = tmpNode.col;
		spill = tmpNode.spill;
		for (int i = 0; i < 8; i++)
		{

			iRow = Get_rowTo(i, row);
			iCol = Get_colTo(i, col);

			if (flag.IsProcessed(iRow, iCol)) continue;

			iSpill = dem.asFloat(iRow, iCol);
			if (iSpill <= spill)
			{
				//depression cell
				dem.Set_Value(iRow, iCol, spill);
				flag.SetFlag(iRow, iCol);
				tmpNode.row = iRow;
				tmpNode.col = iCol;
				tmpNode.spill = spill;
				depressionQue.push(tmpNode);
				ProcessPit_Direct(dem, flag, depressionQue, traceQueue, priorityQueue, count, percentFive);
			}
			else
			{
				//slope cell
				flag.SetFlag(iRow, iCol);
				tmpNode.row = iRow;
				tmpNode.col = iCol;
				tmpNode.spill = iSpill;
				traceQueue.push(tmpNode);
			}
			ProcessTraceQue_Direct(dem, flag, traceQueue, priorityQueue, count, percentFive);
		}
	}
	timeEnd = time(NULL);
	double consumeTime = difftime(timeEnd, timeStart);
	std::cout << "Time used:" << consumeTime << " seconds" << endl;
	double min, max, mean, stdDev;
	//����DEM��ͳ����Ϣ����Сֵ�����ֵ��ƽ��ֵ����׼�
	calculateStatistics(dem, &min, &max, &mean, &stdDev);

	//��������DEM���ݱ���ΪGeoTIFF��ʽ���ļ���
	CreateGeoTIFF(outputFilledPath, dem.Get_NY(), dem.Get_NX(),
		(void*)dem.getDEMdata(), GDALDataType::GDT_Float32, geoTransformArgs,
		&min, &max, &mean, &stdDev, -9999);
	return;
}
