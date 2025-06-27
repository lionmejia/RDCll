#define _POSIX_C_SOURCE 200809L
#include "server.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h> // para manejo de argumentos variables
#include <unistd.h> // para close()
#include <string.h>
#include <errno.h>

// Función para cerrar un descriptor de archivo y manejar errores
// Recibe el fd (descriptor) y una etiqueta para imprimir mensajes de error claros
void close_fd(int fd, const char *label)
{
  if (close(fd) < 0) // intenta cerrar el descriptor
  {
    fprintf(stderr, "Error cerrando %s: ", label);
    perror(NULL); // imprime error detallado del sistema
  }
}

// Función segura para imprimir formato en un descriptor (similar a printf pero a fd)
// Usa vdprintf para escribir con formato, con manejo de errores
ssize_t safe_dprintf(int fd, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  ssize_t ret = vdprintf(fd, format, args); // escribe con formato en fd
  va_end(args);

  if (ret < 0) // chequea si hubo error al imprimir
  {
    perror("dprintf error: ");
  }
  return ret; // retorna cantidad de bytes escritos o error
}
