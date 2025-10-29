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
char* path_with_executable(const char* executable, char* path);
char* input_read();
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
    char *command  = input_read();
    char *output = calloc(1024, sizeof(char));
    int res = eval(command, output);
    if ((res == 0)||(res ==1))
      exit(res);
    print(output);
  }

  return 0;
}

char* input_read()
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

int type_executable_in_path(const char* command, char* output, char* path)
{
  const char* fullPath = path_with_executable(command +strlen("type "), path);
  if (fullPath == NULL)
  {
    strcpy(output, command + strlen("type "));
    strcat(output, ": not found\n");

  } else
  {
    strcpy(output, command + strlen("type "));
    strcat(output, " is ");
    strcat(output, fullPath);
    strcat(output, "\n");

  }
  return -1;


}

char* path_with_executable(const char* executable, char* path)
{
  if (path == NULL || *path == '\0')
  {
    return nullptr;
  }

    //parse path : clip first path to consider
    int i = 0;
    while (path[i] != '\0' && path[i] != path_delim)
    {
      i++; //substring of next path to be considered is path[0,i-1] i-th char excluded
      {
        //printf("%c, i = %d \n", path[i], i);
        //fflush(stdout);
      }
    }
    char* path_to_consider = calloc(i, sizeof(char));
    strncpy(path_to_consider, path, i);
    //printf("path_to_consider : %s\n", path_to_consider);
    //tfflush(stdout);

    if (executable_is_in_path(executable, path_to_consider))
    {
      char* output = calloc(1024, sizeof(char));

      strcpy(output, path_to_consider);
      strcat(output, "/");
      strcat(output, executable);
      return output;
    }
  const size_t step = strlen(path_to_consider);
  free(path_to_consider);
  return path_with_executable(executable, path+step+1);
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
  //printf("path to search : %s\n", path);
  return type_executable_in_path(command, output, path);

}

int echo_(const char* command, char* output)
{
  output = strncpy(output, command+strlen("echo "), strlen(command)-strlen("echo "));
  strcat(output, "\n");
  return -1;
}

char* parse_till_space(const char* command)
{
  int i = 0;
  while (command[i] != '\0' && command[i] != ' ')
  {
    i++;
  }
  char* prefix = calloc(i, sizeof(char));
  strncpy(prefix, command, i);
  return prefix;
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

  char* path = getenv("PATH");
  //printf("command: %s\n", command);
  const char* executable = parse_till_space(command);
  char* full_path = path_with_executable(executable ,path);
  if (full_path != NULL)
  { //The command exists
    // Retrieve arguments
    //Set up array for storing the arguments
    char** argc = calloc((size_t) 10, sizeof(char*)); //10 = max number of arguments
    for (int i = 0; i<10 ; i++)
    {
      argc[i] = calloc(strlen(full_path)+1, sizeof(char));
    }
    argc[0] = full_path;
    //Keeping track of the rank of our current argument
    int argument_rank=1;
    size_t index_next_argument = strlen(executable) + 1;
    while (command[index_next_argument] != '\0')
    {
      char* current_argument = parse_till_space(command+index_next_argument);
      argc[argument_rank] = current_argument;
      index_next_argument += strlen(current_argument) + 1;
      argument_rank++;

    }
    for (int i = 0; i<10 ; i++)
    {
      //printf("argc[%d] = %s\n", i, argc[i]);
    }
    return execv(full_path, argc);
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
