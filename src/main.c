#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char* read();
int eval(const char* command, char* output);
void print(char* output);
bool hasPrefix(const char* command, const char* prefix);

char empty_string[1024];

int main(int argc, char *argv[]) {
  // Flush after every printf

  while (true)
  {
    char *command  = read();
    char *output = calloc(1024, sizeof(char));
    int res = eval(command, output);
    if ((res == 0)||(res ==1))
      exit(res);
    print(output);
  }

  return 0;
}

char* read()
{
  char *line = nullptr;
  size_t lineSize = 0;
  //printf("at read\n");
  printf("$ ");
  getline(&line, &lineSize , stdin);
  if (lineSize > 1)
    line[strlen(line) - 1] = '\0';
  return line;

}

bool hasPrefix(const char* command, const char* prefix)
{
  if (strlen(prefix) > strlen(command))
    return false;
  return (strncmp(prefix, command, strlen(prefix)-1) == 0);
}

char suffix(const char* string)
{
  if (string == NULL || strlen(string) == 0)
    return false;
  return string[strlen(string) - 1]-48;
}

int eval(const char* command, char* output)
{
  //printf("at eval\n");
  strcpy(output, empty_string);
  //printf("%d" , hasPrefix(command, "exit"));
  if (hasPrefix(command, "exit"))
  {
    return suffix(command);
  }
  if (hasPrefix(command, "echo"))
  {
    output = strncpy(output, command+strlen("echo "), strlen(command)-strlen("echo "));
    strcat(output, "\n");
    return -1;
  }
  strcpy(output, command);
  const char* suffix = ": command not found\n";
  strcat(output, suffix);
  return -1;
}

void print(char* output)
{
  printf("%s", output);
}
