//
// CS252: MyMalloc Project
//
// The current implementation gets memory from the OS
// every time memory is requested and never frees memory.
//
// You will implement the free() component as indicated in the handout.
// the allocator is implemented for you.
// 
// Also you will need to add the necessary locking mechanisms to
// support multi-threaded programs.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include "MyMalloc.h"

static pthread_mutex_t mutex;

const int ArenaSize = 2097152;
const int NumberOfFreeLists = 1;

// Header of an object. Used both when the object is allocated and freed
struct ObjectHeader {
    size_t _objectSize;         // Real size of the object.
    int _allocated;             // 1 = yes, 0 = no 2 = sentinel
    struct ObjectHeader * _next;       // Points to the next object in the freelist (if free).
    struct ObjectHeader * _prev;       // Points to the previous object.
};

struct ObjectFooter {
    size_t _objectSize;
    int _allocated;
};

  //STATE of the allocator

  // Size of the heap
  static size_t _heapSize;

  // initial memory pool
  static void * _memStart;

  // number of chunks request from OS
  static int _numChunks;

  // True if heap has been initialized
  static int _initialized;

  // Verbose mode
  static int _verbose;

  // # malloc calls
  static int _mallocCalls;

  // # free calls
  static int _freeCalls;

  // # realloc calls
  static int _reallocCalls;
  
  // # realloc calls
  static int _callocCalls;

  // Free list is a sentinel
  static struct ObjectHeader _freeListSentinel; // Sentinel is used to simplify list operations
  static struct ObjectHeader *_freeList;


  //FUNCTIONS

  //Initializes the heap
  void initialize();

  // Allocates an object 
  void * allocateObject( size_t size );

  // Frees an object
  void freeObject( void * ptr );

  // Returns the size of an object
  size_t objectSize( void * ptr );

  // At exit handler
  void atExitHandler();

  //Prints the heap size and other information about the allocator
  void print();
  void print_list();

  // Gets memory from the OS
  void * getMemoryFromOS( size_t size );

  void increaseMallocCalls() { _mallocCalls++; }

  void increaseReallocCalls() { _reallocCalls++; }

  void increaseCallocCalls() { _callocCalls++; }

  void increaseFreeCalls() { _freeCalls++; }

extern void
atExitHandlerInC()
{
  atExitHandler();
}

void initialize()
{
  // Environment var VERBOSE prints stats at end and turns on debugging
  // Default is on
  _verbose = 1;
  const char * envverbose = getenv( "MALLOCVERBOSE" );
  if ( envverbose && !strcmp( envverbose, "NO") ) {
    _verbose = 0;
  }

  pthread_mutex_init(&mutex, NULL);
  void * _mem = getMemoryFromOS( ArenaSize + (2*sizeof(struct ObjectHeader)) + (2*sizeof(struct ObjectFooter)) );

  // In verbose mode register also printing statistics at exit
  atexit( atExitHandlerInC );

  //establish fence posts
  struct ObjectFooter * fencepost1 = (struct ObjectFooter *)_mem;
  fencepost1->_allocated = 1;
  fencepost1->_objectSize = 123456789;
  char * temp = 
      (char *)_mem + (2*sizeof(struct ObjectFooter)) + sizeof(struct ObjectHeader) + ArenaSize;
  struct ObjectHeader * fencepost2 = (struct ObjectHeader *)temp;
  fencepost2->_allocated = 1;
  fencepost2->_objectSize = 123456789;
  fencepost2->_next = NULL;
  fencepost2->_prev = NULL;

  //initialize the list to point to the _mem
  temp = (char *) _mem + sizeof(struct ObjectFooter);
  struct ObjectHeader * currentHeader = (struct ObjectHeader *) temp;
  temp = (char *)_mem + sizeof(struct ObjectFooter) + sizeof(struct ObjectHeader) + ArenaSize;
  struct ObjectFooter * currentFooter = (struct ObjectFooter *) temp;
  _freeList = &_freeListSentinel;
  currentHeader->_objectSize = ArenaSize + sizeof(struct ObjectHeader) + sizeof(struct ObjectFooter); //2MB
  currentHeader->_allocated = 0;
  currentHeader->_next = _freeList;
  currentHeader->_prev = _freeList;
  currentFooter->_allocated = 0;
  currentFooter->_objectSize = currentHeader->_objectSize;
  _freeList->_prev = currentHeader;
  _freeList->_next = currentHeader; 
  _freeList->_allocated = 2; // sentinel. no coalescing.
  _freeList->_objectSize = 0;
  _memStart = (char*) currentHeader;
}

