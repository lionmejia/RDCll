// main.c

#include "arguments.h" // Manejo de argumentos de línea de comandos
#include "server.h"    // Inicialización del servidor y conexión con clientes
#include "utils.h"     // Utilidades generales (como close_fd)
#include "signals.h"   // Manejo de señales del sistema (ej. SIGINT)
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

// Punto de entrada del servidor FTP
int main(int argc, char *argv[])
{
    struct arguments args;

    // Analiza los argumentos del programa (por ejemplo, IP y puerto)
    if (parse_arguments(argc, argv, &args) != 0)
    {
        fprintf(stderr, "Error: argumentos inválidos.\n");
        return EXIT_FAILURE;
    }

    // Imprime en consola que el servidor está iniciando
    printf("Iniciando servidor en %s:%d\n", args.address, args.port);

    // Inicializa el socket maestro que escuchará nuevas conexiones
    int master_socket = server_init(args.address, args.port);
    if (master_socket < 0)
    {
        fprintf(stderr, "Error: no se pudo iniciar el servidor.\n");
        return EXIT_FAILURE;
    }

    // Configura manejo de señales (ej. para cerrar el servidor con Ctrl+C)
    setup_signals();

    // Bucle principal del servidor: aceptar y atender clientes uno por uno
    while (1)
    {
        struct sockaddr_in client_addr;                              // Dirección del cliente
        int new_socket = server_accept(master_socket, &client_addr); // Espera conexión

        if (new_socket < 0)
        {
            fprintf(stderr, "Error al crear socket\n");
            perror(NULL);
        }

        // Obtiene IP legible del cliente
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

        // Informa que se aceptó una conexión entrante
        printf("Conexión desde %s:%d aceptada\n", client_ip, ntohs(client_addr.sin_port));

        // Inicia el ciclo de comandos FTP para el cliente
        server_loop(new_socket);

        // Conexión finalizada, se informa por consola
        printf("Conexión desde %s:%d cerrada\n", client_ip, ntohs(client_addr.sin_port));
    }

    // Nunca se alcanza este punto por el bucle infinito
    close_fd(master_socket, "socket maestro");

    return EXIT_SUCCESS; // Retorno estándar de éxito
}
