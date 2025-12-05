#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

// System dependant macros

#ifdef _WIN32
const char path_delim = ';';
#else
const char path_delim = ':';
#endif

// Constants
#define OUTPUT_SIZE 1024
const char empty_string[1024];
///list of builtin commands
char *const builtin_list[] = {"type", "echo", "shell", "exit", "pwd", "cd"};
int const builtin_list_length = 6;


// Signatures

// REPL functions
/**
 * @brief performs the repl loop
 * requires interruption from outside context to exit otherwise loops perpetually
 */
void repl(void);


/**
 * @brief Listens for input from the stdin
 * @return a string containing the input from stdin
 */
char *input_read();

/**
 * @brief evaluates the command and updates the output accordingly
 * @param command pointer to the string representing the command
 * @param output pointer to the region to be updated by eval
 * @note eval assumes output is allocated to a size of OUTPUT_SIZE
 * @return -1 in all cases or {0,1} in the of exit{0,1}
 */
int eval(const char *command, char *output);

/**
 * @brief prints the output string to stdout
 * @param output pointer to the string to be printed
 */
void print(char *output);

// Specific commands

/**
 * @brief performs a CHNGE command with the argv arguments
 * @param argv arguments of the CHNGE command
 * @param output the output to be updated
 * @return
 */

/**
 * @brief performs a cd command with the argv arguments
 * @param argv arguments of the cd command
 * @param output the output to be updated
 * @return -1
 * @note meant to be called in any case of command prefixed with cd
 */
int cd_command(char *const *argv, char *output);

/**
 * @brief performs a cd command to an absolute path
 * @param path the absolute path to set the working directory to
 * @param output the output to be updated
 * @return -1
 */
int cd_absolute_path_subcommand(const char *path, char *output);

/**
 * @brief performs a cd command to a relative path
 * @param path the relative path to set the working directory to
 * @param output the output to be updated
 * @return -1
 */
int cd_relative_path_subcommand(const char *path, char *output);

/**
 * @brief performs an exit command
 * @param argv arguments of the exit command
 * @param output to be updated, in this case is not modified
 * @return 0 or 1 in normal case
 * @note assumes the second argument is in {'1'|'0'}
 * if not returns with a value that isn't significant
 */
int exit_(char *const *argv, char *output);

/**
 * @brief performs a pwd command with the argv arguments
 * @param argv arguments of the pwd command
 * @param output the output to be updated
 * @return -1
 */
int pwd_command(char *const *argv, char *output);

/**
 * @brief performs a type command with the argv arguments
 * @param argv arguments of the type command
 * @param output the output to be updated
 * @return -1
 */
int type_(char *const *argv, char *output);

/**
 * @brief checks whether a command is builtin. If it is, updates the output as per a "type \command" command
 * @param command the command to be checked
 * @param output the output to be updated
 * @return whether the command was builtin or not
 * @note meant to be called first and the result to be observed before assuming command is not builtin
 */
bool if_builtin_type_subcommand(const char *command, char *output);

/**
 * @brief performs a "type \executable_candidate" command where executable_candidate is assumed to not be builtin
 * @param executable_candidate the executable to be found
 * @param output the output to be updated
 * @return -1
 */
int type_not_builtin_subcommand(const char *executable_candidate,
                                char *output);

/**
 * @brief performs a type command with the argv arguments
 * @param entire_command entirety of the command that is prefixed with "echo "
 * @param output the output to be update
 * @return -1
 * @note is called only in the context of an echo command
 * it was not made to conform to the other command function to accomodate for many space chars
 */
int echo_(const char *entire_command, char *output);




//Parsing methods
/**
 * parses a command into its arguments
 * @param command pointer to the entire string of the command
 * @return pointer to const strings of the arguments in the same manner as argv
 * @note this method does not account for special behavior in the case of an echo command
 */
char *const *parse_command(const char *command);

/**
 * parses the first argument of a command and writes it into argv
 * @param to_parse the command to parse
 * @param argv pointer to the string of the next argument to set
 * @return the part of the command that is left to parse
 */
