# lab1p1 - Part 1

## How to build the code (using gcc):

Compile the code using gcc:

```bash
gcc lab1p1.c -o lab1p1
```
## How to build the code (using shell script):
`build.sh` shell script will do the compiling of `lab1p1.c`

To give relevant executable permissions to shell scripts when running:
```
chmod +x build.sh
```

Now `lab1p1.c` can be built using `build.sh`
```
./build.sh lab1p1
```

Now the binary `lab1p1` is created.

## Example Usage

```bash
./lab1p1 /bin/echo Linux is cool + /bin/echo But I am sleepy + /bin/echo Going to sleep now + /bin/sleep 5 + /bin/echo Now I am awake
```

This will result in the following commands being executed:

* /bin/echo Linux is cool
* /bin/echo But I am sleepy
* /bin/echo Going to sleep now
* /bin/sleep 5
* /bin/echo Now I am awake

# lab1p2 - Part 2

## How to build the code (using gcc):

Compile the code using gcc:

```bash
gcc lab1p1.c -o lab1p2
```
## How to build the code (using shell script):
`build.sh` shell script will do the compiling of `lab1p2.c`

To give relevant executable permissions to shell scripts when running:
```
chmod +x build.sh
```

Now `lab1p2.c` can be built using `build.sh`
```
./build.sh lab1p2
```

Now the binary `lab1p1` is created.

First enter the command to be executed repetitively with no of `%` (Not exceeding 8 times).
No of `%` will be the maximum no of inputs that can be entered repetitively. (Can enter any number of input lines)
Once finish, exit with EOF `Ctrl+D` (Keyboard input).

## Example Usage

```bash
./lab1p2 ./lab1p2 gcc % % % % % %
-c a.c
-c b.c
-c c.c
-o a.o b.o c.o
```


This will result in the following commands being executed:

* gcc -c a.c
* gcc -c b.c
* gcc -c c.c
* gcc -0 a.0 b.0 c.0
