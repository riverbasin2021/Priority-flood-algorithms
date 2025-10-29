#include <iostream> // ������׼�����������  
#include <string> // �����ַ������  
#include <fstream> // �����ļ������������  
#include <queue> // ��������������  
#include <algorithm> // �����㷨��  
#include "dem.h" // ����DEM������ص�ͷ�ļ�  
#include "Node.h" // �����ڵ����ͷ�ļ�  
#include "utils.h" // �������ߺ�����ͷ�ļ�  
#include <time.h> // ����ʱ�䴦���ͷ�ļ�  
#include <list> // ����˫�����������⣨��Ȼ����δ�����δֱ��ʹ�ã�  
#include <stack> // ����ջ�����⣨��Ȼ����δ�����δֱ��ʹ�ã�  
#include <unordered_map> // ��������ӳ�������⣨��Ȼ����δ�����δֱ��ʹ�ã�  

using namespace std; // ʹ�ñ�׼�����ռ�  

// ����Node����������  
typedef std::vector<Node> NodeVector;
// �������ȼ����У�ʹ��Node��ΪԪ�أ�NodeVector��Ϊ�ײ�������Node::Greater��Ϊ�ȽϺ���  
typedef std::priority_queue<Node, NodeVector, Node::Greater> PriorityQueue;

// ��ʼ�����ȼ����У������߽絥Ԫ��������  
void InitPriorityQue_onepass(CDEM& dem, Flag& flag, queue<Node>& traceQueue, PriorityQueue& priorityQueue, int& percentFive)
{
    // ��ȡDEM�Ŀ�Ⱥ͸߶�  
    int width = dem.Get_NX();
    int height = dem.Get_NY();
    int validElementsCount = 0; // ��ЧԪ�ؼ���  
    Node tmpNode; // ��ʱ�ڵ�  
    int iRow, iCol; // ѭ����������ʾ�к���  
    // ����DEM��ÿ����Ԫ��  
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            // �����ǰ��Ԫ����NoData  
            if (!dem.is_NoData(row, col))
            {
                validElementsCount++; // ��ЧԪ�ؼ�����һ  
                // ������ǰ��Ԫ���8���ھ�  
                for (int i = 0; i < 8; i++)
                {
                    // �����ھӵ��к���  
                    iRow = Get_rowTo(i, row);
                    iCol = Get_colTo(i, col);
                    // ����ھӲ��������ڻ���NoData  
                    if (!dem.is_InGrid(iRow, iCol) || dem.is_NoData(iRow, iCol))
                    {
                        // ����ǰ��Ԫ��������ȼ�����  
                        tmpNode.col = col;
                        tmpNode.row = row;
                        tmpNode.spill = dem.asFloat(row, col);
                        priorityQueue.push(tmpNode);

                        // ��ǵ�ǰ��Ԫ���Ѵ���  
                        flag.SetFlag(row, col);
                        break; // ֻ�����һ�������������ھ�  
                    }
                }
            }
            else
            {
                // ���NoData��Ԫ���Ѵ���  
                flag.SetFlag(row, col);
            }
        }
    }

    // ����ÿ5%���ȵ�Ԫ������  
    percentFive = validElementsCount / 20;
}

// ����׷�ٶ����еĽڵ�  
void ProcessTraceQue_onepass(CDEM& dem, Flag& flag, queue<Node>& traceQueue, PriorityQueue& priorityQueue, int& count, int percentFive)
{

    // ��Ҫ�߼��Ǳ���׷�ٶ��У�����ÿ���ڵ���ھӣ���������������׷�ٶ��к����ȼ�����
    int iRow, iCol, i;
    float iSpill;
    Node N, node, headNode;
    int width = dem.Get_NX();
    int height = dem.Get_NY();
    int total = 0, nPSC = 0;
    bool bInPQ = false;
    bool isBoundary;
    int j, jRow, jCol;
    while (!traceQueue.empty())
    {
        node = traceQueue.front();
        traceQueue.pop();
        total++;
        if ((count + total) % percentFive == 0)
        {
            std::cout << "Progress:" << (count + total) / percentFive * 5 << "%\r";
        }
        bInPQ = false;
        for (i = 0; i < 8; i++)
        {
            iRow = Get_rowTo(i, node.row);
            iCol = Get_colTo(i, node.col);
            if (flag.IsProcessedDirect(iRow, iCol)) continue;

            iSpill = dem.asFloat(iRow, iCol);

            if (iSpill <= node.spill) {
                if (!bInPQ) {
                    //decide  whether (iRow, iCol) is a true border cell
                    isBoundary = true;
                    for (j = 0; j < 8; j++)
                    {
                        jRow = Get_rowTo(j, iRow);
                        jCol = Get_colTo(j, iCol);
                        if (flag.IsProcessedDirect(jRow, jCol) && dem.asFloat(jRow, jCol) < iSpill)
                        {
                            isBoundary = false;
                            break;
                        }
                    }
                    if (isBoundary) {
                        priorityQueue.push(node);
                        bInPQ = true;
                        nPSC++;
                    }
                }
                continue;
            }
            //otherwise
            //N is unprocessed and N is higher than C
            N.col = iCol;
            N.row = iRow;
            N.spill = iSpill;
            traceQueue.push(N);
            flag.SetFlag(iRow, iCol);
        }
    }
    count += total - nPSC;
}

