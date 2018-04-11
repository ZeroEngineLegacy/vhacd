#pragma once
// We use precompiled headers to support the fastest possible compilation
#define ZeroImportDll
#include "Zilch.hpp"
#include "Core.hpp"
#include "ZeroEngine.hpp"

// Declares our Zilch Library (where we get LibraryBuilder from)
// This also handles the plugin initialization
ZilchDeclareStaticLibraryAndPlugin(HacdPluginLibrary, HacdPluginPlugin);

using Zilch::Real;
using Zilch::Real2;
using Zilch::Real3;
using Zilch::Real4;

using ZeroEngine::Mesh;
using ZeroEngine::MultiConvexMesh;

// We also encourage including all files within here, rather than within headers
// If another project needed to include any headers from our project, then they would simply
// include our precompiled header instead (ideally within their own precompiled header)
// This also must means you must order headers in dependency order (who depends on who)

#include "VHACD.h"

#include "HacdPlugin.hpp"
// Auto Includes (used by Visual Studio plugins, do not remove this line)
