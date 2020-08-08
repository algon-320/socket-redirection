#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int max(int a, int b) {
  if (a > b) return a;
  return b;
}

// call write syscall repeatedly until entire bytes are written.
ssize_t write_all(int fd, char *buf, size_t count) {
  ssize_t nw, total = 0;
  while (total < count) {
    ssize_t nw = write(fd, buf + total, count - total);
    if (nw == -1) return nw;
    total += nw;
  }
  return total;
}

// stdin --> sock (write)
// stdout <-- sock (read)
int redirect(int sock) {
  const size_t BUF_LEN = 1024;

  char buf[BUF_LEN + 1];
  fd_set readfds;
  int stdin_closed = 0, sock_closed = 0;

  while (!sock_closed) {
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(sock, &readfds);

    int ret = select(max(STDIN_FILENO, sock) + 1, &readfds, NULL, NULL, NULL);
    // when some error happen
    if (ret == -1) {
      perror("select");
      return 1;
    }

    // read bytes from stdin and write them to sock
    if (!stdin_closed && FD_ISSET(STDIN_FILENO, &readfds)) {
      ssize_t nread, nwrite;
      for (;;) {
        nread = read(STDIN_FILENO, buf, BUF_LEN);
        if (nread == -1) {
          perror("read stdin");
          return 2;
        }
        if (nread == 0) {  // EOF
          stdin_closed = 1;
          break;
        }

        if (write_all(sock, buf, nread) == -1) {
          perror("write sock");
          return 3;
        }

        if (nread < BUF_LEN) break;
      }
    }

    // read bytes from sock and write them to stdout
    if (!sock_closed && FD_ISSET(sock, &readfds)) {
      ssize_t nread, nwrite;
      for (;;) {
        nread = read(sock, buf, BUF_LEN);
        if (nread == -1) {
          perror("read sock");
          return 2;
        }
        if (nread == 0) {  // EOF
          sock_closed = 1;
          break;
        }

        if (write_all(STDOUT_FILENO, buf, nread) == -1) {
          perror("write stdout");
          return 3;
        }

        if (nread < BUF_LEN) break;
      }
    }
  }
  return 0;
}

int main(void) {
  const short SERVER_PORT = 12345;
  const char *SERVER_ADDR = "127.0.0.1";

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(SERVER_PORT);
  server.sin_addr.s_addr = inet_addr(SERVER_ADDR);

  if (connect(sock, (struct sockaddr *)&server, sizeof server) == -1) {
    perror("connect");
    exit(EXIT_FAILURE);
  }

  int ret;
  if ((ret = redirect(sock)) != 0) {
    return ret;
  }

  close(sock);
}
