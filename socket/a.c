#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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
  struct pollfd pollfds[2];
  int idx[2], src_fd[2], dst_fd[2], closed[2] = {0};
  int nfd;

  src_fd[0] = STDIN_FILENO;
  dst_fd[0] = sock;
  src_fd[1] = sock;
  dst_fd[1] = STDOUT_FILENO;

  while (!closed[1]) {
    nfd = 0;
    for (int i = 0; i < 2; i++) {
      if (!closed[i]) {
        pollfds[nfd].fd = src_fd[i];
        pollfds[nfd].events = POLLIN;
        pollfds[nfd].revents = 0;
        idx[nfd] = i;
        nfd += 1;
      }
    }

    int ret = poll(pollfds, nfd, -1);
    // when some error happen
    if (ret == -1) {
      perror("poll");
      return 1;
    }

    for (int i = 0; i < nfd; i++) {
      if (!closed[idx[i]] && (pollfds[i].revents & POLLIN)) {
        ssize_t nread, nwrite;
        for (;;) {
          nread = read(src_fd[idx[i]], buf, BUF_LEN);
          if (nread == -1) {
            perror("read");
            return 2;
          }
          if (nread == 0) {
            closed[idx[i]] = 1;
            break;
          }
          if (write_all(dst_fd[idx[i]], buf, nread) == -1) {
            perror("write");
            return 3;
          }
          if (nread < BUF_LEN) break;
        }
      }
      if (pollfds[i].revents & POLLHUP) {
        closed[idx[i]] = 1;
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
