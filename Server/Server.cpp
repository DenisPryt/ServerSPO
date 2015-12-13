#include "Server.h"
#include <algorithm>
#include <time.h>

#define THREADS_COUNT 5

Server::Server( const char *host, const char *port )
{
  m_server = SocketHelper::createTcpServerSocket( host, port, 5 );
  m_udpSocket = SocketHelper::createUdpSocket( port );
  if ( !SocketHelper::setNonBlocking(m_udpSocket) )
    std::cout << "NONBLOCKING ERROR";
}

Server::~Server()
{
  SocketHelper::closeSockDesc( m_server );
    //  for ( size_t i = 0; i < m_clients.size(); ++i )
    //    SocketHelper::closeSockDesc( m_clients[ i ] );
}

void Server::printSync( const char *msg ) const
{
  std::lock_guard<std::recursive_mutex> guard( m_printMutex );
  
  std::cout << msg << std::endl;
}

SockDesc Server::acceptSync() const
{
  std::lock_guard<std::recursive_mutex> guard( m_acceptMutex );
  
  sockaddr  newClientAddr = { 0 };
  socklen_t newClientAddrLen = sizeof( sockaddr_in );
  
  return accept( m_server, &newClientAddr, &newClientAddrLen );
}

void Server::run()
{
  for ( int i = 0; i < THREADS_COUNT; ++i ){
    m_threads.emplace_back( &Server::exec, this );
  }
  
  m_threads[0].join();
  for ( int i = 1; i < m_threads.size(); ++i )
    m_threads[ i ].detach();
  
    //exec();
}

void Server::exec()
{
  fd_set  readset;
  char buf[ 1024 ];

  std::vector<SockDesc>                 m_clients;
  std::map< SockDesc, DataFromClients > m_socket2Data;
  
  auto dropClient = [&]( size_t i )
  {
    SocketHelper::closeSockDesc( m_clients[ i ] );
    m_socket2Data.erase( m_clients[ i ] );
    m_clients.erase( m_clients.begin() + i );
  };
  
  while ( 1 )
  {
    int selectResult = 0;
    do {
      FD_ZERO( &readset );
      SockDesc maxfd = m_server;
      FD_SET( m_server, &readset );
      
      for ( size_t i = 0; i < m_clients.size(); ++i )
      {
        if ( m_clients[ i ] > maxfd )
          maxfd = m_clients[ i ];

        FD_SET( m_clients[ i ], &readset );
      }
      
      selectResult = select( maxfd + 1, &readset, NULL, NULL, NULL );
    } while ( selectResult == -1 && errno == EINTR );

    if ( selectResult < 0 ){
      printSync( "ERROR SELECT" );
      continue;
    }
    
    if ( m_acceptMutex.try_lock() ){
      if ( FD_ISSET( m_server, &readset ) ){
        sockaddr  newClientAddr = { 0 };
        socklen_t newClientAddrLen = sizeof( sockaddr_in );
        
        SockDesc  newClient = accept( m_server, &newClientAddr, &newClientAddrLen );
      
        if ( SocketHelper::isInvalidSockDesc(newClient) ){
          printSync( "ERROR" );
        }
        else{
          printSync( "+ client" );
          m_clients.push_back( newClient );
          m_socket2Data[ newClient ] = DataFromClients( newClient );
        }
      }
      m_acceptMutex.unlock();
    }
    

    for ( size_t i = 0; i < m_clients.size(); ++i )
    {
      if ( FD_ISSET( m_clients[ i ], &readset ) )
      {
        Server::DataFromClients &clientData = m_socket2Data[ m_clients[ i ] ];
        long n = recv( m_clients[ i ], buf, sizeof(buf) - clientData.buffer().size(), 0 );
        if ( n == 0 )
        {
          printSync( "- client" );
          dropClient( i );
        }
        else if ( n < 0 )
        {
          if ( errno == EAGAIN )
              ; //The kernel didn't have any data for us to read.
          else
          {
            printSync( "error \n - client" );
            dropClient( i );
          }
        }
        else
        {
          clientData.buffer().add( buf, n );
          while ( m_socket2Data[ m_clients[i] ].processData() );
        }
      }
    }
  }
}


  //void Server::dropClient( size_t i )
  //{
  //  SocketHelper::closeSockDesc( m_clients[ i ] );
  //  m_socket2Data.erase( m_clients[ i ] );
  //  m_clients.erase( m_clients.begin() + i );
  //}

  //bool Server::processData( SockDesc socket )
  //{
  //  return m_socket2Data[ socket ].processData();
  //}


