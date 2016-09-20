
#include <stdlib.h>
#include <stdio.h>
#include "MyMalloc.h"

int allocations = 10000;

int
main( int argc, char **argv )
{
  char **alist = (char**)malloc(allocations*sizeof(char*)); // Meta-testing!
  char *p1;

  printf("\n---- Running test2 ---\n");
  
  //test performs many small allocations, up to 2MB
  int i;
  for ( i = 0; i < allocations; i++ ) {
    alist[i] = (char *) malloc(100 );
    p1 = alist[i];
    *p1 = 100;
  }
  printf("After allocation\n");
  print_list();

  //free small allocations
  for (i = 0; i < allocations; i++) {
    free(alist[i]);
  }
  free(alist);

  printf("After freeing\n");
  print_list();

  exit( 0 );
}

