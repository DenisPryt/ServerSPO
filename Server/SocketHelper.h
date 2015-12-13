#ifndef createSockets_h
#define createSockets_h

#if defined(WIN32) && !defined(UNIX)

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef SOCKET SockDesc;

#else

//gcc -Wall -pedantic -std=c99 internet.c -lws2_32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
typedef int SockDesc;

#endif

#include <iostream>
#include <stdio.h>
#include <string.h>

class SocketHelper
{
protected:
  SocketHelper();

public:
  ~SocketHelper();

  static bool isInvalidSockDesc( SockDesc socket );
  static bool closeSockDesc( SockDesc socket );
  //static SockDesc createTcpSocket(char *hostName, unsigned short port, struct sockaddr_in *sin);
  static SockDesc createTcpServerSocket(const char *hostName, const char* port, int qlen);
  static SockDesc createUdpSocket( const char *port, long addr = INADDR_ANY );
  static bool setNonBlocking( SockDesc socket );

private:
  static const SocketHelper m_objToInit;
  bool initLib();
  bool m_inited;
};


/*
SockDesc createTcpSocket(char *hostName, unsigned short port, struct sockaddr_in *sin)
{
    memset( sin, 0, sizeof(*sin) );
    sin->sin_family = AF_INET;
    
    struct hostent *hptr = gethostbyname(hostName);
    if ( hptr != NULL )
        memcpy(&sin->sin_addr, hptr->h_addr, hptr->h_length);   // sin_addr contain ip which bind to socket
    else {
        perror("Incorrect host name");
        return -1;
    }
    
    sin->sin_port = htons(port);        // convert to network byte order
    
    struct protoent *pptr = getprotobyname("TCP");
    if (pptr == NULL) {
        fprintf(stderr, "Incorrect protocol name\n");
        return -1;
    }
    // Create socket
    SockDesc socketDescriptor = socket( PF_INET, SOCK_STREAM, pptr->p_proto );

    //keepalive section
//    int keepalive = 150;
//    setsockopt(socketDescriptor, SOL_SOCKET, SO_KEEPALIVE, &keepalive , sizeof(keepalive ));
    
    if ( socketDescriptor < 0 ){
        perror("Create socket error");
        return -1;
    }
    
    return socketDescriptor;
}
*/

#endif /* createSockets_h */
