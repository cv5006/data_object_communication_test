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
        DataObject_CallSDO(header.dod_id, header.obj_id, &req);

        return 0;
    }

    return -1;
}