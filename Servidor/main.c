#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Necesario para enlazar con Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "5000"

void mandar_mensaje(SOCKET ClientSocket, char* mensaje, int recvbuflen);

char generar_vocal();
char generar_consonante();
char generar_alfanumerico();
void generar_nombre_usuario(SOCKET ClientSocke, int longitud);
void generar_contrasena(SOCKET ClientSocket, int longitud);
void validar_y_generar(SOCKET ClientSocket, char tipo, int longitud);
void procesar_solicitud(SOCKET ClientSocket, char* recvbuf);

int __cdecl main(void)
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    // Inicializa Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resuelve la dirección y el puerto del servidor
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Crea un SOCKET para que el servidor escuche las conexiones de los clientes.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Establece el timeout
    int timeOut = 120000;
    if (setsockopt(ListenSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeOut, sizeof(timeOut))) {
        closesocket(ListenSocket);
        WSACleanup();
    }

    // Configura el socket TCP para escuchar
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    while(TRUE){
        // Acepta una conexión de un cliente
        printf("\n##########################");
        printf("\n\nEsperando conexion..\n");

        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        printf("Conexion aceptada.\n");

        // Recibe hasta que la conexión sea cerrada
        do {
            memset(recvbuf, 0, recvbuflen);
            iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
            if (iResult == SOCKET_ERROR) {
                if (WSAETIMEDOUT == WSAGetLastError()) {
                    printf("Conexion cerrada por inactividad.\n");
                }
            } else {
                if (iResult > 0) {
                    procesar_solicitud(ClientSocket, recvbuf);
                } else if (iResult == 0) {
                    printf("Conexion cerrada.\n");
                } else {
                    printf("recv failed with error: %d\n", WSAGetLastError());
                    closesocket(ClientSocket);
                    WSACleanup();
                    return 1;
                }
            }
        } while (iResult > 0);

        // Cierra la conexión
        iResult = shutdown(ClientSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
    }

    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}

void mandar_mensaje(SOCKET ClientSocket, char* recvbuf, int recvbuflen) {
    int iResult = send(ClientSocket, recvbuf, recvbuflen, 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return;
    }
    memset(recvbuf, 0, recvbuflen);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// GENERAR USUARIO Y PASSWORD  ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

char generar_vocal() {
    char vocales[] = "aeiou";
    return vocales[rand() % 5];
}

char generar_consonante() {
    char consonantes[] = "bcdfghjklmnpqrstvwxyz";
    return consonantes[rand() % 21];
}

char generar_alfanumerico() {
    char alfanumerico[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    return alfanumerico[rand() % 62];
}

void generar_nombre_usuario(SOCKET ClientSocket, int longitud) {
    char nombre_usuario[longitud + 1];
    int empezar_por_vocal = rand() % 2;

    for (int i = 0; i < longitud; i++) {
        if ((i % 2 == 0 && empezar_por_vocal) || (i % 2 != 0 && !empezar_por_vocal)) {
            nombre_usuario[i] = generar_vocal();
        } else {
            nombre_usuario[i] = generar_consonante();
        }
    }
    nombre_usuario[longitud] = '\0';
    printf("Respuesta: %s\n", nombre_usuario);
    mandar_mensaje(ClientSocket, nombre_usuario, strlen(nombre_usuario));
}

void generar_contrasena(SOCKET ClientSocket, int longitud) {
    char contrasena[longitud + 1];
    for (int i = 0; i < longitud; i++) {
        contrasena[i] = generar_alfanumerico();
    }
    contrasena[longitud] = '\0';
    printf("Respuesta: %s\n", contrasena);
    mandar_mensaje(ClientSocket, contrasena, strlen(contrasena));
}

void validar_y_generar(SOCKET ClientSocket, char tipo, int longitud) {
    char error[100];
    if (tipo == 'a') {
        if (longitud < 5 || longitud > 15) {
            printf("Solicitud: Generar usuario de %d caracteres. Se envia msj de error.\n", longitud);
            snprintf(error, sizeof(error), "Error: Longitud del usuario debe ser entre 5 y 15 caracteres.");
            mandar_mensaje(ClientSocket, error, strlen(error));
        } else {
            printf("Solicitud: Generar usuario de %d caracteres.\n", longitud);
            generar_nombre_usuario(ClientSocket, longitud);
        }
    } else if (tipo == 'b') {
        if (longitud < 8 || longitud >= 50) {
            printf("Solicitud: Generar contrasena de %d caracteres. Se envia msj de error.\n", longitud);
            snprintf(error, sizeof(error), "Error: Longitud de la contrasena debe ser entre 8 y 49 caracteres.");
            mandar_mensaje(ClientSocket, error, strlen(error));
        } else {
            printf("Solicitud: Generar contrasena de %d caracteres.\n", longitud);
            generar_contrasena(ClientSocket, longitud);
        }
    }
}

void procesar_solicitud(SOCKET ClientSocket, char* recvbuf) {
    char tipo = recvbuf[0];  // 'a' para usuario, 'b' para contraseña
    char longitud_str[20];
    strncpy(longitud_str, recvbuf + 2, 20);
    longitud_str[19] = '\0';  // Asegurar el final de la cadena
    int longitud = atoi(longitud_str);
    validar_y_generar(ClientSocket, tipo, longitud);
}
