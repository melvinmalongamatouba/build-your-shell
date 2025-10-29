#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#ifdef _WIN32
  const char path_delim = ';';
#else
const char path_delim = ':';
#endif

bool executable_is_in_path(const char* str, char* path_to_consider);
char* _read();
int eval(const char* command, char* output);
void print(char* output);
bool hasPrefix(const char* command, const char* prefix);

char empty_string[1024];
char* const builtin[]= {
  "type",
  "echo",
  "shell",
  "exit"
};
int const builtin_length = 4;

int main(int argc, char *argv[]) {
  // Flush after every printf
  executable_is_in_path("",".");
  while (true)
  {
    char *command  = _read();
    char *output = calloc(1024, sizeof(char));
    int res = eval(command, output);
    if ((res == 0)||(res ==1))
      exit(res);
    print(output);
  }

  return 0;
}

char* _read()
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

int exit_(const char* command)
{
  if (command == NULL || strlen(command) == 0)
    return false;
  return command[strlen(command) - 1]-48;
}

//-------------------------- Different eval behaviors ---------------------------
int builtin_type_(const char* command, char* output)
{

  for (int i = 0; i<builtin_length; i++)
  {
    if (strcmp(command + strlen("type "), builtin[i]) == 0)
    {
      strcpy(output, command + strlen("type "));
      strcat(output, " is a shell builtin\n");
      return 1;
    }
  }
  return -1;

}

bool executable_is_in_path(const char* str, char* path_to_consider)
{
  char fullPath [1024];
  snprintf(fullPath, sizeof(fullPath), "%s/%s", path_to_consider, str);

  if (access(fullPath, X_OK) == 0)
    return true;
  return false;
}

int show_executable_in_path(const char* command, char* output, char* path)
{
  if (path == NULL || *path == '\0')
  {
    strcpy(output, command + strlen("type "));
    strcat(output, ": not found\n");
    return -1;
  }

    //parse path : clip first path to consider
    int i = 0;
    while (path[i] != '\0' && path[i] != path_delim)
    {
      i++; //substring of next path to be considered is path[0,i-1] i-th char excluded
      if (i<21)
      {
        //printf("%c, i = %d \n", path[i], i);
        //fflush(stdout);
      }
    }
    char* path_to_consider = calloc(i, sizeof(char));
    strncpy(path_to_consider, path, i);
    //printf("path_to_consider : %s\n", path_to_consider);
    //fflush(stdout);

    if (executable_is_in_path(command + strlen("type "), path_to_consider))
    {

      strcpy(output, command + strlen("type "));
      strcat(output, " is ");
      strcat(output, path_to_consider);
      strcat(output, "\n");
      return -1;
    }

  return show_executable_in_path(command, output, path+strlen(path_to_consider)+1);
  // Search in entire path environment variable minus the path already tested for

}

int type_(const char* command, char* output)
{
  //printf("at type_");
  fflush(stdout);
  int res;
  if ((res = builtin_type_(command, output)) == 1) //it was a builtin type and builtin_type set output accordingly
    return -1;
  //Search path
  char* path = getenv("PATH");
  printf("path to search : %s\n", path);
  return show_executable_in_path(command, output, path);

}

int echo_(const char* command, char* output)
{
  output = strncpy(output, command+strlen("echo "), strlen(command)-strlen("echo "));
  strcat(output, "\n");
  return -1;
}

int eval(const char* command, char* output)
{
  //printf("at eval\n");
  strcpy(output, empty_string);
  //printf("%d" , hasPrefix(command, "exit"));
  if (hasPrefix(command, "exit"))
  {
    return exit_(command);
  }
  if (hasPrefix(command, "echo"))
  {
    return echo_(command, output);
  }
  if (hasPrefix(command, "type"))
  {
    return type_(command, output);
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
