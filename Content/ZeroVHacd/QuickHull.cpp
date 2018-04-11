#include "ZeroVHacdPrecompiled.hpp"

#include "QuickHull.hpp"
#include "Voxelizer.hpp"

bool QuickHull::Build(Voxelizer& voxelizer)
{
  mQuickHull = ZilchAllocate(ZeroEngine::QuickHull3D);
  ZeroEngine::QuickHull3D* quickHull = mQuickHull;

  Real3 size = voxelizer.mAabb.GetHalfSize() * 2;
  Real3 delta = size / ToReal3(voxelizer.mSubDivisions);
  Real3 start = voxelizer.mAabb.mMin;
  Real3 halfOffset = Real3(0.5) * delta;// / this.SubDivisions;
  Real3 offset = halfOffset;

  for (int z = 0; z < voxelizer.mSubDivisions.z; ++z)
  {
    offset.z = delta.z * z + halfOffset.z;
    for (int y = 0; y < voxelizer.mSubDivisions.y; ++y)
    {
      offset.y = delta.y * y + halfOffset.y;
      for (int x = 0; x < voxelizer.mSubDivisions.x; ++x)
      {
        //offset.X = (x + halfOffset.X) / this.SubDivisions.X;
        offset.x = delta.x * x + halfOffset.x;
        //offset.X += delta.X;

        Real3 voxelCenter = start + offset;

        //var voxelAabb = Aabb(start + offset, halfOffset * 0.25);
        Integer3 voxelCoord = Integer3(x, y, z);
        VoxelState::Enum voxel = voxelizer.GetVoxel(voxelCoord);
        //var shape = DebugObb(voxelAabb);
        //var alpha = 0.1;
        //shape.Filled = true;
        if (voxel == VoxelState::Surface)
        {
          Real3 min = voxelCenter - voxelizer.mVoxelSize * 0.5;
          Real3 max = voxelCenter + voxelizer.mVoxelSize * 0.5;
          quickHull->Add(Real3(min.x, min.y, min.z));
          quickHull->Add(Real3(min.x, min.y, max.z));
          quickHull->Add(Real3(min.x, max.y, min.z));
          quickHull->Add(Real3(min.x, max.y, max.z));
          quickHull->Add(Real3(max.x, min.y, min.z));
          quickHull->Add(Real3(max.x, min.y, max.z));
          quickHull->Add(Real3(max.x, max.y, min.z));
          quickHull->Add(Real3(max.x, max.y, max.z));
          //quickHull->Add(voxelCenter);
        }
      }
    }
  }

  bool result = quickHull->Build();
  return result;
}

bool QuickHull::Build(Zilch::Array<Real3>& vertices)
{
  mQuickHull = ZilchAllocate(ZeroEngine::QuickHull3D);
  ZeroEngine::QuickHull3D* quickHull = mQuickHull;

  for (size_t i = 0; i < vertices.Size(); ++i)
  {
    quickHull->Add(vertices[i]);
  }
  return quickHull->Build();
}

bool QuickHull::Build(QuickHull& hull0, QuickHull& hull1)
{
  return Build(hull0.mQuickHull, hull1.mQuickHull);
}

bool QuickHull::Build(ZeroEngine::QuickHull3D* hull0, ZeroEngine::QuickHull3D* hull1)
{
  mQuickHull = ZilchAllocate(ZeroEngine::QuickHull3D);
  ZeroEngine::QuickHull3D* quickHull = mQuickHull;

  AddPoints(hull0);
  AddPoints(hull1);
  return quickHull->Build();
}

void QuickHull::AddPoints(ZeroEngine::QuickHull3D* hull)
{
  Zilch::HandleOf<ZeroEngine::IndexedHalfEdgeMesh> meshHandle = hull->GetMesh();
  ZeroEngine::IndexedHalfEdgeMesh* mesh = meshHandle;
  Zilch::HandleOf<ZeroEngine::IndexedHalfEdgeMeshVertexArray> vertices = mesh->GetVertices();

  size_t count = vertices->GetCount();
  for (size_t i = 0; i < count; ++i)
    mQuickHull->Add(vertices->Get(i));
}

Real QuickHull::ComputeVolume()
{
  Real hullVolume = 0.0;
  Zilch::HandleOf<ZeroEngine::IndexedHalfEdgeMesh> meshHandle = mQuickHull->GetMesh();
  ZeroEngine::IndexedHalfEdgeMesh* mesh = meshHandle;

  Zilch::HandleOf<ZeroEngine::IndexedHalfEdgeMeshFaceArrayRange> faces = mesh->GetFaces()->GetAll();
  for (; faces->GetIsNotEmpty(); faces->MoveNext())
  {
    Zilch::HandleOf<ZeroEngine::IndexedHalfEdgeFace> faceHandle = faces->GetCurrent();
    Zilch::HandleOf<ZeroEngine::IndexedHalfEdgeFaceEdgeIndexArray> faceEdges = faceHandle->GetEdges();
    int edgeCount = faceEdges->GetCount();
    for (int i = 2; i < edgeCount; ++i)
    {
      auto e0 = mesh->GetEdges()->Get(faceEdges->Get(0));
      auto e1 = mesh->GetEdges()->Get(faceEdges->Get(i - 1));
      auto e2 = mesh->GetEdges()->Get(faceEdges->Get(i));

      Real3 p0 = mesh->GetVertices()->Get(e0->GetVertexIndex());
      Real3 p1 = mesh->GetVertices()->Get(e1->GetVertexIndex());
      Real3 p2 = mesh->GetVertices()->Get(e2->GetVertexIndex());

      Math::Mat3 m(p0.x, p0.y, p0.z, p1.x, p1.y, p1.z, p2.x, p2.y, p2.z);
      Real subVolume = m.Determinant() / 6.0f;
      hullVolume += subVolume;
    }
  }
  return hullVolume;
}
