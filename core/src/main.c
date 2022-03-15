#include "csv_parser.h"
#include "data_object.h"

#include "cvector.h"

cvector_vector_type(DataObjectDictionary*) dods;

float data[3];

void PrintData(){
    for (int i = 0; i < 3; ++i) {
        printf("%.3f, ", data[i]);
    } printf("\n");
}

void PrintBuffer(uint8_t* buff, uint16_t len){
    printf("buffer: \"");
    for (int i = 0; i < len; ++i) {
        printf("x%02x/", buff[i]);
    } printf("\" %dB\n", len);
}

int main() {
    // Create DOD
    DataObjectDictionary dict1;
    cvector_push_back(dods, &dict1);

    // Create PDO    
    DataObejct_CreatePDO(dods[0], 123, "pdo001", Float32, 3, data);

    // Set target data
    data[0] = -1.23;
    data[1] = 45.6;
    data[2] = -7.89;

    printf("1. %s target: ", dict1.pdo[0].name);
    PrintData();

    // Tx PDO
    uint8_t buff[128];
    uint16_t len;
    
    DataObject_TxProtocol(buff, &len, 0, DATA_OBJECT_TYPE_PDO, 123);
    PrintBuffer(buff, len);

    // Reset data
    data[0] = 0;
    data[1] = 0;
    data[2] = 0;
    printf("2. %s resset: ", dict1.pdo[0].name);
    PrintData();

    // Rx PDO
    printf("Rx PDO from buffer\n");
    
    DataObject_RxProtocol(buff, len);
        
    printf("3. %s after rx: ", dict1.pdo[0].name);
    PrintData();    
}
