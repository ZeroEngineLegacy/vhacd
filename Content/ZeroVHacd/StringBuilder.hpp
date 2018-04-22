#pragma once

#include <string>
#include <sstream>

class StringBuilder
{
public:
  void Append(const std::string& data)
  {
    mBuffer << data.c_str();
  }
  void Append(const Zero::String& data)
  {
    mBuffer << data.c_str();
  }

  void Deallocate()
  {
    mBuffer.clear();
  }

  std::string ToString()
  {
    return mBuffer.str().c_str();
  }

  Zero::String ToZeroString()
  {
    return Zero::String(mBuffer.str().c_str());
  }

  std::stringstream mBuffer;
};

inline StringBuilder& operator<<(StringBuilder& builder, const Zero::String& str)
{
  builder.mBuffer << str.c_str();
  return builder;
}
inline StringBuilder& operator<<(StringBuilder& builder, const std::string& str)
{
  builder.mBuffer << str.c_str();
  return builder;
}
inline StringBuilder& operator<<(StringBuilder& builder, char rune)
{
  builder.mBuffer << rune;
  return builder;
}
template <typename T>
inline StringBuilder& operator<<(StringBuilder& builder, const T& obj)
{
  builder.mBuffer << obj;
  return builder;
}

template <typename T1, typename T2>
String BuildString(const T1& param0, const T2& param1)
{
  StringBuilder builder;
  builder << param0;
  builder << param1;
  return builder.ToZeroString();
}

template <typename T1, typename T2, typename T3>
String BuildString(const T1& param0, const T2& param1, const T3& param2)
{
  StringBuilder builder;
  builder << param0;
  builder << param1;
  builder << param2;
  return builder.ToZeroString();
}
