#include "ZeroVHacdPrecompiled.hpp"

//***************************************************************************
ZilchDefineType(ZeroVHacd, builder, type)
{
  // This is required for component binding
  ZilchBindDestructor();
  ZilchBindConstructor();

  ZilchBindMethod(Compute);
  ZilchBindMethod(GetHullCount);
  ZilchBindMethod(GetHull);
  ZilchBindFieldProperty(mSubDivisions);
  ZilchBindFieldProperty(mRecursions);
  ZilchBindFieldProperty(mMaxHulls);
  ZilchBindFieldProperty(mConcavity);
}

ZeroVHacd::ZeroVHacd()
{
  mSubDivisions = Integer3(50, 50, 50);
  mRecursions = 3;
  mMaxHulls = 15;
  mConcavity = 0.001f;
}

ZeroVHacd::~ZeroVHacd()
{
}

void ZeroVHacd::Compute(Zilch::HandleOf<Mesh>& meshHandle)
{
  Clear();

  Mesh* mesh = meshHandle;

  mVHacd.mMaxHulls = mMaxHulls;
  mVHacd.mConcavity = mConcavity;
  mVHacd.Compute(mSubDivisions, mRecursions, mesh);

  mHulls.Clear();
  mHulls.Resize(mVHacd.mHulls.Size());

  for (size_t i = 0; i < mVHacd.mHulls.Size(); ++i)
    mHulls[i] = mVHacd.mHulls[i].mQuickHull;
}

void ZeroVHacd::Clear()
{
  for (size_t i = 0; i < mHulls.Size(); ++i)
  {
    mHulls[i]->Clear();
  }
  mHulls.Clear();

  mVHacd.Clear();
}

int ZeroVHacd::GetHullCount()
{
  return mHulls.Size();
}

Zilch::HandleOf<ZeroEngine::QuickHull3D> ZeroVHacd::GetHull(int index)
{
  return mHulls[index];
}
