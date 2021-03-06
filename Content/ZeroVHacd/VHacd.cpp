#include "ZeroVHacdPrecompiled.hpp"

#include "VHacd.hpp"

const float sVoxelizationWeight = 0.02f;
const float sRecursionWeight = 0.8f;
const float sMergingWeight = 0.16f;
const float sResamplingWeight = 0.02f;

VHacd::VHacd()
{
  mCallbackFn = nullptr;
  mClientData = nullptr;
  mForceStop = false;
  mTotalPercent = 0;
  mStepPercent = 0;
  mFast = false;

  mResampleMesh = true;
  mAllowedConcavityVolumeError = 0.001f;
  mAllowedVolumeSurfaceAreaRatio = 2.0f / 2.5f;
  mBalanceWeight = 0.05f;
  mSymmetryWeight = 0.05f;
  mRefinement = 0.9f;
  mRefinementStep = 5;
}

void VHacd::SetProgressCallback(CallbackFn callbackFn, void* clientData)
{
  mCallbackFn = callbackFn;
  mClientData = clientData;
}

void VHacd::Compute(Real fidelity, int recursions, TriangleMesh& mesh)
{
  mStepPercent = 0;
  mTotalPercent = 0;

  mFidelity = fidelity;
  mMaxRecusionDepth = recursions;

  Initialize(mesh);
  ComputeApproximateConvexDecomposition();
  MergeHulls();
  Resample();

  if (mForceStop)
    Clear();
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
    mHulls[i].Clear();
  mHulls.Clear();
}

void VHacd::ComputeSubDivisions(Aabb& aabb)
{
  float minVoxels = 100;
  float maxVoxels = 1000000;
  // Use quadratic interpolation to get a better feel from fidelity
  float t = mFidelity * mFidelity;
  int targetVoxelCount = (int)Math::Lerp(minVoxels, maxVoxels, t);

  Real3 extents = aabb.GetSize();
  Real volume = extents.x * extents.y * extents.z;
  Real xySA = extents.x * extents.y;
  Real xzSA = extents.x * extents.z;
  Real yzSA = extents.y * extents.z;

  Real targetVoxelVolume = volume / targetVoxelCount;
  Real targetVoxelSize = Math::Pow(targetVoxelVolume, 1 / 3.0f);

  // Given a target voxel count, how do we "best" split up the given volume?
  int largestAxis = 0;
  if (extents.x > extents.y && extents.x > extents.z)
    largestAxis = 0;
  else if (extents.y > extents.x && extents.y > extents.z)
    largestAxis = 1;
  else
    largestAxis = 2;

  // Find the which two axis have the largest surface area
  int offAxis = 0;
  if (xySA > xzSA && xySA > yzSA)
    offAxis = 2;
  else if (xzSA > xySA && xzSA > yzSA)
    offAxis = 1;
  else
    offAxis = 0;

  int axis1 = (largestAxis + 1) % 3;
  int axis2 = (largestAxis + 2) % 3;
  Integer3 old = mSubDivisions;
  mSubDivisions[largestAxis] = (int)Math::Ceil(extents[largestAxis] / targetVoxelSize);
  mSubDivisions[axis1] = (int)Math::Ceil(extents[axis1] / targetVoxelSize);
  mSubDivisions[axis2] = (int)Math::Ceil(extents[axis2] / targetVoxelSize);

  // Compute the min step value we can take as a quarter of the largest axis of 10, whichever is smaller.
  // Doing more steps causes better speed and results as too few steps causes the full recursion
  // to happen which creates a very large number of hulls.
  float minSteps = Math::Min(10.0f, (float)mSubDivisions[largestAxis] / 4.0f);
  Real refinementValue = Math::Lerp(minSteps, 1.0f, mRefinement);
  mRefinementStep = (int)Math::Floor(refinementValue);
}

