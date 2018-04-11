#pragma once
#include "VHacd.hpp"

// For more information on binding and using Zilch APIs, visit: http://zilch.digipen.edu/
// For auto binding specifically, visit: http://zilch.digipen.edu/home/AutomaticBinding.html

// An example component being bound to the engine
class ZeroVHacd : public ZeroEngine::ZilchComponent
{
public:
  ZilchDeclareType(Zilch::TypeCopyMode::ReferenceType);
  
  ZeroVHacd();
  ~ZeroVHacd();
  
  void Initialize(ZeroEngine::CogInitializer* initializer);
  
  void Compute(Zilch::HandleOf<Mesh>& meshHandle);
  void Clear();
  //void OnLogicUpdate(ZeroEngine::UpdateEvent* event);
  
  // A method that we want to expose to script
  //Zilch::String Speak();
  
  // A field that we want to expose to script
  //int mLives;
  
  // A getter/setter that we want to expose to script
  //float GetHealth();
  //void SetHealth(float value);

  int GetHullCount();
  Zilch::HandleOf<ZeroEngine::QuickHull3D> GetHull(int index);
  
private:
  Integer3 mSubDivisions;
  int mRecursions;
  int mMaxHulls;

  VHacd mVHacd;

  Zero::Array<Zilch::HandleOf<ZeroEngine::QuickHull3D> > mHulls;
};

// An example of a custom event that we can send
class ZeroVHacdEvent : public ZeroEngine::ZilchEvent
{
public:
  ZilchDeclareType(Zilch::TypeCopyMode::ReferenceType);
  
  int mLives;
};
