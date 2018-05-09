#include "ZeroVHacdPrecompiled.hpp"

#include "QuickHull.hpp"
#include "Voxelizer.hpp"

bool QuickHull::Build(Voxelizer& voxelizer)
{
  Real3 size = voxelizer.mAabb.GetHalfSize() * 2;
  Real3 delta = size / ToReal3(voxelizer.mSubDivisions);
  Real3 start = voxelizer.mAabb.mMin;
  Real3 halfOffset = Real3(0.5) * delta;
  Real3 offset = halfOffset;

  Array<Real3> points;

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
          Real3 min = voxelCenter - voxelizer.mVoxelSize * 0.5f;
          Real3 max = voxelCenter + voxelizer.mVoxelSize * 0.5f;
          points.PushBack(Real3(min.x, min.y, min.z));
          points.PushBack(Real3(min.x, min.y, max.z));
          points.PushBack(Real3(min.x, max.y, min.z));
          points.PushBack(Real3(min.x, max.y, max.z));
          points.PushBack(Real3(max.x, min.y, min.z));
          points.PushBack(Real3(max.x, min.y, max.z));
          points.PushBack(Real3(max.x, max.y, min.z));
          points.PushBack(Real3(max.x, max.y, max.z));
        }
      }
    }
  }

  return Build(points);
}

bool QuickHull::Build(QuickHull& hull0, QuickHull& hull1)
{
  Clear();
  Array<Real3> vertices;
  vertices = hull0.mVertices;
  //vertices.Insert(vertices.End(), hull1.mVertices.Begin, hull1.mVertices.End());
  vertices.Append(hull1.mVertices.All());

  return Build(vertices);
}

bool QuickHull::Build(Zilch::Array<Real3>& vertices)
{
  Clear();
  Zero::IncrementalQuickHull3D quickHull;
  bool success = quickHull.Build(vertices);

  if (success)
    BakeHull(quickHull);
  return success;
}

void QuickHull::Clear()
{
  mVertices.Clear();
  mEdges.Clear();
  mFaces.Clear();
}

Real QuickHull::ComputeVolume()
{
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

void QuickHull::BakeHull(Zero::IncrementalQuickHull3D& quickHull)
{
  typedef Zero::QuickHull3D::QuickHullVertex QuickHullVertex;
  typedef Zero::QuickHull3D::QuickHullEdge QuickHullEdge;
  typedef Zero::QuickHull3D::QuickHullFace QuickHullFace;
  typedef Zero::QuickHull3D::VertexList VertexList;
  typedef Zero::QuickHull3D::EdgeList EdgeList;
  typedef Zero::QuickHull3D::FaceList FaceList;

  // Compute how much memory is required
  int vertexCount = quickHull.ComputeVertexCount();
  int halfEdgeCount = quickHull.ComputeHalfEdgeCount();
  int faceCount = quickHull.ComputeFaceCount();

  // Allocate all the memory we need
  mVertices.Clear();
  mEdges.Clear();
  mFaces.Clear();
  mVertices.Resize(vertexCount);
  mEdges.Resize(halfEdgeCount);
  mFaces.Resize(faceCount);

  // Build up point to id mappings. Use an ordered hash-map for determinism.
  typedef Zero::OrderedHashMap<QuickHullVertex*, int> VertexMap;
  typedef Zero::OrderedHashMap<QuickHullEdge*, int> EdgeMap;
  typedef Zero::OrderedHashMap<QuickHullFace*, int> FaceMap;
  VertexMap mVertexIds;
  EdgeMap mEdgeIds;
  FaceMap mFaceIds;
  int vertexId = 0;
  int edgeId = 0;
  int faceId = 0;

  // Compute the ids of all faces, edges, and vertices
  for (FaceList::range faces = quickHull.GetFaces(); !faces.Empty(); faces.PopFront())
  {
    QuickHullFace* face = &faces.Front();
    mFaceIds[face] = faceId;
    ++faceId;

    for (EdgeList::range edges = face->mEdges.All(); !edges.Empty(); edges.PopFront())
    {
      QuickHullEdge* edge = &edges.Front();
      mEdgeIds[edge] = edgeId;
      ++edgeId;

      // Check to see if this vertex has already been mapped
      if (!mVertexIds.ContainsKey(edge->mTail))
      {
        mVertexIds[edge->mTail] = vertexId;
        ++vertexId;
      }
    }
  }

  // Copy all vertices
  forRange(VertexMap::PairType& pair, mVertexIds.All())
  {
    mVertices[pair.second] = pair.first->mPosition;
  }
  // Copy all edges, making sure to map the vertex, twin, and faces
  forRange(EdgeMap::PairType& pair, mEdgeIds.All())
  {
    Edge* outEdge = &mEdges[pair.second];
    QuickHullEdge* inEdge = pair.first;
    outEdge->mFaceIndex = mFaceIds[inEdge->mFace];
    outEdge->mVertexIndex = mVertexIds[inEdge->mTail];
    outEdge->mTwinIndex = mEdgeIds[inEdge->mTwin];
  }
  // Copy all faces, making sure to map all edges
  forRange(auto& pair, mFaceIds.All())
  {
    Face* outFace = &mFaces[pair.second];
    QuickHullFace* inFace = pair.first;

    for (EdgeList::range edges = inFace->mEdges.All(); !edges.Empty(); edges.PopFront())
    {
      QuickHullEdge* edge = &edges.Front();
      outFace->mEdges.PushBack(mEdgeIds[edge]);
    }
  }
}

Zilch::HandleOf<ZeroEngine::QuickHull3D> QuickHull::ToHandle()
{
  Zilch::HandleOf<ZeroEngine::QuickHull3D> handle = ZilchAllocate(ZeroEngine::QuickHull3D);
  for (size_t i = 0; i < mVertices.Size(); ++i)
  {
    handle->Add(mVertices[i]);
  }
  handle->Build();
  return handle;
}