void VHacd::Initialize(TriangleMesh& mesh)
{
  // Construct a more run-time friend mesh format
  mMesh = mesh;

  Aabb aabb = mMesh.mAabb;
  // Expand the initial aabb by a small amount to account for float errors
  aabb = Aabb::BuildFromCenterAndHalfExtents(aabb.GetCenter(), aabb.GetHalfSize() * 1.1f);

  ComputeSubDivisions(aabb);

  UpdateProgress(mTotalPercent, "Voxelizing", mStepPercent, String());

  // Build the first voxelizer
  Voxelizer& voxelizer = mVoxelizers.PushBack();
  voxelizer.CreateVoxels(mSubDivisions, aabb, VoxelState::Unknown);

  // Write each triangle in the mesh to the voxel grid
  for (size_t i = 0; i < mMesh.GetCount(); ++i)
  {
    Triangle tri = mMesh.GetTriangle(i);
    voxelizer.WriteTriangle(tri);
  }
  // Flood fill to mark the inside/outside voxels
  voxelizer.FillOutside();
  voxelizer.Fill();

  mTotalPercent += sVoxelizationWeight;
  UpdateProgress(mTotalPercent, "Voxelizing", mStepPercent, "Finished");
}

void VHacd::ComputeApproximateConvexDecomposition()
{
  mStepPercent = 0;
  UpdateProgress(mTotalPercent, "Approximate Convex Decomposition", mStepPercent, String());

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

  mTotalPercent += sRecursionWeight;
  UpdateProgress(mTotalPercent, "Approximate Convex Decomposition", mStepPercent, "Finished");
}

void VHacd::Recurse(int depth)
{
  // Cap out at some max recursion depth
  if (depth >= mMaxRecusionDepth || mForceStop == true)
    return;
  if (mVoxelizers.Empty())
    return;

  // We'll have at max have 2 times as many voxelizers 
  Zero::Array<Voxelizer> newVoxelizers;
  newVoxelizers.Reserve(mVoxelizers.Size() * 2);

  for (size_t i = 0; i < mVoxelizers.Size(); ++i)
  {
    // Try to split each voxel grid into two
    Voxelizer& voxelizer = mVoxelizers[i];
    SplitVoxelizer(voxelizer, newVoxelizers, depth);
  }

  mVoxelizers.Swap(newVoxelizers);

  Recurse(depth + 1);
}

bool VHacd::SplitVoxelizer(Voxelizer& voxelizer, Array<Voxelizer>& newVoxelizers, int depth)
{
  if (mForceStop == true)
    return false;

  UpdateProgress(mTotalPercent, "Approximate Convex Decomposition", mStepPercent, BuildString("Depth: ", depth));
  float pow = Math::Pow(2.0f, (float)depth);
  float invPow = (1.0f / pow);
  mStepPercent += invPow / (float)mMaxRecusionDepth;

  Real voxelVolume = voxelizer.ComputeVolume();
  QuickHull hull;

  // Failed to build a hull (the voxel has no volume? Shouldn't happen...)
  bool success = hull.Build(voxelizer);
  if (!success)
    return false;

  Real hullVolume = hull.ComputeVolume();
  // Cache the original convex hull volume as a normalization factor
  if (depth == 0)
  {
    mInitialConvexHullVolume = hullVolume;
  }

  // Measure how much error there is between the volume of the voxel grid and the volume of the convex hull of the voxel grid.
  // Normalize this by the original hull's volume so we know when we reach really small discrepancies with respect to the original mesh size.
  Real measuredConcavityVolumeError = (hullVolume - voxelVolume) / mInitialConvexHullVolume;
  bool tooMuchError = measuredConcavityVolumeError > mAllowedConcavityVolumeError;

  // Initially in a larger mesh, the volume error should be larger than the surface area by a fair amount (aka volume error / surface area >> 1).
  // As we improve results we'll reach a point where the volume error is really close to the surface area of the mesh.
  // This means we can't actually refine that much more, typically because the mesh has gotten too small.
  Real surfaceAreaRatio = voxelizer.GetSurfaceVolume() / mInitialConvexHullVolume;
  Real measuredVolumeToSurfaceAreaRatio = measuredConcavityVolumeError / surfaceAreaRatio;
  bool tooSmall = measuredVolumeToSurfaceAreaRatio < mAllowedVolumeSurfaceAreaRatio;

  // This sub-mesh is convex enough (no further refinement necessary) if it doesn't
  // have too much error or it becomes too small to work with.
  bool isConvexEnough = !tooMuchError || tooSmall;
  if (isConvexEnough)
  {
    mFinalVoxelizers.PushBack(voxelizer);
    mStepPercent += invPow * ((float)mMaxRecusionDepth - depth - 1) / (float)mMaxRecusionDepth;
    UpdateProgress(mTotalPercent, "Approximate Convex Decomposition", mStepPercent, BuildString("Depth: ", depth));
    return false;
  }

  // Compute the principle axes so we can measure the "axis of revolution".
  // See FindBestSplitPlane for an explanation of this term.
  voxelizer.ComputeSymmetryComponents();

  // Compute all of the possible split planes for the given voxel set based upon some input parameters
  Array<SplitPlane> planes;
  ComputePossibleSplitPlanes(voxelizer, planes);

  // Find what the best split plane
  SplitPlane bestSplitPlane;
  bool foundSplitPlane = false;
  if(!mFast)
    foundSplitPlane = FindBestSplitPlane(voxelizer, planes, measuredConcavityVolumeError, bestSplitPlane);
  else
    foundSplitPlane = FindBestSplitPlaneNew(voxelizer, planes, measuredConcavityVolumeError, bestSplitPlane);

  // If we couldn't find a split plane then there's nothing more to do with this voxel grid.
  // This is likely due to a lower refinement that made the test split completely lopsided.
  if(!foundSplitPlane)
  {
    mFinalVoxelizers.PushBack(voxelizer);
    return false;
  }
  
  // @ToDo
  bool refinement = false;
  if (refinement)
  {
    // Test an area around the best plane for a better split point
  }

  
  // @ JoshD: Optimize these copies
  Voxelizer front;
  Voxelizer back;
  bool isSplit;
  if (!mFast)
    isSplit = voxelizer.Split(bestSplitPlane.mAxis, bestSplitPlane.mAxisValue, front, back);
  else
    isSplit = voxelizer.Split(bestSplitPlane.mAxis, bestSplitPlane.mAxisDiscretizedValue, front, back);
  if (isSplit)
  {
    newVoxelizers.PushBack(front);
    newVoxelizers.PushBack(back);
  }
  else
  {
    mFinalVoxelizers.PushBack(voxelizer);
    return false;
  }
  return true;
}

