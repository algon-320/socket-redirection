.PHONY: all
all: a b

.PHONY: clean
clean:
	-rm a
	-rm b

.PHONY: fmt
fmt: a.c b.c
	clang-format -style=Google -i a.c b.c
