
// MyAnimNode.h
#pragma once

#include "CoreMinimal.h"
//#include "Animation/AnimNodeBase.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "Components/ActorComponent.h"
#include "ControlRig.h"

#include "MyAnimNode.generated.h"

USTRUCT(BlueprintType)
struct MOCAP_HARDWARE_API FMyAnimNode : public FAnimNode_SkeletalControlBase
{
    GENERATED_BODY()

    // Add any properties you need here
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Output")
    FString TextData;

    virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
    virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)  override;

    virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
    virtual void EvaluateComponentSpaceInternal(FComponentSpacePoseContext& Context) override;

    virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
};


