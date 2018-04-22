#include "ZeroVHacdPrecompiled.hpp"

//***************************************************************************
ZilchDefineType(ZeroVHacd, builder, type)
{
  // This is required for component binding
  ZilchBindDestructor();
  ZilchBindConstructor();
  ZilchBindMethod(Initialize);

  ZilchBindMethod(Compute);
  ZilchBindMethod(Cancel);
  ZilchBindMethod(Clear);
  ZilchBindMethod(GetHullCount);
  ZilchBindMethod(GetHull);

  ZilchBindMethod(OnJobProgress);
  ZilchBindMethod(OnJobFinished);

  ZilchBindGetterSetterProperty(Fidelity);
  ZilchBindGetterSetterProperty(Refinement);
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
  mTask = nullptr;
  mFidelity = 0.75f;
  mRefinement = 0.9f;
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

void ZeroVHacd::Initialize(ZeroEngine::CogInitializer* initializer)
{
  ZeroConnectThisTo(this->GetOwner(), "JobProgress", "OnJobProgress");
  ZeroConnectThisTo(this->GetOwner(), "JobFinished", "OnJobFinished");
}

void ZeroVHacd::OnJobProgress(PluginBackgroundTaskEvent* event)
{
}

void ZeroVHacd::OnJobFinished(PluginBackgroundTaskEvent* event)
{
  VHacdTask* task = (VHacdTask*)event->mTask;
  mHulls.Resize(task->mVHacd.mHulls.Size());

  for (size_t i = 0; i < task->mVHacd.mHulls.Size(); ++i)
    mHulls[i] = task->mVHacd.mHulls[i].ToHandle();

  Zilch::HandleOf<ZeroEngine::ZilchEvent> toSend = ZilchAllocate(ZeroEngine::ZilchEvent);
  GetOwner()->DispatchEvent("Finished", toSend);
  mTask = nullptr;
}

void ZeroVHacd::Compute(Zilch::HandleOf<Mesh>& meshHandle)
{
  Clear();

  Mesh* mesh = meshHandle;
  
  mTask = new VHacdTask();
  mTask->mZeroVHacd = this;
  mTask->mOwner = GetOwner();

  mTask->mVHacd.mMaxHulls = mMaxHulls;
  mTask->mVHacd.mAllowedConcavityVolumeError = mAllowedConcavityVolumeError;
  mTask->mVHacd.mResampleMesh = mResampleMesh;
  mTask->mVHacd.mAllowedVolumeSurfaceAreaRatio = mAllowedVolumeSurfaceAreaRatio;
  mTask->mVHacd.mBalanceWeight = mBalanceWeight;
  mTask->mVHacd.mSymmetryWeight = mSymmetryWeight;
  mTask->mVHacd.mRefinement = mRefinement;

  mTask->mMesh.Create(mesh);

  JobSystem::GetInstance()->AddJob(mTask);
}

void ZeroVHacd::Cancel()
{
  if(mTask != nullptr)
  {
    mTask->Cancel();
    mTask = nullptr;
  }
}

void ZeroVHacd::Clear()
{
  for (size_t i = 0; i < mHulls.Size(); ++i)
  {
    mHulls[i]->Clear();
  }
  mHulls.Clear();
}

Real ZeroVHacd::GetFidelity()
{
  return mFidelity;
}

void ZeroVHacd::SetFidelity(Real fidelity)
{
  mFidelity = Math::Clamp(fidelity, 0.0f, 1.0f);
}

Real ZeroVHacd::GetRefinement()
{
  return mRefinement;
}

void ZeroVHacd::SetRefinement(Real refinement)
{
  mRefinement = Math::Clamp(refinement, 0.0f, 1.0f);
}

int ZeroVHacd::GetHullCount()
{
  return mHulls.Size();
}

Zilch::HandleOf<ZeroEngine::QuickHull3D> ZeroVHacd::GetHull(int index)
{
  return mHulls[index];
}

void VHacdTask::Run()
{
  mVHacd.SetProgressCallback(&VHacdTask::ProgressCallback, this);
  mVHacd.Compute(mZeroVHacd->mFidelity, mZeroVHacd->mMaxRecusionDepth, mMesh);
  Finished();
}

void VHacdTask::Cancel()
{
  mVHacd.mForceStop = true;
}

void VHacdTask::ProgressCallback(float totalPercent, const String& stepName, float stepPercent, const String& stepMessage, void* clientData)
{
  VHacdTask* self = (VHacdTask*)clientData;
  self->UpdateProgress(totalPercent, stepName, stepPercent, stepMessage);
}
