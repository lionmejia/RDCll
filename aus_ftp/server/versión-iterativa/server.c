#include "server.h"
#include "utils.h"
#include "config.h"
#include "pi.h"
#include "session.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>

extern int server_socket; // Variable global para el socket del servidor

// Inicializa el servidor TCP escuchando en la IP y puerto indicados.
// Retorna el socket escuchando o -1 en caso de error.
int server_init(const char *ip, int port)
{
    struct sockaddr_in server_addr;

    // Crear socket TCP
    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket < 0)
    {
        fprintf(stderr, "Error al crear socket\n");
        perror(NULL);
        return -1;
    }

    // Permitir reutilizar dirección y puerto para reinicios rápidos
    const int opt = 1;
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        fprintf(stderr, "Error al configurar socket\n");
        perror(NULL);
        close(listen_socket);
        return -1;
    }

#ifdef SO_REUSEPORT
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
    {
        fprintf(stderr, "Error setting SO_REUSEPORT: \n");
        perror(NULL);
        close(listen_socket);
        return -1;
    }
#endif

    // Configurar estructura sockaddr_in
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
        fprintf(stderr, "Dirección IP inválida: %s\n", ip);
        close(listen_socket);
        return -1;
    }

    // Bind socket a la dirección y puerto
    if (bind(listen_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind falló: ");
        perror(NULL);
        close(listen_socket);
        return -1;
    }

    char ip_buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &server_addr.sin_addr, ip_buf, sizeof(ip_buf));
    printf("Escuchando en %s:%d\n", ip_buf, port);

    // Comenzar a escuchar conexiones entrantes
    if (listen(listen_socket, SOMAXCONN) < 0)
    {
        fprintf(stderr, "Error al escuchar: \n");
        perror(NULL);
        close(listen_socket);
        return -1;
    }

    server_socket = listen_socket;
    return listen_socket;
}

// Acepta una conexión entrante en el socket de escucha.
// Si client_addr es NULL, no se devuelve información del cliente.
// Retorna socket nuevo para conexión o -1 en error.
int server_accept(int listen_socket, struct sockaddr_in *client_addr)
{
    socklen_t addrlen = sizeof(*client_addr);
    int new_socket = accept(listen_socket, (struct sockaddr *)client_addr, &addrlen);

    // Ignorar error EINTR que puede ocurrir por señales
    if (new_socket < 0 && errno != EINTR)
    {
        fprintf(stderr, "Error al aceptar conexión: \n");
        perror(NULL);
        return -1;
    }

    return new_socket;
}

// Bucle principal para manejar sesión FTP con un cliente.
// Inicializa la sesión, envía bienvenida y procesa comandos hasta cierre.
void server_loop(int socket)
{
    // Inicializar estructura de sesión para el cliente
    session_init(socket);

    // Enviar mensaje de bienvenida FTP
    if (welcome(current_sess) < 0)
        return;

    // Procesar comandos en bucle
    while (1)
    {
        if (getexe_command(current_sess) < 0)
            break;
    }

    // Limpiar la sesión al terminar
    session_cleanup();
}
