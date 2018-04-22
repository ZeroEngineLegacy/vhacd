#pragma once

class BackgroundTask;

class JobManager : public ZeroEngine::ZilchComponent
{
public:
  ZilchDeclareType(Zilch::TypeCopyMode::ReferenceType);
  
  JobManager();
  ~JobManager();
  
  void Initialize(ZeroEngine::CogInitializer* initializer);
  void OnCogDestroy(ZeroEngine::Event* e);
  
  void OnLogicUpdate(ZeroEngine::UpdateEvent* event);

  int GetThreadCount();
  void SetThreadCount(int threadCount);

  int mThreadCount;
};

// Sent for progress on a background task (in plugins)
class PluginBackgroundTaskEvent : public ZeroEngine::ZilchEvent
{
public:
  ZilchDeclareType(Zilch::TypeCopyMode::ReferenceType);

  BackgroundTask* mTask;
  
  float mTotalPercentage;
  float mStepPercentage;
  String mStepName;
  String mStepMessage;
};
