#pragma once
#include "VHacd.hpp"
#include "Jobs.hpp"
#include "TriangleMesh.hpp"
class PluginBackgroundTaskEvent;

// For more information on binding and using Zilch APIs, visit: http://zilch.digipen.edu/
// For auto binding specifically, visit: http://zilch.digipen.edu/home/AutomaticBinding.html

class VHacdTask;

// An example component being bound to the engine
class ZeroVHacd : public ZeroEngine::ZilchComponent
{
public:
  ZilchDeclareType(Zilch::TypeCopyMode::ReferenceType);
  
  ZeroVHacd();
  ~ZeroVHacd();
  void ZeroVHacd::Initialize(ZeroEngine::CogInitializer* initializer);
  void OnJobProgress(PluginBackgroundTaskEvent* event);
  void OnJobFinished(PluginBackgroundTaskEvent* event);
  
  void Compute(Zilch::HandleOf<Mesh>& meshHandle);
  void Cancel();
  void Clear();

  Real GetFidelity();
  void SetFidelity(Real fidelity);

  Real GetRefinement();
  void SetRefinement(Real refinement);

  int GetHullCount();
  Zilch::HandleOf<ZeroEngine::QuickHull3D> GetHull(int index);
  
//private:
  Real mFidelity;
  Real mRefinement;
  Integer3 mSubDivisions;
  int mMaxRecusionDepth;
  int mMaxHulls;
  Real mAllowedConcavityVolumeError;
  bool mResampleMesh;
  Real mAllowedVolumeSurfaceAreaRatio;
  Real mBalanceWeight;
  Real mSymmetryWeight;
  bool mFast;

  Zero::Array<Zilch::HandleOf<ZeroEngine::QuickHull3D> > mHulls;
  VHacdTask* mTask;
};

class VHacdTask : public BackgroundTask
{
public:
  void Run() override;
  void Cancel() override;

  static void ProgressCallback(float totalPercent, const String& stepName, float stepPercent, const String& stepMessage, void* clientData);

  ZeroVHacd* mZeroVHacd;
  TriangleMesh mMesh;
  VHacd mVHacd;
};
