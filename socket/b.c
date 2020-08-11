#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void do_something() {
  // read a integer from stdin and write it to stdout with a message.
  int x;
  scanf("%d", &x);
  fprintf(stderr, "scanned x = %d\n", x);
  printf("x = %d\n", x);
  fflush(stdout);

  // then do the same thing for a string.
  char msg[1024];
  scanf("%s", msg);
  fprintf(stderr, "scanned msg = %s\n", msg);
  printf("msg = %s\n", msg);
  fflush(stdout);
}

int main(void) {
  const short LISTEN_PORT = 12345;

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(LISTEN_PORT);
  addr.sin_addr.s_addr = INADDR_ANY;

  int yes = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof yes) == -1) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  if (bind(sock, (struct sockaddr *)&addr, sizeof addr) == -1) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  if (listen(sock, 1) == -1) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  {
    struct sockaddr_in client;
    socklen_t client_addr_size = sizeof client;
    int fd = accept(sock, (struct sockaddr *)&client, &client_addr_size);
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    // dup2(fd, STDERR_FILENO);

    do_something();

    shutdown(fd, SHUT_RDWR);
  }
  close(sock);
}
