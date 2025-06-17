// Aca vamos a armar toda la arquitectura de software

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "serverausftp.h"

// Esto es toda la parte de comunicacion
#define MSG_220 "220 srvFtp version 1.0\r\n"
#define MSG_331 "331 Password required for %s\r\n"
#define MSG_230 "230 User %s logged in\r\n"
#define MSG_530 "530 Login incorrect\r\n"
#define MSG_221 "221 Goodbye\r\n"
#define MSG_550 "550 %s: no such file or directory\r\n"
#define MSG_299 "299 File %s size %ld bytes\r\n"
#define MSG_226 "226 Transfer complete\r\n"

// Primero declaro la parte de recepción de comandos

/*
 * recv_cmd recepciona un comando desde el socketDescriptor.
 * recv hace el receive y es una llamada definida en sysocket.
 * strtock se comporta de mandera distitna en cada llamada / devuelve tokens.
 * En operation obtengo la operacion, los cuales serian los comandos basicos de ftp, es decir que es lo que me envió el cliente.
 */
int recv_cmd(int socketDescriptor, char *operation, char *param)
{
    char buffer[BUFSIZE];
    char *token;

    if (recv(socketDescriptor, buffer, BUFSIZE, 0) < 0)
    {
        fprintf(stderr, "error receiving data");
        return 1;
    }

    buffer[strcspn(buffer, "\r\n")] = 0;
    token = strtok(buffer, " ");

    if (token == NULL || strlen(token) < 4)
    {
        fprintf(stderr, "not valid ftp command");
        return 1;
    }
    else
    {
        strcpy(operation, token);
        token = strtok(NULL, " "); // Aca le indicamos que trabaja con el buffer que tenia.
        // Este if y endif significa que si cuando compilamos en el gcc ponemos una -D debug, todo lo que pongamos ahi adentro se compila son esto.
        #if DEBUG
        printf("par %s\n", token);
        #endif
        if (token != NULL)
            strcpy(param, token);
    }
}