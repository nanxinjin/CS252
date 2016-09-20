
const char * usage =
  "                                                               \n"
  "daytime-server:                                                \n"
  "                                                               \n"
  "Simple server program that shows how to use socket calls       \n"
  "in the server side.                                            \n"
  "                                                               \n"
  "To use it in one window type:                                  \n"
  "                                                               \n"
  "   daytime-server <port>                                       \n"
  "                                                               \n"
  "Where 1024 < port < 65536.             \n"
  "                                                               \n"
  "In another window type:                                       \n"
  "                                                               \n"
  "   telnet <host> <port>                                        \n"
  "                                                               \n"
  "where <host> is the name of the machine where daytime-server  \n"
  "is running. <port> is the port number you used when you run   \n"
  "daytime-server.                                               \n"
  "                                                               \n"
  "Then type your name and return. You will get a greeting and   \n"
  "the time of the day.                                          \n"
  "                                                               \n";


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>

int QueueLength = 5;

// Processes time request
void processTimeRequest( int socket );
//this function is for thread
void processRequest(int socket);
//this function is for thread pool
void poolSlave(int socket);

//Kill zombie process
extern "C" void killzombie(int sig)
{
  while (waitpid(-1, NULL, WNOHANG) > 0);
}

pthread_mutex_t mutex;
void processRequest(int socket) {
  processTimeRequest(socket);
  close(socket);
}

void poolSlave(int socket) {
  while (1) {
    struct sockaddr_in clientIPAddress;
    int alen = sizeof( clientIPAddress );
    pthread_mutex_lock(&mutex);
    int slaveSocket = accept( socket,
                              (struct sockaddr *)&clientIPAddress,
                              (socklen_t*)&alen);
    pthread_mutex_unlock(&mutex);
    if ( slaveSocket < 0 ) {
      perror( "accept" );
      exit( -1 );
    }

    // Process request.
    processTimeRequest( slaveSocket );
    close(slaveSocket);

  }

}