// �����ݵص�Ԫ��  
void ProcessPit_onepass(CDEM& dem, Flag& flag, queue<Node>& depressionQue, queue<Node>& traceQueue, PriorityQueue& priorityQueue, int& count, int percentFive)
{

    // ��Ҫ�߼��Ǳ����ݵض��У�����ÿ���ݵص�Ԫ����ھӣ����������������ݵض��к�׷�ٶ���  
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
            { //slope cell
                N.row = iRow;
                N.col = iCol;
                N.spill = iSpill;
                flag.SetFlag(iRow, iCol);
                traceQueue.push(N);
                continue;
            }

            //depression cell
            flag.SetFlag(iRow, iCol);
            dem.Set_Value(iRow, iCol, node.spill);
            N.row = iRow;
            N.col = iCol;
            N.spill = node.spill;
            depressionQue.push(N);
        }
    }
}

// ʹ��Zhou��һ���㷨���DEM  
void FillDEM_Zhou_OnePass(const char* inputFile, const char* outputFilledPath)
{
    // ����׷�ٶ��к��ݵض���  
    queue<Node> traceQueue;
    queue<Node> depressionQue;

    // ��ȡDEM����  
    CDEM dem;
    double geoTransformArgs[6]; // ����任����  
    

    cout << "Reading tiff files..." << endl;
    //readTIFF���ڶ�ȡGeoTIFF�ļ������������ļ�·�����������͡�DEM�������ú͵���任����
    if (!readTIFF(inputFile, GDALDataType::GDT_Float32, dem, geoTransformArgs))
    {
        printf("Error occurred while reading GeoTIFF file!\n");
        return;
    }

    cout << "Finish reading data" << endl;
    
    // ��¼��ʼʱ��  
    time_t timeStart, timeEnd;
    int width = dem.Get_NX();
    int height = dem.Get_NY();
     
    cout << "DEM size: " << width << " x " << height << endl;

    timeStart = time(NULL);
    cout << "Using the one-pass implementation of the proposed variant to fill DEM" << endl;

    // ��ʼ���������  
    Flag flag;
    if (!flag.Init(width, height)) {
        printf("Failed to allocate memory!\n");
        return;
    }

    // �������ȼ�����  
    PriorityQueue priorityQueue;
    int percentFive; // ÿ5%���ȵ�Ԫ������  
    int count = 0, potentialSpillCount = 0; // �������� 
    int iRow, iCol, row, col;
    float iSpill, spill;

    // ��ʼ�����ȼ�����  
    InitPriorityQue_onepass(dem, flag, traceQueue, priorityQueue, percentFive);
    // �������ȼ������еĽڵ�  
    while (!priorityQueue.empty())
    {
        Node tmpNode = priorityQueue.top();
        priorityQueue.pop();
        count++;
        // ���������Ϣ  
        if (count % percentFive == 0)
        {
            cout << "Progress:" << count / percentFive * 5 << "%\r";
        }

        // ��Ҫ�߼��Ǳ�����ǰ�ڵ���ھӣ����������������ݵض��С�׷�ٶ��к����ȼ�����  
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
                ProcessPit_onepass(dem, flag, depressionQue, traceQueue, priorityQueue, count, percentFive);
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
            ProcessTraceQue_onepass(dem, flag, traceQueue, priorityQueue, count, percentFive);
        }
    }
    // ��¼����ʱ��  
    timeEnd = time(NULL);
    double consumeTime = difftime(timeEnd, timeStart);
    cout << "Time used:" << consumeTime << " seconds" << endl;

    // ����ͳ��������������ļ�  
    double min, max, mean, stdDev;
    calculateStatistics(dem, &min, &max, &mean, &stdDev);
    CreateGeoTIFF(outputFilledPath, dem.Get_NY(), dem.Get_NX(),
        (void*)dem.getDEMdata(), GDALDataType::GDT_Float32, geoTransformArgs,
        &min, &max, &mean, &stdDev, -9999);

    return;
}