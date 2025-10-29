#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char* read(char* command);
char* eval(char* command);
void print(char* output);

int main(int argc, char *argv[]) {
  // Flush after every printf

  while (true)
  {
    char command[1024];
    read(command);
    char* output = eval(command);
    print(output);
  }

  return 0;
}

char* read(char* command)
{
  printf("$ ");
  scanf("%s", command ,stdin);
  return command;

}

char* eval(char* command)
{
  char* output = calloc(1024, sizeof(char));
  strcpy(output, command);
  const char* suffix = ": command not found\n";
  strcat(output, suffix);
  return output;
}

void print(char* output)
{
  printf("%s", output);
}
