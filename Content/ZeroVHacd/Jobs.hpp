#pragma once

class Job
{
public:
  virtual void Run() {};
  virtual void MarkForShutdown() {};
};

class BackgroundTask : public Job
{
public:

  void UpdateProgress(float percentage);
  void UpdateProgress(float percentage, Zilch::StringParam message);
  void UpdateProgress(float percentage, Zilch::StringParam message, const String& eventName);
  void Finished();
  ZeroEngine::Cog* mOwner;
};


class ProgressEvent
{
public:
  String mMessage;
  float mPercentage;
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

  static void Initialize();
  static void Shutdown();
  static JobSystem* GetInstance();

  void Startup();
  void ShutdownInstance();

  void AddJob(Job* job);
  void AddEvent(ProgressEvent* event);
  void GetEvents(Zero::Array<ProgressEvent*>& events);


  Zero::Array<Job*> mPendingJobs;
  Zero::Array<Job*> mActiveJobs;
  Zero::Array<Job*> mFinishedJobs;
  Zero::Array<ProgressEvent*> mEvents;
  Zero::Thread mThread;

  bool mShutingdown;
};

