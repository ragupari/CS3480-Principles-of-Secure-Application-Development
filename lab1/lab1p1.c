#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int execute(char strings[8][100], int s_index) {
  char *args[9];
  for (int i = 0; i < s_index; i++) {
    args[i] = strings[i];
  }
  args[s_index] = NULL;
  pid_t pid = fork();
  if (pid == 0) {
    execve(strings[0], args, NULL);
    perror("execve failed");
    _exit(1); 
  } else if (pid > 0) {
    wait(NULL);
  } else {
    perror("fork failed");
    return -1;
  }

  return 0;
}

int main(int argc, char *argv[]) {
  printf("You passed %d arguments:\n", argc - 1);
  char strings[8][100] = {0};
  int s_index = 0;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "+") == 0) {
      if (s_index == 0) {
        strcpy(strings[0], "/bin/true");
        s_index = 1;
      }
      execute(strings, s_index);
      memset(strings, 0, sizeof(strings));
      s_index = 0;
    } else {
      if (s_index < 8) {
        strcpy(strings[s_index], argv[i]);
        s_index++;
      }
    }
  }

  if (s_index > 0) {
    execute(strings, s_index);
  }

  return 0;
}