void VHacd::ComputePossibleSplitPlanes(Voxelizer& voxelizer, Array<SplitPlane>& planes)
{
  size_t offset = mRefinementStep;
  for(size_t axis = 0; axis < 3; ++axis)
  {
    for (size_t i = 0; i < (size_t)voxelizer.mSubDivisions[axis]; i += offset)
    {
      Integer3 voxelCoord = Integer3::cZero;
      voxelCoord[axis] = i;
      Aabb aabb = voxelizer.GetVoxelAabb(voxelCoord);
      SplitPlane splitPlane(axis, aabb.GetCenter()[axis]);
      splitPlane.mAxisDiscretizedValue = i;
      planes.PushBack(splitPlane);
    }
  }
}

bool VHacd::FindBestSplitPlane(Voxelizer& voxelizer, Array<SplitPlane>& planes, Real parentConcavity, SplitPlane& result)
{
  size_t bestPlaneIndex = 0;
  float bestScore = Math::PositiveMax();
  // Test each plane, keeping the one with the lowest score
  for (size_t i = 0; i < planes.Size(); ++i)
  {
    SplitPlane& planeData = planes[i];
    int planeAxis = planeData.mAxis;
    float axisValue = planeData.mAxisValue;
    float score = TestSplit(voxelizer, planeAxis, axisValue, parentConcavity);
    if (score < bestScore)
    {
      bestScore = score;
      bestPlaneIndex = i;
    }
  }

  result = planes[bestPlaneIndex];
  return true;
}

bool VHacd::FindBestSplitPlaneNew(Voxelizer& voxelizer, Array<SplitPlane>& planes, Real parentConcavity, SplitPlane& result)
{
  // Keep track that we actually found a valid split plane (the sub-divisions might be too small for our refinement)
  bool splitPlaneFound = false;
  float bestScore = Math::PositiveMax();
  SplitPlane bestSplitPlane;
  
  // For each axis, find all of the possible split volume values and keep track of the best scoring one.
  for (size_t axis = 0; axis < 3; ++axis)
  {
    Array<SplitData> axisSplitData;
    GetSplitVolumes(voxelizer, axis, axisSplitData);

    // Find which split plane on this axis is the best (of all axes tried so far)
    for (size_t i = 0; i < axisSplitData.Size(); ++i)
    {
      SplitData& splitData = axisSplitData[i];

      Real score = ComputeScore(splitData, axis, voxelizer, parentConcavity);
      if (score < bestScore)
      {
        bestScore = score;
        bestSplitPlane = SplitPlane(axis, splitData.mSplitValue);
        bestSplitPlane.mAxisDiscretizedValue = splitData.mFrontIndex;
        splitPlaneFound = true;
      }
    }
  }

  result = bestSplitPlane;
  return splitPlaneFound;
}

