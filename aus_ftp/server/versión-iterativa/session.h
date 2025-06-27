#pragma once

#include "config.h"
#include <netinet/in.h> // sockaddr_in

typedef struct
{
  int control_sock;                // Descriptor del socket de conexión de control
  int data_sock;                   // Descriptor del socket de conexión de datos
  struct sockaddr_in data_addr;    // Dirección para la conexión de datos (comando PORT)
  char current_user[USERNAME_MAX]; // Nombre del usuario en sesión
  uint8_t logged_in;               // Estado de autenticación: 0 = no autenticado, 1 = autenticado
} ftp_session_t;

// Puntero global a la sesión actual (establecido por cada proceso hijo)
extern ftp_session_t *current_sess;

// Obtiene un puntero a la sesión actual
ftp_session_t *session_get(void);

// Inicializa la sesión con el socket de control recibido
void session_init(int control_fd);

// Limpia y cierra la sesión actual
void session_cleanup(void);
