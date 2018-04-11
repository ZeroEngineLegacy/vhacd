#pragma once

inline Real3 ToReal3(const Integer3& in)
{
  return Real3((Real)in.x, (Real)in.y, (Real)in.z);
}
