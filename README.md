# Socket / named-pipe redirection

## What's this?

This is a sample program that aims to show
how to redirect the stdin/stdout of a process to that of another process.

```
                               |                                 
STDIN (a) ---> ||   || ---(socket/a2b)---> STDIN (b) ---> ||   ||
               || a ||         |                          || b ||
STDOUT (a) <-- ||   || <--(socket/b2a)---- STDOUT (b) <-- ||   ||
                               |                                 
```

Namely, you can interact with `b` through `a`'s stdio.

## Usage
1. Open a terminal and execute as follows:
```
$ make
$ ./a
```
2. then open another terminal and execute as follows:
```
$ ./b
```
3. type any integer and string into `a`'s terminal.

---

The results of a (suppose you type "1234" and "hello"):
```
1234
x = 1234
hello
msg = hello
```
The results of b: (they are written to stderr)
```
scanned x = 1234
scanned msg = hello
```
