// Aca en este archivo va a estar el main

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "serverdtp.h"

#define VERSION "0.1" // Definimos la version del programa
#define PTODEFAULT 21

int main(int argc, char const *argv[])
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
    }
    else
    {
        port = PTODEFAULT; // Puerto por default ftp.
    }

    if (port == 0)
    {
        fprintf(stderr, "Error: puerto inválido.\n");
        return 1;
    }

    printf("%d\n", port);

    printf("Devuelve %d\n", check_credentials("test", "pepe"));

    int mastersocket, slavesocket;
    struct sockaddr_in masteraddr, slaveaddr;
    socklen_t slaveaddrlen;

    mastersocket = socket(AF_INET, SOCK_STREAM, 0);

    masteraddr.sin_family = AF_INET;
    masteraddr.sin_addr.s_addr = INADDR_ANY;
    masteraddr.sin_port = htons(port);

    bind(mastersocket, (struct sockaddr *)&masteraddr, sizeof(masteraddr));

    listen(mastersocket, 5);

    while (true)
    {
        slaveaddrlen = sizeof(slaveaddr);
        slavesocket = accept(mastersocket, (struct sockaddr *)&slaveaddr, &slaveaddrlen);
        send(slavesocket, "220 1", sizeof("220 1"), 0);
        printf("Funciona hasta aca.\n");
    }

    close(mastersocket); // Esto no sucede al menos que le mandemos una señal, como un kill por ejemplo.

    return 0;
}