#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void do_something() {
  // read a integer from stdin and write it to stdout with the message
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
  int a2b, b2a;
  if ((a2b = open("a2b", O_RDONLY)) == -1) {
    perror("cannot open pipe");
    exit(1);
  }
  if ((b2a = open("b2a", O_WRONLY)) == -1) {
    perror("cannot open pipe");
    goto close_a2b;
  }

  dup2(a2b, STDIN_FILENO);
  dup2(b2a, STDOUT_FILENO);
  // dup2(b2a, STDERR_FILENO);

  do_something();

  close(b2a);
close_a2b:
  close(a2b);
}
