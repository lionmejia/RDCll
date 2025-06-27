#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h> // Para waitpid y pid_t

#include "signals.h"
#include "session.h" // Para current_sess
#include "utils.h"   // Para close_fd()
#include <asm-generic/signal.h>

int server_socket = -1;

// --- Manejador de SIGINT (Ctrl+C) ---
static void handle_sigint(int sig)
{
  (void)sig;
  static volatile sig_atomic_t in_handler = 0;

  if (in_handler)
  {
    fprintf(stderr, "SIGINT handler reentered!\n");
    return; // Evita reentrancia simultánea del handler
  }
  in_handler = 1;

  static int sigint_count = 0;
  fprintf(stderr, "SIGINT handler called (count = %d) in PID %d\n", ++sigint_count, getpid());

  printf("[+] SIGINT received. Shutting down...\n");
  fflush(stdout);

  // Cierra el socket del servidor si está abierto
  if (server_socket >= 0)
  {
    close_fd(server_socket, "listen socket");
    server_socket = -1;
  }

  // Bloquea SIGINT mientras se realiza la limpieza
  sigset_t blockset, oldset;
  sigemptyset(&blockset);
  sigaddset(&blockset, SIGINT);
  if (sigprocmask(SIG_BLOCK, &blockset, &oldset) < 0)
  {
    perror("sigprocmask");
  }

  // Termina todo el grupo de procesos
  pid_t pgid = getpgrp();
  if (killpg(pgid, SIGTERM) < 0)
  {
    perror("killpg");
  }

  // Recolecta procesos hijos terminados (evita zombis)
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;

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
    fprintf(stderr, "SIGTERM handler reentered in parent!\n");
    return;
  }
  in_handler = 1;

  fprintf(stderr, "[+] SIGTERM received in parent. Shutting down (PID %d)...\n", getpid());

  // Cierra el socket del servidor
  if (server_socket >= 0)
  {
    close_fd(server_socket, "listen socket");
    server_socket = -1;
  }

  // Termina todos los procesos del grupo
  pid_t pgid = getpgrp();
  fprintf(stderr, "[DEBUG] Sending SIGTERM to (GROUP %d)...\n", (int)pgid);
  if (killpg(pgid, SIGTERM) < 0)
  {
    perror("killpg (parent)");
  }

  // Recolecta hijos
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;

  exit(EXIT_SUCCESS);
}

// --- Manejador de SIGTERM para procesos hijo ---
static void handle_sigterm_child(int sig)
{
  (void)sig;

  printf("[*] Child PID %d received SIGTERM, cleaning up...\n", getpid());
  fflush(stdout);

  // Cierra sockets de control y datos del hijo si están abiertos
  if (current_sess)
  {
    if (current_sess->control_sock >= 0)
    {
      close_fd(current_sess->control_sock, "control socket");
      current_sess->control_sock = -1;
    }
    if (current_sess->data_sock >= 0)
    {
      close_fd(current_sess->data_sock, "data socket");
      current_sess->data_sock = -1;
    }
  }

  exit(EXIT_SUCCESS);
}

// --- Configura señales en el proceso padre ---
void setup_signals(void)
{
  struct sigaction sa;

  // Asegura que el padre es líder de grupo de procesos
  if (setpgid(0, 0) < 0)
  {
    perror("setpgid parent");
    exit(EXIT_FAILURE);
  }

  printf("[DEBUG] Setting up signal handlers for parent in PID %d with PGID %d\n", getpid(), getpgrp());

  // Manejador para SIGINT (Ctrl+C)
  sigemptyset(&sa.sa_mask);
  sigaddset(&sa.sa_mask, SIGINT);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = handle_sigint;
  if (sigaction(SIGINT, &sa, NULL) == -1)
  {
    perror("sigaction SIGINT");
    exit(EXIT_FAILURE);
  }
  printf("[DEBUG] SIGINT handler installed in PID %d\n", getpid());

  // Manejador para SIGTERM
  sa.sa_handler = handle_sigterm;
  if (sigaction(SIGTERM, &sa, NULL) == -1)
  {
    perror("sigaction SIGTERM");
    exit(EXIT_FAILURE);
  }

  // Ignora SIGCHLD para evitar procesos zombis
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGCHLD, &sa, NULL) == -1)
  {
    perror("sigaction SIGCHLD");
    exit(EXIT_FAILURE);
  }
}

// --- Configura señales específicas en procesos hijo ---
void setup_child_signals(void)
{
  struct sigaction sa;

  printf("[DEBUG] Setting up signal handlers for child in PID %d\n", getpid());

  // Manejador para SIGTERM en el hijo
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handle_sigterm_child;
  if (sigaction(SIGTERM, &sa, NULL) == -1)
  {
    perror("sigaction SIGTERM (child)");
    exit(EXIT_FAILURE);
  }
  printf("[DEBUG] SIGTERM handler for child installed in PID %d\n", getpid());

  // Ignora SIGINT en el hijo (lo maneja el padre)
  sa.sa_handler = SIG_IGN;
  if (sigaction(SIGINT, &sa, NULL) == -1)
  {
    perror("sigaction SIGINT (child)");
    exit(EXIT_FAILURE);
  }

  // Restaura comportamiento por defecto para SIGCHLD
  sa.sa_handler = SIG_DFL;
  sigaction(SIGCHLD, &sa, NULL);
}

// --- Restaura manejadores por defecto (usado antes de terminar) ---
void reset_signals(void)
{
  struct sigaction sa;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = SIG_DFL;

  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGCHLD, &sa, NULL);
}
