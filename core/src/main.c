#include "csv_parser.h"
#include "data_object.h"

#include "cvector.h"

cvector_vector_type(DataObjectDictionary*) dods;

#define N 3
float data[N];

void PrintData(float* data, int len){
    for (int i = 0; i < len; ++i) {
        printf("%.3f, ", data[i]);
    } printf("\n");
}

void PrintBuffer(uint8_t* buff, uint16_t len){
    printf("buffer: \"");
    for (int i = 0; i < len; ++i) {
        printf("x%02X/", buff[i]);
    } printf("\" %dB\n", len);
}


void Float_x10(SDOargs* req, SDOargs* res)
{    
    printf("Float_x10 Callback\n");

    printf("Req args: ");
    for (int i = 0; i < req->size ; ++i) {
        printf("%.3f, ", ((float*)(req->data))[i]);
    } printf("\n");

    res->data = malloc(sizeof(float)* req->size);
    res->size = req->size;

    printf("Res = Req * 10\n");
    printf("Res args: ");
    for (int i = 0; i < req->size ; ++i) {
        ((float*)res->data)[i] = ((float*)(req->data))[i] * 10;
        printf("%.3f, ", ((float*)res->data)[i]);
    } printf("\n");

}

void RepeatReq(SDOargs* req, SDOargs* res)
{    
    printf("RepeatReq Callback\n");

    printf("Req args: ");
    for (int i = 0; i < req->size ; ++i) {
        printf("%.3f, ", ((float*)(req->data))[i]);
    } printf("\n");

    res->size = req->size * 2;
    res->data = malloc(sizeof(float)* res->size);

    printf("Res = Req, Req\n");
    printf("Res args: ");
    memcpy((float*)res->data,               (float*)req->data, req->size);
    memcpy((float*)(res->data) + req->size, (float*)req->data, req->size);
    for (int i = 0; i < req->size ; ++i) {
        printf("%.3f, ", ((float*)res->data)[i]);
    } printf("\n");
}

void DataObjectTest()
{
    printf("*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*\n");
    printf("*.*.*.*.*. Data Object Test! .*.*.*.*\n");
    printf("*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*\n");

    uint8_t dod_id = 0;
    uint8_t obj_id = 123;

    // Create DOD
    DataObejct_CreateDOD(dod_id);

    /* PDO Test */

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
    
    // DataObject_TxProtocol(buff, &len, 0, DATA_OBJECT_TYPE_PDO, obj_id);
    len = DataObject_PubPDO(dod_id, obj_id, (void*)buff);
    
    PrintBuffer(buff, len);

    // Reset data
    data[0] = 0;
    data[1] = 0;
    data[2] = 0;
    printf("2. %s resset: ", dods[dod_id]->pdo[dod_id].name);
    PrintData(data, N);

    // Rx PDO
    printf("Rx PDO from buffer\n");
    
    // DataObject_RxProtocol(buff, len);
    len = DataObject_SubPDO(dod_id, obj_id, (void*)buff);
        
    printf("3. %s after rx: ", dods[dod_id]->pdo[dod_id].name);
    PrintData(data, N);


    /* SDO Test */
    
    // Create SDO
    DataObejct_CreateSDO(dod_id, obj_id, "copy", Float32, Float_x10);


    SDOargs req;

    req.data = data;
    req.size = N;
    
    DataObject_CallSDO(dod_id, obj_id, &req);

    PrintData((float*)(dods[dod_id]->sdo[0].response.data),
                       dods[dod_id]->sdo[0].response.size);

    free(dods[dod_id]->sdo[0].response.data);
    dods[dod_id]->sdo[0].response.data = NULL;

    DataObject_FreeDODs();
    
    printf("\n");
}


void DOPTest()
{
    printf("*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*\n");
    printf("*.*.*.*.*. Protocol Test .*.*.*.*\n");
    printf("*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*\n");

    uint8_t dod1 = 0, dod2 = 1;
    uint16_t pdo1_1 = 123, pdo1_2 = 456, pdo2_1 = 123;
    uint16_t sdo2_1 = 11, sdo2_2 = 22;

    float float_1 = 0;
    int16_t int16_2[2] = {0, 0};
    float flaot_3[3] = {0, 0, 0};

    // Create DODs
    DataObejct_CreateDOD(dod1);
    DataObejct_CreatePDO(dod1, pdo1_1, "pdo 1-1", Float32, 1, &float_1);
    DataObejct_CreatePDO(dod1, pdo1_2, "pdo 1-2", Int16, 2, int16_2);

    DataObejct_CreateDOD(dod2);
    DataObejct_CreatePDO(dod2, pdo2_1, "pdo 2-1",   Float32, 3, &flaot_3);
    DataObejct_CreateSDO(dod2, sdo2_1, "float x10", Float32, Float_x10);
    DataObejct_CreateSDO(dod2, sdo2_2, "rep req",   Float32, RepeatReq);

  

    // Free DODs
    DataObject_FreeDODs();

    printf("\n");
}



int main() {
    // DataObjectTest();
    DOPTest();
}
