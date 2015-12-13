#ifndef Server_hpp
#define Server_hpp

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "SocketHelper.h"

#include <iostream>
#include <stdio.h>
#include <string.h>

#include "DataBuffer.h"
#include <set>
#include <map>
#include <thread>

typedef unsigned long long ulong;

class UdpFileData
{
public:
  UdpFileData(){
    FileSize = 0;
    FileDesc = nullptr;
    SizeNameTransfered = false;
    DatagramCount = 0;
    WaitimgDatagramId = 0;
    memset((void*)&ClientAddr, 0, sizeof(ClientAddr));
  }
  sockaddr_in   ClientAddr;

  std::string   FileName;
  long long     FileSize;
  FILE         *FileDesc;

  bool          SizeNameTransfered;

  long          DatagramCount;
  ulong         WaitimgDatagramId;           // Все датаграммы с номерами меньшей чем эта, уже долшли
  ulong         MaxDatagramId;
  std::set< ulong >   RecivedDatagrams;       // Будут храниться ID датаграмv, которые пришли не по порядку (быстрее)
};

class Server
{
public:
    Server(const char *host = "", const char *port = "1091");
    ~Server();
    void run();

protected:
    void            exec();
    //    bool            processData( SockDesc socket );
    //    void            dropClient( size_t i );

    void            printSync(const char *msg) const;
    SockDesc        acceptSync() const;
  
    SockDesc                            m_udpSocket;
    std::map< ulong, UdpFileData >      m_udpFiles;

    SockDesc                            m_server;
    //std::vector<SockDesc>               m_clients;

    class DataFromClients
    {
    public:
      DataFromClients( SockDesc socket = 0 );
      ~DataFromClients();
      bool processData();
      void sendToServer( const char *msg, size_t size );

      DataBuffer &buffer();

    private:
      enum Action{
        None = 0,
        FileContent = 1,
        FileName    = 2,
        File = 3,
        Time = 4,
        Echo = 5
      };

      SockDesc                            m_socket;
      DataBuffer                          m_buffer;
      Action                              m_action;
      FILE                               *m_file;
      long long                           m_fileSize;
      long long                           m_fileWritten;
      std::string                         m_fileName;
      short                               m_fileNameSize;
    };

    //std::map< SockDesc, DataFromClients > m_socket2Data;
  
  std::vector< std::thread >            m_threads;
  mutable std::recursive_mutex          m_acceptMutex;
  
  mutable std::recursive_mutex          m_printMutex;
  
};

#endif /* Server_hpp */
