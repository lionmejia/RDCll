#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h> // para pid_t

#include "signals.h"
#include "session.h" // para current_sess
#include "utils.h"   // para close_fd()
#include <asm-generic/signal.h>

// Descriptor del socket del servidor (escucha)
int server_socket = -1;

// Manejador de la señal SIGINT (Ctrl+C)
static void handle_sigint(int sig)
{
  (void)sig;
  static volatile sig_atomic_t in_handler = 0;

  // Evitar reentradas concurrentes del manejador
  if (in_handler)
  {
    fprintf(stderr, "¡Manejador SIGINT reentrado!\n");
    return;
  }
  in_handler = 1;

  static int sigint_count = 0;
  fprintf(stderr, "Manejador SIGINT llamado (contador = %d) en PID %d\n", ++sigint_count, getpid());

  printf("[+] SIGINT recibido. Cerrando servidor...\n");
  fflush(stdout);

  // Cerrar socket de escucha si está abierto
  if (server_socket >= 0)
  {
    close_fd(server_socket, "socket de escucha");
    server_socket = -1;
  }

  // Bloquear SIGINT mientras se realiza el apagado
  // para evitar problemas si llegan señales múltiples
  sigset_t blockset, oldset;
  sigemptyset(&blockset);
  sigaddset(&blockset, SIGINT);
  if (sigprocmask(SIG_BLOCK, &blockset, &oldset) < 0)
  {
    perror("sigprocmask");
  }

  // Restaurar máscara anterior de señales (opcional aquí porque salimos)
  sigprocmask(SIG_SETMASK, &oldset, NULL);

  exit(EXIT_SUCCESS);
}

// --- Manejador de SIGTERM para el proceso padre ---
static void handle_sigterm(int sig)
{
  (void)sig;

  static volatile sig_atomic_t in_handler = 0;
  if (in_handler)
  {
    fprintf(stderr, "¡Manejador SIGTERM reentrado!\n");
    return;
  }
  in_handler = 1;

  fprintf(stderr, "[+] SIGTERM recibido. Cerrando servidor (PID %d)...\n", getpid());

  // Cerrar socket de escucha si está abierto
  if (server_socket >= 0)
  {
    close_fd(server_socket, "socket de escucha");
    server_socket = -1;
  }

  exit(EXIT_SUCCESS);
}

// Configura los manejadores de señales SIGINT y SIGTERM
void setup_signals(void)
{
  struct sigaction sa;

  printf("[DEBUG] Configurando manejadores de señales en PID %d\n", getpid());

  // Configurar SIGINT y SIGTERM para el proceso padre

  sigemptyset(&sa.sa_mask);
  sigaddset(&sa.sa_mask, SIGINT); // Bloquear SIGINT mientras corre el manejador

  sa.sa_flags = SA_RESTART; // Reanudar syscalls interrumpidas

  sa.sa_handler = handle_sigint;

  // Instalar manejador de SIGINT
  if (sigaction(SIGINT, &sa, NULL) == -1)
  {
    perror("sigaction SIGINT");
    exit(EXIT_FAILURE);
  }
  printf("[DEBUG] Manejador SIGINT instalado en PID %d\n", getpid());

  // Instalar manejador de SIGTERM con misma máscara y flags
  sa.sa_handler = handle_sigterm;

  if (sigaction(SIGTERM, &sa, NULL) == -1)
  {
    perror("sigaction SIGTERM");
    exit(EXIT_FAILURE);
  }
}

// --- Restaurar comportamientos por defecto de señales ---
// Usado por procesos hijos antes de exec o para salida limpia
void reset_signals(void)
{
  struct sigaction sa;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = SIG_DFL;

  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
}
