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


/*
  ___       _           ___  _     _        _     ___  _    _   _                        
 |   \ __ _| |_ __ _   / _ \| |__ (_)___ __| |_  |   \(_)__| |_(_)___ _ _  __ _ _ _ _  _ 
 | |) / _` |  _/ _` | | (_) | '_ \| / -_) _|  _| | |) | / _|  _| / _ \ ' \/ _` | '_| || |
 |___/\__,_|\__\__,_|  \___/|_.__// \___\__|\__| |___/|_\__|\__|_\___/_||_\__,_|_|  \_, |
                                |__/                                                |__/ 
*/

// Create Data Object & Dictionary
void DataObejct_CreateDOD(uint8_t dod_id)
{
    DataObjectDictionary* dod = (DataObjectDictionary*)malloc(sizeof(DataObjectDictionary));
    dod->pdo = NULL;
    dod->sdo = NULL;

    if (cvector_size(dods) <= dod_id) {
        cvector_reserve(dods, dod_id + 1);
    }
    dods[dod_id] = dod;
}


void DataObejct_CreatePDO(uint8_t dod_id, uint16_t obj_id, char* name, DataTypeEnum type, uint16_t len, void* addr)
{
    DataObjectDictionary* dod = dods[dod_id];
    PDOStruct new_pdo;

    new_pdo.id   = obj_id;
    new_pdo.name = (char*)malloc(strlen(name)+1); strcpy(new_pdo.name, name);
    new_pdo.type = type;
    new_pdo.len  = len;
    new_pdo.addr = addr;

    new_pdo.bytelen = GetDataTypeInfo(type).size * len;

    cvector_push_back(dod->pdo, new_pdo);
}

void DataObejct_CreateSDO(uint8_t dod_id, uint16_t obj_id, char* name, DataTypeEnum type, SDOcallback callback)
{
    DataObjectDictionary* dod = dods[dod_id];
    SDOStruct new_sdo;

    new_sdo.id          = obj_id;
    new_sdo.name        = (char*)malloc(strlen(name)+1); strcpy(new_sdo.name, name);
    new_sdo.type        = type;
    new_sdo.callback    = callback;
    new_sdo.response.result = 0;
    new_sdo.response.size   = 0;
    new_sdo.response.data   = NULL;

    cvector_push_back(dod->sdo, new_sdo);
}


// PDO Pub & Sub
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

// SDO call
void DataObject_CallSDO(uint8_t dod_id, uint16_t obj_id, SDOargs* req)
{
    DataObjectDictionary* dod = dods[dod_id];
    SDOStruct* sdo = &dod->sdo[FindSDO(dod, obj_id)];
    sdo->callback(req, &sdo->response);
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



void DataObject_FreePDO(uint8_t dod_id)
{
    DataObjectDictionary* dod = dods[dod_id];
    for (int i = 0; i < cvector_size(dod->pdo); ++i) {
        if (dod->pdo[i].name) {
            free(dod->pdo[i].name);
            dod->pdo[i].name = NULL;
        }
    }
    cvector_free(dod->pdo);
}

void DataObject_FreeSDO(uint8_t dod_id)
{
    DataObjectDictionary* dod = dods[dod_id];
    for (int i = 0; i < cvector_size(dod->sdo); ++i) {
        if (dod->sdo[i].name != NULL) {
            free(dod->sdo[i].name);
            dod->sdo[i].name = NULL;
        }

        if (dod->sdo[i].response.data != NULL) {
            free(dod->sdo[i].response.data);
            dod->sdo[i].response.data = NULL;
        }
    }
    cvector_free(dod->sdo);
}

void DataObject_FreeDOD()
{
    cvector_free(dods);
}