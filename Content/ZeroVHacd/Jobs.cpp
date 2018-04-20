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

    }
    sThreadLock.Unlock();

    if (job == nullptr)
    {
      Zero::Os::Sleep(10);
      continue;
    }

    job->Run();

    sThreadLock.Lock();
    jobSystem->mFinishedJobs.PushBack(job);
    sThreadLock.Unlock();
  }

  return 0;
}

void BackgroundTask::UpdateProgress(float percentage)
{
  UpdateProgress(percentage, String());
}

void BackgroundTask::UpdateProgress(float percentage, Zilch::StringParam message)
{
  ProgressEvent* toSend = new ProgressEvent();
  toSend->mPercentage = percentage;
  toSend->mTask = this;
  toSend->mOwner = mOwner;
  toSend->mMessage = message;

  JobSystem* jobSystem = JobSystem::GetInstance();
  if(jobSystem != nullptr)
    jobSystem->AddEvent(toSend);
}

JobSystem* sJobSystem = nullptr;

void JobSystem::Initialize()
{
  sJobSystem = new JobSystem();
  sJobSystem->Startup();
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
  mThread.WaitForCompletion();
}

void JobSystem::Startup()
{
  if (mThread.IsValid())
  {
    ShutdownInstance();
  }

  mThread.Initialize(ThreadFn, this, "MyThread");
  mThread.Resume();
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
  //event->mOwner->DispatchEvent();
  sThreadLock.Unlock();
}

void JobSystem::GetEvents(Zero::Array<ProgressEvent*>& events)
{
  sThreadLock.Lock();
  events.Swap(mEvents);
  sThreadLock.Unlock();
}
