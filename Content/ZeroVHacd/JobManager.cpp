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
}

//***************************************************************************
JobManager::JobManager()
{
}

//***************************************************************************
JobManager::~JobManager()
{
}

//***************************************************************************
void JobManager::Initialize(ZeroEngine::CogInitializer* initializer)
{
  ZeroConnectThisTo(this->GetSpace(), "LogicUpdate", "OnLogicUpdate");

  JobSystem::Initialize();
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
      Zilch::HandleOf<DownloadJobEvent> toSend = ZilchAllocate(DownloadJobEvent);
      toSend->mPercentage = pEvent->mPercentage;
      toSend->mProgressText = pEvent->mMessage;
      toSend->mTask = pEvent->mTask;
      pEvent->mOwner->DispatchEvent(pEvent->mEventName, toSend);
    }

    delete pEvent;
  }
}

//***************************************************************************
ZilchDefineType(DownloadJobEvent, builder, type)
{
  // This is required for event binding
  ZilchBindDestructor();
  ZilchBindConstructor();

  ZilchBindFieldProperty(mPercentage);
  ZilchBindFieldProperty(mProgressText);
  ZilchBindFieldProperty(mFinished);
}
