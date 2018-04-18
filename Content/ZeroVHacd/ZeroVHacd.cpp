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
  ZilchBindFieldProperty(mMaxRecusionDepth);
  ZilchBindFieldProperty(mMaxHulls);
  ZilchBindFieldProperty(mAllowedConcavityVolumeError);
  ZilchBindFieldProperty(mResampleMesh);
  ZilchBindFieldProperty(mAllowedVolumeSurfaceAreaRatio);
}

ZeroVHacd::ZeroVHacd()
{
  mSubDivisions = Integer3(50, 50, 50);
  mMaxRecusionDepth = 6;
  mMaxHulls = 15;
  mResampleMesh = true;
  mAllowedConcavityVolumeError = 0.001f;
  mAllowedVolumeSurfaceAreaRatio = 2.0f / 2.5f;
}

ZeroVHacd::~ZeroVHacd()
{
}

void ZeroVHacd::Compute(Zilch::HandleOf<Mesh>& meshHandle)
{
  Clear();

  Mesh* mesh = meshHandle;

  mVHacd.mMaxHulls = mMaxHulls;
  mVHacd.mAllowedConcavityVolumeError = mAllowedConcavityVolumeError;
  mVHacd.mResampleMesh = mResampleMesh;
  mVHacd.mAllowedVolumeSurfaceAreaRatio = mAllowedVolumeSurfaceAreaRatio;
  mVHacd.Compute(mSubDivisions, mMaxRecusionDepth, mesh);

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
