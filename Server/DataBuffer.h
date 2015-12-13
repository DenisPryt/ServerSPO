#pragma once

#include <vector>

class DataBuffer
{
public:
  DataBuffer( size_t size = 1024 );
  ~DataBuffer();

  size_t size() const;
  size_t empty() const;

        char &operator[]( size_t i );
  const char &operator[]( size_t i ) const;

  const char *start() const;
        char *pop( size_t size );

  void truncate();

  void add( const char *data, size_t size );

  friend DataBuffer& operator>>( DataBuffer &buffer, char &data );
  friend DataBuffer& operator>>( DataBuffer &buffer, short &data );
  friend DataBuffer& operator>>( DataBuffer &buffer, long long &data );

private:
  size_t index( size_t i ) const;

  size_t              m_start;
  std::vector< char > m_buffer;
};

