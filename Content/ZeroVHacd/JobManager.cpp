#include "ZeroVHacdPrecompiled.hpp"

//***************************************************************************
ZilchDefineType(JobManager, builder, type)
{
  // This is required for component binding
  ZilchBindDestructor();
  ZilchBindConstructor();
  ZilchBindMethod(Initialize);
  
  // Note: All event connection methods must be bound
  ZilchBindMethod(OnCogDestroy);
  ZilchBindMethod(OnLogicUpdate);

  ZilchBindGetterSetterProperty(ThreadCount);
}

//***************************************************************************
JobManager::JobManager()
{
  mThreadCount = 3;
}

//***************************************************************************
JobManager::~JobManager()
{
}

//***************************************************************************
void JobManager::Initialize(ZeroEngine::CogInitializer* initializer)
{
  ZeroConnectThisTo(this->GetSpace(), "LogicUpdate", "OnLogicUpdate");

  JobSystem::Initialize(mThreadCount);
  JobSystem* jobSystem = JobSystem::GetInstance();

  ZeroConnectThisTo(this->GetSpace(), "CogDestroy", "OnCogDestroy");
}


void JobManager::OnCogDestroy(ZeroEngine::Event* e)
{
  JobSystem::Shutdown();
}

//***************************************************************************
void JobManager::OnLogicUpdate(ZeroEngine::UpdateEvent* event)
{
  JobSystem* jobSystem = JobSystem::GetInstance();
  if (jobSystem == nullptr)
    return;

  Zero::Array<ProgressEvent*> events;
  jobSystem->GetEvents(events);


  for (size_t i = 0; i < events.Size(); ++i)
  {
    ProgressEvent* pEvent = events[i];
    
    if (pEvent->mOwner != nullptr)
    {
      Zilch::HandleOf<PluginBackgroundTaskEvent> toSend = ZilchAllocate(PluginBackgroundTaskEvent);
      toSend->mTask = pEvent->mTask;
      toSend->mTotalPercentage = pEvent->mTotalPercentage;
      toSend->mStepPercentage = pEvent->mStepPercentage;
      toSend->mStepName = pEvent->mStepName;
      toSend->mStepMessage = pEvent->mStepMessage;
      pEvent->mOwner->DispatchEvent(pEvent->mEventName, toSend);
    }

    delete pEvent;
  }
}

int JobManager::GetThreadCount()
{
  return mThreadCount;
}

void JobManager::SetThreadCount(int threadCount)
{
  mThreadCount = Math::Clamp(threadCount, 1, 20);
}

//***************************************************************************
ZilchDefineType(PluginBackgroundTaskEvent, builder, type)
{
  // This is required for event binding
  ZilchBindDestructor();
  ZilchBindConstructor();

  ZilchBindFieldProperty(mTotalPercentage);
  ZilchBindFieldProperty(mStepPercentage);
  ZilchBindFieldProperty(mStepName);
  ZilchBindFieldProperty(mStepMessage);
}
