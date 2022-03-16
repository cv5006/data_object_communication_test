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
  ___         _               _ 
 | _ \_ _ ___| |_ ___  __ ___| |
 |  _/ '_/ _ \  _/ _ \/ _/ _ \ |
 |_| |_| \___/\__\___/\__\___/_|
                                

PDO Packet
+---------+------------------+------+
|           Header           |      |
+--------+----------+--------+ Data |
| DOD ID | OBJ Type | OBJ ID |      |
+--------+----------+--------+------+
| 0      | 1        | 2  3   | 4    |
+--------+----------+--------+------+

SDO Packet
+---------+------------------+--------------+------+
|           Header           |     Info     |      |
+--------+----------+--------+--------+-----+ Data |
| DOD ID | OBJ Type | OBJ ID | Result | Len |      |
+--------+----------+--------+--------+-----+------+
| 0      | 1        | 2  3   | 4      | 5 6 | 7    |
+--------+----------+--------+--------+-----+------+

*/ 


#define DATA_OBJECT_TYPE_PDO 0x50
#define DATA_OBJECT_TYPE_SDO 0x53


typedef struct DataObjectHeader
{
    uint8_t dod_id;
    uint8_t obj_type;
    uint16_t obj_id;
} DataObjectHeader;


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


void DataObejct_CreatePDO(DataObjectDictionary* dod, uint16_t id, char* name, DataTypeEnum type, uint16_t len, void* addr);
void DataObejct_CreateSDO(DataObjectDictionary* dod, uint16_t id, char* name, DataTypeEnum type, SDOcallback callback);

int DataObject_TxProtocol(uint8_t* byte_arr, uint16_t* byte_len, uint8_t dod_id, uint8_t obj_type, uint16_t obj_id);
int DataObject_RxProtocol(uint8_t* byte_arr, uint16_t byte_len);

void DataObject_PubPDO(uint8_t dod_id, uint16_t obj_id, uint8_t* data, uint16_t* len);
void DataObject_SubPDO(uint8_t dod_id, uint16_t obj_id, uint8_t* data);

void DataObject_ResponseSDO(uint8_t dod_id, uint16_t obj_id, SDOargs* req, SDOargs* res);

void DataObject_PrintDictionary(DataObjectDictionary* dod);
int DataObject_ExportDictionaryCSVStr(DataObjectDictionary* dod, char** csv_str);

#endif // DATA_OBJECT_H_
