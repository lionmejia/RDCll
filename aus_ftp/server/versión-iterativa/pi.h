#pragma once // Evita inclusiones múltiples

#include <stddef.h>  // Para tipos estándar como size_t
#include "session.h" // Definición de ftp_session_t

// Envía el mensaje de bienvenida al cliente cuando se conecta.
// Retorna 0 en éxito, -1 en error.
int welcome(ftp_session_t *sess);

// Recibe y ejecuta un comando FTP enviado por el cliente.
// Procesa la línea recibida, busca el handler y lo ejecuta.
// Retorna 0 para continuar, -1 para cerrar sesión.
int getexe_command(ftp_session_t *sess);

// Estructura que asocia el nombre de un comando FTP
// con su función manejadora correspondiente.
typedef struct
{
  const char *name;                  // Nombre del comando (ej. "USER")
  void (*handler)(const char *args); // Función que maneja el comando
} ftp_command_t;

// Declaraciones de los handlers para los comandos FTP básicos.
void handle_USER(const char *args);
void handle_PASS(const char *args);
void handle_QUIT(const char *args);
void handle_SYST(const char *args);
void handle_TYPE(const char *args);
void handle_PORT(const char *args);
void handle_RETR(const char *args);
void handle_STOR(const char *args);
void handle_NOOP(const char *args);
