#include "csv_parser.h"
#include "data_object.h"
#include "data_object_protocol.h"

#include "cvector.h"

cvector_vector_type(DataObjectDictionary*) dods;

#define N 3
float data[N];
#define BUFF_SIZE 512
uint8_t buff[BUFF_SIZE];

void PrintData(float* data, int len){
    for (int i = 0; i < len; ++i) {
        printf("%.3f, ", data[i]);
    } printf("\n");
}

void PrintBuffer(uint8_t* buff, uint16_t len){
    int cols = 32;
    int row_written = 0;
    printf("buffer filled %dB:\n", len);

    printf("   |");
    for (int i = 0; i < cols; ++i) {
        printf(" %2d", i);
    } printf("\n");

    printf("---+");
    for (int i = 0; i < cols; ++i) {
        printf("---");
    }

    for (int i = 0; i < len; ++i) {
        if (i == ((cols) * row_written)) {
            printf("\n%3d|", i);
            ++row_written;
        }
        printf(" %02X", buff[i]);
    } printf("\n");
}

void ResetRes(uint8_t dod_id, uint16_t id)
{
    SDOargs* res = DataObject_GetSDOres(dod_id, id);
    if (res == NULL) {
        return;
    }

    res->status = DATA_OBJECT_SDO_IDLE;
    if (res->data != NULL) {
        free(res->data);
        res->data = NULL;
    }
    res->size = 0;
}

void PrintRes(uint8_t dod_id, uint16_t id)
{
    SDOStruct* sdo = DataObejct_FindSDO(dod_id, id);
    SDOargs* res = &sdo->args;
    printf("Result of SDO \"%s\"\n", sdo->name);
    printf("status: ");
    switch (res->status) {
        case DATA_OBJECT_SDO_SUCC: printf("Success\n"); break;
        case DATA_OBJECT_SDO_FAIL: printf("Fail\n");    break;
        case DATA_OBJECT_SDO_IDLE: printf("Idle\n");    break;
        case DATA_OBJECT_SDO_REQU: printf("Request\n"); break;
        default: printf("Invalid status\n"); break;
    }

    switch (sdo->type) {
    case Char: printf("%s", (char*)res->data); break;
    case Float32:
        for (int i = 0; i < res->size; ++i) {
            printf("%.3f, ", ((float*)res->data)[i]);
        } printf("\n");
        break;
    default: printf("No printf for this type\n"); break;
    }
    printf("\n");
}


void Float_x10(SDOargs* req, SDOargs* res)
{    
    printf("Float_x10 Callback\n");

    res->data = malloc(sizeof(float)* req->size);
    res->size = req->size;

    for (int i = 0; i < req->size ; ++i) {
        ((float*)res->data)[i] = ((float*)(req->data))[i] * 10;
    }

    res->status = DATA_OBJECT_SDO_SUCC;
}

void RepeatReq(SDOargs* req, SDOargs* res)
{    
    printf("RepeatReq Callback\n");

    res->size = req->size * 2;
    int data_size = sizeof(float) * req->size;
    res->data = malloc(data_size * 2);

    memcpy(res->data,             req->data, data_size);
    memcpy(res->data + data_size, req->data, data_size);

    res->status = DATA_OBJECT_SDO_SUCC;
}




void DataObjectTest()
{
    printf("*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*\n");
    printf("*.*.*.*.*. Data Object Test! .*.*.*.*\n");
    printf("*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*\n");

    uint8_t dod_id = 0;
    uint8_t obj_id = 123;

    // Create DOD
    DataObejct_CreateDOD(dod_id, "TestDOD");

    /* PDO Test */
    PDOStruct* pdo = NULL;

    // Create PDO
    DataObejct_CreatePDO(dod_id, obj_id, "pdo1", Float32, 3, data);

    // Set target data
    data[0] = -1.23;
    data[1] = 45.6;
    data[2] = -7.89;

    printf("1. %s target: ", dods[dod_id]->pdo[dod_id].name);
    PrintData(data, N);

    // Tx PDO
    uint8_t buff[128];
    uint16_t len;
    
    pdo = DataObejct_FindPDO(dod_id, obj_id);
    len = DataObject_PubPDO(pdo, (void*)buff);
    
    PrintBuffer(buff, len);

    // Reset data
    data[0] = 0;
    data[1] = 0;
    data[2] = 0;
    printf("2. %s resset: ", dods[dod_id]->pdo[dod_id].name);
    PrintData(data, N);

    // Rx PDO
    printf("Rx PDO from buffer\n");
    
    pdo = DataObejct_FindPDO(dod_id, obj_id);
    len = DataObject_SubPDO(pdo, (void*)buff);
        
    printf("3. %s after rx: ", dods[dod_id]->pdo[dod_id].name);
    PrintData(data, N);


    /* SDO Test */
    SDOStruct* sdo = NULL;

    // Create SDO
    DataObejct_CreateSDO(dod_id, obj_id, "copy", Float32, Float_x10);

    // Set Request
    SDOargs req;
    req.data = data;
    req.size = N;
    
    // Call SDO
    sdo = DataObejct_FindSDO(dod_id, obj_id);
    DataObject_CallSDO(sdo, &req);

    PrintData((float*)(dods[dod_id]->sdo[0].args.data),
                       dods[dod_id]->sdo[0].args.size);

    // Free Objects
    free(dods[dod_id]->sdo[0].args.data);
    dods[dod_id]->sdo[0].args.data = NULL;

    DataObject_FreeDODs();
    
    printf("\n");
}