void VHacd::GetSplitVolumes(Voxelizer& voxelizer, int axis, Array<SplitData>& splitDataList)
{
  // To efficiently compute the best split plane on an axis, we can incrementally build 2 convex hulls,
  // one going back-to-front and the other going front-to-back. This avoids having to recompute a
  // lot of convex hull data and drastically speeds up the search.
  Zero::IncrementalQuickHull3D hullFront;
  Zero::IncrementalQuickHull3D hullBack;
  // Needed to compute the volume of a convex hull
  QuickHull bakedHull;

  // Compute all of the split indices to test add given our sub-division count
  size_t offset = mRefinementStep;
  int axisSubDivisions = voxelizer.mSubDivisions[axis];
  int lastAxisIndex = axisSubDivisions - 1;
  Array<int> indices;
  for (int i = 0; i < axisSubDivisions; i += offset)
    indices.PushBack(i);
  
  // Always add the last possible index if it doesn't create a duplicate. This is needed for the back-to-front set.
  if(indices.Back() != lastAxisIndex)
    indices.PushBack(lastAxisIndex);

  // As two of the split indices are always 0 and the last index, if we only have 2 then there's nothing to do.
  int total = indices.Size();
  if (total < 2)
    return;

  // Since the indices is padded with the first and last index, the actual possible split plane count is the total - 2.
  int count = total - 2;
  splitDataList.Resize(count);

  // These are cached outside the loop so memory will be re-used
  Array<Real3> surfaceVoxelCenters;
  Array<Real3> points;

  // Keep track of the accumulated volume for each side as we iterate
  Real accumulatedLeftVolume = 0;
  Real accumulatedRightVolume = 0;
  for (int i = 0; i < count; ++i)
  {
    // Compute the indices into the splitDataList for both sides
    int leftIndex = i;
    int rightIndex = count - i - 1;
    SplitData& leftSplitData = splitDataList[leftIndex];
    SplitData& rightSplitData = splitDataList[rightIndex];

    // Compute the current sub-range of indices for the left side. The left side is inclusive on the boundary
    // (e.g. it includes the end point) otherwise we'd miss some data. This would cause duplicate testing on
    // the start though so we have to make the start exclusive (except on the first pass we have to make sure to include 0)
    int leftStart = indices[i];
    int leftEnd = indices[i + 1];
    if (i != 0)
      ++leftStart;
    
    // Compute the sub-range on the right side. The right sides range is [start, end)
    int rightStart = indices[total - i - 1];
    int rightEnd = indices[total - i - 2];
    
    // Set the split point for the split data. This is only needs to be computed for one of the passes
    Integer3 voxelCoord = Integer3::cZero;
    voxelCoord[axis] = leftEnd;
    Aabb aabb = voxelizer.GetVoxelAabb(voxelCoord);
    leftSplitData.mSplitValue = aabb.GetCenter()[axis];
    leftSplitData.mFrontIndex = leftEnd;

    bool success;

    // Attempt to split the left side. This side needs to be inclusive on the end so add one.
    Real leftSubVolume = 0;
    surfaceVoxelCenters.Clear();
    success = voxelizer.GetSplitTest(axis, leftStart, leftEnd + 1, surfaceVoxelCenters, leftSubVolume);
    // If this is a valid split (there were voxels) then record volume information
    if (success)
    {
      // Accumulate total volume
      accumulatedLeftVolume += leftSubVolume;
      leftSplitData.mFrontVoxelVolume = accumulatedLeftVolume;

      // Incrementally expand this side's hull
      points.Clear();
      voxelizer.AabbCentersToPoints(surfaceVoxelCenters, points);
      hullFront.Expand(points);
      // Compute the total hull's volume
      bakedHull.BakeHull(hullFront);
      leftSplitData.mFrontHullVolume = bakedHull.ComputeVolume();
    }

    // Now do the same for the right side.
    Real rightSubVolume = 0;
    surfaceVoxelCenters.Clear();
    success = voxelizer.GetSplitTest(axis, rightStart, rightEnd, surfaceVoxelCenters, rightSubVolume);
    if (success)
    {
      // Accumulate total volume
      accumulatedRightVolume += rightSubVolume;
      rightSplitData.mBackVoxelVolume = accumulatedRightVolume;

      // Incrementally expand this side's hull
      points.Clear();
      voxelizer.AabbCentersToPoints(surfaceVoxelCenters, points);
      hullBack.Expand(points);
      // Compute the total hull's volume
      bakedHull.BakeHull(hullBack);
      rightSplitData.mBackHullVolume = bakedHull.ComputeVolume();
    }
  }
}