char *parse_argument(const char* to_parse, char **argv);

/**
 * @brief returns whether a string has a prefix
 * @param string pointer to the string
 * @param prefix pointer to the candidate prefix string
 * @return whether the string command has the string prefix at the start
 */
bool hasPrefix(const char *string, const char *prefix);

/**
 * @brief returns the substring of a string before the first occurence of " "
 * @param string pointer to the string to parse
 * @return the substring
 */
char *take_until_space(const char *string);

/**
 * @brief returns the substring of a string before the first occurence of "'"
 * @param string pointer to the string to parse
 * @return the substring
 */
char *take_until_quote(const char *string);

/**
 * removes leading ( |\t) characters
 * @param str
 * @return
 */
char *remove_leading_whitespace(char *str);

/**
 * @brief returns a string with the trailing char removes
 * @param string the original string
 * @param length the number of characters to remove
 * @return the string with the trailing characters removed
 */
char *remove_trailing(const char *string, size_t length);

/**
 * @brief returns whether there is an executable file called executable it the path_to_consider
 * @param executable pointer to the candidate executable name
 * @param path_to_consider pointer to the candidate absolute path name
 * @return whether the executable was found with the appropriate permission
 */
bool is_executable_in_this_path(const char *executable, char *path_to_consider);

/**
 * @brief converts a relative path into an absolute path using the cwd as origin
 * @param path_to_compute string representing a relative path
 * @return absolute path
 */
char *from_relative_to_absolute_path(const char *path_to_compute);

/**
 * @brief converts a relative path into an absolute path provided an absolute path as origin
 * @param path_to_compute string representing a relative path
 * @param current_location_path string representing an absolute path
 * @return absolute path
 */
char *from_relative_to_absolute_path_rec(const char *path_to_compute,
                                         const char *current_location_path);

/**
 * @brief returns the absolute path to the executable provided it can be reached from the path environment
 * @param executable candidate executable name
 * @return NULL or absolute path to an executable
 * @note if the executable is not found with the correct permission in the path environment, NULL is returned
 */
char *find_path_to_executable(const char *executable);

/**
 * @brief returns the absolute path to the executable among the list of candidate paths passed in argument
 * @param executable string of the executable file name
 * @param paths pointer to a string containing all path location separated by the global path_delim parameter
*  @return NULL or absolute path to an executable
 * @note if the executable is not found with the correct permission in the candidate paths, NULL is returned
 */
char *find_path_to_executable_rec(const char *executable, char *paths);

/**
 * @brief returns the last indirection in a path as a string
 * @param path string name of the path
 * @return string of the final suffix /$ or entire path
 */
char *last_indirection(const char *path);

/**
 * @brief returns the parent folder of the file described by path or route
 * @param path string name of the path
 * @return path file to parent folder
 * @note if the path does not have a parent then root is returned
 */
char *parent_folder(const char *path);

// Test function
/**
 * @brief test repl meant to be edited to test specific functionalities
 */
void test_repl();

//Main

int main(int argc, char *argv[]) {
  repl();

  return 0;
}


// Implementations

void repl(void) {
  while (true) {
    const char *command = input_read();
    char *output = calloc(OUTPUT_SIZE, sizeof(char));
    const int res = eval(command, output);
    if ((res == 0) || (res == 1))
      exit(res);
    print(output);
  }
}



char *input_read() {
  char *line = NULL;
  size_t lineSize = 0;
  printf("$ ");
  getline(&line, &lineSize, stdin);
  if (lineSize > 1)
    line[strlen(line) - 1] = '\0';
  return line;
}

