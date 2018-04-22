#include "ZeroVHacdPrecompiled.hpp"

#include "Jobs.hpp"

Zero::ThreadLock sThreadLock;

Zero::OsInt ThreadFn(void* data)
{
  JobSystem* jobSystem = (JobSystem*)data;

  while (true)
  {
    if (jobSystem->mShutingdown == true)
      break;

    Job* job = nullptr;
    sThreadLock.Lock();
    if (jobSystem->mPendingJobs.Size() != 0)
    {
      job = jobSystem->mPendingJobs[0];
      jobSystem->mPendingJobs.PopFront();
      jobSystem->mActiveJobs.PushBack(job);
    }
    sThreadLock.Unlock();

    if (job == nullptr)
    {
      Zero::Os::Sleep(10);
      continue;
    }

    job->Run();

    sThreadLock.Lock();
    jobSystem->mActiveJobs.EraseValue(job);
    jobSystem->mFinishedJobs.PushBack(job);
    sThreadLock.Unlock();
  }

  return 0;
}

void BackgroundTask::UpdateProgress(float totalPercent, const String& stepName, float stepPercent, const String& stepMessage)
{
  UpdateProgress(totalPercent, stepName, stepPercent, stepMessage, "JobProgress");
}

void BackgroundTask::UpdateProgress(float totalPercent, const String& stepName, float stepPercent, const String& stepMessage, const String& eventName)
{
  ProgressEvent* toSend = new ProgressEvent();
  toSend->mTotalPercentage = totalPercent;
  toSend->mStepName = stepName;
  toSend->mStepPercentage = stepPercent;
  toSend->mStepMessage = stepMessage;

  toSend->mTask = this;
  toSend->mOwner = mOwner;
  toSend->mEventName = eventName;

  JobSystem* jobSystem = JobSystem::GetInstance();
  if (jobSystem != nullptr)
    jobSystem->AddEvent(toSend);
}

void BackgroundTask::Finished()
{
  UpdateProgress(1.0f, String(), 1.0f, String());
  UpdateProgress(1.0f, String(), 1.0f, String(), "JobFinished");
}

JobSystem* sJobSystem = nullptr;

void JobSystem::Initialize(int threads)
{
  // Already Running
  if (sJobSystem != nullptr)
    return;

  sJobSystem = new JobSystem();
  sJobSystem->Startup(threads);
}

void JobSystem::Shutdown()
{
  if(sJobSystem != nullptr)
    sJobSystem->ShutdownInstance();
  delete sJobSystem;
  sJobSystem = nullptr;
}

JobSystem* JobSystem::GetInstance()
{
  return sJobSystem;
}

JobSystem::JobSystem()
{
  mShutingdown = false;
}

JobSystem::~JobSystem()
{
  sJobSystem->ShutdownInstance();
}

void JobSystem::ShutdownInstance()
{
  mShutingdown = true;
  sThreadLock.Lock();
  for (size_t i = 0; i < mActiveJobs.Size(); ++i)
    mActiveJobs[i]->Cancel();
  sThreadLock.Unlock();

  for (size_t i = 0; i < mThreads.Size(); ++i)
    mThreads[i]->WaitForCompletion();

  for (size_t i = 0; i < mThreads.Size(); ++i)
    delete mThreads[i];
  mThreads.Clear();
}

void JobSystem::Startup(int threads)
{
  if (!mShutingdown)
    ShutdownInstance();

  mShutingdown = false;
  sThreadLock.Lock();
  mThreads.Resize(threads);
  for (size_t i = 0; i < mThreads.Size(); ++i)
  {
    mThreads[i] = new Zero::Thread();
    mThreads[i]->Initialize(ThreadFn, this, "BackgroundThread");
    mThreads[i]->Resume();
  }
  sThreadLock.Unlock();
}

void JobSystem::AddJob(Job* job)
{
  sThreadLock.Lock();
  mPendingJobs.PushBack(job);
  sThreadLock.Unlock();
}

void JobSystem::AddEvent(ProgressEvent* event)
{
  sThreadLock.Lock();
  mEvents.PushBack(event);
  sThreadLock.Unlock();
}

void JobSystem::GetEvents(Zero::Array<ProgressEvent*>& events)
{
  sThreadLock.Lock();
  events.Swap(mEvents);
  sThreadLock.Unlock();
}
