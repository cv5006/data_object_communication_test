#include "data_object.h"

cvector_vector_type(DataObjectDictionary*) dods;


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


/*
  ___     _          _         ___             _   _             
 | _ \_ _(_)_ ____ _| |_ ___  | __|  _ _ _  __| |_(_)___ _ _  ___
 |  _/ '_| \ V / _` |  _/ -_) | _| || | ' \/ _|  _| / _ \ ' \(_-<
 |_| |_| |_|\_/\__,_|\__\___| |_| \_,_|_||_\__|\__|_\___/_||_/__/
                                                                 
*/
static int CompareID(const void* lt, const void* rt)
{
    return (int)*(uint8_t *)lt - (int)*(uint8_t *)rt;
}

static int FindPDO(DataObjectDictionary* dod, uint16_t id)
{
    if (!dod->pdo) {
        return -2;
    }

    for (int i = 0; i < cvector_size(dod->pdo); ++i) {
        if (id == dod->pdo[i].id) {
            return i;
        }
    }

    return -1;
}

static int FindSDO(DataObjectDictionary* dod, uint16_t id)
{
    if (!dod->sdo) {
        return -2;
    }

    for (int i = 0; i < cvector_size(dod->sdo); ++i) {
        if (id == dod->sdo[i].id) {
            return i;
        }
    }

    return -1;
}

static DataObjectHeader GetHeader(uint8_t* byte_arr)
{
    DataObjectHeader header;
    memcpy(&header, byte_arr, sizeof(DataObjectHeader));
    // TODO: Header validation
}

static DataObjectHeader SetHeader(uint8_t* byte_arr, int* byte_len, uint8_t dod_id, uint8_t obj_type, uint16_t obj_id)
{
    // TODO: Header validation
    DataObjectHeader header;
    header.dod_id   = dod_id;
    header.obj_type = obj_type;
    header.obj_id   = obj_id;

    int len =sizeof(DataObjectHeader);
    memcpy(byte_arr, &header, len);
    *byte_len += len;
}

static uint8_t* Byte2PDOdata(uint8_t* byte_arr)
{
    return &byte_arr[sizeof(DataObjectHeader)];
}

static SDOargs Byte2SDOreq(uint8_t* byte_arr)
{
    SDOargs req;    
    // Result member is not required for request
    int idx = sizeof(DataObjectHeader) + sizeof(req.result);
    int len = sizeof(req.size);
    memcpy(&req.size, &byte_arr[idx], len);
    req.data = &byte_arr[idx + len];

    return req;
}


/*
  ___       _           ___  _     _        _     ___  _    _   _                        
 |   \ __ _| |_ __ _   / _ \| |__ (_)___ __| |_  |   \(_)__| |_(_)___ _ _  __ _ _ _ _  _ 
 | |) / _` |  _/ _` | | (_) | '_ \| / -_) _|  _| | |) | / _|  _| / _ \ ' \/ _` | '_| || |
 |___/\__,_|\__\__,_|  \___/|_.__// \___\__|\__| |___/|_\__|\__|_\___/_||_\__,_|_|  \_, |
                                |__/                                                |__/ 
*/

// Create Data Object
void DataObejct_CreatePDO(DataObjectDictionary* dod, uint16_t id, char* name, DataTypeEnum type, uint16_t len, void* addr)
{
    PDOStruct new_pdo;

    new_pdo.id   = id;
    new_pdo.name = (char*)malloc(strlen(name)); strcpy(new_pdo.name, name);
    new_pdo.type = type;
    new_pdo.len  = len;
    new_pdo.addr = addr;

    new_pdo.bytelen = GetDataTypeInfo(type).size * len;

    cvector_push_back(dod->pdo, new_pdo);
}

void DataObejct_CreateSDO(DataObjectDictionary* dod, uint16_t id, char* name, DataTypeEnum type, SDOcallback callback)
{
    SDOStruct new_sdo;

    new_sdo.id   = id;
    new_sdo.name = (char*)malloc(strlen(name)); strcpy(new_sdo.name, name);
    new_sdo.type = type;
    new_sdo.callback = callback;

    cvector_push_back(dod->sdo, new_sdo);
}

// TxRx Protocols
int DataObject_TxProtocol(uint8_t* byte_arr, uint16_t* byte_len, uint8_t dod_id, uint8_t obj_type, uint16_t obj_id)
{
    *byte_len = 0;

    DataObjectHeader header = GetHeader(byte_arr);
    
    *byte_len += sizeof(DataObjectHeader);
    memcpy(byte_arr, &header, *byte_len);

    
    if (obj_type == DATA_OBJECT_TYPE_PDO) {
        uint16_t len = 0;
        DataObject_PubPDO(dod_id, obj_id, Byte2PDOdata(byte_arr), &len);
        *byte_len += len;
        return 0;
    }

    if (obj_type == DATA_OBJECT_TYPE_SDO) {
        return 0;
    }

    return -1;
}

