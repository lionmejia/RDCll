#include "arguments.h"
#include "server.h"
#include "utils.h"
#include "signals.h"
#include <stdio.h>
#include <stdlib.h> // EXIT_*
#include <string.h>
#include <unistd.h>    // for close()
#include <arpa/inet.h> // for inet_ntoa()
#include <errno.h>
#include <signal.h>

int main(int argc, char **argv)
{
  struct arguments args;

  // Parseo de argumentos: obtenemos la dir. IP y puerto en los que se levantara el server y lo almacenamos en la estructura arguments args.
  if (parse_arguments(argc, argv, &args) != 0)
    return EXIT_FAILURE;

  printf("Starting server on %s:%d\n", args.address, args.port);

  // Inicializamos el servidor: server_init(), crea un socket, lo asocia a la dir. y puerto proporcionados, y lo pone en modo escucha.
  int listen_fd = server_init(args.address, args.port);
  if (listen_fd < 0)
    return EXIT_FAILURE;

  // Manejo de señales: configura señales del sistema para que el servidor pueda limpiar procesos hijos o cerrarse correctamente.
  setup_signals();

  // Bucle infinito que acepta clientes.
  while (1)
  {
    struct sockaddr_in client_addr;

    memset(&client_addr, 0, sizeof(client_addr));
    int new_socket = server_accept(listen_fd, &client_addr);
    if (new_socket < 0)
    {
      continue;
    }

    // Creamos un proceso hijo que maneje al cliente.
    pid_t pid = fork();
    if (pid < 0)
    {
      perror("fork");
      close_fd(new_socket, "client (fork failed)");
      continue;
    }

    // En el proceso hijo
    if (pid == 0)
    {
      // Join parent's PGID
      pid_t pgid = getpgrp(); // parent's PGID
      if (setpgid(0, pgid) < 0)
      {
        perror("setpgid child");
      }
      printf("Child PID %d PGID %d\n", getpid(), getpgrp());

      setup_child_signals();

      close(listen_fd); // Don't need the listener

      char client_ip[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
      printf("[+] New connection from %s:%d handled by child PID %d\n", client_ip, ntohs(client_addr.sin_port), getpid());

      server_loop(new_socket); // Each child sets its own session

      printf("[-] Child PID %d closing connection for %s:%d\n", getpid(), client_ip, ntohs(client_addr.sin_port));

      // https://en.cppreference.com/w/c/program/EXIT_status
      exit(EXIT_SUCCESS);
    }
    else
    {
      // Parent process
      close_fd(new_socket, "client socket");
    }
  }

  // Código muerto (nunca alcanzado): nunca se alcanza porque el bucle principal es infinito.
  close_fd(listen_fd, "listening socket");

  // https://en.cppreference.com/w/c/program/EXIT_status
  return EXIT_SUCCESS;
}
#comentario temporal
