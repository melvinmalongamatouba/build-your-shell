#include <stdio.h>
#include <stdlib.h>
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
    printf("%s: command not found", command);
  }

  return 0;
}

char* read(char* command)
{
  printf("$ \n");
  scanf("%s", command ,stdin);
  return command;

}

char* eval(char* command)
{
  return ("%s : command not found\n");
}

void print(char* output)
{
  printf("%s", output);
}
