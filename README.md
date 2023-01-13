# KeyLogger / UDP networking in C

## About

client program launch in the background with `hide_console` function --> comment it to display bytes sent from client
Press 'q' to kill the program. add more characters in the switch statement...

## Servers and clients

>There are two types of socket network applications: Servers and Clients.
General model for creating a streaming TCP/IP server and client

Server:

1. Initialize Winsock.
2. Create a socket.
3. Bind the socket.
4. Listen on the socket for a client.
5. Accept a connection from a client.
6. Receive and send data.
7. Disconnect.

Client:

1. Initialize Winsock.
2. Create a socket.
3. Connect to the server.
4. Send and Receive data.
5. Disconnect.

### Initializing Winsock

1. Create `WSDATA` object called wsaData.

    `WSADATA wsaData`

    What's WSADTA type ? A structure that contains information about Windows Sockets.

    [Read about WSDATA type](https://learn.microsoft.com/en-us/windows/win32/api/winsock/ns-winsock-wsadata).

2. Call `WSAStartup` and return its value as an integer.

    [Read about WSAStartup function](https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsastartup)

    ```

    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    ```

    >The WSADATA structure contains information about the Windows Sockets implementation. The MAKEWORD(2,2) parameter of WSAStartup makes a request for version 2.2 of Winsock on the system, and sets the passed version as the highest version of Windows Sockets support that the caller can use.

### Creating a Socket for the client

To Create the socket:

1. Declare addrInfo object that contains a sockaddr structure and initialize these values.

    The addrInfo structure hold host address information.

    ```
    
    typedef struct addrinfo {
        int             ai_flags;
        int             ai_family;
        int             ai_socktype;
        int             ai_protocol;
        size_t          ai_addrlen;
        char            *ai_canonname;
        struct sockaddr *ai_addr;
        struct addrinfo *ai_next;
    } ADDRINFOA, *PADDRINFOA;
    ```

    Contain the sockAddr structure.

    ```
    struct sockaddr {
            ushort  sa_family;
            char    sa_data[14];
    };

    struct sockaddr_in {
            short   sin_family;
            u_short sin_port;
            struct  in_addr sin_addr;
            char    sin_zero[8];
    };
    ```

    ```
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    ```

2. Call the getaddrinfo() function that requesting the Server

    ```
    #define DEFAULT_PORT "27015"

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }
    ```

3. Create `SOCKET` object

    SOCKET ConnectSocket = INVALID_SOCKET;

4. Call the socket() function and return its value

    ```
    // Attempt to connect to the first address returned by
    // the call to getaddrinfo
    ptr=result;

    // Create a SOCKET for connecting to server
    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
        ptr->ai_protocol);
    ```

    The socket function:

    ```
    SOCKET WSAAPI socket(
        [in] int af,
        [in] int type,
        [in] int protocol
    );
    ```

### Connect the socket

Call the connect() function and pass the created socket as parameter.

```
    // Connect to server.
    iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }

    // Should really try the next address returned by getaddrinfo
    // if the connect call failed
    // But for this simple example we just free the resources
    // returned by getaddrinfo and print an error message

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }
```

### Send and receive data

```
#define DEFAULT_BUFLEN 512

int recvbuflen = DEFAULT_BUFLEN;

const char *sendbuf = "this is a test";
char recvbuf[DEFAULT_BUFLEN];

int iResult;

// Send an initial buffer
iResult = send(ConnectSocket, sendbuf, (int) strlen(sendbuf), 0);
if (iResult == SOCKET_ERROR) {
    printf("send failed: %d\n", WSAGetLastError());
    closesocket(ConnectSocket);
    WSACleanup();
    return 1;
}

printf("Bytes Sent: %ld\n", iResult);

// shutdown the connection for sending since no more data will be sent
// the client can still use the ConnectSocket for receiving data
iResult = shutdown(ConnectSocket, SD_SEND);
if (iResult == SOCKET_ERROR) {
    printf("shutdown failed: %d\n", WSAGetLastError());
    closesocket(ConnectSocket);
    WSACleanup();
    return 1;
}

// Receive data until the server closes the connection
do {
    iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
    if (iResult > 0)
        printf("Bytes received: %d\n", iResult);
    else if (iResult == 0)
        printf("Connection closed\n");
    else
        printf("recv failed: %d\n", WSAGetLastError());
} while (iResult > 0);

```




