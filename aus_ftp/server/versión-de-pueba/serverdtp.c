// Este archivo se va a encargar de acceder, alterar y mover el disco.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define PWDFILE "/etc/ausftp/ftpusers" // Ponemos todo lo que creamos antes

bool check_credentials(char *user, char *pass)
{
    FILE *file;
    char *path = PWDFILE, *line = NULL, cred[100];
    size_t len = 0;
    bool found = false;

    // make the credential string
    sprintf(cred, "%s:%s", user, pass);

    // check if it is present in any ftpusers line
    file = fopen(path, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error: no se pudo abrir el archivo de usuarios.\n");
        return false;
    }
    while (getline(&line, &len, file) != -1)
    {
        strtok(line, "\n");
        if (strcmp(line, cred) == 0)
        {
            found = true;
            break;
        }
    }
    fclose(file);
    if (line)
        free(line);
    return found;
}