// Attempts to allocate a block to satisfy a memory request. If unsuccessful, returns NULL
void * tryAllocate(int roundedSize){
  struct ObjectHeader * currentHeader = _freeList->_next;
  while(currentHeader != _freeList){
    
    //Let's examine the current object:
    //is it big enough?
    if(currentHeader->_objectSize >= roundedSize){
      
      //can we split it?
      if((currentHeader->_objectSize-roundedSize) > (sizeof(struct ObjectHeader)+sizeof(struct ObjectFooter)+8)){
        
        //yes, the remainder is large enough to split.
        char * splitBlock = (char *)currentHeader + roundedSize;
        struct ObjectHeader * splitHeader = (struct ObjectHeader *)splitBlock;
        splitHeader->_objectSize = currentHeader->_objectSize - roundedSize;
        splitHeader->_allocated = 0;
        splitHeader->_next = currentHeader->_next;
        splitHeader->_prev = currentHeader->_prev;
        char * temp = splitBlock + splitHeader->_objectSize - sizeof(struct ObjectFooter);
        struct ObjectFooter * splitFooter = (struct ObjectFooter *) temp;
        splitFooter->_allocated = 0;
        splitFooter->_objectSize = splitHeader->_objectSize;

	    // Update pointers to this block
        splitHeader->_prev->_next = splitHeader;
	    splitHeader->_next->_prev = splitHeader;

        //return the current block (split from the remainder)
        currentHeader->_objectSize = roundedSize;
        currentHeader->_next = NULL;
        currentHeader->_prev = NULL;
        currentHeader->_allocated = 1; 
        temp = (char *)currentHeader + roundedSize - sizeof(struct ObjectFooter);
        struct ObjectFooter * currentFooter = (struct ObjectFooter *) temp;
        currentFooter->_objectSize = roundedSize;
        currentFooter->_allocated = 1;
        char * returnMem = (char *) currentHeader + sizeof(struct ObjectHeader);
        return (void *) returnMem;
      }
      else{
        
        //otherwise, just return the current block.
        currentHeader->_prev->_next = currentHeader->_next;
	    currentHeader->_next->_prev = currentHeader->_prev;

        currentHeader->_next = NULL;
        currentHeader->_prev = NULL;
        currentHeader->_allocated = 1;

        char * temp = (char *)currentHeader + currentHeader->_objectSize - sizeof(struct ObjectFooter);
        struct ObjectFooter * currentFooter = (struct ObjectFooter *) temp;
        currentFooter->_allocated = 1;
        char * returnMem = (char *) currentHeader + sizeof(struct ObjectHeader);
        return (void *) returnMem;
      }
    }
    
    //endif
    currentHeader = currentHeader->_next;
  } //endwhile
  
  return NULL;
}