Server::DataFromClients::DataFromClients( SockDesc socket )
: m_buffer( 1024 )
{
  m_socket        = socket;
  m_action        = None;
  m_fileName      = "";
  m_fileNameSize  = 0;
  m_fileSize      = 0;
  m_fileWritten   = 0;
  m_file          = nullptr;
}

Server::DataFromClients::~DataFromClients()
{
  if ( m_file != nullptr )
    fclose( m_file );
}

void Server::DataFromClients::sendToServer( const char *msg, size_t size )
{
  size_t sended = 0;
  while ( size > sended )
  {
    ssize_t sendRes = send( m_socket, msg + sended, size - sended, 0 );
    if ( sendRes > 0 )
      sended += sendRes;
    else if ( sendRes < 0 && errno != EINTR )
    {
      std::cout << "error send" << std::endl;
      return;
    }
  }
}

bool Server::DataFromClients::processData()
{
  class Truncater{
    DataBuffer &m_buffer;
  public:
    Truncater( DataBuffer &buffer ) : m_buffer( buffer ){};
    ~Truncater(){ m_buffer.truncate(); }
  };

  Truncater tr( m_buffer );

  long oldBufSize = m_buffer.size();

  if ( m_action == None )
  {
    if ( m_buffer.size() < sizeof(char) )
      return false;

    char action = 0;
    m_buffer >> action;
    m_action = (Action)action;

  }

  if ( m_action == File )
  {
    if ( m_buffer.size() < sizeof(short) )
      return false;

    m_buffer >> m_fileNameSize;
    m_action = FileName;
  }

  if ( m_action == FileName )
  {
    if ( m_fileNameSize == 0 )
    {
      if ( m_buffer.size() < sizeof(long long) )
        return false;

      m_buffer >> m_fileSize;
      m_file      = fopen( m_fileName.data(), "wb" );
      m_action    = FileContent;
    }
    else
    {   // m_fileNameSize != 0
      while ( m_buffer.size() > 0 && m_fileNameSize > 0 )
      {
        m_fileName += *m_buffer.pop( 1 );
        m_fileNameSize--;
      }
    }
  }

  if ( m_action == FileContent )
  {
    size_t writeDataSize  = std::min<size_t>( m_buffer.size(), m_fileSize - m_fileWritten );
    size_t fileWritten    = fwrite( m_buffer.start(), sizeof(char), writeDataSize, m_file );

    m_buffer.pop( fileWritten );
    m_fileWritten += fileWritten;

    if ( m_fileWritten == m_fileSize )
    {
      fclose( m_file );
      m_action        = None;
      m_fileName      = "";
      m_fileNameSize  = 0;
      m_fileSize      = 0;
      m_fileWritten   = 0;
      
      sendToServer( "File transfered", 15 );
      std::cout << "File transfered" << std::endl;
    }
  }

  if ( m_action == Echo )
  {
    char ch = 1;
    while ( !m_buffer.empty() && ch != 0 )
    {
      m_buffer >> ch;
      m_fileName += ch;
    }

    if ( ch == 0 )
    {
      sendToServer( m_fileName.data(), m_fileName.length() );
      
      m_fileName = "";
      m_action = None;
    }
  }

  if ( m_action == Time )
  {
    time_t now          = time( NULL );
    std::string timeStr = ctime( &now );
    
    sendToServer( timeStr.data(), timeStr.length() );

    m_action = None;
  }

  if ( m_buffer.empty() || m_buffer.size() == oldBufSize )
    return false;

  return true;
}

DataBuffer & Server::DataFromClients::buffer()
{
  return m_buffer;
}
