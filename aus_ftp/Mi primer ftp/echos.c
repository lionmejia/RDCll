#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define RCVBUFFSIZE 512

int main() {

    int sh, client_sh, msglen;
    struct sockaddr_in addr, client_addr;
    char msg[RCVBUFFSIZE];

    if ((sh = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        printf("Failed to create socket\n");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(3333);
    
    if (bind(sh, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("Failed to bind\n");
        close(sh);
        exit(1);
    }

    if (listen(sh, 5) < 0) {
        printf("Failed to listen at port %d\n", 3333);
        close(sh);
        exit(1);
    }

    while (1) {
        socklen_t client_addr_len = sizeof(client_addr);

        if ((client_sh = accept(sh, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
            printf("Failed to accept connection\n");
            close(sh);
            exit(1);
        }

        if ((msglen = recv(client_sh, msg, RCVBUFFSIZE, 0)) < 0) {
            printf("Failed to receive message\n");
            close(client_sh);
        } else {
            while (msglen > 0) {
                if ((send(client_sh, msg, msglen, 0)) != msglen) {
                    printf("Failed to send message\n");
                    close(client_sh);
                    break;
                }
                if ((msglen = recv(client_sh, msg, RCVBUFFSIZE, 0)) < 0) {
                    printf("Failed to receive message\n");
                    close(client_sh);
                    break;
                }
            }
        }
    }
    
    close(sh);
    exit(0);
}