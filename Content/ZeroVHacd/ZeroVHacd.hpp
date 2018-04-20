#pragma once
#include "VHacd.hpp"
#include "Jobs.hpp"
#include "TriangleMesh.hpp"
class DownloadJobEvent;

// For more information on binding and using Zilch APIs, visit: http://zilch.digipen.edu/
// For auto binding specifically, visit: http://zilch.digipen.edu/home/AutomaticBinding.html

// An example component being bound to the engine
class ZeroVHacd : public ZeroEngine::ZilchComponent
{
public:
  ZilchDeclareType(Zilch::TypeCopyMode::ReferenceType);
  
  ZeroVHacd();
  ~ZeroVHacd();
  void ZeroVHacd::Initialize(ZeroEngine::CogInitializer* initializer);
  void OnJobProgress(DownloadJobEvent* event);
  
  void Compute(Zilch::HandleOf<Mesh>& meshHandle);
  void Clear();

  Real GetFidelity();
  void SetFidelity(Real fidelity);

  int GetHullCount();
  Zilch::HandleOf<ZeroEngine::QuickHull3D> GetHull(int index);
  
//private:
  Real mFidelity;
  Integer3 mSubDivisions;
  int mMaxRecusionDepth;
  int mMaxHulls;
  Real mAllowedConcavityVolumeError;
  bool mResampleMesh;
  Real mAllowedVolumeSurfaceAreaRatio;
  Real mBalanceWeight;
  Real mSymmetryWeight;

  Zero::Array<Zilch::HandleOf<ZeroEngine::QuickHull3D> > mHulls;
};

class VHacdTask : public BackgroundTask
{
public:
  void Run() override;

  ZeroVHacd* mZeroVHacd;
  TriangleMesh mMesh;
  VHacd mVHacd;
};
