#define _GNU_SOURCE // Habilita funciones extendidas de GNU (como getline)

#include "dtp.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// --- Verifica si las credenciales (usuario:contraseña) existen en el archivo PWDFILE ---
int check_credentials(char *user, char *pass)
{
    FILE *file;
    char *path = PWDFILE, *line = NULL, cred[100];
    size_t len = 0;
    int found = -1;

    // --- Construye la cadena de credenciales con formato "usuario:contraseña" ---
    sprintf(cred, "%s:%s", user, pass);

    // --- Abre el archivo de usuarios definidos (PWDFILE) ---
    file = fopen(path, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error: no se pudo abrir el archivo de usuarios.\n");
        return -1;
    }

    // --- Lee línea por línea y compara con las credenciales ingresadas ---
    while (getline(&line, &len, file) != -1)
    {
        strtok(line, "\n");          // Elimina el salto de línea
        if (strcmp(line, cred) == 0) // Compara la línea con "usuario:contraseña"
        {
            found = 0; // Coincidencia encontrada
            break;
        }
    }

    // --- Limpieza ---
    fclose(file);
    if (line)
        free(line);

    return found; // Devuelve 0 si es válido, -1 si no
}
