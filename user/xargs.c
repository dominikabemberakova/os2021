#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define LINE_LEN 256

//Checking for ending characters
int check_character(char character) { 
  return character == ' ' || character == '\n'; 
  }

void xargs(int argc, char *argv[]){
  
  char *arguments[MAXARG];
  //Allocating memory for array of args
  memcpy(arguments, &argv[1], (argc - 1) * sizeof(argv[1]));

  char line[LINE_LEN + 1];

  while (strlen(gets(line, LINE_LEN + 1)) > 0) {
    //Checking line is  crosed
    if (strlen(line) > LINE_LEN) {
      printf("Line length crossed %d", LINE_LEN);
      exit(1);
    }
    int xargc = argc - 1;
    int i = 0;
    int line_len = strlen(line);

    while (i < line_len) {
      if (check_character(line[i])) {
        i++;
        continue;
      }
      // Locating begining of argument
      arguments[xargc++] = &line[i];
      // Iterating argument to the end
      while (i < line_len && !check_character(line[i])) 
        i++;
      line[i++] = '\0';
    }
    arguments[xargc] = 0;
    if (fork() == 0) {
      //Child is executing commands
      exec(arguments[0], arguments);
      exit(1);
    } else {
      //Parent waiting for child
      wait(0);
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Too little arguments\n");
    exit(1);
  }

  xargs(argc, argv);
  exit(0);
}