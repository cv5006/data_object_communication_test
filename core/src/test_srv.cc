#include <iostream>

#include "unistd.h"
#include "netinet/in.h"
#include "sys/socket.h"
#include "arpa/inet.h"

#include <chrono>

#include "data_object_protocol.h"

using namespace std;

static int SocketSrvBegin(int* srvsock, int* clisock);
static int SocketRecv(int* clisock, char* rx_buff);
static int SocketSend(int* clisock, char* tx_buff, int tx_len);

int RxTx(int* clisock, char* tx_buff, int* tx_len, char* rx_buff, int* rx_len);

void TestSDO(SDOargs* req, SDOargs* res)
{
    for (int i=0; i < req->data_size; ++i) {
        cout << ((int16_t*)req->data)[i] << " ";        
    }
    cout << endl;

    res->status = DATA_OBJECT_SDO_SUCC;
    
}

int main(int argc, char* argv[])
{
    int srvsock, clisock;
    char rx_buff[1024];
    char tx_buff[1024];
    int rx_len = 0;
    int tx_len = 0;

    SocketSrvBegin(&srvsock, &clisock);

    DOP_Init();

    int32_t pdo1[10];
    uint32_t cnt = 0;
    DataObejct_CreateDOD(1, (char*)"test_dod");
    DataObejct_CreatePDO(1, 1, (char*)"pdo1", Int32, 10, pdo1);
    DataObejct_CreatePDO(1, 123, (char*)"count", UInt32, 1, &cnt);

    DataObejct_CreateDOD(2, (char*)"dod2");
    DataObejct_CreateSDO(2, 10, (char*)"TestSDO", Int16, TestSDO);

    DOP_Header pdos[1];
    pdos[0] = {1, 1};

    cout << "----------- Test Start -----------" << endl;
    while(1){
        cout << "Test #" << ++cnt << endl;
        if (RxTx(&clisock, tx_buff, &tx_len, rx_buff, &rx_len) < 0) {
            break;
        }

        cout << "pdo1: ";
        for (int i = 0; i < 10; ++i) {
            cout << pdo1[i] << ", ";
        }
        cout << endl;

        cout << "----------------------------------\n" << endl;
    }

    return 0;
}


static int SocketSrvBegin(int* srvsock, int* clisock)
{
    // Create
    *srvsock = socket(AF_INET, SOCK_STREAM, 0);
    if (*srvsock < 0) {
        cout << "[Socket Error] " << strerror(errno) << endl;
		return -1;
	}

    int on = 1;
    if(setsockopt(*srvsock, SOL_SOCKET, SO_REUSEADDR, (const void*) &on, sizeof(on)) < 0){
        cout << "[Socket Error] " << strerror(errno) << endl;
        return -1;
    }
    cout << "[Socket Created]" << endl;

    // Bind
    struct sockaddr_in srvaddr;
    srvaddr.sin_addr.s_addr = INADDR_ANY;
    srvaddr.sin_port = htons(8765);
    srvaddr.sin_family = AF_INET;

    if(bind(*srvsock, (struct sockaddr *) &srvaddr, sizeof(srvaddr)) < 0) {
        cout << "[Bind Error] " << strerror(errno) << endl;
        return -2;
    }
    cout << "[Bind Success]" << endl;

    // Listen
    cout << "Waiting for client..." << endl;
    if(listen(*srvsock, 1) < 0) {
        cout << "[Listen Error] " << strerror(errno) << endl;
        return -3;
    }

    struct sockaddr_in cliaddr;
    int cliaddr_size = sizeof(cliaddr);

    // Accept
    *clisock = accept(*srvsock, (struct sockaddr *) &cliaddr, (socklen_t *) &cliaddr_size);
    if(*clisock < 0) {
        cout << "[Accept Error] " << strerror(errno) << endl;
        return -1;
    }

    cout << "[Accept Success] " << inet_ntoa(cliaddr.sin_addr) << ":" << ntohs(cliaddr.sin_port) << endl;

    return 0;
}

static int SocketRecv(int* clisock, char* rx_buff)
{
    int recv_len = recv(*clisock, rx_buff, 1024, 0);
    if (recv_len > 0) {
        return recv_len;
    }

    cout << "[Client Disconnected] " << endl;
    close(*clisock);
    *clisock = 0;
    return -1;
}

static int SocketSend(int* clisock, char* tx_buff, int tx_len)
{
    if (send(*clisock, tx_buff, tx_len, 0) < 0) {
        return -1;
    }
    return 0;
}

int RxTx(int* clisock, char* tx_buff, int* tx_len, char* rx_buff, int* rx_len)
{
    int len = SocketRecv(clisock, rx_buff);
    if (len < 0) {
        return len;
    }
    if (DOP_Rx((uint8_t*)rx_buff, *rx_len) < 0) {
        cout << "Faulty Rx DOP" << endl;
    }

    if (DOP_Tx((uint8_t*)tx_buff, (uint16_t*)tx_len) < 0) {
        cout << "Faulty Tx DOP" << endl;
    }
    SocketSend(clisock, tx_buff, *tx_len);

    *rx_len = len;
    return len;
}