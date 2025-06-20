// Aca vamos a armar toda la arquitectura de software

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "serverpi.h"
#include "serverausftp.h"

int is_valid_command(const char *command)
{
    int i = 0;
    while (valid_commands[i] != NULL)
    {
        if (strcmp(command, valid_commands[i]) == 0)
        {
            return arg_commands[i];
        }
        i++;
    }
    return -1;
}

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
    int args_number;

    if (recv(socketDescriptor, buffer, BUFSIZE, 0) < 0)
    {
        fprintf(stderr, "Error: no se puede recibir el comando.\n");
        return 1;
    }

    buffer[strcspn(buffer, "\r\n")] = 0;
    token = strtok(buffer, " ");
    if (token == NULL || strlen(token) < 3 || (args_number = is_valid_command(token)) < 0)
    {
        fprintf(stderr, "Error: comando no válido.\n");
        return 1;
    }
    strcpy(operation, token);
    if (!args_number)
        return 0;              // No hay parámetro, se retorna inmediatamente.
    token = strtok(NULL, " "); // Aca le indicamos que trabaja con el buffer que tenia.
    // Este if y endif significa que si cuando compilamos en el gcc ponemos una -D debug, todo lo que pongamos ahi adentro se compila son esto.
    #if DEBUG
    printf("par %s\n", token);
    #endif
    if (token != NULL)
        strcpy(param, token);
    else
    {
        fprintf(stderr, "Error: se esperaba un argumento para el comando %s.\n", operation);
        return 1;
    }
    return 0;
}