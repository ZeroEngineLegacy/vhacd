#pragma once
#include "VHacd.hpp"

// For more information on binding and using Zilch APIs, visit: http://zilch.digipen.edu/
// For auto binding specifically, visit: http://zilch.digipen.edu/home/AutomaticBinding.html

// An example component being bound to the engine
class ZeroVHacd : public ZeroEngine::ZilchComponent
{
public:
  ZilchDeclareType(Zilch::TypeCopyMode::ReferenceType);
  
  ZeroVHacd();
  ~ZeroVHacd();
  
  
  void Compute(Zilch::HandleOf<Mesh>& meshHandle);
  void Clear();

  Real GetFidelity();
  void SetFidelity(Real fidelity);

  int GetHullCount();
  Zilch::HandleOf<ZeroEngine::QuickHull3D> GetHull(int index);
  
private:
  Real mFidelity;
  Integer3 mSubDivisions;
  int mMaxRecusionDepth;
  int mMaxHulls;
  Real mAllowedConcavityVolumeError;
  bool mResampleMesh;
  Real mAllowedVolumeSurfaceAreaRatio;
  Real mBalanceWeight;
  Real mSymmetryWeight;

  VHacd mVHacd;

  Zero::Array<Zilch::HandleOf<ZeroEngine::QuickHull3D> > mHulls;
};