int DataObject_RxProtocol(uint8_t* byte_arr, uint16_t byte_len)
{
    DataObjectHeader header = GetHeader(byte_arr);
    
    if (header.obj_type == DATA_OBJECT_TYPE_PDO) {
        DataObject_SubPDO(header.dod_id, header.obj_id, Byte2PDOdata(byte_arr));
        return 0;
    }

    if (header.obj_type == DATA_OBJECT_TYPE_SDO) {
        SDOargs req, res;
        req = Byte2SDOreq(byte_arr);
        DataObject_ResponseSDO(header.dod_id, header.obj_id, &req, &res);

        return 0;
    }

    return -1;
}


// PDO PubSub
void DataObject_PubPDO(uint8_t dod_id, uint16_t obj_id, uint8_t* data, uint16_t* len)
{
    DataObjectDictionary* dod = dods[dod_id];
    PDOStruct* pdo = &dod->pdo[FindPDO(dod, obj_id)];
    memcpy(data, pdo->addr, pdo->bytelen);
    *len = pdo->bytelen;
}


void DataObject_SubPDO(uint8_t dod_id, uint16_t obj_id, uint8_t* data)
{
    DataObjectDictionary* dod = dods[dod_id];
    PDOStruct* pdo = &dod->pdo[FindPDO(dod, obj_id)];
    memcpy(pdo->addr, data, pdo->bytelen);
}


void DataObject_ResponseSDO(uint8_t dod_id, uint16_t obj_id, SDOargs* req, SDOargs* res)
{
    DataObjectDictionary* dod = dods[dod_id];
    SDOStruct* sdo = &dod->sdo[FindSDO(dod, obj_id)];
    sdo->callback(req, res);
}


void DataObject_PrintDictionary(DataObjectDictionary* dod)
{
    // int n_id = dod->occupied;
    // uint8_t id_list[n_id];
    // for (int i = 0; i < n_id; i++) {
    //     id_list[i] = dod->obj[i].id;
    // }

    // qsort(id_list, n_id, sizeof(uint8_t), CompareID);

    // printf("┌-----------------------------------------------\n");
    // printf("|Data Object Dictionary\n");
    // printf("|Occupied/Capacity: %d/%d\n", n_id, dod->capacity);
    // printf("|-----------------------------------------------\n");
    // printf("|Id\t|Type    |Name      |Addr\n");
    // for (int i = 0; i < n_id; i++) {
    //     int idx = FindID(dod->id_table, id_list[i]);
    //     printf("|%02d\t|%-7s |%-10s|%p\n",
    //         dod->obj[idx].id,
    //         GetDataTypeInfo(dod->obj[idx].type).name,
    //         dod->obj[idx].name,
    //         dod->obj[idx].addr
    //     );
    // }
    // printf("└-----------------------------------------------\n");
}


int DataObject_ExportDictionaryCSVStr(DataObjectDictionary* dod, char** csv_str)
{
    // int n_id = dod->occupied;
    // uint8_t id_list[n_id];
    // for (int i = 0; i < n_id; i++) {
    //     id_list[i] = dod->obj[i].id;
    // }
    // qsort(id_list, n_id, sizeof(uint8_t), CompareID);

    // char* col_name_str = "ID,Type,Name";

    // int csv_len = 0;
    // int row_len[n_id+1];

    // row_len[0] = snprintf(NULL, 0, "%s\n", col_name_str);
    // csv_len += row_len[0];
    // for (int row = 1; row <= n_id; row++) {
    //     int idx = FindID(dod->id_table, id_list[row-1]);
    //     row_len[row] = snprintf(NULL, 0, "%d,%d,%s\n",
    //                     dod->obj[idx].id,
    //                     dod->obj[idx].type,
    //                     dod->obj[idx].name
    //                     );
    //     csv_len += row_len[row];
    // }
    // csv_len += 1; // '/0'

    // *csv_str = malloc(csv_len);
    // int cursor = 0;

    // snprintf(*csv_str + cursor, row_len[0]+1, "%s\n", col_name_str);
    // cursor += row_len[0];
    // for (int row = 1; row <= n_id; row++) {
    //     int idx = FindID(dod->id_table, id_list[row-1]);
    //     snprintf(*csv_str + cursor, row_len[row]+1, "%d,%d,%s\n",
    //         dod->obj[idx].id,
    //         dod->obj[idx].type,
    //         dod->obj[idx].name
    //         );
    //     cursor += row_len[row];
    // }
    // return 0;
}