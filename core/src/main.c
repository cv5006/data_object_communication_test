#include "csv_parser.h"
#include "data_object.h"


int main() {
    DataObjectDictionary dod = {NULL, 0, 1};

    uint16_t var1[2] = {1, 2};
    uint32_t var2 = 0;
    float var3[4] = {0.0, 0.1, 1.0, 1.1};

    uint8_t var1_id = 0x2A;
    uint8_t var2_id = 0x0D;
    uint8_t var3_id = 0x13;

    DataObject_PrintDictionary(&dod);
    DataObejct_Create(&dod, var1_id, UInt16,  2, "int16arr", (void*)var1);
    DataObejct_Create(&dod, var2_id, UInt32,  1, "int",      (void*)&var2);
    DataObejct_Create(&dod, var3_id, Float32, 4, "floatarr", (void*)var3);
    DataObejct_Create(&dod, 125, Float64, 6, "dummy1", NULL);
    DataObejct_Create(&dod, 173, Int16,   1, "dummy2", NULL);
    DataObejct_Create(&dod, 233, Int8,    3, "dummy3", NULL);
    DataObject_PrintDictionary(&dod);

    uint8_t input[20];
    
    printf("Before:\t%d, %d\n", var1[0], var1[1]);
    var1[0] = 0x1234;
    var1[1] = 0xABCD;
    DataObject_Serialize(&dod, input, var1_id);
    DataObject_Deserialize(&dod, input);
    printf("After:\t0x%X, 0x%X\n", var1[0], var1[1]);

    printf("Before:\t%d\n", var2);
    var2 = 0xA1B2C3D4;
    DataObject_Serialize(&dod, input, var2_id);
    DataObject_Deserialize(&dod, input);
    printf("After:\t0x%X\n", var2);

    printf("Before:\t%.2f, %.2f, %.2f, %.2f\n", var3[0], var3[1], var3[2], var3[3]);
    var3[0] = 1.23;
    var3[1] = 4.56;
    var3[2] = 7.89;
    var3[3] = 1.01;
    DataObject_Serialize(&dod, input, var3_id);
    DataObject_Deserialize(&dod, input);
    printf("After:\t%.2f, %.2f, %.2f, %.2f\n", var3[0], var3[1], var3[2], var3[3]);

    DataObject_ExportDictionaryCSV(&dod);
}
