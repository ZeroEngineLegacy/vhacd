#include "HacdPluginPrecompiled.hpp"

//***************************************************************************
ZilchDefineStaticLibraryAndPlugin(HacdPluginLibrary, HacdPluginPlugin, ZilchDependencyStub(Core) ZilchDependencyStub(ZeroEngine))
{
  ZilchInitializeType(HacdPlugin);
  ZilchInitializeType(HacdPluginEvent);
  // Auto Initialize (used by Visual Studio plugins, do not remove this line)
}

//***************************************************************************
void HacdPluginPlugin::Initialize()
{
  // One time startup logic goes here
  // This runs after our plugin library/reflection is built
  Zilch::Console::WriteLine("HacdPluginPlugin::Initialize");
}

//***************************************************************************
void HacdPluginPlugin::Uninitialize()
{
  // One time shutdown logic goes here
  Zilch::Console::WriteLine("HacdPluginPlugin::Uninitialize");
}
