#pragma once

class Job
{
public:
  virtual void Run() {};
  virtual void Cancel() {};
};

class BackgroundTask : public Job
{
public:

  void UpdateProgress(float totalPercent, const String& stepName, float stepPercent, const String& stepMessage);
  void UpdateProgress(float totalPercent, const String& stepName, float stepPercent, const String& stepMessage, const String& eventName);

  void Finished();
  ZeroEngine::Cog* mOwner;
};

class ProgressEvent
{
public:
  float mTotalPercentage;
  float mStepPercentage;
  String mStepName;
  String mStepMessage;

  String mEventName;
  BackgroundTask* mTask;
  ZeroEngine::Cog* mOwner;
};

class JobSystem
{
public:

private:
  JobSystem();
  ~JobSystem();
public:

  static void Initialize(int threads = 3);
  static void Shutdown();
  static JobSystem* GetInstance();

  void Startup(int threads);
  void ShutdownInstance();

  void AddJob(Job* job);
  void AddEvent(ProgressEvent* event);
  void GetEvents(Zero::Array<ProgressEvent*>& events);


  Zero::Array<Job*> mPendingJobs;
  Zero::Array<Job*> mActiveJobs;
  Zero::Array<Job*> mFinishedJobs;
  Zero::Array<ProgressEvent*> mEvents;

  Array<Zero::Thread*> mThreads;

  bool mShutingdown;
};

