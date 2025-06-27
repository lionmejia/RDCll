#include "server.h"    // Declaraciones relacionadas al servidor FTP
#include "pi.h"        // Contiene los handlers de los comandos FTP
#include "responses.h" // Mensajes predefinidos de respuesta FTP (ej. MSG_220, MSG_530, etc.)
#include "utils.h"     // Funciones auxiliares (ej. safe_dprintf, close_fd)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>  // inet_pton, etc.
#include <sys/socket.h> // sockets
#include <errno.h>
#include <strings.h> // strcasecmp

// Tabla de comandos FTP y sus funciones correspondientes
static ftp_command_t ftp_commands[] = {
    {"USER", handle_USER},
    {"PASS", handle_PASS},
    {"QUIT", handle_QUIT},
    {"SYST", handle_SYST},
    {"TYPE", handle_TYPE},
    {"PORT", handle_PORT},
    {"RETR", handle_RETR},
    {"STOR", handle_STOR},
    {"NOOP", handle_NOOP},
    {NULL, NULL} // Marca el fin del arreglo
};

// Envía el mensaje de bienvenida inicial al cliente
int welcome(ftp_session_t *sess)
{
  // Enviamos el mensaje 220 ("Service ready") y validamos si se mandó completo
  if (safe_dprintf(sess->control_sock, MSG_220) != sizeof(MSG_220) - 1)
  {
    fprintf(stderr, "Error al enviar el mensaje de bienvenida\n");
    close_fd(sess->control_sock, "socket cliente");
    return -1;
  }

  return 0;
}

// Procesa el comando recibido por el cliente y ejecuta el handler correspondiente
int getexe_command(ftp_session_t *sess)
{
  char buffer[BUFSIZE];

  // Recibe el comando desde el socket de control
  ssize_t len = recv(sess->control_sock, buffer, sizeof(buffer) - 1, 0);
  if (len < 0)
  {
    perror("Falló la recepción: ");
    close_fd(sess->control_sock, "socket cliente");
    return -1;
  }

  // Si len == 0, el cliente cerró la conexión de forma inesperada
  if (len == 0)
  {
    sess->current_user[0] = '\0';
    close_fd(sess->control_sock, "socket cliente");
    sess->control_sock = -1;
    return -1;
  }

  // Null-terminamos el buffer
  buffer[len] = '\0';

  // Eliminamos posibles terminadores CRLF (\r\n)
  char *cr = strchr(buffer, '\r');
  if (cr)
    *cr = '\0';
  char *lf = strchr(buffer, '\n');
  if (lf)
    *lf = '\0';

  // Inicializamos punteros a comando y argumento
  char *arg = NULL;
  char *cmd = buffer;

  // Si el comando está vacío (por ejemplo, solo \r\n)
  if (cmd[0] == '\0')
  {
    safe_dprintf(sess->control_sock, "500 Empty command.\r\n");
    return 0;
  }

  // Separamos el comando del argumento si hay un espacio
  char *space = strchr(buffer, ' ');
  if (space)
  {
    *space = '\0';
    arg = space + 1;
    while (*arg == ' ')
      arg++; // Salta espacios adicionales
  }

  // Buscamos el comando en la tabla
  ftp_command_t *entry = ftp_commands;
  while (entry->name)
  {
    if (strcasecmp(entry->name, cmd) == 0) // Insensible a mayúsculas
    {
      entry->handler(arg ? arg : ""); // Llama al handler
      return (sess->control_sock < 0) ? -1 : 0;
    }
    entry++;
  }

  // Si el comando no se encuentra, enviamos error 502
  safe_dprintf(sess->control_sock, "502 Command not implemented.\r\n");
  return 0;
}
