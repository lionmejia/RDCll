#pragma once // Evita que este archivo se incluya múltiples veces

// --- Constantes básicas de configuración ---
#define APP_NAME "miniftp"                            // Nombre de la aplicación
#define VERSION "1.0"                                 // Versión de la aplicación
#define OS_NAME "Linux"                               // Sistema operativo objetivo (referencial)
#define BANNER APP_NAME " version " VERSION " Ready " // Mensaje mostrado al iniciar
#define BUG_EMAIL "francoleo1889@gmail.com"           // Email de contacto para reportar errores

// --- Parámetros de red por defecto ---
#define FTP_PORT 21           // Puerto por defecto para FTP (requiere root)
#define LOCALHOST "127.0.0.1" // Dirección IP por defecto

// --- Límites y tamaños ---
#define BUFSIZE 512     // Tamaño del buffer general
#define USERNAME_MAX 64 // Longitud máxima del nombre de usuario

// --- Macros auxiliares para convertir valores a strings ---
#define STR_HELPER(x) #x     // Convierte un literal a string (primer paso)
#define STR(x) STR_HELPER(x) // Aplica el paso anterior (necesario para valores macro)

// --- Descripciones para la línea de comandos (usadas con Argp) ---
#define PORT_DOC "Port number (default: " STR(FTP_PORT) ")"  // Texto de ayuda para --port
#define ADDR_DOC "Local IP address (default: " LOCALHOST ")" // Texto de ayuda para --address
