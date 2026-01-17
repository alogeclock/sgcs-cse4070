#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>

int
main (int argc, char *argv[]) 
{
  if (argc != 5) {
    printf("Usage: ./additional [num1] [num2] [num3] [num4]\n");
    return EXIT_FAILURE;
  }

  int a = atoi(argv[1]);
  int b = atoi(argv[2]);
  int c = atoi(argv[3]);
  int d = atoi(argv[4]);

  int fibonacci_result = fibonacci (a);
  int max_result = max_of_four_int (a, b, c, d);
  printf("%d %d\n", fibonacci_result, max_result);

  return EXIT_SUCCESS;
}