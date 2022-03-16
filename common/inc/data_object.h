#ifndef DATA_OBJECT_H_
#define DATA_OBJECT_H_

#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "stdlib.h"
#include "unistd.h"

#include "cvector.h"

#pragma pack(1)

#define DATA_OBJECT_MAX_ITEMS  255


/*
  ___       _          _____               
 |   \ __ _| |_ __ _  |_   _|  _ _ __  ___ 
 | |) / _` |  _/ _` |   | || || | '_ \/ -_)
 |___/\__,_|\__\__,_|   |_| \_, | .__/\___|
                            |__/|_|         
*/

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


DataTypeInfoStruct GetDataTypeInfo(DataTypeEnum type);


/*
  ___ ___   ___    _                 _ _        
 / __|   \ / _ \  | |_  __ _ _ _  __| | |___ ___
 \__ \ |) | (_) | | ' \/ _` | ' \/ _` | / -_|_-<
 |___/___/ \___/  |_||_\__,_|_||_\__,_|_\___/__/
                                                
*/

typedef struct SDOargs
{
    int8_t result;
    void* data;
    uint16_t size;
} SDOargs;

typedef void (*SDOcallback) (SDOargs*, SDOargs*);


/*
  ___       _           ___  _     _        _   
 |   \ __ _| |_ __ _   / _ \| |__ (_)___ __| |_ 
 | |) / _` |  _/ _` | | (_) | '_ \| / -_) _|  _|
 |___/\__,_|\__\__,_|  \___/|_.__// \___\__|\__|
                                |__/            
*/

typedef struct PDOStruct
{
    uint16_t id;
    char* name;

    DataTypeEnum type;
    uint16_t len;

    void* addr;
    uint16_t bytelen;
} PDOStruct;


typedef struct SDOStruct
{
    uint16_t id;
    char* name;

    DataTypeEnum type;
    
    SDOcallback callback;
    SDOargs response;
} SDOStruct;


/*
  ___       _           ___  _     _        _     ___  _    _   _                        
 |   \ __ _| |_ __ _   / _ \| |__ (_)___ __| |_  |   \(_)__| |_(_)___ _ _  __ _ _ _ _  _ 
 | |) / _` |  _/ _` | | (_) | '_ \| / -_) _|  _| | |) | / _|  _| / _ \ ' \/ _` | '_| || |
 |___/\__,_|\__\__,_|  \___/|_.__// \___\__|\__| |___/|_\__|\__|_\___/_||_\__,_|_|  \_, |
                                |__/                                                |__/ 
*/

typedef struct DataObjectDictionary
{
    cvector_vector_type(PDOStruct) pdo;
    cvector_vector_type(SDOStruct) sdo;
} DataObjectDictionary;

extern cvector_vector_type(DataObjectDictionary*) dods;


void DataObejct_CreateDOD(uint8_t dod_id);
void DataObejct_CreatePDO(uint8_t dod_id, uint16_t obj_id, char* name, DataTypeEnum type, uint16_t len, void* addr);
void DataObejct_CreateSDO(uint8_t dod_id, uint16_t obj_id, char* name, DataTypeEnum type, SDOcallback callback);

uint16_t DataObject_PubPDO(uint8_t dod_id, uint16_t obj_id, void* data);
uint16_t DataObject_SubPDO(uint8_t dod_id, uint16_t obj_id, void* data);

uint16_t DataObject_CallSDO(uint8_t dod_id, uint16_t obj_id, SDOargs* req);
SDOargs DataObject_GetSDOReponse(uint8_t dod_id, uint16_t obj_id);

void DataObject_PrintDictionary(DataObjectDictionary* dod);
int DataObject_ExportDictionaryCSVStr(DataObjectDictionary* dod, char** csv_str);


void DataObject_FreePDO(uint8_t dod_id);
void DataObject_FreeSDO(uint8_t dod_id);
void DataObject_FreeDOD();

#endif // DATA_OBJECT_H_
