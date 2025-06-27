// handlers.c

#include "responses.h" // Mensajes de respuesta estándar (ej. MSG_530, MSG_200)
#include "pi.h"        // Proporciona acceso al socket de control
#include "dtp.h"       // Funciones para verificación de credenciales
#include "session.h"   // Gestión de la sesión actual del usuario
#include "utils.h"     // Funciones auxiliares como safe_dprintf
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

// --- Comando USER ---
// Recibe el nombre de usuario e inicia sesión parcialmente
void handle_USER(const char *args)
{
  ftp_session_t *sess = session_get(); // Obtener la sesión actual

  if (!args || strlen(args) == 0)
  {
    safe_dprintf(sess->control_sock, MSG_501); // Error de sintaxis
    return;
  }

  // Guardar nombre de usuario en la sesión
  strncpy(sess->current_user, args, sizeof(sess->current_user) - 1);
  sess->current_user[sizeof(sess->current_user) - 1] = '\0';

  // Solicita contraseña
  safe_dprintf(sess->control_sock, MSG_331);
}

// --- Comando PASS ---
// Verifica la contraseña y autentica al usuario
void handle_PASS(const char *args)
{
  ftp_session_t *sess = session_get();

  if (sess->current_user[0] == '\0')
  {
    safe_dprintf(sess->control_sock, MSG_503); // Usuario no enviado primero
    return;
  }

  if (!args || strlen(args) == 0)
  {
    safe_dprintf(sess->control_sock, MSG_501); // Sintaxis incorrecta
    return;
  }

  // Verificar credenciales con el archivo de usuarios
  if (check_credentials(sess->current_user, (char *)args) == 0)
  {
    sess->logged_in = 1;
    safe_dprintf(sess->control_sock, MSG_230); // Login exitoso
  }
  else
  {
    // Fallo de autenticación: resetear usuario
    safe_dprintf(sess->control_sock, MSG_530);
    sess->current_user[0] = '\0';
    sess->logged_in = 0;
  }
}

// --- Comando QUIT ---
// Finaliza la sesión actual
void handle_QUIT(const char *args)
{
  ftp_session_t *sess = session_get();
  (void)args; // argumento no usado

  safe_dprintf(sess->control_sock, MSG_221);     // Mensaje de despedida
  sess->current_user[0] = '\0';                  // Borrar usuario
  close_fd(sess->control_sock, "client socket"); // Cerrar socket de control
  sess->control_sock = -1;
}

// --- Comando SYST ---
// Informa el tipo de sistema del servidor
void handle_SYST(const char *args)
{
  ftp_session_t *sess = session_get();
  (void)args;

  safe_dprintf(sess->control_sock, MSG_215); // Enviar info del sistema
}

// --- Comando TYPE ---
// Establece el tipo de transferencia (solo binario 'I' soportado)
void handle_TYPE(const char *args)
{
  ftp_session_t *sess = session_get();

  if (!sess->logged_in)
  {
    safe_dprintf(sess->control_sock, MSG_530); // Requiere login
    return;
  }

  if (!args || strlen(args) == 0 || args[0] == 'I')
  {
    // Solo modo binario está permitido
    safe_dprintf(sess->control_sock, "Type set to binary.\r\n");
    safe_dprintf(sess->control_sock, MSG_200);
  }
  else
  {
    safe_dprintf(sess->control_sock, MSG_504); // Tipo no soportado
  }
}

