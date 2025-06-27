#pragma once // Evita inclusiones múltiples del mismo encabezado

// --- Ruta del archivo que contiene las credenciales válidas ---
#define PWDFILE "/etc/ausftp/ftpusers"

// --- Declaración de la función que verifica credenciales de usuario ---
int check_credentials(char *user, char *pass);
