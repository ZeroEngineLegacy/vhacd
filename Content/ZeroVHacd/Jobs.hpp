#pragma once

class Job
{
public:
  virtual void Run() {};
};

class BackgroundTask : public Job
{
public:

  //void Run() override;

  void UpdateProgress(float percentage);
  void UpdateProgress(float percentage, Zilch::StringParam message);
  ZeroEngine::Cog* mOwner;
};


class ProgressEvent
{
public:
  String mMessage;
  float mPercentage;
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
  Zero::Array<Job*> mFinishedJobs;
  Zero::Array<ProgressEvent*> mEvents;
  Zero::Thread mThread;

  bool mShutingdown;
};

