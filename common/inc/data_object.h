#ifndef DATA_OBJECT_H_
#define DATA_OBJECT_H_

#if __cplusplus
extern "C"{
#endif

#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "stdlib.h"
#include "unistd.h"

#include "cvector.h"

#pragma pack(1)

/*
  ___       _          _____               
 |   \ __ _| |_ __ _  |_   _|  _ _ __  ___ 
 | |) / _` |  _/ _` |   | || || | '_ \/ -_)
 |___/\__,_|\__\__,_|   |_| \_, | .__/\___|
                            |__/|_|         
*/

typedef enum {
    Char,
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

#define DATA_OBJECT_SDO_IDLE  2
#define DATA_OBJECT_SDO_REQU  1
#define DATA_OBJECT_SDO_SUCC  0
#define DATA_OBJECT_SDO_FAIL -1


typedef struct SDOargs
{
    int8_t status;
    void* data;
    uint16_t size;
    uint16_t data_size;
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
    void* last_pub;
    uint16_t bytelen;
} PDOStruct;


typedef struct SDOStruct
{
    uint16_t id;
    char* name;

    DataTypeEnum type;
    
    SDOcallback callback;
    SDOargs args;
} SDOStruct;


/*
  ___       _           ___  _     _        _     ___  _    _   _                        
 |   \ __ _| |_ __ _   / _ \| |__ (_)___ __| |_  |   \(_)__| |_(_)___ _ _  __ _ _ _ _  _ 
 | |) / _` |  _/ _` | | (_) | '_ \| / -_) _|  _| | |) | / _|  _| / _ \ ' \/ _` | '_| || |
 |___/\__,_|\__\__,_|  \___/|_.__// \___\__|\__| |___/|_\__|\__|_\___/_||_\__,_|_|  \_, |
                                |__/                                                |__/ 
*/


#define DATA_OBJECT_DEFAULT_DOD 0
#define DATA_OBJECT_SDO_GET_DOD_LIST 0
#define DATA_OBJECT_SDO_GET_PDO_LIST 1
#define DATA_OBJECT_SDO_GET_SDO_LIST 2

typedef struct DataObjectDictionary
{
    char* name;
    cvector_vector_type(PDOStruct) pdo;
    cvector_vector_type(SDOStruct) sdo;
} DataObjectDictionary;

void DataObejct_InitDefaultDOD();

void DataObejct_CreateDOD(uint8_t dod_id, char* name);
void DataObejct_CreatePDO(uint8_t dod_id, uint16_t obj_id, char* name, DataTypeEnum type, uint16_t len, void* addr);
void DataObejct_CreateSDO(uint8_t dod_id, uint16_t obj_id, char* name, DataTypeEnum type, SDOcallback callback);

PDOStruct* DataObejct_FindPDO(uint8_t dod_id, uint16_t id);
SDOStruct* DataObejct_FindSDO(uint8_t dod_id, uint16_t id);

uint16_t DataObject_SendPDO(PDOStruct* pdo, void* data);
uint16_t DataObject_RecvPDO(PDOStruct* pdo, void* data);

void DataObject_SetSDOreq(uint8_t dod_id, uint16_t id, void* data, uint16_t size);
SDOargs* DataObject_GetSDOres(uint8_t dod_id, uint16_t id);

uint16_t DataObject_CallSDO(SDOStruct* sdo, SDOargs* req);
uint16_t DataObejct_SetSDOargs(SDOStruct* sdo, SDOargs* args);

int DataObject_DODlist(char** list_str, uint16_t* list_len);
int DataObject_POD2CSV(char** csv_str, uint16_t* csv_len, uint8_t dod_id);
int DataObject_SOD2CSV(char** csv_str, uint16_t* csv_len, uint8_t dod_id);


void DataObject_FreeDODs();



/*
  ___       __           _ _     ___   ___  ___  
 |   \ ___ / _|__ _ _  _| | |_  |   \ / _ \|   \ 
 | |) / -_)  _/ _` | || | |  _| | |) | (_) | |) |
 |___/\___|_| \__,_|\_,_|_|\__| |___/ \___/|___/ 
                                                 
*/
void DefaultDOD_GetDODs(SDOargs* req, SDOargs* res);
void DefaultDOD_GetPDOs(SDOargs* req, SDOargs* res);
void DefaultDOD_GetSDOs(SDOargs* req, SDOargs* res);


#if __cplusplus
}
#endif

#endif // DATA_OBJECT_H_
