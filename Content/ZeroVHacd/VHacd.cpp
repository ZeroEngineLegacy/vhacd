#include "ZeroVHacdPrecompiled.hpp"

#include "VHacd.hpp"

void VHacd::Compute(const Integer3& subDivisions, int recursions, Mesh* mesh)
{
  mSubDivisions = subDivisions;
  mRecursions = recursions;

  Initialize(mesh);
  ComputeApproximateConvexDecomposition();
  MergeHulls();
}

void VHacd::Clear()
{
  for (size_t i = 0; i < mFinalVoxelizers.Size(); ++i)
    mFinalVoxelizers[i].mVoxels.Clear();
  for (size_t i = 0; i < mVoxelizers.Size(); ++i)
    mVoxelizers[i].mVoxels.Clear();
  mFinalVoxelizers.Clear();
  mVoxelizers.Clear();

  for (size_t i = 0; i < mHulls.Size(); ++i)
    mHulls[i].mQuickHull->Clear();
  mHulls.Clear();
}

void VHacd::Initialize(Mesh* mesh)
{
  // Construct a more run-time friend mesh format
  mMesh.Create(mesh);

  Aabb aabb = mMesh.mAabb;
  // Expand the initial aabb by a small amount to account for float errors
  aabb = Aabb::BuildFromCenterAndHalfExtents(aabb.GetCenter(), aabb.GetHalfSize() * 1.1f);

  // Build the first voxelizer
  Voxelizer& voxelizer = mVoxelizers.PushBack();
  voxelizer.CreateVoxels(mSubDivisions, aabb);

  // Write each triangle in the mesh to the voxel grid
  for (size_t i = 0; i < mMesh.GetCount(); ++i)
  {
    Triangle tri = mMesh.GetTriangle(i);
    voxelizer.WriteTriangle(tri);
  }
  // Flood fill to mark the inside/outside voxels
  voxelizer.FillOutside();
  voxelizer.Fill();
}

void VHacd::ComputeApproximateConvexDecomposition()
{
  Recurse(0);

  for (size_t i = 0; i < mVoxelizers.Size(); ++i)
  {
    mFinalVoxelizers.PushBack(mVoxelizers[i]);
  }
  mVoxelizers.Clear();

  mHulls.Resize(mFinalVoxelizers.Size());
  for (size_t i = 0; i < mFinalVoxelizers.Size(); ++i)
  {
    mHulls[i].Build(mFinalVoxelizers[i]);
  }
}

void VHacd::Recurse(int depth)
{
  if (depth >= mRecursions)
    return;

  // We'll at max have 2 times as many voxelizers 
  Zero::Array<Voxelizer> newVoxelizers;
  newVoxelizers.Reserve(mVoxelizers.Size() * 2);

  for (size_t i = 0; i < mVoxelizers.Size(); ++i)
  {
    Voxelizer& voxelizer = mVoxelizers[i];

    SplitVoxelizer(voxelizer, newVoxelizers, depth);
  }

  mVoxelizers.Swap(newVoxelizers);

  Recurse(depth + 1);
}

bool VHacd::SplitVoxelizer(Voxelizer& voxelizer, Array<Voxelizer>& newVoxelizers, int depth)
{
  Real voxelVolume = voxelizer.ComputeVolume();
  QuickHull hull;

  // Failed to build a hull (the voxel has no volume? Shouldn't happen)
  bool success = hull.Build(voxelizer);
  if (!success)
    return false;

  Real hullVolume = hull.ComputeVolume();
  // Cache the original convex hull volume as a normalization factor
  if (depth == 0)
  {
    mInitialConvexHullVolume = hullVolume;
  }


  Real concavity = (hullVolume - voxelVolume) / mInitialConvexHullVolume;
  Real ratio = voxelVolume / hullVolume;

  Real concavityError = mConcavity;

  Real error = 1.01f * voxelizer.GetSurfaceVolume() / mInitialConvexHullVolume;

  bool tooMuchError = concavity > concavityError && concavity > error;
  if (!tooMuchError)
  {
    mFinalVoxelizers.PushBack(voxelizer);
    return false;
  }

  
  Array<Real> scores;
  Array<Real2> planes;
  Real3 center = voxelizer.mAabb.GetCenter();
  
  Real3 min = voxelizer.mAabb.mMin;
  Real3 max = voxelizer.mAabb.mMax;

  size_t subDivisions = 5;
  for (size_t subDivision = 0; subDivision < subDivisions; ++subDivision)
  {
    Real t = ((float)subDivision + 0.5f) / subDivisions;
    Real3 value = Math::Lerp(min, max, t);
    planes.PushBack(Real2(0, value[0]));
    planes.PushBack(Real2(1, value[1]));
    planes.PushBack(Real2(2, value[2]));
  }
  

  float bestScore = Math::PositiveMax();
  size_t bestPlaneIndex = 0;
  for (size_t i = 0; i < planes.Size(); ++i)
  {
    Real2 planeData = planes[i];
    int planeAxis = (int)planeData.x;
    float axisValue = planeData.y;
    float score = TestSplit(voxelizer, planeAxis, axisValue);
    scores.PushBack(score);
    if (score < bestScore)
    {
      bestScore = score;
      bestPlaneIndex = i;
    }
  }

  Real2 planeData = planes[bestPlaneIndex];
  int planeAxis = (int)planeData.x;
  float axisValue = planeData.y;
  Voxelizer front;
  Voxelizer back;
  //int axis = (depth + 1) % 3;
  //Real splitValue = voxelizer.mAabb.GetCenter()[axis];

  voxelizer.Split(planeAxis, axisValue, front, back);
  newVoxelizers.PushBack(front);
  newVoxelizers.PushBack(back);
  return true;
}

