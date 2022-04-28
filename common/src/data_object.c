#include "data_object.h"

DataObjectDictionary** dods;
int dods_size;


DataTypeInfoStruct GetDataTypeInfo(DataTypeEnum type)
{
    DataTypeInfoStruct res;
    switch (type) {
    case Char   : res.name = "char"   ; res.size = sizeof(char);     break;
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

static DataObjectDictionary* FindDOD(uint8_t dod_id)
{
    if (dods_size < dod_id) {
        return NULL;
    }
    
    return dods[dod_id];
}



void DataObejct_InitDefaultDOD()
{
    DataObejct_CreateDOD(DATA_OBJECT_DEFAULT_DOD, "defualt_dod");
    DataObejct_CreateSDO(DATA_OBJECT_DEFAULT_DOD, DATA_OBJECT_SDO_GET_DOD_LIST, "dod_list", Char,  DefaultDOD_GetDODs);
    DataObejct_CreateSDO(DATA_OBJECT_DEFAULT_DOD, DATA_OBJECT_SDO_GET_PDO_LIST, "pdo_list", Char, DefaultDOD_GetPDOs);
    DataObejct_CreateSDO(DATA_OBJECT_DEFAULT_DOD, DATA_OBJECT_SDO_GET_SDO_LIST, "sdo_list", Char, DefaultDOD_GetSDOs);
}

/*
  ___       _           ___  _     _        _     ___  _    _   _                        
 |   \ __ _| |_ __ _   / _ \| |__ (_)___ __| |_  |   \(_)__| |_(_)___ _ _  __ _ _ _ _  _ 
 | |) / _` |  _/ _` | | (_) | '_ \| / -_) _|  _| | |) | / _|  _| / _ \ ' \/ _` | '_| || |
 |___/\__,_|\__\__,_|  \___/|_.__// \___\__|\__| |___/|_\__|\__|_\___/_||_\__,_|_|  \_, |
                                |__/                                                |__/ 
*/

// Create Data Object & Dictionary
void DataObejct_CreateDOD(uint8_t dod_id, char* name)
{
    if (dods_size <= 0) {
        // Alloc one for defualt, one for yours
        dods = (DataObjectDictionary**)malloc(sizeof(DataObjectDictionary*) * 2);
        dods[0] = NULL; dods[1] = NULL;
        dods_size = 1;
        DataObejct_InitDefaultDOD();
    }

    DataObjectDictionary* dod = (DataObjectDictionary*)malloc(sizeof(DataObjectDictionary));
    dod->pdo = NULL;
    dod->sdo = NULL;
    dod->name = (char*)malloc(strlen(name)+1);
    strcpy(dod->name, name);

    if (dods_size <= dod_id) {
        dods_size = dod_id + 1;
        dods = realloc(dods, sizeof(DataObjectDictionary)*dods_size);
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

    new_pdo.bytelen  = GetDataTypeInfo(type).size * len;
    new_pdo.last_pub = malloc(new_pdo.bytelen);

    memset(new_pdo.last_pub, 0xFF, new_pdo.bytelen);

    cvector_push_back(dod->pdo, new_pdo);
}

void DataObejct_CreateSDO(uint8_t dod_id, uint16_t obj_id, char* name, DataTypeEnum type, SDOcallback callback)
{
    DataObjectDictionary* dod = dods[dod_id];
    SDOStruct new_sdo;

    new_sdo.id        = obj_id;
    new_sdo.name      = (char*)malloc(strlen(name)+1); strcpy(new_sdo.name, name);
    new_sdo.type      = type;
    new_sdo.callback  = callback;

    new_sdo.args.status = DATA_OBJECT_SDO_IDLE;
    new_sdo.args.size   = 0;
    new_sdo.args.data   = NULL;
    new_sdo.args.data_size = GetDataTypeInfo(type).size;

    cvector_push_back(dod->sdo, new_sdo);
}

// Find Object
PDOStruct* DataObejct_FindPDO(uint8_t dod_id, uint16_t id)
{
    DataObjectDictionary* dod = FindDOD(dod_id);
    if (dod == NULL || dod->pdo == NULL) {
        return NULL;
    }

    for (int i = 0; i < cvector_size(dod->pdo); ++i) {
        if (id == dod->pdo[i].id) {
            return &dod->pdo[i];
        }
    }

    return NULL;
}

SDOStruct* DataObejct_FindSDO(uint8_t dod_id, uint16_t id)
{
    DataObjectDictionary* dod = FindDOD(dod_id);
    if (dod == NULL || dod->sdo == NULL) {
        return NULL;
    }

    for (int i = 0; i < cvector_size(dod->sdo); ++i) {
        if (id == dod->sdo[i].id) {
            return &dod->sdo[i];
        }
    }

    return NULL;
}


// PDO Pub & Sub
uint16_t DataObject_PubPDO(PDOStruct* pdo, void* data)
{    
    if (memcmp(pdo->addr, pdo->last_pub, pdo->bytelen) == 0) {
        // No Pub if data is same with last published one
        return 0;
    }

    memcpy(data, pdo->addr, pdo->bytelen);
    memcpy(pdo->last_pub, pdo->addr, pdo->bytelen);
    return pdo->bytelen;
}


uint16_t DataObject_SubPDO(PDOStruct* pdo, void* data)
{
    memcpy(pdo->addr, data, pdo->bytelen);
    return pdo->bytelen;
}


// SDO call
uint16_t DataObject_CallSDO(SDOStruct* sdo, SDOargs* req)
{
    if (sdo->args.data != NULL) {
        free(sdo->args.data);
        sdo->args.data = NULL;
    }

    sdo->args.status = DATA_OBJECT_SDO_IDLE;
    sdo->callback(req, &sdo->args);
    return req->size * sdo->args.data_size;
}

void DataObject_SetSDOreq(uint8_t dod_id, uint16_t id, void* data, uint16_t size)
{
    // TODO: exception
    SDOStruct* sdo = DataObejct_FindSDO(dod_id, id);
    if (sdo == NULL) {
        return;
    }

    SDOargs req;
    req.status = DATA_OBJECT_SDO_REQU;
    req.data = data;
    req.size = size;
    DataObejct_SetSDOargs(sdo, &req);
}

SDOargs* DataObject_GetSDOres(uint8_t dod_id, uint16_t id)
{
    // TODO: exception
    SDOStruct* sdo = DataObejct_FindSDO(dod_id, id);
    if (sdo == NULL) {
        return NULL;
    }

    return &sdo->args;
}



uint16_t DataObejct_SetSDOargs(SDOStruct* sdo, SDOargs* args)
{
    // Copy status
    sdo->args.status = args->status;
    
    // Copy size
    int total_size = sdo->args.data_size * args->size;
    sdo->args.size = args->size;
    if (total_size <= 0) {
        return 0;
    }

    // Copy data
    if (sdo->args.data != NULL) {
        free(sdo->args.data);
        sdo->args.data = NULL;
    }
    sdo->args.data = malloc(total_size);
    memcpy(sdo->args.data, args->data, total_size);

    return total_size;
}


/*
  ___              ___   ___  ___     
 | __| _ ___ ___  |   \ / _ \|   \ ___
 | _| '_/ -_) -_) | |) | (_) | |) (_-<
 |_||_| \___\___| |___/ \___/|___//__/
                                      
*/
static void DataObject_FreePDO(uint8_t dod_id)
{
    DataObjectDictionary* dod = dods[dod_id];
    for (int i = 0; i < cvector_size(dod->pdo); ++i) {
        if (dod->pdo[i].name) {
            free(dod->pdo[i].name);
            dod->pdo[i].name = NULL;
        }

        if (dod->pdo[i].last_pub) {
            free(dod->pdo[i].last_pub);
            dod->pdo[i].last_pub = NULL;
        }     
    }
    cvector_free(dod->pdo);
}

static void DataObject_FreeSDO(uint8_t dod_id)
{
    DataObjectDictionary* dod = dods[dod_id];
    for (int i = 0; i < cvector_size(dod->sdo); ++i) {
        if (dod->sdo[i].name != NULL) {
            free(dod->sdo[i].name);
            dod->sdo[i].name = NULL;
        }

        if (dod->sdo[i].args.data != NULL) {
            free(dod->sdo[i].args.data);
            dod->sdo[i].args.data = NULL;
        }
    }
    cvector_free(dod->sdo);
}

void DataObject_FreeDODs()
{
    for (int i = 0; i < dods_size; ++i) {
        if (dods[i] != NULL) {
            DataObject_FreePDO(i);
            DataObject_FreeSDO(i);
            free(dods[i]->name);
            free(dods[i]);
            dods[i] = NULL;
        }
    }
    free(dods);
    dods = NULL;
    dods_size = 0;
}



/*
  ___                   _     ___   ___  ___     
 | __|_ ___ __  ___ _ _| |_  |   \ / _ \|   \ ___
 | _|\ \ / '_ \/ _ \ '_|  _| | |) | (_) | |) (_-<
 |___/_\_\ .__/\___/_|  \__| |___/ \___/|___//__/
         |_|                                     
*/

static int PDO_CSV_Row(uint8_t dod_id, int row, char* csv_str, uint16_t csv_len)
{
    PDOStruct* pdo = &FindDOD(dod_id)->pdo[row];
    return snprintf(csv_str, csv_len, "%d,%d,%d,%s\n", pdo->id, (uint8_t)pdo->type, pdo->len, pdo->name);
}

static int SDO_CSV_Row(uint8_t dod_id, int row, char* csv_str, uint16_t csv_len)
{
    SDOStruct* sdo = &FindDOD(dod_id)->sdo[row];
    return snprintf(csv_str, csv_len, "%d,%d,%s\n", sdo->id, (uint8_t)sdo->type, sdo->name);
}

int DataObject_DODlist(char** list_str, uint16_t* list_len)
{
    *list_len = 0;
    if (*list_str != NULL) {
        free(*list_str);
    }
    *list_str = NULL;

    if (dods_size <= 0) {
        char* str = "No DOD found.\n";
        *list_len = strlen(str)+1;
        *list_str = malloc(*list_len);
        strcpy(*list_str, str);
        return -1;
    }
    char sep;
    int name_len[dods_size];
    int cursor = 0;
    for (int i = 0; i < dods_size; ++i) {
        sep = (i == dods_size-1) ? '\n' : ',';
        name_len[i] = snprintf(NULL, 0, "%s%c",dods[i]->name, sep);


        cursor += name_len[i];
    }
    cursor += 1; // string end

    *list_len = cursor;
    *list_str = (char*)malloc(cursor);

    cursor = 0;
    for (int i = 0; i < dods_size; ++i) {
        sep = (i == dods_size-1) ? '\n' : ',';
        snprintf(*list_str + cursor, name_len[i]+1, "%s%c",dods[i]->name, sep);
        cursor += name_len[i];
    }

    return 0;
}

int DataObject_PDO2CSV(char** csv_str, uint16_t* csv_len, uint8_t dod_id)
{
    *csv_len = 0;
    if (*csv_str != NULL) {
        free(*csv_str);
    }
    *csv_str = NULL;

    DataObjectDictionary* dod = FindDOD(dod_id);
    if (dod == NULL) {
        // TODO: return fail reason
        char* str = "Exporting csv failed.\n";
        *csv_len = strlen(str)+1;
        *csv_str = malloc(*csv_len);
        strcpy(*csv_str, str);
        return -1;
    }

    int cursor = 0;
    int rows = cvector_size(dod->pdo);
    if (rows <= 0) {
        char* str = "No PDO found.\n";
        *csv_len = strlen(str)+1;
        *csv_str = malloc(*csv_len);
        strcpy(*csv_str, str);
        return -1;
    }

    int row_len[rows];
    for (int i = 0; i < rows; ++i) {
        row_len[i] = PDO_CSV_Row(dod_id, i, NULL, 0);
        cursor += row_len[i];
    }
    cursor += 1; // string end
    
    *csv_str = malloc(cursor);
    *csv_len = cursor;
    
    cursor = 0;
    for (int i = 0; i < rows; ++i) {
        cursor += PDO_CSV_Row(dod_id, i, *csv_str + cursor, row_len[i]+1);
    }

    return 0;
}

int DataObject_SDO2CSV(char** csv_str, uint16_t* csv_len, uint8_t dod_id)
{
    *csv_len = 0;
    if (*csv_str != NULL) {
        free(*csv_str);
    }
    *csv_str = NULL;

    DataObjectDictionary* dod = FindDOD(dod_id);
    if (dod == NULL) {
        // TODO: return fail reason
        char* str = "Exporting csv failed.\n";
        *csv_len = strlen(str)+1;
        *csv_str = malloc(*csv_len);
        strcpy(*csv_str, str);
        return -1;
    }

    int cursor = 0;
    int rows = cvector_size(dod->sdo);
    if (rows <= 0) {
        char* str = "No SDO found.\n";
        *csv_len = strlen(str)+1;
        *csv_str = malloc(*csv_len);
        strcpy(*csv_str, str);
        return -1;
    }

    int row_len[rows];
    for (int i = 0; i < rows; ++i) {
        row_len[i] = SDO_CSV_Row(dod_id, i, NULL, 0);
        cursor += row_len[i];
    }
    cursor += 1; // string end
    
    *csv_str = malloc(cursor);
    *csv_len = cursor;
    
    cursor = 0;
    for (int i = 0; i < rows; ++i) {
        cursor += SDO_CSV_Row(dod_id, i, *csv_str + cursor, row_len[i]+1);
    }

    return 0;
}

void DefaultDOD_GetDODs(SDOargs* req, SDOargs* res)
{
    if (DataObject_DODlist((char**)&res->data, &res->size) < 0) {
        res->status = DATA_OBJECT_SDO_FAIL;
    } else {
        res->status = DATA_OBJECT_SDO_SUCC;
    }
    
}

void DefaultDOD_GetPDOs(SDOargs* req, SDOargs* res)
{
    if (DataObject_PDO2CSV((char**)&res->data, &res->size, ((uint8_t*)req->data)[0]) < 0) {
        res->status = DATA_OBJECT_SDO_FAIL;
    } else {
        res->status = DATA_OBJECT_SDO_SUCC;
    }
}

void DefaultDOD_GetSDOs(SDOargs* req, SDOargs* res)
{
    if (DataObject_SDO2CSV((char**)&res->data, &res->size, ((uint8_t*)req->data)[0]) < 0) {
        res->status = DATA_OBJECT_SDO_FAIL;
    } else {
        res->status = DATA_OBJECT_SDO_SUCC;
    }
}