int
main( int argc, char ** argv )
{
  // Print usage if not enough arguments

  int port;
  char * mode;
  int flag = 0;

  //Kill zombie process
  struct sigaction signalAction;
  signalAction.sa_handler = killzombie;
  sigemptyset(&signalAction.sa_mask);
  signalAction.sa_flags = SA_RESTART;
  int err = sigaction(SIGCHLD, &signalAction, NULL);
  if (err) {
    perror("sigaction");
    exit(-1);
  }

  // Get the port from the arguments
  if (argc == 2) {
    port = atoi( argv[1] );

  } else if (argc == 3) {
    port = atoi( argv[2] );
    flag = 1;
  } else if (argc == 1) {
    port = 9999;
  }

  // Set the IP address and port for this server
  struct sockaddr_in serverIPAddress;
  memset( &serverIPAddress, 0, sizeof(serverIPAddress) );
  serverIPAddress.sin_family = AF_INET;
  serverIPAddress.sin_addr.s_addr = INADDR_ANY;
  serverIPAddress.sin_port = htons((u_short) port);

  // Allocate a socket
  int masterSocket =  socket(PF_INET, SOCK_STREAM, 0);
  if ( masterSocket < 0) {
    perror("socket");
    exit( -1 );
  }

  // Set socket options to reuse port. Otherwise we will
  // have to wait about 2 minutes before reusing the sae port number
  int optval = 1;
  err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR,
                   (char *) &optval, sizeof( int ) );

  // Bind the socket to the IP address and port
  int error = bind( masterSocket,
                    (struct sockaddr *)&serverIPAddress,
                    sizeof(serverIPAddress) );
  if ( error ) {
    perror("bind");
    exit( -1 );
  }

  // Put socket in listening mode and set the
  // size of the queue of unprocessed connections
  error = listen( masterSocket, QueueLength);
  if ( error ) {
    perror("listen");
    exit( -1 );
  }


  if (flag == 0) {
    //basic case
    while (1) {
      struct sockaddr_in clientIPAddress;
      int alen = sizeof( clientIPAddress );
      int slaveSocket = accept( masterSocket,
                                (struct sockaddr *)&clientIPAddress,
                                (socklen_t*)&alen);
      if ( slaveSocket == -1 && errno == EINTR) {
        perror( "accept" );
        continue;
      }
      processTimeRequest( slaveSocket );
      close(slaveSocket);
    }
  } else if (flag == 1 && !strcmp(argv[1], "-f")) {
    //fork a process
    while (1) {
      struct sockaddr_in clientIPAddress;
      int alen = sizeof( clientIPAddress );
      int slaveSocket = accept( masterSocket,
                                (struct sockaddr *)&clientIPAddress,
                                (socklen_t*)&alen);
      if ( slaveSocket == -1 && errno == EINTR) {
        perror( "accept" );
        continue;
      }
      int pid = fork();
      if (pid == 0) {
        //child process
        processTimeRequest( slaveSocket );
        close(slaveSocket);
        exit(2);
      }
      //parent process
      close(slaveSocket);
    }
  } else if (flag == 1 && !strcmp(argv[1], "-t")) {
    while (1) {
      //thread option
      struct sockaddr_in clientIPAddress;
      int alen = sizeof( clientIPAddress );
      int slaveSocket = accept( masterSocket,
                                (struct sockaddr *)&clientIPAddress,
                                (socklen_t*)&alen);
      pthread_t t;
      pthread_attr_t attr;
      pthread_attr_init( &attr );
      pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
      pthread_create( &t, &attr, (void * (*)(void *)) processRequest,
                      (void *) slaveSocket);
    }
  } else if (flag == 1 && !strcmp(argv[1], "-p")) {
    //thread pool
    pthread_t t[5];
    pthread_attr_t attr;
    pthread_attr_init( &attr );
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_mutex_init(&mutex, NULL);
    for (int i = 0; i <= 4; i++) {
      pthread_create( &t[i], NULL, (void * (*)(void *)) poolSlave,
                      (void *) masterSocket);
    }
    for (int i = 0; i <= 4; i++) {
      pthread_join(t[i], NULL);
    }
      //pthread_join(t[0], NULL);
  }

}


