#pragma once

// For more information on binding and using Zilch APIs, visit: http://zilch.digipen.edu/
// For auto binding specifically, visit: http://zilch.digipen.edu/home/AutomaticBinding.html

// An example of a custom event that we can send
class HacdPluginEvent : public ZeroEngine::ZilchEvent
{
public:
  ZilchDeclareType(Zilch::TypeCopyMode::ReferenceType);

  Zilch::HandleOf<Mesh> mMesh;
  Zilch::HandleOf<MultiConvexMesh> mPhysicsMesh;
};


// An example component being bound to the engine
class HacdPlugin : public ZeroEngine::ZilchComponent
{
public:
  ZilchDeclareType(Zilch::TypeCopyMode::ReferenceType);
  
  HacdPlugin();
  ~HacdPlugin();
  
  void Initialize(ZeroEngine::CogInitializer* initializer);
  
  void OnLogicUpdate(ZeroEngine::UpdateEvent* event);
  void OnCompute(HacdPluginEvent* event);

  void GraphicsMeshToMeshData(Mesh* mesh, std::vector<float>& vertices, std::vector<uint32_t>& indices);
  void HacdToPhysicsMesh(VHACD::IVHACD* hacd, MultiConvexMesh* physicsMesh);
  
private:

  float mConcavity;
  int mMode;
  float mAlpha;
  float mBeta;
  int mResolution;
  bool mConvexHullApproximation;
  int mConvexHullDownSamping;
  int mPlaneDownSampling;
  float mMinVolumePerCH;
  float mMaxVerticesPerCH;
  int mMaxConvexHulls;
  bool mProjectHullVertices;
};

