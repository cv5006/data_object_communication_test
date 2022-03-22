#include "data_object_protocol.h"


cvector_vector_type(DOP_Header) sdos_to_res;


static DOP_Header GetHeader(uint8_t* byte_arr)
{
    DOP_Header header;
    memcpy(&header, byte_arr, sizeof(DOP_Header));
    // TODO: Header validation
    return header;
}

static DOP_Header SetHeader(uint8_t* byte_arr, int* byte_len, uint8_t dod_id, uint8_t obj_type, uint16_t obj_id)
{
    // TODO: Header validation
    DOP_Header header;
    header.dod_id   = dod_id;
    header.obj_id   = obj_id;

    int len =sizeof(DOP_Header);
    memcpy(byte_arr, &header, len);
    *byte_len += len;
}

static uint8_t* Byte2PDOdata(uint8_t* byte_arr)
{
    return &byte_arr[sizeof(DOP_Header)];
}

static SDOargs Byte2SDOreq(uint8_t* byte_arr, uint16_t *byte_len)
{
    SDOargs req;
    *byte_len = 0;

    int idx = sizeof(req.status);
    int len = sizeof(req.size);

    memcpy(&req.size, &byte_arr[idx], len);
    *byte_len += len;

    req.data = &byte_arr[idx + len];


    req.status = byte_arr[0];
    *byte_len += 1;

    return req;
}


static int DOP_PubPDOSeq(DOP_Header* header, uint8_t* byte_arr)
{
    int header_size = sizeof(DOP_Header);
    // Publish PDO    
    PDOStruct* pdo = DataObejct_FindPDO(header->dod_id, header->obj_id);
    if (pdo == NULL) {
        return -2;
    }

    uint16_t n_bytes = DataObject_PubPDO(pdo, byte_arr + header_size);
    if (n_bytes < 0) { // Publish error
        return -1;
    } else if (n_bytes == 0) { // Nothing to publish
        return 0;
    }

    // Set PDO Header
    memcpy(byte_arr, header, header_size);
    return header_size + n_bytes; // written bytes
}

static int DOP_GetSDOargsSeq(DOP_Header* header, uint8_t* byte_arr)
{
    int byte_written = 0;
    // Set SDO Header
    memcpy(byte_arr, header, sizeof(DOP_Header));
    byte_written += sizeof(DOP_Header);

    // Return Response
    SDOStruct* sdo = DataObejct_FindSDO(header->dod_id, header->obj_id);
    if (sdo == NULL) {
        return -2;
    }

    memcpy(byte_arr + byte_written, &sdo->args.status, sizeof(sdo->args.status));
    byte_written += sizeof(sdo->args.status);
    memcpy(byte_arr + byte_written, &sdo->args.size,   sizeof(sdo->args.size));
    byte_written += sizeof(sdo->args.size);

    int data_len = sdo->args.size * sdo->args.data_size;
    memcpy(byte_arr + byte_written, sdo->args.data, data_len);
    byte_written += data_len;

    return byte_written;
}


static int DOP_SubPDOSeq(uint8_t* byte_arr)
{    
    int byte_read = 0;

    DOP_Header header = GetHeader(byte_arr);
    byte_read += sizeof(DOP_Header);

    PDOStruct* pdo = DataObejct_FindPDO(header.dod_id, header.obj_id);
    if (pdo == NULL) {
        return -2;
    }
    
    uint16_t n_bytes = DataObject_SubPDO(pdo, (void*)(byte_arr + byte_read));
    if (n_bytes < 0) {
        return -1;
    }
    byte_read += n_bytes;
    return byte_read;
}



static int DOP_SDOSeq(uint8_t* byte_arr)
{
    int byte_read = 0;
    DOP_Header header = GetHeader(byte_arr);
    byte_read += sizeof(DOP_Header);
    SDOStruct* sdo = DataObejct_FindSDO(header.dod_id, header.obj_id);
    if (sdo == NULL) {
        return -2;
    }

    uint16_t req_bytes = 0;
    SDOargs req = Byte2SDOreq(byte_arr + byte_read, &req_bytes);
    byte_read += req_bytes;

    uint16_t n_bytes = 0;
    if (req.status == DATA_OBJECT_SDO_REQU) {
        n_bytes = DataObject_CallSDO(sdo, &req);
        // Assign Response
        cvector_push_back(sdos_to_res, header);
    } else if(req.status == DATA_OBJECT_SDO_SUCC || req.status == DATA_OBJECT_SDO_FAIL) {
        n_bytes = DataObejct_SetSDOargs(sdo, &req);
        if (n_bytes < 0) {
            return -1;
        }
    } else {
        return -1;
    }

    byte_read += n_bytes;
    return byte_read;
}


