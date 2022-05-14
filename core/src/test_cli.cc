#include <iostream>

#include "unistd.h"
#include "netinet/in.h"
#include "sys/socket.h"
#include "arpa/inet.h"

#include <chrono>

#include "data_object_protocol.h"

using namespace std;

static int SocketCliBegin(int* srvsock);
static int SocketRecv(int* srvsock, char* rx_buff);
static int SocketSend(int* srvsock, char* tx_buff, int tx_len);

static void TxRx(int* srvsock, char* tx_buff, int* tx_len, char* rx_buff, int* rx_len);

int main(int argc, char* argv[])
{
    int srvsock;
    char rx_buff[1024];
    char tx_buff[1024];
    int rx_len = 0;
    int tx_len = 0;

    SocketCliBegin(&srvsock);

    DOP_Init();

    int32_t pdo1[10];
    uint32_t cnt = 123456;
    char dump_char;
    DataObejct_CreateDOD(1, (char*)"test_dod");
    DataObejct_CreatePDO(1, 1, (char*)"pdo1", Int32, 10, pdo1);
    DataObejct_CreatePDO(1, 123, (char*)"count", UInt32, 1, &cnt);
    DataObejct_CreatePDO(1, 456, (char*)"cli_only_pdo", Char, 1, &dump_char);

    cout << "----------- Test Start -----------" << endl;
    /* Req DOD List */
    cout << "Test #1: Request DOD List"<< endl;
    DOP_AddSDOtoReq(DATA_OBJECT_DEFAULT_DOD, DATA_OBJECT_SDO_GET_DOD_LIST, NULL, 0);

    TxRx(&srvsock, tx_buff, &tx_len, rx_buff, &rx_len);
    cout << (char*)DataObject_GetSDOres(DATA_OBJECT_DEFAULT_DOD, DATA_OBJECT_SDO_GET_DOD_LIST)->data << endl;
    cout << "count: " << cnt << endl;
    cout << "----------------------------------\n" << endl;

    /* Set PDO to sync */
    cout << "Test #2: Set PDO List"<< endl;
    for (int i = 0; i < 10; ++i) { pdo1[i] = i; }
    DOP_AddPDOtoRecv(1, 123);
    DOP_AddPDOtoSend(1, 1);
    TxRx(&srvsock, tx_buff, &tx_len, rx_buff, &rx_len);
    cout << "count: " << cnt << endl;
    cout << "----------------------------------\n" << endl;

    /* Req PDO List */
    cout << "Test #3: Request PDO List"<< endl;
    for (int i = 0; i < 10; ++i) { pdo1[i] = i*2; }
    uint8_t sdo_data = 1;
    DOP_AddSDOtoReq(DATA_OBJECT_DEFAULT_DOD, DATA_OBJECT_SDO_GET_PDO_LIST, (void*)&sdo_data, 1);

    TxRx(&srvsock, tx_buff, &tx_len, rx_buff, &rx_len);
    cout << (char*)DataObject_GetSDOres(DATA_OBJECT_DEFAULT_DOD, DATA_OBJECT_SDO_GET_PDO_LIST)->data << endl;
    cout << "count: " << cnt << endl;
    cout << "----------------------------------\n" << endl;

    /* Req SDO List */
    cout << "Test #4: Request SDO List"<< endl;
    for (int i = 0; i < 10; ++i) { pdo1[i] = i*3; }
    sdo_data = DATA_OBJECT_DEFAULT_DOD;
    DOP_AddSDOtoReq(DATA_OBJECT_DEFAULT_DOD, DATA_OBJECT_SDO_GET_SDO_LIST, (void*)&sdo_data, 1);

    TxRx(&srvsock, tx_buff, &tx_len, rx_buff, &rx_len);
    cout << (char*)DataObject_GetSDOres(DATA_OBJECT_DEFAULT_DOD, DATA_OBJECT_SDO_GET_SDO_LIST)->data << endl;
    cout << "count: " << cnt << endl;
    cout << "----------------------------------\n" << endl;

    /* Invalid PDO */
    cout << "Test #5: Request invalid PDO"<< endl;
    for (int i = 0; i < 10; ++i) { pdo1[i] = i*4; }
    DOP_AddPDOtoSend(1, 456);

    TxRx(&srvsock, tx_buff, &tx_len, rx_buff, &rx_len);
    cout << "count: " << cnt << endl;
    cout << "----------------------------------\n" << endl;
    return 0;
}


static int SocketCliBegin(int* srvsock)
{
    // Create
    *srvsock = socket(AF_INET, SOCK_STREAM, 0);
    if (*srvsock < 0) {
        cout << "[Socket Error] " << strerror(errno) << endl;
		return -1;
	}

    // Connect
    struct sockaddr_in srvaddr;
    srvaddr.sin_addr.s_addr = INADDR_ANY;
    srvaddr.sin_port = htons(8765);
    srvaddr.sin_family = AF_INET;

    if(inet_pton(AF_INET, "127.0.0.1", &srvaddr.sin_addr) <= 0) {
        cout << "[IP Error] Invalid address" << endl;
        return -1;
    }

    if(connect(*srvsock, (struct sockaddr *) &srvaddr, sizeof(srvaddr)) < 0) {
        cout << "[Connect Error] " << strerror(errno) << endl;
        return -2;
    }
    cout << "[Connect Success]" << endl;

    return 0;
}

static int SocketRecv(int* clisock, char* rx_buff)
{
    int recv_len = recv(*clisock, rx_buff, 1024, 0);
    
    if (recv_len == 0) {
        close(*clisock);
        *clisock = 0;
        return -1;
    }

    return recv_len;
}

static int SocketSend(int* srvsock, char* tx_buff, int tx_len)
{
    if (send(*srvsock, tx_buff, tx_len, 0) < 0) {
        return -1;
    }
    return 0;
}

void TxRx(int* srvsock, char* tx_buff, int* tx_len, char* rx_buff, int* rx_len)
{
    if (DOP_Tx((uint8_t*)tx_buff, (uint16_t*)tx_len) < 0) {
        cout << "Faulty Tx DOP" << endl;
        return;
    }

    SocketSend(srvsock, tx_buff, *tx_len);
    *rx_len = SocketRecv(srvsock, rx_buff);

    if (DOP_Rx((uint8_t*)rx_buff, *rx_len) < 0) {
        cout << "Faulty Rx DOP" << endl;
        return;
    }
}