int eval(const char *command, char *output) {
  strcpy(output, empty_string);
  char *const *argv = parse_command(command);
  if (0 == strcmp(argv[0], "cd")) {
    return cd_command(argv, output);
  }
  if (0 == strcmp(argv[0], "pwd")) {
    return pwd_command(argv, output);
  }
  if (0 == strcmp(argv[0], "exit")) {
    return exit_(argv, output);
  }
  if (0 == strcmp(argv[0], "echo")) {
    return echo_(command, output);
  }
  if (0 == strcmp(argv[0], "type")) {
    return type_(argv, output);
  }

  const char *executable = argv[0];
  const char *full_path = find_path_to_executable(executable);
  if (full_path == NULL) {

    strcpy(output, command);
    const char *suffix = ": command not found\n";
    strcat(output, suffix);
    return -1;
  }

  pid_t pid = fork();
  if (pid == 0) {
    execv(full_path, argv);
    perror("execv failed"); // good practice: print error if exec fails
    _exit(EXIT_FAILURE);    // exit child safely if exec fails

  } else {
    int status;
    waitpid(pid, &status, 0);
    return -1;
  }
}

void print(char *output) {
  if (output != NULL && output[0] != '\0')
    printf("%s", output);
}

int cd_command(char *const *argv, char *output) {
  if (argv == NULL || argv[1] == NULL)
    return -1;
  if (argv[2] != NULL) {
    strcpy(output, "cd: too many arguments");
  }
  if (hasPrefix(argv[1], "/") == true) {
    return cd_absolute_path_subcommand(argv[1], output);
  }

  return cd_relative_path_subcommand(argv[1], output);
}

int cd_absolute_path_subcommand(const char *path, char *output) {
  if (path != NULL && strlen(path) > 0 && 0 == chdir(path)) {
    if (output != NULL)
      output[0] = '\0';
    return -1;
  } else {
    struct stat statbuf;
    if (0 == stat(path, &statbuf)) {
      // file exists but cannot be open as directory => assume is not a
      // directory
      strcpy(output, "cd: ");
      strcat(output, path);
      strcat(output, ": Not a directory\n");
      return -1;

    } else {
      // file status could not be obtained => assume file doens't exist
      strcpy(output, "cd: ");
      strcat(output, path);
      strcat(output, ": No such file or directory\n");
      return -1;
    }
  }
}

int cd_relative_path_subcommand(const char *relative_path, char *output) {
  return cd_absolute_path_subcommand(
      from_relative_to_absolute_path(relative_path), output);
}

int pwd_command(char *const *argv, char *output) {

  if (argv != NULL && argv[1] == NULL) {
    getcwd(output, 1024);
    strcat(output, "\n");

  } else {
    strcpy(output, "pwd: too many arguments");
  }

  return -1;
}

int exit_(char *const *argv, char *output) {
  if (argv == NULL || argv[1] == NULL)
    return -1;
  return argv[1][0] - 48;
}

int type_(char *const *argv, char *output) {
  if (argv == NULL || argv[1] == NULL) {
    return -1;
  }

  if (if_builtin_type_subcommand(argv[1], output) ==
      1) // it was a builtin type and builtin_type set output accordingly
        return -1;
  // Search path
  return type_not_builtin_subcommand(argv[1], output);
}


bool if_builtin_type_subcommand(const char *command, char *output) {

  for (int i = 0; i < builtin_list_length; i++) {
    if (strcmp(command, builtin_list[i]) == 0) {
      strcpy(output, command);
      strcat(output, " is a shell builtin\n");
      return true;
    }
  }
  return false;
}

int type_not_builtin_subcommand(const char *executable_candidate,
                                char *output) {
  const char *fullPath = find_path_to_executable(executable_candidate);
  if (fullPath == NULL) {
    strcpy(output, executable_candidate);
    strcat(output, ": not found\n");

  } else {
    strcpy(output, executable_candidate);
    strcat(output, " is ");
    strcat(output, fullPath);
    strcat(output, "\n");
  }
  return -1;
}



int echo_(const char *entire_command, char *output) {
  output = strncpy(output, entire_command + strlen("echo "),
                   strlen(entire_command) - strlen("echo "));
  strcat(output, "\n");
  return -1;
}


