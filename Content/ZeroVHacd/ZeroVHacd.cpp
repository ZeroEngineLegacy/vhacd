#include "ZeroVHacdPrecompiled.hpp"

//***************************************************************************
ZilchDefineType(ZeroVHacd, builder, type)
{
  // This is required for component binding
  ZilchBindDestructor();
  ZilchBindConstructor();
  ZilchBindMethod(Initialize);

  ZilchBindMethod(Compute);
  ZilchBindMethod(GetHullCount);
  ZilchBindMethod(GetHull);
  ZilchBindMethod(OnJobProgress);
  ZilchBindMethod(OnJobFinished);

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

void ZeroVHacd::Initialize(ZeroEngine::CogInitializer* initializer)
{
  ZeroConnectThisTo(this->GetOwner(), "JobProgress", "OnJobProgress");
  ZeroConnectThisTo(this->GetOwner(), "JobFinished", "OnJobFinished");
}

void ZeroVHacd::OnJobProgress(DownloadJobEvent* event)
{
}

void ZeroVHacd::OnJobFinished(DownloadJobEvent* event)
{
  VHacdTask* task = (VHacdTask*)event->mTask;
  mHulls.Resize(task->mVHacd.mHulls.Size());

  for (size_t i = 0; i < task->mVHacd.mHulls.Size(); ++i)
    mHulls[i] = task->mVHacd.mHulls[i].ToHandle();

  Zilch::HandleOf<ZeroEngine::ZilchEvent> toSend = ZilchAllocate(ZeroEngine::ZilchEvent);
  GetOwner()->DispatchEvent("Finished", toSend);
}

void ZeroVHacd::Compute(Zilch::HandleOf<Mesh>& meshHandle)
{
  Clear();

  Mesh* mesh = meshHandle;
  
  VHacdTask* task = new VHacdTask();
  task->mZeroVHacd = this;
  task->mOwner = GetOwner();

  task->mVHacd.mMaxHulls = mMaxHulls;
  task->mVHacd.mAllowedConcavityVolumeError = mAllowedConcavityVolumeError;
  task->mVHacd.mResampleMesh = mResampleMesh;
  task->mVHacd.mAllowedVolumeSurfaceAreaRatio = mAllowedVolumeSurfaceAreaRatio;
  task->mVHacd.mBalanceWeight = mBalanceWeight;
  task->mVHacd.mSymmetryWeight = mSymmetryWeight;

  task->mMesh.Create(mesh);

  JobSystem::GetInstance()->AddJob(task);
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

void VHacdTask::MarkForShutdown()
{
  mVHacd.mForceStop = true;
}

void VHacdTask::ProgressCallback(const String& message, float percentage, void* clientData)
{
  VHacdTask* self = (VHacdTask*)clientData;
  self->UpdateProgress(percentage, message);
}
