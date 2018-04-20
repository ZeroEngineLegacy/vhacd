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
};

// An example of a custom event that we can send
class DownloadJobEvent : public ZeroEngine::ZilchEvent
{
public:
  ZilchDeclareType(Zilch::TypeCopyMode::ReferenceType);

  BackgroundTask* mTask;
  float mPercentage;
  String mProgressText;
};
