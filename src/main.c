#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  // Flush after every printf
  char command[1024];
  scanf("%s", command ,stdin);
  printf("%s: command not found", command);

  return 0;
}