// TxRx Protocols
int DOP_Tx(uint8_t* byte_arr, uint16_t* byte_len, DOP_Header* pdos, int n_pdos, DOP_Header* sdos, int n_sdos)
{
    int cursor = 0;
    
    /* PDO */
    // Set PDO character
    memcpy(&byte_arr[cursor], &(char){DOP_CHAR_PDO}, DOP_OBJ_CHAR_SIZE);
    cursor += DOP_OBJ_CHAR_SIZE;

    // Pub PDO
    int n_pdo_cursor = cursor;
    cursor += DOP_OBJ_NUMS_SIZE;
    uint8_t n_pdo = 0;
    if (n_pdos > 0) {
        for(int i = 0; i < n_pdos; ++i) {
            int temp_cursor = DOP_PubPDOSeq(&pdos[i], &byte_arr[cursor]);
            if (temp_cursor > 0) {
                cursor += temp_cursor;
                ++n_pdo;
            } else if (temp_cursor < 0) {
                return temp_cursor;
            }
        }
    } else {
        n_pdo = 0;
    }

    // Set # of PDOs
    memcpy(&byte_arr[n_pdo_cursor], &n_pdo, DOP_OBJ_NUMS_SIZE);

    /* SDO */
    // Set SDO character
    memcpy(&byte_arr[cursor], &(char){DOP_CHAR_SDO}, DOP_OBJ_CHAR_SIZE);
    cursor += DOP_OBJ_CHAR_SIZE;

    // Res SDOs
    int n_sdo_cursor = cursor;
    cursor += DOP_OBJ_NUMS_SIZE;
    uint8_t n_sdo = 0;
    if (sdos_to_res != NULL) {
        for(int i = 0; i < cvector_size(sdos_to_res); ++i) {
            int temp_cursor = DOP_GetSDOargsSeq(&sdos_to_res[i], &byte_arr[cursor]);
            if (temp_cursor > 0) {
                cursor += temp_cursor;
                ++n_sdo;
            } else if (temp_cursor < 0) {
                return temp_cursor;
            }
        }
        // TODO: use more efficient way than vector
        cvector_free(sdos_to_res);
        sdos_to_res = NULL;
    }

    // Req SDOs
    if (n_sdos > 0) {
        for(int i = 0; i < n_sdos; ++i) {
            int temp_cursor = DOP_GetSDOargsSeq(&sdos[i], &byte_arr[cursor]);
            if (temp_cursor > 0) {
                cursor += temp_cursor;
                ++n_sdo;
            } else if (temp_cursor < 0) {
                return temp_cursor;
            }
        }
    }

    // Set # of PDOs
    memcpy(&byte_arr[n_sdo_cursor], &n_sdo, DOP_OBJ_NUMS_SIZE);

    *byte_len = cursor;

    return 0;
}

int DOP_Rx(uint8_t* byte_arr, uint16_t byte_len)
{
    int cursor = 0;

    /* PDO */
    // Check PDO character
    if (byte_arr[cursor] != DOP_CHAR_PDO) {
        return -2;
    }
    cursor += DOP_OBJ_CHAR_SIZE;

    // Get # of PDOs
    uint8_t n_pdo = 0;
    memcpy(&n_pdo, &byte_arr[cursor], DOP_OBJ_NUMS_SIZE);
    cursor += DOP_OBJ_NUMS_SIZE;

    if (n_pdo > 0) {
        for (int i = 0; i < n_pdo; ++i) {
            int temp_cursor = DOP_SubPDOSeq(&byte_arr[cursor]);
            if (temp_cursor > 0) {
                cursor += temp_cursor;
            } else if (temp_cursor < 0) {
                return temp_cursor;
            }
        }
    }

    /* SDO */
    // Check SDO character
    if (byte_arr[cursor] != DOP_CHAR_SDO) {
        return -2;
    }
    cursor += DOP_OBJ_CHAR_SIZE;

    // Get # of SDOs
    uint16_t n_sdo = 0;
    memcpy(&n_sdo, &byte_arr[cursor], DOP_OBJ_NUMS_SIZE);
    cursor += DOP_OBJ_NUMS_SIZE;
    if (n_sdo > 0) {
        for (int i = 0; i < n_sdo; ++i) {            
            int temp_cursor = DOP_SDOSeq(&byte_arr[cursor]);
            if (temp_cursor > 0) {
                cursor += temp_cursor;
            } else if (temp_cursor < 0) {
                return temp_cursor;
            }
        }
    }

    return 0;
}