// --- Comando PORT ---
// Recibe la IP y puerto donde el cliente escuchará la conexión de datos
void handle_PORT(const char *args)
{
  ftp_session_t *sess = session_get();

  if (!sess->logged_in)
  {
    safe_dprintf(sess->control_sock, MSG_530);
    return;
  }

  if (!args || strlen(args) < 11)
  {
    safe_dprintf(sess->control_sock, MSG_501); // Error de sintaxis
    return;
  }

  int h1, h2, h3, h4, p1, p2;
  if (sscanf(args, "%d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2) != 6)
  {
    safe_dprintf(sess->control_sock, MSG_501);
    return;
  }

  char ip_str[INET_ADDRSTRLEN];
  snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", h1, h2, h3, h4);
  sess->data_addr.sin_family = AF_INET;
  sess->data_addr.sin_port = htons(p1 * 256 + p2);

  if (inet_pton(AF_INET, ip_str, &sess->data_addr.sin_addr) <= 0)
  {
    safe_dprintf(sess->control_sock, MSG_501);
    return;
  }

  safe_dprintf(sess->control_sock, MSG_200); // Comando aceptado
}

// --- Comando RETR ---
// Envía un archivo al cliente a través del canal de datos
void handle_RETR(const char *args)
{
  ftp_session_t *sess = session_get();

  if (!sess->logged_in)
  {
    safe_dprintf(sess->control_sock, MSG_530);
    return;
  }

  if (!args || strlen(args) == 0)
  {
    safe_dprintf(sess->control_sock, MSG_501);
    return;
  }

  if (sess->data_addr.sin_port == 0)
  {
    safe_dprintf(sess->control_sock, MSG_503); // Falta comando PORT
    return;
  }

  int file_fd = open(args, O_RDONLY);
  if (file_fd < 0)
  {
    safe_dprintf(sess->control_sock, MSG_550, args);
    return;
  }

  int data_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (data_sock < 0)
  {
    safe_dprintf(sess->control_sock, MSG_425);
    close(file_fd);
    return;
  }

  if (connect(data_sock, (struct sockaddr *)&sess->data_addr, sizeof(sess->data_addr)) < 0)
  {
    safe_dprintf(sess->control_sock, MSG_425);
    perror(NULL);
    close(file_fd);
    close(data_sock);
    return;
  }

  safe_dprintf(sess->control_sock, MSG_150); // Listo para enviar

  char buf[1024];
  ssize_t bytes;
  while ((bytes = read(file_fd, buf, sizeof(buf))) > 0)
  {
    if (send(data_sock, buf, bytes, 0) != bytes)
    {
      perror("Envio parcial");
      break;
    }
  }

  close(file_fd);
  close(data_sock);

  safe_dprintf(sess->control_sock, MSG_226); // Transferencia completa
}

// --- Comando STOR ---
// Recibe un archivo del cliente y lo guarda localmente
void handle_STOR(const char *args)
{
  ftp_session_t *sess = session_get();

  if (!sess->logged_in)
  {
    safe_dprintf(sess->control_sock, MSG_530);
    return;
  }

  if (!args || strlen(args) == 0)
  {
    safe_dprintf(sess->control_sock, MSG_501);
    return;
  }

  if (sess->data_addr.sin_port == 0)
  {
    safe_dprintf(sess->control_sock, MSG_503);
    return;
  }

  int data_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (data_sock < 0)
  {
    safe_dprintf(sess->control_sock, MSG_425);
    return;
  }

  if (connect(data_sock, (struct sockaddr *)&sess->data_addr, sizeof(sess->data_addr)) < 0)
  {
    perror("Falló la conexión de datos");
    safe_dprintf(sess->control_sock, MSG_425);
    close(data_sock);
    return;
  }

  int file_fd = open(args, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (file_fd < 0)
  {
    safe_dprintf(sess->control_sock, MSG_550, args);
    close(data_sock);
    return;
  }

  safe_dprintf(sess->control_sock, MSG_150); // Listo para recibir

  char buf[1024];
  ssize_t bytes;
  while ((bytes = recv(data_sock, buf, sizeof(buf), 0)) > 0)
  {
    if (write(file_fd, buf, bytes) != bytes)
    {
      perror("Error al escribir en el archivo");
      break;
    }
  }

  close(file_fd);
  close(data_sock);

  safe_dprintf(sess->control_sock, MSG_226); // Transferencia completa
}

// --- Comando NOOP ---
// No hace nada, pero mantiene viva la conexión
void handle_NOOP(const char *args)
{
  ftp_session_t *sess = session_get();
  (void)args;

  safe_dprintf(sess->control_sock, MSG_200); // Comando exitoso
}
