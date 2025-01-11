
// MyAnimNode.cpp
#include "MyAnimNode.h"
#include "Animation/AnimInstanceProxy.h"
#include "ControlRigComponent.h"


void FMyAnimNode::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
    FAnimNode_SkeletalControlBase::Initialize_AnyThread(Context);
    //InputPose.Initialize(Context);
}

void FMyAnimNode::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
    FAnimNode_SkeletalControlBase::CacheBones_AnyThread(Context);
    //InputPose.CacheBones(Context);
}


void FMyAnimNode::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
    // Call the base class method to ensure the pose is evaluated correctly
    EvaluateComponentSpaceInternal(Output);

    const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();
    const TArray<FBoneIndexType>& RequiredBones = BoneContainer.GetBoneIndicesArray();

    for (FBoneIndexType BoneIndex : RequiredBones)
    {
        FMeshPoseBoneIndex MeshPoseBoneIndex(BoneIndex);
        FCompactPoseBoneIndex CompactPoseBoneIndex = BoneContainer.MakeCompactPoseIndex(MeshPoseBoneIndex);
        FTransform BoneTransform = Output.Pose.GetComponentSpaceTransform(CompactPoseBoneIndex);
        OutBoneTransforms.Add(FBoneTransform(CompactPoseBoneIndex, BoneTransform));
    }

    TextData = FString("Hello World!");
}

void FMyAnimNode::EvaluateComponentSpaceInternal(FComponentSpacePoseContext& Context)
{
    FAnimNode_SkeletalControlBase::EvaluateComponentSpaceInternal(Context);
}

bool FMyAnimNode::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
    return true;
}



