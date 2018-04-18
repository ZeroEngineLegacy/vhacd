#pragma once
// We use precompiled headers to support the fastest possible compilation
#define ZeroImportDll
#include "Zilch.hpp"
#include "Core.hpp"
#include "ZeroEngine.hpp"

using Zilch::Integer2;
using Zilch::Integer3;
using Zilch::Integer4;
using Zilch::Real;
using Zilch::Real2;
using Zilch::Real3;
using Zilch::Real4;
using Math::Vector2;
using Math::Vector3;
using Math::Matrix3;
using Zero::Array;

using ZeroEngine::Mesh;
#include "Shapes.hpp"
#include "ExtraMath.hpp"

// Declares our Zilch Library (where we get LibraryBuilder from)
// This also handles the plugin initialization
ZilchDeclareStaticLibraryAndPlugin(ZeroVHacdLibrary, ZeroVHacdPlugin);

// We also encourage including all files within here, rather than within headers
// If another project needed to include any headers from our project, then they would simply
// include our precompiled header instead (ideally within their own precompiled header)
// This also must means you must order headers in dependency order (who depends on who)

#include "ZeroVHacd.hpp"
#include "Voxelizer.hpp"
#include "Shapes.hpp"
#include "Intersection.hpp"
#include "QuickHull.hpp"
#include "ExtraMath.hpp"
#include "TriangleMesh.hpp"
// Auto Includes (used by Visual Studio plugins, do not remove this line)
