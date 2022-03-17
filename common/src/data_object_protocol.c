#include "data_object_protocol.h"


cvector_vector_type(DOP_Header) pdos_to_pub;
cvector_vector_type(DOP_Header) sdos_to_res;


static DOP_Header GetHeader(uint8_t* byte_arr)
{
    DOP_Header header;
    memcpy(&header, byte_arr, sizeof(DOP_Header));
    // TODO: Header validation
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


static int DOP_PubPDOSeq(DOP_Header* header, uint8_t* byte_arr)
{
    int header_size = sizeof(DOP_Header);
    // Publish PDO    
    uint16_t n_bytes = DataObject_PubPDO(header->dod_id, header->obj_id, byte_arr + header_size);
    if (n_bytes < 0) { // Publish error
        return -1;
    } else if (n_bytes == 0) { // Nothing to publish
        return 0;
    }

    // Set PDO Header
    memcpy(byte_arr, header, header_size);
    return header_size + n_bytes; // written bytes
}

static int DOP_ResSDOSeq(DOP_Header* header, uint8_t* byte_arr)
{
    int byte_written = 0;
    // Set SDO Header
    memcpy(byte_arr, &header, sizeof(DOP_Header));
    byte_written += sizeof(DOP_Header);

    // Return Response
    SDOargs res = DataObject_PopSDOReponse(header->dod_id, header->obj_id);
    memcpy(byte_arr + byte_written, &res.result, sizeof(res.result));
    byte_written += sizeof(res.result);        
    memcpy(byte_arr + byte_written, &res.size,   sizeof(res.size));
    byte_written += sizeof(res.size);
    memcpy(byte_arr + byte_written, &res.data,   res.size);        
    byte_written += res.size;

    return byte_written;
}


static int DOP_SubPDOSeq(uint8_t* byte_arr)
{    
    int byte_read = 0;

    DOP_Header header = GetHeader(byte_arr);
    byte_read += sizeof(DOP_Header);

    uint16_t n_bytes = DataObject_SubPDO(header.dod_id, header.obj_id, (void*)(byte_arr + byte_read));
    if (n_bytes < 0) {
        return -1;
    }
    byte_read += n_bytes;
    return byte_read;
}

static int DOP_CallSDOSeq(uint8_t* byte_arr, DOP_Header* called_header)
{
    int byte_read = 0;
    DOP_Header header = GetHeader(byte_arr);
    byte_read += sizeof(DOP_Header);

    SDOargs req = Byte2SDOreq(byte_arr + byte_read);
    uint16_t n_bytes = DataObject_CallSDO(header.dod_id, header.obj_id, &req);
    if (n_bytes < 0) {
        return -1;
    }
    *called_header = header;
    byte_read += n_bytes;
    return byte_read;
}


// TxRx Protocols
int DOP_Tx(uint8_t* byte_arr, uint16_t* byte_len)
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
    for(int i = 0; i < cvector_size(pdos_to_pub); ++i) {
        int temp_cursor = DOP_PubPDOSeq(&pdos_to_pub[i], &byte_arr[cursor]);
        if (temp_cursor > 0) {
            cursor += temp_cursor;
            ++n_pdo;
        } else if (temp_cursor < 0) {
            return temp_cursor;
        }
    }

    // Set # of PDOs
    memcpy(&byte_arr[n_pdo_cursor], &n_pdo, DOP_OBJ_NUMS_SIZE);

    /* SDO */
    // Set SDO character
    memcpy(&byte_arr[cursor], &(char){DOP_CHAR_SDO}, DOP_OBJ_CHAR_SIZE);
    cursor += DOP_OBJ_CHAR_SIZE;

    // Set # of SDOs
    int n_sdo_cursor = cursor;
    cursor += DOP_OBJ_NUMS_SIZE;
    uint8_t n_sdo = 0;
    for(int i = 0; i < cvector_size(sdos_to_res); ++i) {
        int temp_cursor = DOP_ResSDOSeq(&sdos_to_res[i], &byte_arr[cursor]);
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


    // Set # of PDOs
    memcpy(&byte_arr[n_sdo_cursor], &n_sdo, DOP_OBJ_NUMS_SIZE);

    *byte_len = cursor;
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

    for (int i = 0; i < n_pdo; ++i) {
        int temp_cursor = DOP_SubPDOSeq(&byte_arr[cursor]);
        if (temp_cursor > 0) {
            cursor += temp_cursor;
        } else if (temp_cursor < 0) {
            return temp_cursor;
        }
    }

    /* SDO */
    // Check SDO character
    if (byte_arr[cursor] != DOP_CHAR_SDO) {
        return -2;
    }
    cursor += DOP_OBJ_CHAR_SIZE;

    // Get # of SDOs
    uint16_t n_sdo;
    memcpy(&n_sdo, &byte_arr[cursor], DOP_OBJ_NUMS_SIZE);
    cursor += DOP_OBJ_NUMS_SIZE;

    for (int i = 0; i < n_pdo; ++i) {
        DOP_Header called_header;
        int temp_cursor = DOP_CallSDOSeq(&byte_arr[cursor], &called_header);
        if (temp_cursor > 0) {
            // Assign Response
            cvector_push_back(sdos_to_res, called_header);
            cursor += temp_cursor;
        } else if (temp_cursor < 0) {
            return temp_cursor;
        }
    }

    return -1;
}


int DOP_PDOSyncStart(uint8_t dod_id, uint16_t obj_id)
{
    DOP_Header header;
    header.dod_id = dod_id;
    header.obj_id = obj_id;
    cvector_push_back(pdos_to_pub, header);
}


int DOP_PDOSyncClear()
{
    // TODO: use more efficient way than vector
    cvector_free(pdos_to_pub);
    pdos_to_pub = NULL;
}