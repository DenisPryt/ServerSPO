#include "DataBuffer.h"
#include "SocketHelper.h"

DataBuffer::DataBuffer( size_t size )
: m_start( 0 )
{
  m_buffer.reserve( size );
}


DataBuffer::~DataBuffer()
{}

size_t DataBuffer::size() const
{
//  ASSERT( m_buffer.size() - m_start >= 0 );
  return m_buffer.size() - m_start;
}

size_t DataBuffer::empty() const
{
  return size() == 0;
}

char & DataBuffer::operator[]( size_t i )
{
  return m_buffer[ index(i) ];
}

const char & DataBuffer::operator[]( size_t i ) const
{
  return m_buffer[ index(i) ];
}

const char * DataBuffer::start() const
{
  return m_buffer.data() + index(0);
}

char * DataBuffer::pop( size_t size )
{
  if ( size > this->size() ){
    int a = 0;
    a += 3;
  }
  char * resPtr = m_buffer.data() + index(0);
  m_start += size;
  return resPtr;
}

void DataBuffer::truncate()
{
  m_buffer.erase( m_buffer.begin(), m_buffer.begin() + m_start );
  m_start = 0;
}

void DataBuffer::add( const char *data, size_t size )
{
  m_buffer.insert( m_buffer.end(), data, data + size );
}

size_t DataBuffer::index( size_t i ) const
{
//  ASSERT( m_start + i < m_buffer.size() );
  return m_start + i;
}

//////////////////////////////////////////////////////////////////////////

DataBuffer& operator>>(DataBuffer &buffer, char &data)
{
  memcpy( (void *)&data, (void *)buffer.pop( sizeof(char) ), sizeof(char) );
  return buffer;
}

DataBuffer& operator>>( DataBuffer &buffer, short &data )
{
  memcpy( (void *)&data, (void *)buffer.pop( sizeof(short) ), sizeof(short) );
  data = ntohs( data );
  return buffer;
}

DataBuffer& operator>>( DataBuffer &buffer, long long &data )
{
  memcpy( (void *)&data, (void *)buffer.pop(sizeof(long long)), sizeof(long long) );
  data  = ntohll( data );
  return buffer;
}
