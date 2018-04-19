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
  ZilchBindGetterSetterProperty(Fidelity);
  ZilchBindFieldProperty(mMaxRecusionDepth);
  ZilchBindFieldProperty(mMaxHulls);
  ZilchBindFieldProperty(mAllowedConcavityVolumeError);
  ZilchBindFieldProperty(mResampleMesh);
  ZilchBindFieldProperty(mAllowedVolumeSurfaceAreaRatio);
  ZilchBindFieldProperty(mBalanceWeight);
  ZilchBindFieldProperty(mSymmetryWeight);
}

ZeroVHacd::ZeroVHacd()
{
  mFidelity = 0.75f;
  mSubDivisions = Integer3(50, 50, 50);
  mMaxRecusionDepth = 6;
  mMaxHulls = 15;
  mResampleMesh = true;
  mAllowedConcavityVolumeError = 0.001f;
  mAllowedVolumeSurfaceAreaRatio = 2.0f / 2.5f;
  mBalanceWeight = 0.05f;
  mSymmetryWeight = 0.05f;
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
  mVHacd.mBalanceWeight = mBalanceWeight;
  mVHacd.mSymmetryWeight = mSymmetryWeight;
  mVHacd.Compute(mFidelity, mMaxRecusionDepth, mesh);

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

Real ZeroVHacd::GetFidelity()
{
  return mFidelity;
}

void ZeroVHacd::SetFidelity(Real fidelity)
{
  mFidelity = Math::Clamp(fidelity, 0.0f, 1.0f);
}

int ZeroVHacd::GetHullCount()
{
  return mHulls.Size();
}

Zilch::HandleOf<ZeroEngine::QuickHull3D> ZeroVHacd::GetHull(int index)
{
  return mHulls[index];
}