void
processTimeRequest( int fd )
{
  // Buffer used to store the name received from the client
  const int MaxName = 1024;
  char *name = (char*)malloc(1024);
  int nameLength = 0;
  int n;

  // Currently character read
  unsigned char newChar;

  // Last character read
  unsigned char lastChar = 0;

  //
  // The client should send <name><cr><lf>
  // Read the name of the client character by character until a
  // <CR><LF> is found.
  //
  int check = 0;
  while ( nameLength < MaxName &&
          ( n = read( fd, &newChar, sizeof(newChar) ) ) > 0 ) {

    if (check == 0) {
      if ( lastChar == '\015' && newChar == '\012' ) {
        // Discard previous <CR> from name
        //Find the first <Enter>
        name[ nameLength ] = newChar;
        nameLength++;
        lastChar = newChar;
        check = 1;
      } else {
        name[ nameLength ] = newChar;
        nameLength++;
        lastChar = newChar;
      }
    } else if (check == 1) {
      if (newChar == '\015' && lastChar == '\012' ) {
        name[ nameLength ] = newChar;
        nameLength++;
        lastChar = newChar;

      } else if (newChar == '\012' && lastChar == '\015') {
        //Find the second <Enter>
        name[ nameLength ] = newChar;
        nameLength++;
        lastChar = newChar;
        break;

      } else {
        check = 0;
        name[ nameLength ] = newChar;
        nameLength++;
        lastChar = newChar;
      }

    }
  }

  // Add null character at the end of the string
  name[ nameLength ] = '\0';

  char *docPath = (char*)malloc(1024);
  char * d = &docPath[0];
  int index = 0;
  int c = 4;
  while (name[c] != ' ') {
    docPath[index] = name[c];
    index++;
    c++;
  }
  //Add NULL at the end of docPath
  docPath[index] = '\0';
//  printf("docPath =%s\n", docPath);
//  printf("docPath[0] =%d\n", docPath[0]);
//  printf("length of docPath:%d\n", strlen(docPath));
  char * newDocPath = (char*)malloc(1024);
  const char * dir = strdup("http-root-dir");
// const char * htdocs = strdup("http-root-dir/htdocs");
  const char * normal = strdup("http-root-dir/htdocs");
  const char * normal_base = strdup("http-root-dir/htdocs/index.html");
  //combine docPath
  if (strstr(docPath, "/icons") != NULL || strstr(docPath, "/htdocs") != NULL) {
//    printf("inside of i, h");
    strcpy(newDocPath, dir);
    strcat(newDocPath, docPath);

  } else {
    if (docPath[1] == 0) {
//      printf("inside of normal_base\n");
      strcpy(newDocPath, normal_base);

    } else {
//      printf("inside of normal\n");
      strcpy(newDocPath, normal);
      strcat(newDocPath, docPath);
    }

  }

//  printf("&&&&&&&&&&&&&&&&&newDocPath:%s\n", newDocPath);
  //printf( "name=%s\n", name );
  //
  //check the path is directory or a file
  int flag2 = 0;
  if (strchr(newDocPath, '.') == NULL) {
    flag2 = 1;

  }

  int fp;

  char * docData = (char*)malloc(50000);

  if ((fp = open(newDocPath, O_RDONLY)) < 0 || flag2 == 1) {
//    printf("open error!!!!\n");
    const char *notFound = strdup("Files not Found\n");
    write(fd, "HTTP/1.0", 8);
    write(fd, " ", 1);
    write(fd, "404", 3);
    write(fd, "File", 4);
    write(fd, "Not", 3);
    write(fd, "Found", 5);
    write(fd, "\r\n", 2);
    write(fd, "Server:", 7);
    write(fd, " ", 1);
    write(fd, "text/html", 9);
    write(fd, "\r\n", 2);
    write(fd, "Content-type:", 13);
    write(fd, " ", 1);
    write(fd, "CS252 lab5", 10);
    write(fd, "\r\n", 2);
    write(fd, "\r\n", 2);
    write(fd, notFound, strlen(notFound));
    //newDocPath = {0};
    //docData = {0};
    //name = {0};
    //docPath  = {0};
    return;
  }

  char ch;
  
  c = 0;
  while (read(fp, &ch, sizeof(ch)) > 0) {
    docData[c] = ch;
    c++;
  }
  docData[c] = '\0';

  // Get time of day

  const char * replyHeader1 = "HTTP/1.0 200 OK\n";
  const char * replyHeader2 = "Server: CS252 lab5\n";
  const char * replyHeader3;
// const char * replyHeader3 = "Content-type: text/html \r\n\r\n";
  d = &docPath[0];
  while (*d != '.') {
    d++;
  }
  d++;
  if (*d == 'g') {
    replyHeader3 = "Content-type: image/gif \r\n\r\n";
  } else if (*d == 'h') {
    replyHeader3 = "Content-type: text/html \r\n\r\n";
  } else {
    replyHeader3 = "Content-type: text/plain \r\n\r\n";
  }


//  printf("docData:%s\n", docData);
//  printf("strlen(docData):%d\n", strlen(docData));
//  printf("docPath:%s\n", docPath);
//  printf("strlen(docPath):%d\n", strlen(docPath));
//  printf("newDocPath:%s\n", newDocPath);
//  printf("strlen(newDocPath):%d\n", strlen(newDocPath));
//  printf("name:%s\n", name);
//  printf("strlen(name):%d\n", strlen(name));
  write(fd, replyHeader1, strlen(replyHeader1));
  write(fd, replyHeader2, strlen(replyHeader2));
  write(fd, replyHeader3, strlen(replyHeader3));
// printf("docData:%s\n", docData);
  write(fd, docData, c);
  //free(newDocPath);
  //free(docData);
  //free(name);
  //free(docPath);

}
