#pragma once

#include "config.h"
#include <netinet/in.h> // Para INET_ADDRSTRLEN (longitud de direcciones IPv4 en texto)

// --- Estructura para guardar los argumentos parseados ---
struct arguments
{
  int port;                      // Puerto especificado por el usuario
  int port_set;                  // Indicador de si el puerto fue proporcionado explícitamente
  char address[INET_ADDRSTRLEN]; // Dirección IP en formato texto
  int address_set;               // Indicador de si la dirección IP fue proporcionada explícitamente
};

// --- Prototipo de la función que parsea argumentos de línea de comandos ---
// Llena la estructura 'args' con los datos parseados
// Devuelve 0 si fue exitoso, o un valor distinto si hubo error
int parse_arguments(int argc, char **argv, struct arguments *args);