float VHacd::TestSplit(Voxelizer& voxelizer, int axis, Real axisValue)
{
  Voxelizer front;
  Voxelizer back;
  //int axis = (depth + 1) % 3;
  //Real splitValue = voxelizer.mAabb.GetCenter()[axis];

  bool wasSplit = voxelizer.Split(axis, axisValue, front, back);
  if (!wasSplit)
    return Math::PositiveMax();

  QuickHull frontHull, backHull;
  frontHull.Build(front);
  backHull.Build(back);

  Real frontVoxelVolume = front.ComputeVolume();
  Real backVoxelVolume = back.ComputeVolume();
  Real frontHullVolume = frontHull.ComputeVolume();
  Real backHullVolume = backHull.ComputeVolume();

  Real concavityFront = (frontHullVolume - frontVoxelVolume) / mInitialConvexHullVolume;
  Real concavityBack = (backHullVolume - backVoxelVolume) / mInitialConvexHullVolume;
  Real concavity = (concavityFront + concavityBack);
  Real balance = Math::Abs(frontVoxelVolume - backVoxelVolume) / mInitialConvexHullVolume;

  float alpha = 0.05f;
  //alpha = 0;
  Real score = concavity + alpha * balance;

  return score;
}

void VHacd::MergeHulls()
{
  Zilch::Array<float> volumes;
  volumes.Resize(mHulls.Size());

  Zilch::Array<float> combinedVolumes;
  Zilch::Array<QuickHull> combinedHulls;
  
  while (mHulls.Size() > (size_t)mMaxHulls)
  {
    // Rebuild the entire table (slow, only some rows change each time)
    BuildHullTable(combinedHulls, volumes, combinedVolumes);

    // Find two hulls to merge (this is n-squared but hardly the slow part of the algorithm)
    size_t iX, iY;
    FindHullsToMerge(volumes, combinedVolumes, iX, iY);

    // Replace one of the hulls with the combined one and then remove the other hull
    int index = iX + iY * mHulls.Size();
    QuickHull& combinedHull = combinedHulls[index];
    mHulls[iX] = combinedHull;
    mHulls.EraseAt(iY);
  }
}

void VHacd::BuildHullTable(Zilch::Array<QuickHull>& combinedHulls, Zilch::Array<Real>& volumes, Zilch::Array<Real>& combinedVolumes)
{
  volumes.Resize(mHulls.Size());

  // Compute the volume of all of the hulls
  for (size_t i = 0; i < mHulls.Size(); ++i)
  {
    volumes[i] = mHulls[i].ComputeVolume();
  }

  combinedVolumes.Resize(mHulls.Size() * mHulls.Size());
  combinedHulls.Resize(mHulls.Size() * mHulls.Size());
  // Compute upper-diagonal matrix of the combined hulls for every pairing
  for (size_t y = 0; y < mHulls.Size(); ++y)
  {
    for (size_t x = 0; x < mHulls.Size(); ++x)
    {
      int index = x + y * mHulls.Size();

      // Only check the upper diagonal (skipping the diagonal) sections
      if (y > x || y == x)
        continue;

      combinedHulls[index].Build(mHulls[x], mHulls[y]);
      combinedVolumes[index] = combinedHulls[index].ComputeVolume();
    }
  }
}

void VHacd::FindHullsToMerge(Zilch::Array<Real>& volumes, Zilch::Array<Real>& combinedVolumes, size_t& iX, size_t& iY)
{
  float minScore = Math::PositiveMax();
  // Search for the lowest score and keep track of its indices
  size_t size = volumes.Size();
  for (size_t y = 0; y < size; ++y)
  {
    for (size_t x = 0; x < size; ++x)
    {
      // Only check the upper diagonal (skipping the diagonal) sections
      if (y > x || y == x)
        continue;

      float combinedVolume = combinedVolumes[x + y * size];
      float volumeSum = volumes[x] + volumes[y];
      float score = volumeSum / combinedVolume;
      score = combinedVolume - volumeSum;
      if (score < minScore)
      {
        minScore = score;
        iX = x;
        iY = y;
      }
    }
  }
}
