#include "SocketHelper.h"

const SocketHelper SocketHelper::m_objToInit = SocketHelper::SocketHelper();

bool SocketHelper::initLib()
{
#if defined(WIN32) && !defined(UNIX)

  WSADATA wsaData;
  return WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) == 0;

#else
  return true;
#endif
}

SocketHelper::SocketHelper()
{
  m_inited = initLib();
}

SocketHelper::~SocketHelper()
{
#if defined(WIN32) && !defined(UNIX)
  WSACleanup();
#endif
}

bool SocketHelper::isInvalidSockDesc( SockDesc socket )
{
#if defined(WIN32)
  return socket == INVALID_SOCKET;
#else
  return socket < 0;
#endif
}

bool SocketHelper::closeSockDesc( SockDesc socket )
{
#if defined(WIN32) && !defined(UNIX)
  return closesocket( socket ) == 0;
#else
  return close( socket ) == 0;
#endif
  return false;
}

SockDesc SocketHelper::createTcpServerSocket(const char *hostName, const char* port, int qlen)
{
  SockDesc socketDescriptor = 0 ;

  struct addrinfo *res = NULL;
  struct addrinfo hints = { 0 };

  hints.ai_flags    = AI_PASSIVE;
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  int status = getaddrinfo( NULL, port, &hints, &res );
  if ( status != 0 ){
    std::cout << "[ERROR]: " << status << " Unable to get address info for Port " << port << "." << std::endl;
    return 0;
  }

  SockDesc newsock = socket( res->ai_family, res->ai_socktype, res->ai_protocol );
  if ( isInvalidSockDesc( newsock ) ){
    std::cout << "ERROR" << std::endl;
    freeaddrinfo( res );
    return 0;
  }

  if ( bind( newsock, res->ai_addr, res->ai_addrlen ) < 0 ){
    std::cout << "[ERROR]: Unable to bind Socket." << std::endl;
    freeaddrinfo( res );
    closeSockDesc( newsock );
    return 0;
  }

  if ( listen( newsock, qlen ) < 0 ){
    std::cout << "ERROR: Unable to Listen on Port " << port << "." << std::endl;
    closeSockDesc( newsock );
    return 0;
  }

  return newsock;
}

bool SocketHelper::setNonBlocking(SockDesc socket)
{
#if defined(WIN32) && !defined(UNIX)
  u_long iMode = 0;
  int iResult = ioctlsocket(m_socket, FIONBIO, &iMode);
  if ( iResult != NO_ERROR )
    return false;
#else
  int f = 0;
  if ( (f = fcntl(socket, F_GETFL, 0)) == -1 || fcntl(socket, F_SETFL, f | O_NONBLOCK) == -1 )
    return false;
  
    //int opt = 1;
    //ioctl(fd, FIONBIO, &opt);
#endif
  return true;
}

SockDesc SocketHelper::createUdpSocket( const char* port, long addr )
{
  SockDesc sockfd = 0;
  if ( (sockfd = socket( AF_INET, SOCK_DGRAM, 0 )) == -1 )
  {
    std::cout << "Can't create socket";
    return -1;
  }

  sockaddr_in myAddr = { 0 };
  myAddr.sin_family       = AF_INET;
  myAddr.sin_addr.s_addr  = htonl( addr );
  myAddr.sin_port         = htons( atoi( port ) );

  if ( (bind( sockfd, (struct sockaddr *) &myAddr, sizeof( myAddr ) )) < 0 )
  {
    closeSockDesc( sockfd );
    std::cout << "Can't bind";
  }

  return sockfd;
}
