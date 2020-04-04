#ifndef __cppstream_h__
#define __cppstream_h__

#include <Arduino.h>

template<class T>
inline Print& operator<<(Print &stream, T arg)
{
    stream.print(arg);
    return stream;
}

template<class T>
inline Print& operator<<(Print &&stream, T arg)
{
    operator<<(stream, arg);
    return stream;
}

#endif
