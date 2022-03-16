#include "data_object_protocol.h"

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
    int idx = sizeof(req.result);
    int len = sizeof(req.size);
    memcpy(&req.size, &byte_arr[idx], len);
    req.data = &byte_arr[idx + len];

    return req;
}



// TxRx Protocols
int DataObject_TxProtocol(uint8_t* byte_arr, uint16_t* byte_len)
{
    int cursor = 0;
    
    /* PDO */
    // Set PDO character
    memcpy(&byte_arr[cursor], &(char){DATA_OBJECT_CHAR_PDO}, DATA_OBJECT_OBJ_CHAR_SIZE);
    cursor += DATA_OBJECT_OBJ_CHAR_SIZE;

    // Set # of PDOs
    uint8_t n_pdo = cvector_size(pdos_to_pub);
    memcpy(&byte_arr[cursor], &n_pdo, DATA_OBJECT_OBJNUMS__SIZE);
    cursor += DATA_OBJECT_OBJNUMS__SIZE;

    for(int i = 0; i < cvector_size(pdos_to_pub); ++i) {
        // Set PDO Header
        DataObjectHeader header = pdos_to_pub[i];
        memcpy(&byte_arr[cursor], &header, sizeof(DataObjectHeader));
        cursor += sizeof(DataObjectHeader);

        // Publish PDO
        uint16_t n_bytes = DataObject_PubPDO(header.dod_id, header.obj_id, &byte_arr[cursor]);
        if (n_bytes < 0) {
            return -1;
        }
        cursor += n_bytes;
    }

    /* SDO */
    // Set PDO character
    memcpy(&byte_arr[cursor], &(char){DATA_OBJECT_CHAR_SDO}, DATA_OBJECT_OBJ_CHAR_SIZE);
    cursor += DATA_OBJECT_OBJ_CHAR_SIZE;

    // Set # of SDOs
    uint8_t n_sdo = cvector_size(sdos_to_res);
    memcpy(&byte_arr[cursor], &n_sdo, DATA_OBJECT_OBJNUMS__SIZE);
    cursor += DATA_OBJECT_OBJNUMS__SIZE;

    for(int i = 0; i < cvector_size(sdos_to_res); ++i) {
        // Set SDO Header
        DataObjectHeader header = sdos_to_res[i];
        memcpy(&byte_arr[cursor], &header, sizeof(DataObjectHeader));
        cursor += sizeof(DataObjectHeader);

        // Return Response
        SDOargs res = DataObject_GetSDOReponse(header.dod_id, header.obj_id);
        memcpy(&byte_arr[cursor], &res.result, sizeof(res.result)); cursor += sizeof(res.result);        
        memcpy(&byte_arr[cursor], &res.size,   sizeof(res.size));   cursor += sizeof(res.size);
        memcpy(&byte_arr[cursor], &res.data,   res.size);           cursor += res.size;
    }
}

int DataObject_RxProtocol(uint8_t* byte_arr, uint16_t byte_len)
{
    int cursor = 0;

    /* PDO */
    // Check PDO character
    if (byte_arr[cursor] != DATA_OBJECT_CHAR_PDO) {
        return -2;
    }
    cursor += DATA_OBJECT_OBJ_CHAR_SIZE;

    // Get # of PDOs
    uint8_t n_pdo = 0;
    memcpy(&n_pdo, &byte_arr[cursor], DATA_OBJECT_OBJNUMS__SIZE);
    cursor += DATA_OBJECT_OBJNUMS__SIZE;

    for (int i = 0; i < n_pdo; ++i) {
        // Get PDO Header
        DataObjectHeader header = GetHeader(&byte_arr[cursor]);
        cursor += sizeof(DataObjectHeader);

        // Subscribe PDO
        uint16_t n_bytes = DataObject_SubPDO(header.dod_id, header.obj_id, (void*)&byte_arr[cursor]);
        if (n_bytes < 0) {
            return -1;
        }
        cursor += n_bytes;
    }

    /* SDO */
    // Check SDO character
    if (byte_arr[cursor] != DATA_OBJECT_CHAR_SDO) {
        return -2;
    }
    cursor += DATA_OBJECT_OBJ_CHAR_SIZE;

    // Get # of SDOs
    uint16_t n_sdo;
    memcpy(&n_sdo, &byte_arr[cursor], DATA_OBJECT_OBJNUMS__SIZE);
    cursor += DATA_OBJECT_OBJNUMS__SIZE;

    for (int i = 0; i < n_pdo; ++i) {
        // Get SDO Header
        DataObjectHeader header = GetHeader(&byte_arr[cursor]);
        cursor += sizeof(DataObjectHeader);

        // Call SDO
        SDOargs req = Byte2SDOreq(&byte_arr[cursor]);
        uint16_t n_bytes = DataObject_CallSDO(header.dod_id, header.obj_id, &req);
        if (n_bytes < 0) {
            return -1;
        }
        cursor += (n_bytes + sizeof(req.result) + sizeof(req.size));

        // Assign Response
        cvector_push_back(sdos_to_res, header);
    }

    return -1;
}