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


void CopyReq(SDOargs* req, SDOargs* res)
{    
    printf("SDO Callback Test\n");

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



int main() {
    uint8_t dod_id = 0;
    uint8_t obj_id = 123;
    cvector_reserve(dods, 2);

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
    DataObejct_CreateSDO(dod_id, obj_id, "copy", Float32, CopyReq);


    SDOargs req;

    req.data = data;
    req.size = N;
    
    DataObject_CallSDO(dod_id, obj_id, &req);

    PrintData((float*)(dods[dod_id]->sdo[0].response.data),
                       dods[dod_id]->sdo[0].response.size);

    free(dods[dod_id]->sdo[0].response.data);
    dods[dod_id]->sdo[0].response.data = NULL;

    DataObject_FreePDO(dod_id);
    DataObject_FreeSDO(dod_id);
    DataObject_FreeDOD();
}
