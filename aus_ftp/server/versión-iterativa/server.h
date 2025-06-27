#pragma once

#include <netinet/in.h>

// Descriptor del socket cliente actual
extern int client_fd;

// Inicializa el servidor en la IP y puerto dados. Retorna socket de escucha o -1 en error.
int server_init(const char *ip, int port);

// Acepta una nueva conexión entrante.
// Si client_addr es NULL, no se devuelve info del cliente.
// Retorna socket para la conexión o -1 en error.
int server_accept(int listen_fd, struct sockaddr_in *client_addr);

// Setter para asignar el descriptor del socket que escucha
void server_set_listen_fd(int fd);

// Apaga el servidor limpiando recursos
void server_shutdown(void);

// Loop principal para atender la conexión con un cliente
void server_loop(int client_fd);
