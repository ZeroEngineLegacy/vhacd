#pragma once

namespace VHACD
{
  class IVHACD;
}

class VHacd_OrigTask;

class VHacd_Orig : public ZeroEngine::ZilchComponent
{
public:
  ZilchDeclareType(Zilch::TypeCopyMode::ReferenceType);
  
  VHacd_Orig();
  ~VHacd_Orig();
  
  void Initialize(ZeroEngine::CogInitializer* initializer);
  void GraphicsMeshToMeshData(Mesh* mesh, Array<float>& vertices, Array<uint32_t>& indices);

  void Compute(Zilch::HandleOf<Mesh>& meshHandle);
  void Cancel();
  void OnJobFinished(PluginBackgroundTaskEvent* event);

  void Clear();

  int GetHullCount();
  Zilch::HandleOf<ZeroEngine::QuickHull3D> GetHull(int index);

  float mConcavity;
  int mMode;
  float mAlpha;
  float mBeta;
  int mResolution;
  bool mConvexHullApproximation;
  int mConvexHullDownSamping;
  int mPlaneDownSampling;
  float mMinVolumePerCH;
  int mMaxVerticesPerCH;
  int mMaxConvexHulls;
  bool mProjectHullVertices;

  VHacd_OrigTask* mTask;
  Zero::Array<Zilch::HandleOf<ZeroEngine::QuickHull3D> > mHulls;
};


class VHacd_OrigTask : public BackgroundTask
{
public:
  VHacd_OrigTask();
  void Run() override;
  void Cancel() override;

  static void ProgressCallback(const String& message, float percentage, void* clientData);

  VHacd_Orig* mTaskOwner;
  TriangleMesh mMesh;
  VHACD::IVHACD* mVHacd;

  Array<float> mVertices;
  Array<uint32_t> mIndices;
};