void * allocateObject( size_t size )
{
    // Simple implementation

    //Make sure that allocator is initialized
    if ( !_initialized ) {
        _initialized = 1;
    initialize();
    }

    if( size == 0 ){
        size = 1;
    }

    size_t roundedSize = (size + sizeof(struct ObjectHeader) + sizeof(struct ObjectFooter) + 7) & ~7;
    void * retvalue = tryAllocate(roundedSize);
    if(retvalue != NULL){
        pthread_mutex_unlock(&mutex);
        return retvalue;
    }

    //if we made it here, the allocator has run out of memory or cannot satisfy the request
    //GET MORE MEMORY!
    int osRequestedSize = ArenaSize;
    if (osRequestedSize < size) {
        // requested size is larger than memorySize
        osRequestedSize = size;
    }
    
    void * _mem = getMemoryFromOS( osRequestedSize + (2*sizeof(struct ObjectHeader)) + (2*sizeof(struct ObjectFooter)) ); 
  
    //establish fence posts
    struct ObjectFooter * fencepost1 = (struct ObjectFooter *)_mem;
    fencepost1->_allocated = 1;
    fencepost1->_objectSize = 123456789;
    char * temp =
    (char *)_mem + (2*sizeof(struct ObjectFooter)) + sizeof(struct ObjectHeader) + ArenaSize;
    struct ObjectHeader * fencepost2 = (struct ObjectHeader *)temp;
    fencepost2->_allocated = 1;
    fencepost2->_objectSize = 123456789;
    fencepost2->_next = NULL;
    fencepost2->_prev = NULL;
  
    //initialize the new chunk
    temp = (char *) _mem + sizeof(struct ObjectFooter);
    struct ObjectHeader * newBlockHeader = (struct ObjectHeader *) temp;
    temp = (char *)_mem + sizeof(struct ObjectFooter) + sizeof(struct ObjectHeader) + ArenaSize;
    struct ObjectFooter * newBlockFooter = (struct ObjectFooter *) temp;
  
    newBlockHeader->_objectSize = ArenaSize + sizeof(struct ObjectHeader) + sizeof(struct ObjectFooter); //2MB
    newBlockHeader->_allocated = 0;
    newBlockHeader->_next = NULL;
    newBlockHeader->_prev = NULL;
    newBlockFooter->_allocated = 0;
    newBlockFooter->_objectSize = newBlockHeader->_objectSize;

    // Put the block in order for printing
    struct ObjectHeader * insertPtr = _freeList->_next;
    while(insertPtr->_next != _freeList && insertPtr < newBlockHeader) {
        insertPtr = insertPtr->_next;
    }

    // Link
    newBlockHeader->_next = insertPtr;
    newBlockHeader->_prev = insertPtr->_prev;
    insertPtr->_prev->_next = newBlockHeader;
    insertPtr->_prev = newBlockHeader;
  
    //try again
    retvalue = tryAllocate(roundedSize);
    if(retvalue != NULL){
        pthread_mutex_unlock(&mutex);
        return retvalue;
    }

    fprintf(stderr,"CANNOT SATISFY MEMORY REQUEST. RETHINK YOUR CHOICES!\n");
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void freeObject( void * ptr )
{
  // *** Add your code here! ***
  // flag1 and flag2 represent does left and right neighor are free
  // flag3 = 0 represent headerToList will be the last element in _freeList
  	int flag1 = 1;
	int flag2 = 1;
	int flag3 = 0;
	char * temp =(char*) ptr - sizeof(struct ObjectHeader);
	struct ObjectHeader * newHeader = (struct ObjectHeader *)temp;
	char * temp1 = (char*)temp + newHeader->_objectSize - sizeof(struct ObjectFooter);
	struct ObjectFooter * newFooter = (struct ObjectFooter *)temp1;
	newFooter->_allocated = 0;
	newHeader->_allocated = 0;
	size_t freeSize = newHeader->_objectSize;

	//CHECK LEFT NEIGHTBOR
	char * checkLeft = temp - sizeof(struct ObjectFooter);
	struct ObjectFooter * leftFooter = (struct ObjectFooter *)checkLeft;
	struct ObjectHeader * leftHeader;
	size_t leftSize;
	if(leftFooter->_allocated == 0){
		leftSize = leftFooter->_objectSize;
		checkLeft = temp - leftSize;
		leftHeader = (struct ObjectHeader *)checkLeft;
		flag1 = 0;
	}

	//CHECK RIGHT NEIGHTNOR
	char * checkRight = temp + freeSize;
	struct ObjectHeader * rightHeader = (struct ObjectHeader *)checkRight;
	char * checkRight1 = (char*)checkRight + rightHeader->_objectSize - sizeof(struct ObjectFooter);
	struct ObjectFooter * rightFooter = (struct ObjectFooter *)checkRight1;
	size_t rightSize;
	if(rightHeader->_allocated == 0){
		rightSize = rightHeader->_objectSize;
		flag2 = 0;
	}
	//Traversal Header
	struct ObjectHeader * currentHeader = _freeList->_next;
	char * temp2;
	struct ObjectFooter * currentFooter;
	//Create a header and set it to the most left free memory header
	struct ObjectHeader * headerToList;
	//CASE1: left neighbor is free
	if(flag1 == 0 && flag2 == 1){
		leftHeader->_objectSize = leftSize + freeSize;
		newFooter->_objectSize = leftSize + freeSize;
		headerToList = leftHeader;
		//deleting left neighbor
		while(currentHeader != _freeList){
			if(currentHeader == leftHeader){
				currentHeader->_prev->_next = currentHeader->_next;
				currentHeader->_next->_prev = currentHeader->_prev;
			}
			currentHeader = currentHeader->_next;
		}
//		}
	//CASE2: right neighbor is free
	}else if(flag1 == 1 && flag2 == 0){
		newHeader -> _objectSize = freeSize + rightSize;
		rightFooter -> _objectSize = freeSize + rightSize;
		headerToList = newHeader;
		//deleting the right neighbor
		while(currentHeader != _freeList){
			if(currentHeader == rightHeader){
				currentHeader->_prev->_next = currentHeader->_next;
				currentHeader->_next->_prev = currentHeader->_prev;
			}
			currentHeader = currentHeader->_next;
		}
	//CASE3: right and left neighbor are free
	}else if(flag1 == 0 && flag2 == 0){
		leftHeader->_objectSize = leftSize + freeSize + rightSize;
		rightFooter->_objectSize = leftSize + freeSize + rightSize;
		headerToList = leftHeader;
		struct ObjectHeader * reset;

	//deleting the left and right neighbor in the _freeList
		while(currentHeader != _freeList){
			if(currentHeader == leftHeader || currentHeader == rightHeader){
				reset = currentHeader->_prev;
				currentHeader->_prev->_next = currentHeader->_next;
				currentHeader->_next->_prev = currentHeader->_prev;
				currentHeader = reset;
			}
			currentHeader = currentHeader->_next;
		}
	//CASE4: right and left neighbor are not free
	}else if(flag1 == 1 && flag2 == 1){
		headerToList = newHeader;
	}
	
//START to LINK the memort that freed to _freeList
	
  		currentHeader = _freeList->_next;
		if(currentHeader == _freeList){
			//There is no element in _freeList
			_freeList->_next = headerToList;
			_freeList->_prev = headerToList;
			headerToList->_next = _freeList;
			headerToList->_prev = _freeList;
		}else{
			//if this block in in the middle, we just add it in the _freeList, if it is the last element, flag3 will be 0, and we add it in a special case
  			while(currentHeader != _freeList){
				if(currentHeader > headerToList){
					headerToList->_next = currentHeader;
					currentHeader->_prev->_next = headerToList;
					headerToList->_prev = currentHeader->_prev;
					currentHeader->_prev = headerToList;
					flag3 = 1;
					break;
				}
				currentHeader = currentHeader->_next;
			}
			//free block is the last element in the _freeList
			if(flag3 == 0){
				currentHeader = _freeList->_next;
				//finding the last block in the _freeList
				while(currentHeader->_next != _freeList){
					currentHeader = currentHeader->_next;
				}
				headerToList->_next = currentHeader->_next;
				currentHeader->_next->_prev = headerToList;
				headerToList->_prev = currentHeader;
				currentHeader->_next = headerToList;
			}


		}

  	return;

}

size_t objectSize( void * ptr )
{
  // Return the size of the object pointed by ptr. We assume that ptr is a valid obejct.
  struct ObjectHeader * o =
    (struct ObjectHeader *) ( (char *) ptr - sizeof(struct ObjectHeader) );

  // Substract the size of the header
  return o->_objectSize;
}

void print()
{
  printf("\n-------------------\n");

  printf("HeapSize:\t%zd bytes\n", _heapSize );
  printf("# mallocs:\t%d\n", _mallocCalls );
  printf("# reallocs:\t%d\n", _reallocCalls );
  printf("# callocs:\t%d\n", _callocCalls );
  printf("# frees:\t%d\n", _freeCalls );

  printf("\n-------------------\n");
}

void print_list()
{
  printf("FreeList: ");
  if ( !_initialized ) {
    _initialized = 1;
    initialize();
  }
  struct ObjectHeader * ptr = _freeList->_next;
  while(ptr != _freeList){
      long offset = (long)ptr - (long)_memStart;
      printf("[offset:%ld,size:%zd]",offset,ptr->_objectSize);
      ptr = ptr->_next;
      if(ptr != NULL){
          printf("->");
      }
  }
  printf("\n");
}

void * getMemoryFromOS( size_t size )
{
  // Use sbrk() to get memory from OS
  _heapSize += size;
 
  void * _mem = sbrk( size );

  if(!_initialized){
      _memStart = _mem;
  }

  _numChunks++;

  return _mem;
}

void atExitHandler()
{
  // Print statistics when exit
  if ( _verbose ) {
    print();
  }
}

//
// C interface
//

extern void *
malloc(size_t size)
{
  pthread_mutex_lock(&mutex);
  increaseMallocCalls();
  
  return allocateObject( size );
}

extern void
free(void *ptr)
{
  pthread_mutex_lock(&mutex);
  increaseFreeCalls();
  
  if ( ptr == 0 ) {
    // No object to free
    pthread_mutex_unlock(&mutex);
    return;
  }
  
  freeObject( ptr );
}

extern void *
realloc(void *ptr, size_t size)
{
  pthread_mutex_lock(&mutex);
  increaseReallocCalls();
    
  // Allocate new object
  void * newptr = allocateObject( size );

  // Copy old object only if ptr != 0
  if ( ptr != 0 ) {
    
    // copy only the minimum number of bytes
    size_t sizeToCopy =  objectSize( ptr );
    if ( sizeToCopy > size ) {
      sizeToCopy = size;
    }
    
    memcpy( newptr, ptr, sizeToCopy );

    //Free old object
    freeObject( ptr );
  }

  return newptr;
}

extern void *
calloc(size_t nelem, size_t elsize)
{
  pthread_mutex_lock(&mutex);
  increaseCallocCalls();
    
  // calloc allocates and initializes
  size_t size = nelem * elsize;

  void * ptr = allocateObject( size );

  if ( ptr ) {
    // No error
    // Initialize chunk with 0s
    memset( ptr, 0, size );
  }

  return ptr;
}

