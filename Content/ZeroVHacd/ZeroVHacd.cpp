#include "ZeroVHacdPrecompiled.hpp"

//***************************************************************************
ZilchDefineType(ZeroVHacd, builder, type)
{
  // This is required for component binding
  ZilchBindDestructor();
  ZilchBindConstructor();
  ZilchBindMethod(Initialize);
  
  // Note: All event connection methods must be bound
  //ZilchBindMethod(OnLogicUpdate);

  // Using Property at the end is the same as the [Property] attribute
  // You could also use ->AddAttribute after the bind macro
  //ZilchBindMethod(Speak);
  //ZilchBindFieldProperty(mLives);
  //ZilchBindGetterSetterProperty(Health);

  ZilchBindMethod(Compute);
  ZilchBindMethod(GetHullCount);
  ZilchBindMethod(GetHull);
  ZilchBindFieldProperty(mSubDivisions);
  ZilchBindFieldProperty(mRecursions);
  ZilchBindFieldProperty(mMaxHulls);
}

//***************************************************************************
ZeroVHacd::ZeroVHacd()
{
  Zilch::Console::WriteLine("ZeroVHacd::ZeroVHacd (Constructor)");
  // Initialize our default values here (we automatically zero the memory first)
  // In the future we'll support a newer compiler with member initialization
  //mHealth = 100.0f;
  //mLives = 9;
  mSubDivisions = Integer3(50, 50, 50);
  mRecursions = 3;
  mMaxHulls = 15;
}

//***************************************************************************
ZeroVHacd::~ZeroVHacd()
{
  //Zilch::Console::WriteLine("ZeroVHacd::~ZeroVHacd (Destructor)");
  // Always check for null if you are intending
  // to destroy any cogs that you 'own'
}

//***************************************************************************
void ZeroVHacd::Initialize(ZeroEngine::CogInitializer* initializer)
{
  Zilch::Console::WriteLine("ZeroVHacd::Initialize");
}

void ZeroVHacd::Compute(Zilch::HandleOf<Mesh>& meshHandle)
{
  Clear();

  Mesh* mesh = meshHandle;

  mVHacd.mMaxHulls = mMaxHulls;
  mVHacd.Compute(mSubDivisions, mRecursions, mesh);

  mHulls.Clear();
  mHulls.Resize(mVHacd.mHulls.Size());
  //mHulls.Reserve(mVHacd.mVoxelizers.Size() + mVHacd.mFinalVoxelizers.Size());
  //mHulls = mVHacd.mHulls;
  for (size_t i = 0; i < mVHacd.mHulls.Size(); ++i)
    mHulls[i] = mVHacd.mHulls[i].mQuickHull;

  //for(size_t i = 0; i < mVHacd.mFinalVoxelizers.Size(); ++i)
  //{
  //  QuickHull hull;
  //  hull.Build(mVHacd.mFinalVoxelizers[i]);
  //  mHulls.PushBack(hull.mQuickHull);
  //}
  //
  //for (size_t i = 0; i < mVHacd.mVoxelizers.Size(); ++i)
  //{
  //  QuickHull hull;
  //  hull.Build(mVHacd.mVoxelizers[i]);
  //  mHulls.PushBack(hull.mQuickHull);
  //}
}

void ZeroVHacd::Clear()
{
  for (size_t i = 0; i < mHulls.Size(); ++i)
  {
    mHulls[i]->Clear();
  }
  mHulls.Clear();

  mVHacd.Clear();
}

int ZeroVHacd::GetHullCount()
{
  return mHulls.Size();
}

Zilch::HandleOf<ZeroEngine::QuickHull3D> ZeroVHacd::GetHull(int index)
{
  return mHulls[index];
}

//***************************************************************************
//void ZeroVHacd::OnLogicUpdate(ZeroEngine::UpdateEvent* event)
//{
//  // Do we have a Model component?
//  ZeroEngine::Model* model = this->GetOwner()->has(ZeroEngine::Model);
//  if (model != nullptr)
//    Zilch::Console::WriteLine("We have a Model!");
//  
//  // Send our own update event
//  // We could also replace this with ZilchEvent to send basic events
//  // Note: ZilchAllocate should be used for any type that is
//  // typically allocated within Zilch, such as a CastFilter
//  Zilch::HandleOf<ZeroVHacdEvent> toSend = ZilchAllocate(ZeroVHacdEvent);
//  toSend->mLives = mLives;
//  this->GetOwner()->DispatchEvent("ZeroVHacdUpdate", toSend);
//}
//
////***************************************************************************
//Zilch::String ZeroVHacd::Speak()
//{
//  Zilch::String text("Hello World");
//  Zilch::Console::WriteLine(text);
//  return text;
//}
//
////***************************************************************************
//float ZeroVHacd::GetHealth()
//{
//  return mHealth;
//}
//
////***************************************************************************
//void ZeroVHacd::SetHealth(float value)
//{
//  if (value < 0)
//    value = 0;
//  else if (value > 100)
//    value = 100;
//  
//  mHealth = value;
//}

//***************************************************************************
ZilchDefineType(ZeroVHacdEvent, builder, type)
{
  // This is required for event binding
  ZilchBindDestructor();
  ZilchBindConstructor();
  
  ZilchBindFieldProperty(mLives);
}
