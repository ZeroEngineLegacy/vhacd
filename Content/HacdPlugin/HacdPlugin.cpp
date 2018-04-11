#include "HacdPluginPrecompiled.hpp"

#include <vector>

//***************************************************************************
ZilchDefineType(HacdPluginEvent, builder, type)
{
  // This is required for event binding
  ZilchBindDestructor();
  ZilchBindConstructor();

  ZilchBindFieldProperty(mMesh);
  ZilchBindFieldProperty(mPhysicsMesh);
}


//***************************************************************************
ZilchDefineType(HacdPlugin, builder, type)
{
  // This is required for component binding
  ZilchBindDestructor();
  ZilchBindConstructor();
  ZilchBindMethod(Initialize);
  
  // Note: All event connection methods must be bound
  ZilchBindMethod(OnLogicUpdate);
  ZilchBindMethod(OnCompute);
  ZilchBindFieldProperty(mConcavity);
  ZilchBindFieldProperty(mAlpha);
  ZilchBindFieldProperty(mBeta);
  ZilchBindFieldProperty(mResolution);
  ZilchBindFieldProperty(mMode);
  ZilchBindFieldProperty(mConvexHullApproximation);
  ZilchBindFieldProperty(mConvexHullDownSamping);
  ZilchBindFieldProperty(mPlaneDownSampling);
  ZilchBindFieldProperty(mMinVolumePerCH);
  ZilchBindFieldProperty(mMaxVerticesPerCH);
  ZilchBindFieldProperty(mMaxConvexHulls);
  ZilchBindFieldProperty(mProjectHullVertices);
}

//***************************************************************************
HacdPlugin::HacdPlugin()
{
  VHACD::IVHACD::Parameters params;

  mConcavity = params.m_concavity;
  mAlpha = params.m_alpha;
  mBeta = params.m_beta;
  mMode = params.m_mode;
  mResolution = params.m_resolution;
  mConvexHullApproximation = (bool)params.m_convexhullApproximation;
  mConvexHullDownSamping = params.m_convexhullDownsampling;
  mPlaneDownSampling = params.m_planeDownsampling;
  mMinVolumePerCH = params.m_minVolumePerCH;
  mMaxVerticesPerCH = params.m_maxNumVerticesPerCH;
  mMaxConvexHulls = params.m_maxConvexHulls;
  mProjectHullVertices = params.m_projectHullVertices;

  Zilch::Console::WriteLine("HacdPlugin::HacdPlugin (Constructor)");
}

//***************************************************************************
HacdPlugin::~HacdPlugin()
{
  Zilch::Console::WriteLine("HacdPlugin::~HacdPlugin (Destructor)");
  // Always check for null if you are intending
  // to destroy any cogs that you 'own'
}

//***************************************************************************
void HacdPlugin::Initialize(ZeroEngine::CogInitializer* initializer)
{
  Zilch::Console::WriteLine("HacdPlugin::Initialize");
  
  ZeroConnectThisTo(this->GetSpace(), "LogicUpdate", "OnLogicUpdate");
  ZeroConnectThisTo(this->GetOwner(), "Compute", "OnCompute");
}

//***************************************************************************
void HacdPlugin::OnLogicUpdate(ZeroEngine::UpdateEvent* event)
{
  // Do we have a Model component?
  //ZeroEngine::Model* model = this->GetOwner()->has(ZeroEngine::Model);
  //if (model != nullptr)
  //  Zilch::Console::WriteLine("We have a Model!");
  //
  //// Send our own update event
  //// We could also replace this with ZilchEvent to send basic events
  //// Note: ZilchAllocate should be used for any type that is
  //// typically allocated within Zilch, such as a CastFilter
  //Zilch::HandleOf<HacdPluginEvent> toSend = ZilchAllocate(HacdPluginEvent);
  //this->GetOwner()->DispatchEvent("HacdPluginUpdate", toSend);
}

