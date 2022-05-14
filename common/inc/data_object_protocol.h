#ifndef DATA_OBJECT_PROTOCOL_H_
#define DATA_OBJECT_PROTOCOL_H_

#if __cplusplus
extern "C"{
#endif

#include "data_object.h"

#pragma pack(1)

// TODO: buffer overflow check?

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

#define DOP_SUCCESS 0
#define DOP_FAULTY_PDO -1
#define DOP_FAULTY_SDO -2

#define DOP_CHAR_PDO 0x50
#define DOP_CHAR_SDO 0x53

#define DOP_OBJ_CHAR_SIZE 1
#define DOP_OBJ_NUMS_SIZE 1

#define DOP_SDO_SET_PDO_TO_SYNC 3

typedef struct DOP_Header
{
    uint8_t dod_id;
    uint16_t obj_id;
} DOP_Header;


void DOP_Init();

// TxRx Protocols
int DOP_Tx(uint8_t* byte_arr, uint16_t* byte_len);
int DOP_Rx(uint8_t* byte_arr, uint16_t byte_len);

// PDO Helper
void DOP_AddPDOtoSend(uint8_t dod_id, uint16_t obj_id);
void DOP_ClearPDOtoSend();

void DOP_AddPDOtoRecv(uint8_t dod_id, uint16_t obj_id);

// SDO Helper
void DOP_AddSDOtoReq(uint8_t dod_id, uint16_t obj_id, void* data, uint16_t size);

#if __cplusplus
}
#endif

#endif // DATA_OBJECT_PROTOCOL_H_
