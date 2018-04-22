#include "ZeroVHacdPrecompiled.hpp"

//***************************************************************************
ZilchDefineStaticLibraryAndPlugin(ZeroVHacdLibrary, ZeroVHacdPlugin, ZilchDependencyStub(Core) ZilchDependencyStub(ZeroEngine))
{
  ZilchInitializeType(ZeroVHacd);
  ZilchInitializeType(JobManager);
  ZilchInitializeType(DownloadJobEvent);
  ZilchInitializeType(VHacd_Orig);
  // Auto Initialize (used by Visual Studio plugins, do not remove this line)
}

//***************************************************************************
void ZeroVHacdPlugin::Initialize()
{
  // One time startup logic goes here
  // This runs after our plugin library/reflection is built
  Zilch::Console::WriteLine("ZeroVHacdPlugin::Initialize");
}

//***************************************************************************
void ZeroVHacdPlugin::Uninitialize()
{
  // One time shutdown logic goes here
  Zilch::Console::WriteLine("ZeroVHacdPlugin::Uninitialize");
}
