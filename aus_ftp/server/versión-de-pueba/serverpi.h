#ifndef SERVERPI_H // Sirve para evitar redefiniciones

#define SERVERPI_H

// Esto es toda la parte de comunicacion
#define VERSION "1.0" // Definimos la version del programa
#define MSG_220 "220 srvFtp version " VERSION "\r\n"
#define MSG_331 "331 Password required for %s\r\n"
#define MSG_230 "230 User %s logged in\r\n"
#define MSG_530 "530 Login incorrect\r\n"
#define MSG_221 "221 Goodbye\r\n"
#define MSG_550 "550 %s: no such file or directory\r\n"
#define MSG_299 "299 File %s size %ld bytes\r\n"
#define MSG_226 "226 Transfer complete\r\n"
#define MSG_502 "502 Command not implemented\r\n"
#define MSG_215 "215 UNIX Type: L8\r\n"s

static const char *valid_commands[] = {
    "USER", "PASS", "ACCT", "CWD", "CDUP", "SMNT",
    "QUIT", "REIN", "PORT", "PASV", "TYPE", "STRU",
    "MODE", "RETR", "STOR", "STOU", "APPE", "ALLO",
    "REST", "RNFR", "RNTO", "ABOR", "DELE", "RMD",
    "MKD", "PWD", "LIST", "NLST", "SITE", "SYST",
    "STAT", "HELP", "NOOP", "FEAT", NULL};

static const int arg_commands[] = {
    1, 1, 0, 1, 0, 0,
    0, 0, 1, 0, 1, 0,
    1, 1, 1, 1, 1, 0,
    1, 2, 2, 0, 1, 1,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0};

int recv_cmd(int, char *, char *);

#endif