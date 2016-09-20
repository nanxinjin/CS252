
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

int QueueLength = 5;

// Processes time request
void processTimeRequest( int socket );

int
main( int argc, char ** argv )
{
  // Print usage if not enough arguments
  if ( argc < 2 ) {
    fprintf( stderr, "%s", usage );
    exit( -1 );
  }

  // Get the port from the arguments
  int port = atoi( argv[1] );

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
  int err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR,
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

  while ( 1 ) {

    // Accept incoming connections
    struct sockaddr_in clientIPAddress;
    int alen = sizeof( clientIPAddress );
    int slaveSocket = accept( masterSocket,
                              (struct sockaddr *)&clientIPAddress,
                              (socklen_t*)&alen);

    if ( slaveSocket < 0 ) {
      perror( "accept" );
      exit( -1 );
    }

    // Process request.
    processTimeRequest( slaveSocket );

    // Close socket
    close( slaveSocket );
  }

}

void
processTimeRequest( int fd )
{
  // Buffer used to store the name received from the client
  const int MaxName = 1024;
  char name[ MaxName + 1 ];
  int nameLength = 0;
  int n;

  // Send prompt
  const char * prompt = "\nType your name:";
  write( fd, prompt, strlen( prompt ) );

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
        lastChar = newChar;

      } else if (newChar == '\012' && lastChar == '\015') {
        //Find the second <Enter>
        nameLength--;
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
  name[ nameLength ] = 0;

  char docPath[MaxName + 1];
  int index = 0;
  int c = 4;
  while (name[c] != ' ') {
    docPath[index] = name[c];
    index++;
    c++;
  }
  //Add NULL at the end of docPath
  docPath[index] = 0;
  printf("docPath = %s\n", docPath);
  char newDocPath[MaxName + 1] = {0};
  const char * dir = strdup("http-root-dir/");
// const char * htdocs = strdup("http-root-dir/htdocs");
  const char * normal = strdup("http-root-dir/htdocs/index.html");
  //combine docPath
  if (docPath[0] == 'i' && docPath[1] == 'c' && docPath[2] == 'o' && docPath[3] == 'n' && docPath[4] == 's') {
    strcat(newDocPath, dir);
    strcat(newDocPath, docPath);

  } else if (docPath[0] == 'h' && docPath[1] == 't' && docPath[2] == 'd' && docPath[3] == 'o' && docPath[4] == 'c' && docPath[5] == 's') {
    strcat(newDocPath, dir);
    strcat(newDocPath, docPath);
  } else {
    strcat(newDocPath, normal);
  }

  printf("%s\n", newDocPath); 
  printf( "name=%s\n", name );

  int fp;
  if ((fp = open(newDocPath, O_RDONLY)) < 0) {
    const char *notFound = "File not Found";
    write(fd, "HTTP/1.0", strlen("HTTP/1.0"));
    write(fd, " ", 1);
    write(fd, "404", 3);
    write(fd, "File", 4);
    write(fd, "Not", 3);
    write(fd, "Found", 5);
    write(fd, "\r\n", 2);
    write(fd, "Server:", 7);
    write(fd, " ", 1);
    write(fd, "CS252 lab5", strlen("CS252 lab5"));
    write(fd, "\r\n", 2);
    write(fd, "Content-type:", 13);
    write(fd, " ", 1);
    write(fd, "text/html", strlen("text/html"));
    write(fd, "\r\n", 2);
    write(fd, "\r\n", 2);
    write(fd, notFound, strlen(notFound));
    return;
  }

  char ch;
  char docData[MaxName + 1] = {0};
  c = 0;
  while(read(fp,&ch,sizeof(ch)) > 0){
    docData[c] = ch;
    c++;
  }

  printf("docData:%s\n",docData);
  // Get time of day
  time_t now;
  time(&now);
  char  *timeString = ctime(&now);

  // Send name and greetings
  const char * hi = "\nHi ";
  const char * timeIs = " the time is:\n";
  write( fd, hi, strlen( hi ) );
  write( fd, name, strlen( name ) );
  write( fd, timeIs, strlen( timeIs ) );

  // Send the time of day
  write(fd, timeString, strlen(timeString));

  // Send last newline
  const char * newline = "\n";
  write(fd, newline, strlen(newline));

}
