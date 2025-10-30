#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/syslimits.h>

#ifdef _WIN32
  const char path_delim = ';';
#else
const char path_delim = ':';
#endif

char* parse_till_space(const char* command);
bool is_executable_in_this_path(const char* str, char* path_to_consider);
char* find_path_to_executable(const char* executable);
char* find_path_to_executable_rec(const char* executable, char* path);
char* input_read();
int eval(const char* command, char* output);
void print(char* output);
bool hasPrefix(const char* command, const char* prefix);
int cd_command(char* const* argv, char* output);

char empty_string[1024];
char* const builtin_list[]= {
  "type",
  "echo",
  "shell",
  "exit",
  "pwd",
  "cd"
};
int const builtin_list_length = 6;

int main(int argc, char *argv[]) {
  // Flush after every printf
  while (true)
  {
    const char *command  = input_read();
    char *output = calloc(1024, sizeof(char));
    const int res = eval(command, output);
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
  return (strncmp(prefix, command, strlen(prefix)) == 0);
}

//output is unused but is kept so that all behaviors (except for echo) conform to an "interface"
int exit_(char* const* argv, char* output)
{
  if (argv == NULL || argv[1]==NULL )
    return -1;
  return argv[1][0]-48;
}

// -------------------------- General purpose command parser ---------------------
char* const* parse_command(const char* command)
{
  char** argv = calloc(10, sizeof(char*));
  size_t index_in_str_current_argument = 0;
  size_t index_current_argument = 0;
  while (command[index_in_str_current_argument] != '\0')
  {
    char* argument = parse_till_space(command+index_in_str_current_argument);
    argv[index_current_argument] = argument;
    index_current_argument++;
    index_in_str_current_argument += strlen(argument)+1;
  }
  return argv;
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


//------------------------- Helper function for type and external execution ------

bool is_executable_in_this_path(const char* str, char* path_to_consider)
{
  char fullPath [1024];
  snprintf(fullPath, sizeof(fullPath), "%s/%s", path_to_consider, str);

  if (access(fullPath, X_OK) == 0)
    return true;
  return false;
}



char* find_path_to_executable(const char* executable)
{
  char* path = getenv("PATH");
  return find_path_to_executable_rec(executable, path);

}

char* find_path_to_executable_rec(const char* executable, char* path)
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

    if (is_executable_in_this_path(executable, path_to_consider))
    {
      char* output = calloc(1024, sizeof(char));

      strcpy(output, path_to_consider);
      strcat(output, "/");
      strcat(output, executable);
      return output;
    }
  const size_t step = strlen(path_to_consider);
  free(path_to_consider);
  return find_path_to_executable_rec(executable, path+step+1);
  // Search in entire path environment variable minus the path already tested for

}

char* last_indirection(const char* path)
{
  size_t index = strlen(path) - 1;
  while (index != 0 && path[index] != '/')
  {

    index--;
  }
  char* suffix = calloc(strlen(path)-index, sizeof(char));
  suffix = strcpy(suffix, path+index);
  return suffix;
}

char* back(const char* path)
{

  char* suffix = last_indirection(path);
  if (strcmp(suffix, "/") == 0)
  {
    char* new_path = calloc(strlen(path)-1, sizeof(char));
    strncpy(new_path, path, strlen(new_path));
    return back(new_path);
  }
  char* new_path = calloc(strlen(path)-strlen(suffix), sizeof(char));
  strncpy(new_path, path, strlen(new_path));
  return new_path;
}



//-------------------------- Different eval behaviors ---------------------------


int cd_absolute_path_subcommand(const char* path, char* output)
{
  if (path!=NULL && strlen(path)>0 && 0==chdir(path))
  {
    output[0] = '\0';
    return -1;
  } else
  {
    struct stat statbuf;
    if (0==stat(path,&statbuf))
    {
      //file exists but cannot be open as directory => assume is not a directory
      strcpy(output, "cd: ");
      strcat(output, path);
      strcat(output, ": Not a directory\n");
      return -1;

    }
    else
    {
      //file status could not be obtained => assume file doens't exist
      strcpy(output, "cd: ");
      strcat(output, path);
      strcat(output, ": No such file or directory\n");
      return -1;
    }
  }
}

int cd_relative_path_subcommand_rec(char* path_to_compute, char* output, char* current_location_path)
{
  if (hasPrefix(path_to_compute,"../"))
  {
    char* new_path_to_compute = calloc(strlen(path_to_compute)-3, sizeof(char));
    strncpy(new_path_to_compute, path_to_compute + 3, strlen(new_path_to_compute));

    return cd_relative_path_subcommand_rec(new_path_to_compute, output, back(current_location_path));
  }
  else
  {
    char* fullpath = calloc(strlen(path_to_compute)+strlen(current_location_path)+1, sizeof(char));
    strcpy(fullpath, current_location_path);
    strcat(fullpath, "/");
    strcat(fullpath, path_to_compute);
    return cd_absolute_path_subcommand(fullpath, output);
  }
}

int cd_relative_path_subcommand(const char* relative_path, char* output)
{
  char* path_to_compute = calloc(strlen(relative_path)-2, sizeof(char));
  if (hasPrefix(relative_path,"./"))
  {
    strncpy(path_to_compute, relative_path + 2, strlen(path_to_compute));

  } else
  {
    strncpy(path_to_compute, relative_path + 3, strlen(path_to_compute)-1);
  }

    char*start_location = calloc(PATH_MAX, sizeof(char));
    getcwd(start_location, PATH_MAX);
    return cd_relative_path_subcommand_rec(path_to_compute, output, start_location );

}

int cd_command(char* const* argv, char* output)
{
  if (argv==NULL || argv[1] == NULL)
    return -1;
  if (argv[2] != NULL)
  {
    strcpy(output, "cd: too many arguments");
  }
  if (hasPrefix(argv[1],"/")==true)
  {
    return cd_absolute_path_subcommand(argv[1], output);
  }
  if (hasPrefix(argv[1],".")==true)
  {
    return cd_relative_path_subcommand(argv[1], output);
  }

  return -1;
}

int pwd_command(char* const* argv, char* output)
{

  if (argv!= NULL && argv[1] == NULL)
  {
    getcwd(output, 1024);
    strcat(output, "\n");

  }else
  {
    strcpy(output, "pwd: too many arguments");
  }

  return -1;
}

int type_builtin_subcommand(const char* command, char* output)
{

  for (int i = 0; i<builtin_list_length; i++)
  {
    if (strcmp(command, builtin_list[i]) == 0)
    {
      strcpy(output, command);
      strcat(output, " is a shell builtin\n");
      return 1;
    }
  }
  return -1;

}

int type_not_builtin_subcommand(const char* executable_candidate, char* output)
{
  const char* fullPath = find_path_to_executable(executable_candidate);
  if (fullPath == NULL)
  {
    strcpy(output, executable_candidate);
    strcat(output, ": not found\n");

  } else
  {
    strcpy(output, executable_candidate);
    strcat(output, " is ");
    strcat(output, fullPath);
    strcat(output, "\n");

  }
  return -1;
}

int type_(char* const* argv, char* output)
{
  if (argv == NULL || argv[1] == NULL)
  {
    return -1;

  }

  if (type_builtin_subcommand(argv[1], output) == 1) //it was a builtin type and builtin_type set output accordingly
    return -1;
  //Search path
  return type_not_builtin_subcommand(argv[1], output);

}
//We don't use parser because we would need many arguments to accomodate for the uses of " " (currently we only allow up to 10 arguments)
int echo_(const char* entire_command, char* output)
{
  output = strncpy(output, entire_command+strlen("echo "), strlen(entire_command)-strlen("echo "));
  strcat(output, "\n");
  return -1;
}

// -------------------------- Eval behavior ---------------

int eval(const char* command, char* output)
{
  //printf("at eval\n");
  strcpy(output, empty_string);
  //printf("%d" , hasPrefix(command, "exit"));
  char* const* argv = parse_command(command);
  if (0 == strcmp(argv[0], "cd"))
  {
    return cd_command(argv, output);
  }
  if (0 == strcmp(argv[0], "pwd"))
  {
    return pwd_command(argv, output);
  }
  if (0 == strcmp(argv[0], "exit"))
  {
    return exit_(argv, output);
  }
  if (0 == strcmp(argv[0], "echo"))
  {
    return echo_(command, output);
  }
  if (0 == strcmp(argv[0], "type"))
  {
    return type_(argv, output);
  }





  //printf("command: %s\n", command);
  const char* executable = argv[0];
  const char* full_path = find_path_to_executable(executable);
  if (full_path == NULL)
  {

    strcpy(output, command);
    const char* suffix = ": command not found\n";
    strcat(output, suffix);
    return -1;
}

  pid_t pid = fork();
  if (pid == 0)
  {
    execv(full_path, argv);
    perror("execv failed");  // good practice: print error if exec fails
    _exit(EXIT_FAILURE);     // exit child safely if exec fails

  } else
  {
    int status;
    waitpid(pid, &status, 0);
    return -1;
  }



}

void print(char* output)
{
  if (output != NULL && output[0] != '\0')
    printf("%s", output);
}