void HacdPlugin::OnCompute(HacdPluginEvent* event)
{
  Zilch::Console::WriteLine("Compute");

  MultiConvexMesh* physicsMesh = event->mPhysicsMesh;
  std::vector<float> vertices;
  std::vector<uint32_t> indices;
  GraphicsMeshToMeshData(event->mMesh, vertices, indices);



  VHACD::IVHACD* hacd = VHACD::CreateVHACD();
  VHACD::IVHACD::Parameters params;

  params.m_mode = mMode;
  params.m_concavity = mConcavity;
  params.m_alpha = mAlpha;
  params.m_beta = mBeta;
  params.m_resolution = mResolution;
  params.m_convexhullApproximation = mConvexHullApproximation;
  params.m_convexhullDownsampling = mConvexHullDownSamping;
  params.m_planeDownsampling = mPlaneDownSampling;
  params.m_minVolumePerCH = mMinVolumePerCH;
  params.m_maxNumVerticesPerCH = mMaxVerticesPerCH;
  params.m_maxConvexHulls = mMaxConvexHulls;
  params.m_projectHullVertices = mProjectHullVertices;

  int vertexCount = vertices.size() / 3;
  int triangleCount = indices.size() / 3;
  hacd->Compute(vertices.data(), vertexCount, indices.data(), triangleCount, params);

  HacdToPhysicsMesh(hacd, event->mPhysicsMesh);
}

void HacdPlugin::GraphicsMeshToMeshData(Mesh* mesh, std::vector<float>& vertices, std::vector<uint32_t>& indices)
{
  Zilch::HandleOf<ZeroEngine::VertexBuffer> verticesHandle = mesh->GetVertices();
  ZeroEngine::VertexBuffer* VertexBuffer = verticesHandle;
  int vertexCount = VertexBuffer->GetVertexCount();
  for (int i = 0; i < vertexCount; ++i)
  {
    Real4 pos = VertexBuffer->GetVertexData(i, ZeroEngine::VertexSemantic::GetPosition());
    vertices.push_back(pos.x);
    vertices.push_back(pos.y);
    vertices.push_back(pos.z);
  }

  Zilch::HandleOf<ZeroEngine::IndexBuffer> indicesHandle = mesh->GetIndices();
  ZeroEngine::IndexBuffer* indexBuffer = indicesHandle;
  int indexCount = indexBuffer->GetCount();
  for (int i = 0; i < indexCount; ++i)
  {
    int index = indexBuffer->Get(i);
    indices.push_back(index);
  }
}

void HacdPlugin::HacdToPhysicsMesh(VHACD::IVHACD* hacd, MultiConvexMesh* physicsMesh)
{
  Zilch::HandleOf<ZeroEngine::MultiConvexMeshSubMeshData> subMeshesHandle = physicsMesh->GetSubMeshes();
  ZeroEngine::MultiConvexMeshSubMeshData* subMeshes = subMeshesHandle;
  subMeshes->Clear();
  auto physicsVertices = physicsMesh->GetVertices();
  physicsVertices->Clear();

  int startOffset = 0;
  for (size_t i = 0; i < hacd->GetNConvexHulls(); ++i)
  {
    VHACD::IVHACD::ConvexHull convexHull;
    hacd->GetConvexHull(i, convexHull);


    Zilch::HandleOf<ZeroEngine::SubConvexMesh> subMeshHandle = subMeshes->Add();
    ZeroEngine::SubConvexMesh* subMesh = subMeshHandle;


    for (size_t i = 0; i < convexHull.m_nPoints; ++i)
    {
      Real3 pos;
      pos.x = (Real)convexHull.m_points[i * 3 + 0];
      pos.y = (Real)convexHull.m_points[i * 3 + 1];
      pos.z = (Real)convexHull.m_points[i * 3 + 2];


      //Zilch::Console::WriteLine(pos.x, pos.y, pos.z);
      physicsVertices->Add(pos);
    }

    int totalCount = physicsVertices->GetCount();
    auto physicsIndices = subMesh->GetTriangleIndices();
    for (size_t i = 0; i < convexHull.m_nTriangles; ++i)
    {
      int i0 = convexHull.m_triangles[i * 3 + 0] + startOffset;
      int i1 = convexHull.m_triangles[i * 3 + 1] + startOffset;
      int i2 = convexHull.m_triangles[i * 3 + 2] + startOffset;

      //Zilch::Console::WriteLine(i0, i1, i2);
      physicsIndices->Add(i0);
      physicsIndices->Add(i1);
      physicsIndices->Add(i2);
      //if(i > 3)
      //break;
    }
    startOffset += convexHull.m_nPoints;
  }
}