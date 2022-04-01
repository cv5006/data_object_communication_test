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


int main(int argc, char* argv[])
{
    int srvsock;
    char rx_buff[1024];
    char tx_buff[1024];
    int rx_len = 0;
    int tx_len = 0;

    SocketCliBegin(&srvsock);

    tx_len = 10;
    SocketSend(&srvsock, tx_buff, 10);
    
    for (int i = 0; i < tx_len; ++i) {
        cout << +tx_buff[i] << " " << flush;
    } 
    cout << endl;

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