float VHacd::TestSplit(Voxelizer& voxelizer, int axis, Real axisValue, Real parentConcavity)
{
  if (mForceStop == true)
    return Math::PositiveMax();

  Voxelizer front;
  Voxelizer back;
  // Split along the given split plane into two pieces. If we failed then this probably was
  // too small of a grid such that one side became completely empty.
  bool wasSplit = voxelizer.Split(axis, axisValue, front, back);
  if (!wasSplit)
    return Math::PositiveMax();

  // Build the convex hull of both split sides
  QuickHull frontHull;
  QuickHull backHull;
  frontHull.Clear();
  backHull.Clear();
  
  frontHull.Build(front);
  backHull.Build(back);

  SplitData splitData;
  splitData.mFrontHullVolume = frontHull.ComputeVolume();
  splitData.mFrontVoxelVolume = front.ComputeVolume();
  splitData.mBackHullVolume = backHull.ComputeVolume();
  splitData.mBackVoxelVolume = back.ComputeVolume();
  return ComputeScore(splitData, axis, voxelizer, parentConcavity);
}

float VHacd::ComputeScore(SplitData& splitData, int axis, Voxelizer& voxelizer, Real parentConcavity)
{
  // Compute the voxel volume and convex hull volume of both sides.
  Real frontVoxelVolume = splitData.mFrontVoxelVolume;
  Real backVoxelVolume = splitData.mBackVoxelVolume;
  Real frontHullVolume = splitData.mFrontHullVolume;
  Real backHullVolume = splitData.mBackHullVolume;

  // This split has a completely lop-sided grid. There's no reason to compute the score as this is as bad as possible.
  if (backVoxelVolume == 0 || frontVoxelVolume == 0)
    return Math::PositiveMax();

  // The heuristic for HACD is split up into 3 pieces:
  // (all of these are normalized by the initial convex hull's volume

  // 1. Connectivity: the sum of the concavity errors of both sides. Concavity error is measured as the
  // difference in volume between the convex hull and the voxel grid.
  Real concavityFront = frontHullVolume - frontVoxelVolume;
  Real concavityBack = backHullVolume - backVoxelVolume;
  Real concavityError = (concavityFront + concavityBack) / mInitialConvexHullVolume;

  // 2. Balance: How evenly the plane splits the voxel grid. This is measured by how close the two sides are in volume.
  Real balance = Math::Abs(frontVoxelVolume - backVoxelVolume) / mInitialConvexHullVolume;

  // 3. Symmetry: This is the most confusing of the 3 measurements, but this is defined to penalize clipping planes that are close to
  // orthogonal to a "potential revolution axis". A revolution axis is found by computing the 3 principle axes and finding the two that
  // are most similar to each other in length. The other axis is the revolution axis. This is similar to finding the axis of most spread 
  // but a bit different. By finding which axis is the "least similar" this seems to try and favor a wider spread base. 
  // @JoshD: Toy around with actually using the axis of most spread?
  Real w = voxelizer.mRevolutionWeight;

  // symmetry = w * Dot(plane.Normal, revolutionAxis).
  // Since plane.Normal is axis aligned then this simplifies to:
  Real symmetry = w * voxelizer.mRevolutionAxis[axis];

  // These 3 scores are then combined with a few weight values (penalize most heavily based upon concavity).
  // For unknown reason, the other weights are also scaled by the parent's concavity error (so they become worth less the less concave it is?)
  Real score = concavityError + mBalanceWeight * balance * parentConcavity + mSymmetryWeight * symmetry * parentConcavity;

  return score;
}

