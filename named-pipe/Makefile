.PHONY: all
all: a b a2b b2a

.PHONY: clean
clean:
	-rm a
	-rm b
	-rm a2b
	-rm b2a

.PHONY: fmt
fmt: a.c b.c
	clang-format -style=Google -i a.c b.c

a2b:
	mkfifo a2b
b2a:
	mkfifo b2a
