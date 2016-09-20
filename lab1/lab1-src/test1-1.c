
#include <stdlib.h>
#include <stdio.h>
#include "MyMalloc.h"

int
main( int argc, char **argv )
{

  printf("\n---- Running test1-1 ---\n");

  printf("Before any allocation\n");
  print_list();

  //test designed to coalesce from both sides of a block
  int * mem1 = (int *) malloc( 8 );
  mem1[0] = 1;
  mem1[1] = 2;

  printf("After allocation\n");
  print_list();

  free(mem1);

  printf("After freeing\n");
  print_list();

  exit(0);
}
