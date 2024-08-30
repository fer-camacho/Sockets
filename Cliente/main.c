#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

// Necesario para enlazar con Ws2_32.lib, Mswsock.lib, y Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512

void mandar_mensaje(SOCKET ConnectSocket, char* mensaje);
int recibir_mensaje(SOCKET ConnectSocket, char* recvbuf, int recvbuflen, char* mensaje);
int es_numerico(const char* str);

int __cdecl main(int argc, char **argv) {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL, *ptr = NULL, hints;

    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int iResult;

    if (argc != 3) { //VERIFICO PARAMETROS
        printf("usage: %s server-name port\n", argv[0]);
        return 1;
    }

    // Inicializar Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolver la dirección y puerto del servidor
    iResult = getaddrinfo(argv[1], argv[2], &hints, &result); //argv[2] CORRESPONDE AL PUERTO
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Intentar conectarse a una dirección hasta que tenga éxito
    for(ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Crear un SOCKET para conectarse al servidor
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Conectar al servidor.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// INICIO DEL MENU ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

    char opcion = 0;

    while (opcion != '3') {
        system("cls");
        printf("\n1. Generar usuario.\n");
        printf("\n2. Generar contrasena.\n");
        printf("\n3. Cerrar sesion.\n");
        printf("\n\nElija una opcion: ");

        fflush(stdin);
        scanf("%c", &opcion);

        switch(opcion) {
            case '1': {
                char longitud[20];
                system("cls");
                printf("Indique la longitud que desea (5-15): ");
                fflush(stdin);
                gets(longitud);
                while (strcmp(longitud, "volver") != 0) {
                    if (!es_numerico(longitud)) {
                        printf("Error: La longitud debe ser un numero.\n");
                        system("PAUSE");
                    }
                    char msj[] = "a-";
                    strcat(msj, longitud);
                    mandar_mensaje(ConnectSocket, msj);
                    char salida[] = "Cliente desconectado por inactividad\n";
                    iResult = recibir_mensaje(ConnectSocket, recvbuf, recvbuflen, salida);
                    system("PAUSE");
                    if (iResult == 0) { // EN EL CASO DE QUE EXPIRE LA SESION
                        break;
                    }

                    system("cls");
                    printf("Indique la longitud que desea (5-15): ");
                    fflush(stdin);
                    gets(longitud);
                }
                break;
            }
            case '2': {
                char longitud[20];
                system("cls");
                printf("Indique la longitud que desea (8-49): ");
                fflush(stdin);
                gets(longitud);
                while (strcmp(longitud, "volver") != 0) {
                    if (!es_numerico(longitud)) {
                        printf("Error: La longitud debe ser un numero.\n");
                        system("PAUSE");
                    }
                    char msj[] = "b-";
                    strcat(msj, longitud);
                    mandar_mensaje(ConnectSocket, msj);
                    char salida[] = "Cliente desconectado por inactividad\n";
                    iResult = recibir_mensaje(ConnectSocket, recvbuf, recvbuflen, salida);
                    system("PAUSE");
                    if (iResult == 0) { // EN EL CASO DE QUE EXPIRE LA SESION
                        break;
                    }

                    system("cls");
                    printf("Indique la longitud que desea (8-49): ");
                    fflush(stdin);
                    gets(longitud);
                }
                break;
            }
            case '3': {
                iResult = shutdown(ConnectSocket, SD_SEND);
                if (iResult == SOCKET_ERROR) {
                    printf("shutdown failed with error: %d\n", WSAGetLastError());
                    closesocket(ConnectSocket);
                    WSACleanup();
                    return 1;
                }
                break;
            }
            default: {
                if ((opcion != '1') || (opcion != '2') || (opcion != '3')) {
                    printf("\nLa opción elegida es inválida\n\n");
                    system("PAUSE");
                    break;
                }
            }
        }
    }

    // Limpieza
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}

void mandar_mensaje(SOCKET ConnectSocket, char* recvbuf) {
    int iResult = send(ConnectSocket, recvbuf, (int)strlen(recvbuf), 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return;
    }
    memset(recvbuf, 0, strlen(recvbuf));
}

int recibir_mensaje(SOCKET ConnectSocket, char* recvbuf, int recvbuflen, char* mensaje) {
    int iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
    if (iResult > 0)
        printf("%s\n", recvbuf);
    else if (iResult == 0)
        printf("%s\n", mensaje);
    else
        printf("recv failed with error: %d\n", WSAGetLastError());
    memset(recvbuf, 0, recvbuflen);
    return iResult;
}

int es_numerico(const char* str) {
    return strspn(str, "0123456789") == strlen(str);
}
