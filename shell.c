#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

void parse(char* s, char** tokens) {
  char * backup = malloc(BUFSIZ * sizeof(char*));
  strcpy(backup, s);
  const char break_chars[] = " \t";
  char* p;
  int curr = 0;

  memset(tokens, 0, BUFSIZ * sizeof(char*));
  p = strtok(s, break_chars);

  while (p != NULL) {
    tokens[curr++] = p;
    p = strtok(NULL, break_chars);
  }
  s = backup;
}

int main(int argc, const char * argv[]) {  
  char input[BUFSIZ];
  char last_command[BUFSIZ];
  char* tokens[BUFSIZ];
  char new_input[BUFSIZ];
  char* new_tokens[BUFSIZ];
  char* pipe_test = "green";
  char* pipe_test2 = " pipes";
  
  memset(input, 0, BUFSIZ * sizeof(char));
  memset(input, 0, BUFSIZ * sizeof(char));
  bool finished = false;
  bool concurrent = false;
  // int p1[2];
  // int p2[2];

  while (!finished) {
    printf("osh> ");
    fflush(stdout);

    if ((fgets(input, BUFSIZ, stdin)) == NULL) {   // or gets(input, BUFSIZ);
      fprintf(stderr, "no command entered\n");
      exit(1);
    }

    input[strlen(input) - 1] = '\0';          // wipe out newline at end of string
    printf("input was: \n'%s'\n", input);

    if (strncmp(input, "exit()", 6) == 0) {   // only compare first 4 letters
      finished = true;
    } else {
      // check for history (!!) command
      if (strncmp(input, "!!", 2) == 0) {
        if (strlen(last_command) == 0) {
          fprintf(stderr, "no last command to execute\n");
        }
        parse(last_command, tokens);
      } else {
        strcpy(last_command, input);
        parse(input, tokens);
      }

      int fd;
      int redirect = 1;
      int one_prog = true;
      
      // check for |, <, or > and cut the rhs if found
      for (int i = 0; tokens[i] != 0; i++) {
        if (strcmp(tokens[i], "|") == 0) {
          printf("found it\n");
          one_prog = false;
          tokens[i] = 0;
          i++;
          for (int j = 0; tokens[i] != 0; j++, i++) {
            new_tokens[j] = tokens[i];
            tokens[i] = 0;
          }
        } else if (strcmp(tokens[i], ">") == 0) {
            printf("stream out\n");
            tokens[i] = 0;
            i++;
            redirect = 1;
            fd = open(tokens[i], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);            
            for (int j = 0; tokens[i] != 0; j++, i++) {
              tokens[i] = 0;
            }
            printf("weeeeeeeeee\n");
        } else if (strcmp(tokens[i], "<") == 0) {
            printf("stream in\n");
            tokens[i] = 0;
            i++;
            redirect = 2;
            printf("%s\n", tokens[i]);
            fd = open(tokens[i], O_RDONLY);
            for (int j = 0; tokens[i] != 0; j++, i++) {
              tokens[i] = 0;
            }
        }
      }

      int pid = fork();
      if (pid < 0) {
        fprintf(stderr, "Fork failed\n");
        return 1;
      } else if (pid > 0) {                           // parent
        wait(NULL);
      } else {                                        // child

        if (one_prog) {
          if (redirect == 1) {
            dup2(fd, 1);
            close(fd);
          } else if (redirect == 2) {
            dup2(fd, 0);
            close(fd);
          }
          execvp(tokens[0], tokens);
          exit(1);
        }
        
        int p3[2];
        if (pipe(p3) < 0) {
          fprintf(stderr, "Pipe failed\n");
          return 1;
        }

        pid = fork();

        if (pid == 0) {
          dup2(p3[1], 1);
          close(p3[0]);
          close(p3[1]);
          execvp(tokens[0], tokens);
          fprintf(stderr, "Failed to exe first\b");
        } else {
            pid = fork();
            if (pid == 0) {
            dup2(p3[0], 0);
            close(p3[1]);
            close(p3[0]);
            execvp(new_tokens[0], new_tokens);
            fprintf(stderr, "failed to exe second\n");
            } else {
                int status;
                close(p3[0]);
                close(p3[1]);
                waitpid(pid, &status, 0);
            }
        }        
      }
      printf("\n");
    }
  }
  
  printf("osh exited\n");
  printf("program finished\n");
  
  return 0;
}