void VHacd::MergeHulls()
{
  mStepPercent = 0;
  UpdateProgress(mTotalPercent, "Merging Hulls", mStepPercent, String());

  Zilch::Array<float> volumes;
  volumes.Resize(mHulls.Size());

  Zilch::Array<float> combinedVolumes;

  float percentIncrement = 1.0f / (mHulls.Size() - (float)mMaxHulls);
  
  int count = 0;
  int totalCount = mHulls.Size() - mMaxHulls;
  while (mHulls.Size() > (size_t)mMaxHulls)
  {
    if (mForceStop == true)
      break;

    // Rebuild the entire table (slow, only some rows change each time)
    BuildHullTable(volumes, combinedVolumes);

    // Find two hulls to merge (this is n-squared but hardly the slow part of the algorithm)
    size_t iX, iY;
    FindHullsToMerge(volumes, combinedVolumes, iX, iY);

    // Replace one of the hulls with the combined one and then remove the other hull
    int index = iX + iY * mHulls.Size();

    // Rebuild the hull (too much memory to keep all around)
    QuickHull combinedHull;
    combinedHull.Build(mHulls[iX], mHulls[iY]);

    mHulls[iX] = combinedHull;
    mHulls.EraseAt(iY);

    mStepPercent += percentIncrement;
    ++count;
    UpdateProgress(mTotalPercent, "Merging Hulls", mStepPercent, BuildString(count, " of ", totalCount));
  }

  mTotalPercent += sMergingWeight;
  UpdateProgress(mTotalPercent, "Merging Hulls", mStepPercent, "Finished");
}

void VHacd::BuildHullTable(Zilch::Array<Real>& volumes, Zilch::Array<Real>& combinedVolumes)
{
  volumes.Resize(mHulls.Size());

  // Compute the volume of all of the hulls
  for (size_t i = 0; i < mHulls.Size(); ++i)
  {
    volumes[i] = mHulls[i].ComputeVolume();
  }

  combinedVolumes.Resize(mHulls.Size() * mHulls.Size());
  // Compute upper-diagonal matrix of the combined hulls for every pairing
  for (size_t y = 0; y < mHulls.Size(); ++y)
  {
    for (size_t x = 0; x < mHulls.Size(); ++x)
    {
      int index = x + y * mHulls.Size();

      // Only check the upper diagonal (skipping the diagonal) sections
      if (y > x || y == x)
        continue;

      QuickHull combinedHull;
      combinedHull.Build(mHulls[x], mHulls[y]);
      combinedVolumes[index] = combinedHull.ComputeVolume();
      combinedHull.Clear();
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

void VHacd::Resample()
{
  if (!mResampleMesh || mForceStop == true)
    return;

  mStepPercent = 0;
  UpdateProgress(mTotalPercent, "Resampling", mStepPercent, String());

  for (size_t i = 0; i < mHulls.Size(); ++i)
    Resample(mHulls[i], i, mHulls.Size());

  mTotalPercent += sResamplingWeight;
  UpdateProgress(mTotalPercent, "Resampling", mStepPercent, "Finished");
}

void VHacd::Resample(QuickHull& hull, int hullIndex, int totalHulls)
{
  Array<Real3> points;
  points = hull.mVertices;
  Aabb aabb;
  for (size_t i = 0; i < points.Size(); ++i)
  {
    aabb.Expand(points[i]);
  }
  Real3 center = aabb.GetCenter();
  Real diagonal = mFinalVoxelizers[0].mVoxelSize.Length();

  float subPercent = totalHulls / ((float)points.Size() + mHulls.Size());
  for (size_t i = 0; i < points.Size(); ++i)
  {
    Ray ray;
    ray.mStart = center;
    ray.mDirection = Math::Normalized(points[i] - center);
    float distance = Math::Length(points[i] - center);

    float t;
    if (mMesh.CastRay(ray, t, points[i]))
    {
      Real3 newPoint = ray.mStart + ray.mDirection * t;
      Real newDistance = Math::Distance(newPoint, points[i]);
      //float delta = Math::Abs(distance - t);
      if (newDistance / diagonal < 0.25f)
        points[i] = newPoint;
    }

    mStepPercent += subPercent;
    if(i % 10)
      UpdateProgress(mTotalPercent, "Resampling", mStepPercent, BuildString(hullIndex, " of ", totalHulls));
  }

  hull.Build(points);
}

void VHacd::UpdateProgress(float totalPercent, const String& stepName, float stepPercent, const String& stepMessage)
{
  if (mCallbackFn != nullptr)
    mCallbackFn(totalPercent, stepName, stepPercent, stepMessage, mClientData);
}
