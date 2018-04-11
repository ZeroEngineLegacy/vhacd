#pragma once

class Voxelizer;

class QuickHull
{
public:

  bool Build(Voxelizer& voxelizer);
  bool Build(Zilch::Array<Real3>& vertices);
  bool Build(QuickHull& hull0, QuickHull& hull1);
  bool Build(ZeroEngine::QuickHull3D* hull0, ZeroEngine::QuickHull3D* hull1);
  void AddPoints(ZeroEngine::QuickHull3D* hull);

  Real ComputeVolume();

  void BakeHull();

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

  Zilch::HandleOf<ZeroEngine::QuickHull3D> mQuickHull;
};