char *const *parse_command(const char *command) {
  char *left_to_parse = calloc(strlen(command), sizeof(char));
  strcpy(left_to_parse, command);
  char **argv = calloc(10, sizeof(char *));

  size_t index_in_str_current_argument = 0;
  size_t index_current_argument = 0;

  left_to_parse = remove_leading_whitespace(left_to_parse);
  while (*left_to_parse != '\0') {
    left_to_parse =
        parse_argument(left_to_parse, &argv[index_current_argument]);
    index_current_argument++;
  }
  return argv;
}

char *parse_argument(const char* to_parse, char **argv) {
  if (to_parse[0] == '\'') {
    char *argument = take_until_quote(to_parse);
    to_parse += strlen(argument) + 2;
    *argv = argument;
    char *left_to_parse =
        calloc(strlen(to_parse) - strlen(argument), sizeof(char));
    strcpy(left_to_parse, to_parse);
    return left_to_parse;
  } else {
    char *argument = take_until_space(to_parse);
    to_parse += strlen(argument);
    char *left_to_parse =
        calloc(strlen(to_parse) - strlen(argument), sizeof(char));
    strcpy(left_to_parse, to_parse);
    *argv = argument;
    return left_to_parse;
  }
}



bool hasPrefix(const char *string, const char *prefix) {
  if (strlen(prefix) > strlen(string))
    return false;
  return (strncmp(prefix, string, strlen(prefix)) == 0);
}
char *take_until_quote(const char *string) {
  int i = 0;
  while (string[i] != '\0' && string[i] != '\'') {
    i++;
  }
  char *prefix = calloc(i, sizeof(char));
  strncpy(prefix, string, i);
  return prefix;
}

char *take_until_space(const char *string) {
  int i = 0;
  while (string[i] != '\0' && string[i] != ' ') {
    i++;
  }
  char *prefix = calloc(i, sizeof(char));
  strncpy(prefix, string, i);
  return prefix;
}



char *remove_leading_whitespace(char *str) {
  while (*str == ' ' || *str == '\t') {
    str++;
  }
  return str;
}

char *remove_trailing(const char *string, size_t length) {
  char *res = calloc(strlen(string) - length, sizeof(char));
  strncpy(res, string, strlen(string) - length);
  fflush(stdout);
  return res;
}



bool is_executable_in_this_path(const char *executable, char *path_to_consider) {
  char fullPath[OUTPUT_SIZE];
  snprintf(fullPath, sizeof(fullPath), "%s/%s", path_to_consider, executable);

  if (access(fullPath, X_OK) == 0)
    return true;
  return false;
}

char *find_path_to_executable(const char *executable) {
  char *path = getenv("PATH");
  return find_path_to_executable_rec(executable, path);
}

char *find_path_to_executable_rec(const char *executable, char *paths) {
  if (paths == NULL || *paths == '\0') {
    return NULL;
  }

  // parse path : clip first path to consider
  int i = 0;
  while (paths[i] != '\0' && paths[i] != path_delim) {
    i++; // substring of next path to be considered is path[0,i-1] i-th char
         // excluded
  }
  char *path_to_consider = calloc(i, sizeof(char));
  strncpy(path_to_consider, paths, i);

  if (is_executable_in_this_path(executable, path_to_consider)) {
    char *result = calloc(1024, sizeof(char));

    strcpy(result, path_to_consider);
    strcat(result, "/");
    strcat(result, executable);
    return result;
  }
  const size_t step = strlen(path_to_consider);
  free(path_to_consider);
  return find_path_to_executable_rec(executable, paths + step + 1);

}

char *last_indirection(const char *path) {
  size_t index = strlen(path) - 1;
  while (index != 0 && path[index] != '/') {

    index--;
  }
  char *suffix = calloc(strlen(path) - index, sizeof(char));
  suffix = strcpy(suffix, path + index);
  return suffix;
}

char *parent_folder(const char *path) {
  char *new_path = remove_trailing(path, strlen(last_indirection(path)));
  if (strlen(new_path) < 1)
    return "/";
  return new_path;

}

