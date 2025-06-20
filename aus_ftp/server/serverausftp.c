// Aca en este archivo va a estar el main

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "serverausftp.h"

#define PTODEFAULT 21

int main(int argc, char *argv[])
{
    int port;
    // Hacemos el chequeo de errores de argumento
    if (argc > 2) // Si la cantidad de argumentos son mas que 2...
    {
        fprintf(stderr, "Error: número de argumento incorrecto.\n");
        return 1;
    }

    if (argc == 2) // Si la cantidad de argumentos es igual a 2..
    {
        port = atoi(argv[1]); // Se asigna al numero de puerto.
        if (port <= 0 || port > 65535)
        {
            fprintf(stderr, "Error: el número de puerto debe ser un entero entre 1 y 65535.\n");
            return 1;
        }
    }
    else
    {
        port = PTODEFAULT; // Puerto por default ftp.
    }

    int mastersocket, slavesocket;
    struct sockaddr_in masteraddr, slaveaddr;
    socklen_t slaveaddrlen;
    char user_name[BUFSIZE];
    char user_pass[BUFSIZE];
    char buffer[BUFSIZE];
    char command[BUFSIZE];
    int data_len;

    mastersocket = socket(AF_INET, SOCK_STREAM, 0);
    masteraddr.sin_family = AF_INET;
    masteraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    masteraddr.sin_port = htons(port);
    bind(mastersocket, (struct sockaddr *)&masteraddr, sizeof(masteraddr));
    listen(mastersocket, 5);

    while (1)
    {
        slaveaddrlen = sizeof(slaveaddr);
        slavesocket = accept(mastersocket, (struct sockaddr *)&slaveaddr, &slaveaddrlen);
        printf("%s\n", MSG_220);
        if (send(slavesocket, MSG_220, sizeof(MSG_220) - 1, 0) != sizeof(MSG_220) - 1)
        {
            close(slavesocket);
            fprintf(stderr, "Error: no se puede enviar el mensaje.\n");
            break;
        }
        if (recv_cmd(slavesocket, command, user_name) != 0)
        {
            close(slavesocket);
            fprintf(stderr, "Error: no se pudo recibir el comando USER.\n");
            break;
        }
        if (strcmp(command, "USER") != 0)
        {
            close(slavesocket);
            fprintf(stderr, "Error: se esperaba el comando USER.\n");
            continue;
        }
        data_len = snprintf(buffer, BUFSIZE, MSG_331, user_name);
        if (send(slavesocket, buffer, data_len, 0) < 0)
        {
            close(slavesocket);
            fprintf(stderr, "Error: no se pudo enviar el mensaje MSG_331.\n");
            break;
        }
        if (recv_cmd(slavesocket, command, user_pass) != 0)
        {
            close(slavesocket);
            fprintf(stderr, "Error: no se pudo recibir el comando PASS.\n");
            break;
        }
        if (strcmp(command, "PASS") != 0)
        {
            close(slavesocket);
            fprintf(stderr, "Error: se esperaba el comando PASS.\n");
            continue;
        }
        if (!check_credentials(user_name, user_pass))
        {
            data_len = snprintf(buffer, BUFSIZE, MSG_530);
            if (send(slavesocket, buffer, data_len, 0) < 0)
            {
                close(slavesocket);
                fprintf(stderr, "Error: no se pudo enviar el mensaje MSG_530.\n");
                break;
            }
            close(slavesocket);
            continue;
        }
        data_len = snprintf(buffer, BUFSIZE, MSG_230, user_name);
        if (send(slavesocket, buffer, data_len, 0) < 0)
        {
            close(slavesocket);
            fprintf(stderr, "Error: no se pudo enviar el mensaje MSG_230.\n");
            break;
        }
    }
    close(mastersocket); // Esto no sucede al menos que le mandemos una señal, como un kill por ejemplo.
    return 0;
}