#include "ZeroVHacdPrecompiled.hpp"
#include "..\HacdPlugin\src\VHACD.h"

ZilchDefineType(VHacd_Orig, builder, type)
{
  // This is required for component binding
  ZilchBindDestructor();
  ZilchBindConstructor();
  ZilchBindMethod(Initialize);

  // Note: All event connection methods must be bound
  ZilchBindMethod(Compute);
  ZilchBindMethod(Cancel);
  ZilchBindMethod(Clear);
  ZilchBindMethod(GetHullCount);
  ZilchBindMethod(GetHull);

  ZilchBindMethod(OnJobFinished);

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

VHacd_Orig::VHacd_Orig()
{
  mTask = nullptr;
  VHACD::IVHACD::Parameters params;

  mConcavity = (float)params.m_concavity;
  mAlpha = (float)params.m_alpha;
  mBeta = (float)params.m_beta;
  mMode = params.m_mode;
  mResolution = params.m_resolution;
  mConvexHullApproximation = (bool)params.m_convexhullApproximation;
  mConvexHullDownSamping = params.m_convexhullDownsampling;
  mPlaneDownSampling = params.m_planeDownsampling;
  mMinVolumePerCH = (float)params.m_minVolumePerCH;
  mMaxVerticesPerCH = params.m_maxNumVerticesPerCH;
  mMaxConvexHulls = params.m_maxConvexHulls;
  mProjectHullVertices = params.m_projectHullVertices;
}

VHacd_Orig::~VHacd_Orig()
{
}

void VHacd_Orig::Initialize(ZeroEngine::CogInitializer* initializer)
{
  ZeroConnectThisTo(this->GetOwner(), "JobFinished", "OnJobFinished");
}

void VHacd_Orig::Compute(Zilch::HandleOf<Mesh>& meshHandle)
{
  mTask = new VHacd_OrigTask();
  mTask->mTaskOwner = this;
  mTask->mOwner = GetOwner();

  GraphicsMeshToMeshData(meshHandle, mTask->mVertices, mTask->mIndices);

  JobSystem::GetInstance()->AddJob(mTask);
}

void VHacd_Orig::OnJobFinished(PluginBackgroundTaskEvent* event)
{
  VHacd_OrigTask* task = (VHacd_OrigTask*)event->mTask;
  VHACD::IVHACD* vhacd = task->mVHacd;

  int startOffset = 0;
  for (size_t i = 0; i < vhacd->GetNConvexHulls(); ++i)
  {
    VHACD::IVHACD::ConvexHull convexHull;
    vhacd->GetConvexHull(i, convexHull);

    Zilch::HandleOf<ZeroEngine::QuickHull3D> handle = ZilchAllocate(ZeroEngine::QuickHull3D);
    mHulls.PushBack(handle);

    for (size_t i = 0; i < convexHull.m_nPoints; ++i)
    {
      Real3 pos;
      pos.x = (Real)convexHull.m_points[i * 3 + 0];
      pos.y = (Real)convexHull.m_points[i * 3 + 1];
      pos.z = (Real)convexHull.m_points[i * 3 + 2];

      handle->Add(pos);
    }
    handle->Build();
  }

  Zilch::HandleOf<ZeroEngine::ZilchEvent> toSend = ZilchAllocate(ZeroEngine::ZilchEvent);
  GetOwner()->DispatchEvent("Finished", toSend);
  mTask = nullptr;
}

void VHacd_Orig::GraphicsMeshToMeshData(Mesh* mesh, Array<float>& vertices, Array<uint32_t>& indices)
{
  Zilch::HandleOf<ZeroEngine::VertexBuffer> verticesHandle = mesh->GetVertices();
  ZeroEngine::VertexBuffer* VertexBuffer = verticesHandle;
  int vertexCount = VertexBuffer->GetVertexCount();
  for (int i = 0; i < vertexCount; ++i)
  {
    Real4 pos = VertexBuffer->GetVertexData(i, ZeroEngine::VertexSemantic::GetPosition());
    vertices.PushBack(pos.x);
    vertices.PushBack(pos.y);
    vertices.PushBack(pos.z);
  }

  Zilch::HandleOf<ZeroEngine::IndexBuffer> indicesHandle = mesh->GetIndices();
  ZeroEngine::IndexBuffer* indexBuffer = indicesHandle;
  int indexCount = indexBuffer->GetCount();
  for (int i = 0; i < indexCount; ++i)
  {
    int index = indexBuffer->Get(i);
    indices.PushBack(index);
  }
}

void VHacd_Orig::Cancel()
{
  if (mTask != nullptr)
  {
    mTask->Cancel();
    mTask = nullptr;
  }
}

void VHacd_Orig::Clear()
{
  for (size_t i = 0; i < mHulls.Size(); ++i)
  {
    mHulls[i]->Clear();
  }
  mHulls.Clear();
}

int VHacd_Orig::GetHullCount()
{
  return mHulls.Size();
}

Zilch::HandleOf<ZeroEngine::QuickHull3D> VHacd_Orig::GetHull(int index)
{
  return mHulls[index];
}


class VHacdCallback : public VHACD::IVHACD::IUserCallback
{
public:
  VHacdCallback()
  {
    mTask = nullptr;
  }

  void Update(const double overallProgress,
    const double stageProgress,
    const double operationProgress,
    const char* const stage,
    const char* const operation) override
  {
    float totalPercentage = (float)overallProgress / 100.0f;
    float stepPercentage = (float)stageProgress / 100.0f;
    mTask->UpdateProgress(totalPercentage, stage, stepPercentage, operation);
  }

  VHacd_OrigTask* mTask;
};

VHacd_OrigTask::VHacd_OrigTask()
{
  mVHacd = VHACD::CreateVHACD();
}

void VHacd_OrigTask::Run()
{
  VHACD::IVHACD::Parameters params;

  VHacdCallback* callback = new VHacdCallback();
  callback->mTask = this;
  params.m_callback = callback;

  params.m_mode = mTaskOwner->mMode;
  params.m_concavity = mTaskOwner->mConcavity;
  params.m_alpha = mTaskOwner->mAlpha;
  params.m_beta = mTaskOwner->mBeta;
  params.m_resolution = mTaskOwner->mResolution;
  params.m_convexhullApproximation = mTaskOwner->mConvexHullApproximation;
  params.m_convexhullDownsampling = mTaskOwner->mConvexHullDownSamping;
  params.m_planeDownsampling = mTaskOwner->mPlaneDownSampling;
  params.m_minVolumePerCH = mTaskOwner->mMinVolumePerCH;
  params.m_maxNumVerticesPerCH = mTaskOwner->mMaxVerticesPerCH;
  params.m_maxConvexHulls = mTaskOwner->mMaxConvexHulls;
  params.m_projectHullVertices = mTaskOwner->mProjectHullVertices;

  int vertexCount = mVertices.Size() / 3;
  int triangleCount = mIndices.Size() / 3;
  mVHacd->Compute(mVertices.Data(), vertexCount, mIndices.Data(), triangleCount, params);

  Finished();
}

void VHacd_OrigTask::Cancel()
{
  mVHacd->Cancel();
}

void VHacd_OrigTask::ProgressCallback(const String& message, float percentage, void* clientData)
{
}