void DOPTest()
{
    printf("*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*\n");
    printf("*.*.*.*.*. Protocol Test .*.*.*.*\n");
    printf("*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*\n");

    uint8_t dod1 = 1, dod2 = 2;
    uint16_t pdo1_1 = 123, pdo1_2 = 456, pdo2_1 = 123;
    uint16_t sdo2_1 = 11, sdo2_2 = 22;

    float   float_1    = 1.1;
    int16_t int16_2[2] = {11, 22};
    float   float_3[3] = {12.34, 56.78, 90.12};

    // Create DODs
    DataObejct_CreateDOD(dod1, "dod1");
    DataObejct_CreatePDO(dod1, pdo1_1, "pdo 1-1", Float32, 1, &float_1);
    DataObejct_CreatePDO(dod1, pdo1_2, "pdo 1-2", Int16, 2, int16_2);

    DataObejct_CreateDOD(dod2, "dod2");
    DataObejct_CreatePDO(dod2, pdo2_1, "pdo 2-1",   Float32, 3, &float_3);
    DataObejct_CreateSDO(dod2, sdo2_1, "float x10", Float32, Float_x10);
    DataObejct_CreateSDO(dod2, sdo2_2, "rep req",   Float32, RepeatReq);


    // Tx
    uint16_t len;

    printf("\n*.*.*. Tx1 .*.*.*\n");
    DOP_AddPDOtoSend(dod1, pdo1_1);
    DOP_AddPDOtoSend(dod1, pdo1_2);
    DOP_AddPDOtoSend(dod2, pdo2_1);

    DOP_AddSDOtoReq(dod2, sdo2_1, data, N);
    DOP_AddSDOtoReq(dod2, sdo2_2, data, N);
    DOP_AddSDOtoReq(DATA_OBJECT_DEFAULT_DOD, DATA_OBJECT_SDO_GET_DOD_LIST, NULL, 0);
    DOP_AddSDOtoReq(DATA_OBJECT_DEFAULT_DOD, DATA_OBJECT_SDO_GET_PDO_LIST, (void*)&(uint8_t){1}, 1);
    DOP_AddSDOtoReq(DATA_OBJECT_DEFAULT_DOD, DATA_OBJECT_SDO_GET_SDO_LIST, (void*)&(uint8_t){2}, 1);

    if (DOP_Tx(buff, &len) < 0) {
        printf("Tx failed\n");
        return;
    }

    PrintBuffer(buff, len);

    float_1    = 0;
    int16_2[0] = 0;
    int16_2[1] = 0;
    float_3[0] = 0;
    float_3[1] = 0;
    float_3[2] = 0;


    printf("\n*.*.*. Rx1 .*.*.*\n");
    if (DOP_Rx(buff, len) < 0) {
        printf("Rx failed\n");
        return;
    }

    printf("float_1: %.3f\n", float_1);
    printf("int16_2: %d, %d\n", int16_2[0], int16_2[1]);
    printf("float_3: %.3f, %.3f, %.3f\n", float_3[0], float_3[1], float_3[2]);


    printf("\n*.*.*. Tx2 .*.*.*\n");
    if (DOP_Tx(buff, &len) < 0) {
        printf("Tx failed\n");
        return;
    }

    PrintBuffer(buff, len);

    ResetRes(dod2, sdo2_1);
    ResetRes(dod2, sdo2_2);

    printf("\n*.*.*. Rx2 .*.*.*\n");
    if (DOP_Rx(buff, len) < 0) {
        printf("Rx failed\n");
        return;
    }

    PrintRes(dod2, sdo2_1);
    PrintRes(dod2, sdo2_2);
    
    PrintRes(DATA_OBJECT_DEFAULT_DOD, DATA_OBJECT_SDO_GET_DOD_LIST);
    PrintRes(DATA_OBJECT_DEFAULT_DOD, DATA_OBJECT_SDO_GET_PDO_LIST);
    PrintRes(DATA_OBJECT_DEFAULT_DOD, DATA_OBJECT_SDO_GET_SDO_LIST);

    // Free DODs
    DataObject_FreeDODs();

    printf("\n");
}



int main() {
    memset(buff, 0xFF, BUFF_SIZE);
    DataObjectTest();
    DOPTest();
}
