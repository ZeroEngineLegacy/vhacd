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
  Real3 halfOffset = Real3(0.5) * delta;
  Real3 offset = halfOffset;

  for (int z = 0; z < voxelizer.mSubDivisions.z; ++z)
  {
    offset.z = delta.z * z + halfOffset.z;
    for (int y = 0; y < voxelizer.mSubDivisions.y; ++y)
    {
      offset.y = delta.y * y + halfOffset.y;
      for (int x = 0; x < voxelizer.mSubDivisions.x; ++x)
      {
        offset.x = delta.x * x + halfOffset.x;

        Real3 voxelCenter = start + offset;

        Integer3 voxelCoord = Integer3(x, y, z);
        VoxelState::Enum voxel = voxelizer.GetVoxel(voxelCoord);

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
        }
      }
    }
  }

  bool success = quickHull->Build();

  if (success)
    BakeHull();
  return success;
}

bool QuickHull::Build(Zilch::Array<Real3>& vertices)
{
  mQuickHull = ZilchAllocate(ZeroEngine::QuickHull3D);
  ZeroEngine::QuickHull3D* quickHull = mQuickHull;

  for (size_t i = 0; i < vertices.Size(); ++i)
  {
    quickHull->Add(vertices[i]);
  }
  bool success = quickHull->Build();

  if (success)
    BakeHull();
  return success;
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
  bool success = quickHull->Build();

  if (success)
    BakeHull();
  return success;
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
  //Real hullVolume = 0.0;
  //Zilch::HandleOf<ZeroEngine::IndexedHalfEdgeMesh> meshHandle = mQuickHull->GetMesh();
  //ZeroEngine::IndexedHalfEdgeMesh* mesh = meshHandle;
  //
  //Zilch::HandleOf<ZeroEngine::IndexedHalfEdgeMeshFaceArrayRange> faces = mesh->GetFaces()->GetAll();
  //for (; faces->GetIsNotEmpty(); faces->MoveNext())
  //{
  //  Zilch::HandleOf<ZeroEngine::IndexedHalfEdgeFace> faceHandle = faces->GetCurrent();
  //  Zilch::HandleOf<ZeroEngine::IndexedHalfEdgeFaceEdgeIndexArray> faceEdges = faceHandle->GetEdges();
  //  int edgeCount = faceEdges->GetCount();
  //  for (int i = 2; i < edgeCount; ++i)
  //  {
  //    auto eI0 = faceEdges->Get(0);
  //    auto eI1 = faceEdges->Get(i - 1);
  //    auto eI2 = faceEdges->Get(i);
  //
  //    auto e0 = mesh->GetEdges()->Get(eI0);
  //    auto e1 = mesh->GetEdges()->Get(eI1);
  //    auto e2 = mesh->GetEdges()->Get(eI2);
  //
  //    Real3 p0 = mesh->GetVertices()->Get(e0->GetVertexIndex());
  //    Real3 p1 = mesh->GetVertices()->Get(e1->GetVertexIndex());
  //    Real3 p2 = mesh->GetVertices()->Get(e2->GetVertexIndex());
  //
  //    Math::Mat3 m(p0.x, p0.y, p0.z, p1.x, p1.y, p1.z, p2.x, p2.y, p2.z);
  //    Real subVolume = m.Determinant() / 6.0f;
  //    hullVolume += subVolume;
  //  }
  //}
  //return hullVolume;

  Real hullVolume = 0.0;
  
  for (size_t fI = 0; fI < mFaces.Size(); ++fI)
  {
    Face& face = mFaces[fI];
    
    int edgeCount = face.mEdges.Size();
    for (int i = 2; i < edgeCount; ++i)
    {
      size_t eI0 = face.mEdges[0];
      size_t eI1 = face.mEdges[i - 1];
      size_t eI2 = face.mEdges[i];
  
      Edge& e0 = mEdges[eI0];
      Edge& e1 = mEdges[eI1];
      Edge& e2 = mEdges[eI2];
      
      Real3 p0 = mVertices[e0.mVertexIndex];
      Real3 p1 = mVertices[e1.mVertexIndex];
      Real3 p2 = mVertices[e2.mVertexIndex];
  
      Math::Mat3 m(p0.x, p0.y, p0.z, p1.x, p1.y, p1.z, p2.x, p2.y, p2.z);
      Real subVolume = m.Determinant() / 6.0f;
      hullVolume += subVolume;
    }
  }
  return hullVolume;
}

void QuickHull::BakeHull()
{
  Zilch::HandleOf<ZeroEngine::IndexedHalfEdgeMesh> meshHandle = mQuickHull->GetMesh();
  ZeroEngine::IndexedHalfEdgeMesh* mesh = meshHandle;

  Zilch::HandleOf<ZeroEngine::IndexedHalfEdgeMeshVertexArray> verticesHandle = mesh->GetVertices();
  ZeroEngine::IndexedHalfEdgeMeshVertexArray* vertices = verticesHandle;
  size_t vertexCount = vertices->GetCount();
  mVertices.Resize(vertexCount);
  for (size_t i = 0; i < vertexCount; ++i)
  {
    mVertices[i] = vertices->Get(i);
  }

  Zilch::HandleOf<ZeroEngine::IndexedHalfEdgeMeshEdgeArray> edgesHandle = mesh->GetEdges();
  ZeroEngine::IndexedHalfEdgeMeshEdgeArray* edges = edgesHandle;
  size_t edgeCount = edges->GetCount();
  mEdges.Resize(edgeCount);
  for (size_t i = 0; i < edgeCount; ++i)
  {
    Zilch::HandleOf<ZeroEngine::IndexedHalfEdge> edgeHandle = edges->Get(i);
    ZeroEngine::IndexedHalfEdge* edge = edgeHandle;

    Edge& bakedEdge = mEdges[i];
    bakedEdge.mFaceIndex = edge->GetFaceIndex();
    bakedEdge.mTwinIndex = edge->GetTwinIndex();
    bakedEdge.mVertexIndex = edge->GetVertexIndex();
  }

  Zilch::HandleOf<ZeroEngine::IndexedHalfEdgeMeshFaceArray> facesHandle = mesh->GetFaces();
  ZeroEngine::IndexedHalfEdgeMeshFaceArray* faces = facesHandle;
  size_t faceCount = faces->GetCount();
  mFaces.Resize(faceCount);
  for (size_t i = 0; i < faceCount; ++i)
  {
    Zilch::HandleOf<ZeroEngine::IndexedHalfEdgeFace> faceHandle = faces->Get(i);
    ZeroEngine::IndexedHalfEdgeFace* face = faceHandle;

    Zilch::HandleOf<ZeroEngine::IndexedHalfEdgeFaceEdgeIndexArray> faceEdgesHandle = face->GetEdges();
    ZeroEngine::IndexedHalfEdgeFaceEdgeIndexArray* faceEdges = faceEdgesHandle;

    size_t faceEdgesCount = faceEdges->GetCount();
    Face& bakedFace = mFaces[i];
    bakedFace.mEdges.Resize(faceEdgesCount);
    for (size_t eI = 0; eI < faceEdgesCount; ++eI)
    {
      bakedFace.mEdges[eI] = faceEdges->Get(eI);
    }
  }
}
