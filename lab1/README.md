# lab1p1 - Process Execution with execve()

## Description

`lab1p1.c` is a C program that mimics simple shell-like behavior by parsing and executing a sequence of commands separated by the '+' symbol. It creates a new child process for each command using `fork()`, and replaces the child with the desired program using `execve()`. Empty commands are handled using `/bin/true`.

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

## Features

* Uses `execve()` only (no `system()` or `execvp()`)
* Handles multiple commands separated by `+`
* Supports up to 8 arguments per command
* Uses `/bin/true` for empty commands

## Compilation

Compile the code using gcc:

```bash
gcc lab1p1.c -o lab1p1
```

## Notes

* Ensure all executable paths used exist on your system (e.g., `/bin/echo`, `/bin/sleep`)
* Add execute permissions to any shell scripts used for testing:

```bash
chmod +x test.sh
```

## Author

Your Name Here

## License

This project is provided for educational purposes.
