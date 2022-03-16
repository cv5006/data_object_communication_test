#ifndef DATA_OBJECT_PROTOCOL_H_
#define DATA_OBJECT_PROTOCOL_H_

#include "data_object.h"

#pragma pack(1)

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


#define DATA_OBJECT_CHAR_PDO 0x50
#define DATA_OBJECT_CHAR_SDO 0x53

#define DATA_OBJECT_OBJ_CHAR_SIZE 1
#define DATA_OBJECT_OBJNUMS__SIZE 1

typedef struct DataObjectHeader
{
    uint8_t dod_id;
    uint16_t obj_id;
} DataObjectHeader;

cvector_vector_type(DataObjectHeader) pdos_to_pub;
cvector_vector_type(DataObjectHeader) sdos_to_res;


// TxRx Protocols
int DataObject_TxProtocol(uint8_t* byte_arr, uint16_t* byte_len);

int DataObject_RxProtocol(uint8_t* byte_arr, uint16_t byte_len);





#endif // DATA_OBJECT_PROTOCOL_H_
