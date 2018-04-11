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

  Zilch::HandleOf<ZeroEngine::QuickHull3D> mQuickHull;
};