//-------------------------- Different eval behaviors
//---------------------------



char *from_relative_to_absolute_path_rec(const char *path_to_compute,
                                         const char *current_location_path) {
  if (hasPrefix(path_to_compute, "../")) {
    return from_relative_to_absolute_path_rec(
        path_to_compute + strlen("../"), parent_folder(current_location_path));
  }
  if (hasPrefix(path_to_compute, "..")) {
    return from_relative_to_absolute_path_rec(
        path_to_compute + strlen(".."), parent_folder(current_location_path));
  }
  if (hasPrefix(path_to_compute, "~/")) {
    char *home_location = getenv("HOME");
    return from_relative_to_absolute_path_rec(path_to_compute + strlen("~/"),
                                              home_location);
  }
  if (hasPrefix(path_to_compute, "~")) {
    char *home_location = getenv("HOME");
    return from_relative_to_absolute_path_rec(path_to_compute + strlen("~"),
                                              home_location);
  }
  char *fullpath =
      calloc(strlen(path_to_compute) + strlen(current_location_path) + 1,
             sizeof(char));
  strcpy(fullpath, current_location_path);
  strcat(fullpath, "/");
  strcat(fullpath, path_to_compute);
  return fullpath;
}
char *from_relative_to_absolute_path(const char *path_to_compute) {
  char *current_location = calloc(OUTPUT_SIZE, sizeof(char));
  getcwd(current_location, OUTPUT_SIZE);
  if (hasPrefix(path_to_compute, "./")) {
    return from_relative_to_absolute_path_rec(path_to_compute + strlen("./"),
                                              current_location);
  }
  return from_relative_to_absolute_path_rec(path_to_compute, current_location);
}




//------------------------- test --------------------------------------

void test_has_prefix(const char *command, char *output, char *prefix) {
  char *positive_message = calloc(1024, sizeof(char));
  char *negative_message = calloc(1024, sizeof(char));
  strcpy(negative_message, "The string does not have prefix ");
  strcpy(positive_message, "The string has prefix ");
  strcat(negative_message, prefix);
  strcat(negative_message, "\n");
  strcat(positive_message, prefix);
  strcat(positive_message, "\n");

  strcpy(output,
         (hasPrefix(command, prefix) ? positive_message : negative_message));
}

void test_parent_folder(const char *command, char *output) {
  strcpy(output, parent_folder(command));
}

void test_last_indirection(const char *command, char *output) {
  char *const *argv = parse_command(command);
  strcat(output, last_indirection(argv[0]));
  strcat(output, "\n");
}

void test_remove_suffix(const char *command, char *output) {
  char *const *argv = parse_command(command);
  char *res = remove_trailing(argv[0], strlen(argv[1]));
  strcat(output, res);
  strcat(output, "\n");
}

void test_from_relative_to_absolute_path(const char *command, char *output) {
  strcat(output, from_relative_to_absolute_path(command));
}

void test_parse_argument(const char *command, char *output) {
  char **to_parse = calloc(1, sizeof(char *));
  to_parse[0] = calloc(strlen(command), sizeof(char));
  strcpy(to_parse[0], command);
  // strcat(output, parse_argument(to_parse));
}

void test_parse_command(const char *command, char *output) {
  char *const *argv = parse_command(command);
  int index = 0;


  while (index<10) {
    strcat(output, "arg: ");
    snprintf(output + strlen(output), sizeof(output) - strlen(output), "%d", index );
    strcat(output, argv[index]);
    strcat(output, "\n");
    index++;
  }

}

int test_eval(const char *command, char *output) {
  test_parse_command(command, output);
  return -1;
}

void test_repl() {
  while (true) {
    const char *command = input_read();
    char *output = calloc(OUTPUT_SIZE, sizeof(char));
    const int res = test_eval(command, output);
    if ((res == 0) || (res == 1))
      exit(res);
    print(output);
  }
}
