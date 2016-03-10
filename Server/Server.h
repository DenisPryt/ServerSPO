#ifndef Server_hpp
#define Server_hpp

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "SocketHelper.h"

typedef unsigned long long ulong;

class Server
{
public:
  Server(const char *host = "", const char *port = "1091");
  ~Server();
  Server(const Server &other) = delete;
  Server operator =(const Server &other) = delete;
  
  void run();

protected:
  void            exec();
  bool            processData( SockDesc socket );
  void            dropClient( size_t i );

  void            printSync(const char *msg) const;

private:
  class Private;
  class DataFromClients;
  
  std::atomic_bool m_exit;
  Private &d;
};

#endif /* Server_hpp */
