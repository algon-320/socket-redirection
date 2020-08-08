#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/stat.h>
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

// stdin --> a2b (write)
// stdout <-- b2a (read)
int redirect(int a2b, int b2a) {
  const size_t BUF_LEN = 1024;

  char buf[BUF_LEN + 1];
  fd_set readfds;
  int stdin_closed = 0, b2a_closed = 0;

  while (!b2a_closed) {
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(b2a, &readfds);

    int ret = select(max(STDIN_FILENO, b2a) + 1, &readfds, NULL, NULL, NULL);
    // when some error happen
    if (ret == -1) {
      perror("select");
      return 1;
    }

    // read bytes from stdin and write them to a2b
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

        if (write_all(a2b, buf, nread) == -1) {
          perror("write a2b");
          return 3;
        }

        if (nread < BUF_LEN) break;
      }
    }

    // read bytes from b2a and write them to stdout
    if (!b2a_closed && FD_ISSET(b2a, &readfds)) {
      ssize_t nread, nwrite;
      for (;;) {
        nread = read(b2a, buf, BUF_LEN);
        if (nread == -1) {
          perror("read b2a");
          return 2;
        }
        if (nread == 0) {  // EOF
          b2a_closed = 1;
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
  int a2b, b2a;
  if ((a2b = open("a2b", O_WRONLY)) == -1) {
    perror("cannot open pipe");
    exit(EXIT_FAILURE);
  }
  if ((b2a = open("b2a", O_RDONLY)) == -1) {
    perror("cannot open pipe");
    exit(EXIT_FAILURE);
  }

  int ret;
  if ((ret = redirect(a2b, b2a)) != 0) {
    return ret;
  }
  close(a2b);
  close(b2a);
}
