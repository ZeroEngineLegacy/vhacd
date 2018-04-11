#include "ZeroVHacdPrecompiled.hpp"

#include "VHacd.hpp"

void VHacd::Compute(const Integer3& subDivisions, int recursions, Mesh* mesh)
{
  mSubDivisions = subDivisions;
  mRecursions = recursions;

  Zero::Array<Real3> vertices;
  Zero::Array<int> indices;

  Zilch::HandleOf<ZeroEngine::VertexBuffer> verticesHandle = mesh->GetVertices();
  ZeroEngine::VertexBuffer* VertexBuffer = verticesHandle;
  int vertexCount = VertexBuffer->GetVertexCount();

  Aabb aabb;
  for (int i = 0; i < vertexCount; ++i)
  {
    Real4 pos = VertexBuffer->GetVertexData(i, ZeroEngine::VertexSemantic::GetPosition());
    Real3 point = Real3(pos.x, pos.y, pos.z);
    vertices.PushBack(point);

    aabb.Expand(point);
  }

  Zilch::HandleOf<ZeroEngine::IndexBuffer> indicesHandle = mesh->GetIndices();
  ZeroEngine::IndexBuffer* indexBuffer = indicesHandle;
  int indexCount = indexBuffer->GetCount();
  for (int i = 0; i < indexCount; ++i)
  {
    int index = indexBuffer->Get(i);
    indices.PushBack(index);
  }

  aabb = Aabb::BuildFromCenterAndHalfExtents(aabb.GetCenter(), aabb.GetHalfSize() * 1.1f);
  Voxelizer voxelizer;
  voxelizer.CreateVoxels(mSubDivisions, aabb);

  for (size_t i = 0; i < indices.Size(); i += 3)
  {
    Triangle tri;
    tri.mP0 = vertices[indices[i + 0]];
    tri.mP1 = vertices[indices[i + 1]];
    tri.mP2 = vertices[indices[i + 2]];
    voxelizer.WriteTriangle(tri);
  }
  voxelizer.FillOutside();
  voxelizer.Fill();

  mVoxelizers.PushBack(voxelizer);
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

void VHacd::Recurse(int depth)
{
  if (depth >= mRecursions)
    return;

  Zero::Array<Voxelizer> newVoxelizers;
  newVoxelizers.Reserve(mVoxelizers.Size() * 2);

  for (size_t i = 0; i < mVoxelizers.Size(); ++i)
  {
    Voxelizer& voxelizer = mVoxelizers[i];
    Real voxelVolume = voxelizer.ComputeVolume();
    QuickHull hull;

    bool success = hull.Build(voxelizer);
    if (!success)
      continue;

    Real hullVolume = hull.ComputeVolume();

    Real ratio = voxelVolume / hullVolume;

    if(0.95f < ratio && ratio < 1.05f)
    {
      mFinalVoxelizers.PushBack(voxelizer);
      continue;
    }


    Voxelizer front;
    Voxelizer back;
    int axis = (depth + 1) % 3;
    Real splitValue = voxelizer.mAabb.GetCenter()[axis];
    
    voxelizer.Split(axis, splitValue, front, back);
    newVoxelizers.PushBack(front);
    newVoxelizers.PushBack(back);
  }

  mVoxelizers.Swap(newVoxelizers);

  Recurse(depth + 1);
}

void VHacd::MergeHulls()
{
  Zilch::Array<float> volumes;
  volumes.Resize(mHulls.Size());

  Zilch::Array<float> combinedVolumes;
  Zilch::Array<QuickHull> combinedHulls;
  

  while (mHulls.Size() > (size_t)mMaxHulls)
  {
    Build(combinedHulls, volumes, combinedVolumes);

    size_t iX, iY;
    FindHullsToMerge(volumes, combinedVolumes, iX, iY);
    int index = iX + iY * mHulls.Size();

    QuickHull& combinedHull = combinedHulls[index];
    Zilch::HandleOf<ZeroEngine::QuickHull3D> hullX = mHulls[iX].mQuickHull;
    Zilch::HandleOf<ZeroEngine::QuickHull3D> hullY = mHulls[iY].mQuickHull;
    mHulls[iX] = combinedHull;
    mHulls.EraseAt(iY);
  }
}

void VHacd::Build(Zilch::Array<QuickHull>& combinedHulls, Zilch::Array<Real>& volumes, Zilch::Array<Real>& combinedVolumes)
{
  volumes.Resize(mHulls.Size());

  for (size_t i = 0; i < mHulls.Size(); ++i)
  {
    volumes[i] = mHulls[i].ComputeVolume();
  }

  combinedVolumes.Resize(mHulls.Size() * mHulls.Size());
  combinedHulls.Resize(mHulls.Size() * mHulls.Size());

  for (size_t y = 0; y < mHulls.Size(); ++y)
  {
    for (size_t x = 0; x < mHulls.Size(); ++x)
    {
      int index = x + y * mHulls.Size();

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
  size_t size = volumes.Size();
  for (size_t y = 0; y < size; ++y)
  {
    for (size_t x = 0; x < size; ++x)
    {
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
