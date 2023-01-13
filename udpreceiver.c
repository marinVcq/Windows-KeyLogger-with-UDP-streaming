#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

#define DEFAULT_PORT 8080

void main(void)
{
    WSADATA wsaData;
    SOCKET ReceivingSocket;
    SOCKADDR_IN ReceiverAddr;
    char *buf = malloc(50);
    SOCKADDR_IN SenderAddr;
    int SenderAddrSize = sizeof(SenderAddr);
    int Ret;
    BOOL Continue = TRUE;

    // Initialize Winsock version 2.2
    if ((Ret = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
    {
        printf("ERROR: WSAStartup failed with error %d\n", Ret);
        return;
    }

    // Create a new socket to receive datagrams on.
    if ((ReceivingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
    {
        printf("ERROR: socket failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    // Setup a SOCKADDR_IN structure that will tell bind that we
    // want to receive datagrams from all interfaces using port 8080
    ReceiverAddr.sin_family = AF_INET;
    ReceiverAddr.sin_port = htons(DEFAULT_PORT);
    ReceiverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Associate the address information with the socket using bind.
    if (bind(ReceivingSocket, (SOCKADDR *)&ReceiverAddr, sizeof(ReceiverAddr)) == SOCKET_ERROR)
    {
        printf("ERROR: bind failed with error %d\n", WSAGetLastError());
        closesocket(ReceivingSocket);
        WSACleanup();
        return;
    }

    printf("We are ready to receive datagram from any interface on port %d...\n",
           DEFAULT_PORT);

    // At this point you can receive datagrams on your bound socket.
    while (Continue)
    {
        if ((Ret = recvfrom(ReceivingSocket, buf, 50, 0,
                            (SOCKADDR *)&SenderAddr, &SenderAddrSize)) == SOCKET_ERROR)
        {
            printf("ERROR: recvfrom failed with error %d\n", WSAGetLastError());
            closesocket(ReceivingSocket);
            WSACleanup();
            return;
        }
        printf("%s", buf);
    }

    // Cslose the socket.
    closesocket(ReceivingSocket);

    // When your application is finished call WSACleanup.
    WSACleanup();
}
