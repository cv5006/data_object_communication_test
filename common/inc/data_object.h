#ifndef DATA_OBJECT_H_
#define DATA_OBJECT_H_

#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "stdlib.h"
#include "unistd.h"


#define DATA_OBJECT_MAX_ITEMS 255
#define DATA_OBJECT_ID_INDEX 0
#define DATA_OBJECT_LEN_INDEX 1
#define DATA_OBJECT_DATA_INDEX 2


typedef enum {
    UInt8,
    UInt16,
    UInt32,
    Int8,
    Int16,
    Int32,
    Float32,
    Float64
} DataTypeEnum;

typedef struct DataTypeInfo
{
    char* name;
    uint8_t size;
} DataTypeInfoStruct;


DataTypeInfoStruct GetDataTypeInfo(DataTypeEnum type)
{
    DataTypeInfoStruct res;
    switch (type) {
    case UInt8  : res.name = "uint8"  ; res.size = sizeof(uint8_t);  break;
    case UInt16 : res.name = "uint16" ; res.size = sizeof(uint16_t); break;
    case UInt32 : res.name = "uint32" ; res.size = sizeof(uint32_t); break;
    case Int8   : res.name = "int8"   ; res.size = sizeof(int8_t);   break;
    case Int16  : res.name = "int16"  ; res.size = sizeof(int16_t);  break;
    case Int32  : res.name = "int32"  ; res.size = sizeof(int32_t);  break;
    case Float32: res.name = "float32"; res.size = sizeof(float);    break;
    case Float64: res.name = "float64"; res.size = sizeof(double);   break;
    default: break;
    }
    return res;
}

static int CompareID(const void* lt, const void* rt)
{
    return (int)*(uint8_t *)lt - (int)*(uint8_t *)rt;
}

static int FindID(uint8_t* table, uint8_t id)
{
    //TODO: Range check
    return table[id];
}

static int AssignID(uint8_t* table, uint8_t id, int index)
{
    //TODO: Range & dup check
    table[id] = index;
    return 0;
}

typedef struct DataObjectStruct
{
    uint8_t id;
    DataTypeEnum type;
    char* name;

    void* addr;
    uint8_t unit_len;
} DataObjectStruct;

typedef struct DataObjectDictionary
{
    DataObjectStruct* obj;
    int occupied;
    int capacity;
    uint8_t id_table[DATA_OBJECT_MAX_ITEMS];
} DataObjectDictionary;


void DataObejct_Create(DataObjectDictionary* dod, uint8_t id, DataTypeEnum type, char* name, void* addr)
{
    if (dod->obj == NULL) {
        dod->capacity = 1;
        dod->occupied = 0;
        dod->obj = (DataObjectStruct*)malloc(sizeof(DataObjectStruct)*dod->capacity);
    }

    if (dod->occupied >= dod->capacity) {
        // Create temp object array
        int occupied_len = sizeof(DataObjectStruct)*dod->occupied;
        DataObjectStruct* tmp = (DataObjectStruct*)malloc(occupied_len);

        // Copy contents to temp
        memcpy(tmp, dod->obj, occupied_len);

        // Free original object array
        free(dod->obj);
        dod->obj = NULL;

        // Create object array with increased capacity
        dod->capacity = dod->capacity*2;
        int capable_len = sizeof(DataObjectStruct)*dod->capacity;
        dod->obj = (DataObjectStruct*)malloc(capable_len);
        
        // Copy contents to object array with increased capacity
        memcpy(dod->obj, tmp, occupied_len);

        // Free temp with increased capacity
        free(tmp);
        tmp = NULL;
    }

    uint8_t index = dod->occupied;
    dod->obj[index].id = id;
    dod->obj[index].type = type;
    dod->obj[index].unit_len = GetDataTypeInfo(type).size;
    dod->obj[index].name = (char*)malloc(strlen(name));
    strcpy(dod->obj[index].name, name);
    dod->obj[index].addr = addr;
    AssignID(dod->id_table, id, index);
    dod->occupied++;
}

void DataObject_Serialize(DataObjectDictionary* dod, uint8_t* byte_arr, uint8_t id, int len)
{
    int idx = FindID(dod->id_table, id);
    byte_arr[DATA_OBJECT_ID_INDEX] = id;
    byte_arr[DATA_OBJECT_LEN_INDEX] = len;
    memcpy(&byte_arr[DATA_OBJECT_DATA_INDEX], dod->obj[idx].addr, dod->obj[idx].unit_len*len);
}

void DataObject_Deserialize(DataObjectDictionary* dod, uint8_t* byte_arr)
{
    int idx = FindID(dod->id_table, byte_arr[DATA_OBJECT_ID_INDEX]);
    int len = byte_arr[DATA_OBJECT_LEN_INDEX];
    memcpy(dod->obj[idx].addr, &byte_arr[DATA_OBJECT_DATA_INDEX], dod->obj[idx].unit_len*len);
}

void DataObject_PrintDictionary(DataObjectDictionary* dod)
{
    int n_id = dod->occupied;
    uint8_t id_list[n_id];
    for (int i = 0; i < n_id; i++) {
        id_list[i] = dod->obj[i].id;
    }

    qsort(id_list, n_id, sizeof(uint8_t), CompareID);

    printf("┌-----------------------------------------------\n");
    printf("|Data Object Dictionary\n");
    printf("|Occupied/Capacity: %d/%d\n", n_id, dod->capacity);
    printf("|-----------------------------------------------\n");
    printf("|Id\t|Type    |Name      |Addr\n");
    for (int i = 0; i < n_id; i++) {
        int idx = FindID(dod->id_table, id_list[i]);
        printf("|%02d\t|%-7s |%-10s|%p\n",
            dod->obj[idx].id,
            GetDataTypeInfo(dod->obj[idx].type).name,
            dod->obj[idx].name,
            dod->obj[idx].addr
        );
    }
    printf("└-----------------------------------------------\n");
}

int DataObject_ExportDictionaryCSVStr(DataObjectDictionary* dod, char** csv_str)
{
    int n_id = dod->occupied;
    uint8_t id_list[n_id];
    for (int i = 0; i < n_id; i++) {
        id_list[i] = dod->obj[i].id;
    }
    qsort(id_list, n_id, sizeof(uint8_t), CompareID);

    char* col_name_str = "ID,Type,Name";

    int csv_len = 0;
    int row_len[n_id+1];

    row_len[0] = snprintf(NULL, 0, "%s\n", col_name_str);
    csv_len += row_len[0];
    for (int row = 1; row <= n_id; row++) {
        int idx = FindID(dod->id_table, id_list[row-1]);
        row_len[row] = snprintf(NULL, 0, "%d,%d,%s\n",
                        dod->obj[idx].id,
                        dod->obj[idx].type,
                        dod->obj[idx].name
                        );
        csv_len += row_len[row];
    }
    csv_len += 1; // '/0'

    *csv_str = malloc(csv_len);
    int cursor = 0;

    snprintf(*csv_str + cursor, row_len[0]+1, "%s\n", col_name_str);
    cursor += row_len[0];
    for (int row = 1; row <= n_id; row++) {
        int idx = FindID(dod->id_table, id_list[row-1]);
        snprintf(*csv_str + cursor, row_len[row]+1, "%d,%d,%s\n",
            dod->obj[idx].id,
            dod->obj[idx].type,
            dod->obj[idx].name
            );
        cursor += row_len[row];
    }
    return 0;
}


#endif // DATA_OBJECT_H_
