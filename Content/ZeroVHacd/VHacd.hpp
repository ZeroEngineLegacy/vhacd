#pragma once

#include "TriangleMesh.hpp"
#include "Voxelizer.hpp"
#include "QuickHull.hpp"

struct SplitData
{
  Real mFrontHullVolume;
  Real mFrontVoxelVolume;
  Real mBackHullVolume;
  Real mBackVoxelVolume;

  Real mSplitValue;
  int mFrontIndex;
};

class VHacd
{
public:
  VHacd();

  typedef void(*CallbackFn)(float totalPercent, const String& stepName, float stepPercent, const String& stepMessage, void* clientData);
  void SetProgressCallback(CallbackFn callbackFn, void* clientData);

  void Compute(Real fidelity, int recursions, TriangleMesh& mesh);
  void Clear();


  //--Internal
  struct SplitPlane
  {
    SplitPlane() {}
    SplitPlane(int axis, Real axisValue)
    {
      mAxis = axis;
      mAxisValue = axisValue;
    }
    Real mAxisValue;
    int mAxis;
    int mAxisDiscretizedValue;
  };

  void ComputeSubDivisions(Aabb& aabb);
  void Initialize(TriangleMesh& mesh);
  void ComputeApproximateConvexDecomposition();
  void Recurse(int depth);

  bool SplitVoxelizer(Voxelizer& voxelizer, Array<Voxelizer>& newVoxelizers, int depth);
  void ComputePossibleSplitPlanes(Voxelizer& voxelizer, Array<SplitPlane>& planes);
  bool FindBestSplitPlane(Voxelizer& voxelizer, Array<SplitPlane>& planes, Real parentConcavity, SplitPlane& result);
  bool FindBestSplitPlaneNew(Voxelizer& voxelizer, Array<SplitPlane>& planes, Real parentConcavity, SplitPlane& result);
  void GetSplitVolumes(Voxelizer& voxelizer, int axis, Array<SplitData>& splitData);
  float TestSplit(Voxelizer& voxelizer, int axis, Real axisValue, Real parentConcavity);
  float ComputeScore(SplitData& splitData, int axis, Voxelizer& voxelizer, Real parentConcavity);

  void MergeHulls();
  void BuildHullTable(Zilch::Array<Real>& volumes, Zilch::Array<Real>& combinedVolumes);
  void FindHullsToMerge(Zilch::Array<Real>& volumes, Zilch::Array<Real>& combinedVolumes, size_t& iX, size_t& iY);

  void Resample();
  void Resample(QuickHull& hull, int hullIndex, int totalHulls);

  void UpdateProgress(float totalPercent, const String& stepName, float stepPercent, const String& stepMessage);

  Zero::Array<Voxelizer> mVoxelizers;

  Zero::Array<Voxelizer> mFinalVoxelizers;
  Zero::Array<QuickHull> mHulls;

  Real mFidelity;
  Real mRefinement;
  int mRefinementStep;
  Integer3 mSubDivisions;
  int mMaxRecusionDepth;
  int mMaxHulls;
  bool mResampleMesh;
  Real mAllowedConcavityVolumeError;
  Real mAllowedVolumeSurfaceAreaRatio;

  Real mInitialConvexHullVolume;
  Real mBalanceWeight;
  Real mSymmetryWeight;

  TriangleMesh mMesh;
  CallbackFn mCallbackFn;
  void* mClientData;
  float mTotalPercent;
  float mStepPercent;
  bool mForceStop;
  bool mFast;
};
