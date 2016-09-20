

#include <stdlib.h>
#include <stdio.h>
#include "MyMalloc.h"

int allocations = 50000;

int
main( int argc, char **argv )
{
    char **alist = (char**)malloc(allocations*sizeof(char*));
    char *p1;

    printf("\n---- Running test4 ---\n");
  
    //test performs many small allocations, beyond 2MB.
    //this means more blocks must be requested from the OS.
    int i;
    for ( i = 0; i < allocations; i++ ) {
        alist[i] = (char *) malloc(100 );
        p1 = alist[i];
        *p1 = 100;
    }
    
    printf("After allocations\n");
    print_list();
    
    // Then free every *other* allocation
    for (i = 0; i < allocations; i += 2) {
        free(alist[i]);
    }
    
    printf("After partial free\n");
    print_list();

    // Then free the rest
    for (i = 1; i < allocations; i += 2) {
        free(alist[i]);
    }
    free(alist);

    printf("After total free\n");
    print_list();

    exit( 0 );
}

