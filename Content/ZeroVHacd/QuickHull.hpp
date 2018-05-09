#pragma once

class Voxelizer;

#include "QuickHull3D.hpp"

class QuickHull
{
public:

  bool Build(Voxelizer& voxelizer);
  bool Build(QuickHull& hull0, QuickHull& hull1);
  bool Build(Zilch::Array<Real3>& vertices);
  void Clear();

  Real ComputeVolume();

  void BakeHull(Zero::IncrementalQuickHull3D& quickHull);

  Zilch::HandleOf<ZeroEngine::QuickHull3D> ToHandle();

  struct Edge
  {
    size_t mFaceIndex;
    size_t mTwinIndex;
    size_t mVertexIndex;
  };
  struct Face
  {
    Array<size_t> mEdges;
  };

  Array<Real3> mVertices;
  Array<Edge> mEdges;
  Array<Face> mFaces;
};
