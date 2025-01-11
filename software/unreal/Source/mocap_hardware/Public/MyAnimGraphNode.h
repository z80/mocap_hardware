
// MyAnimGraphNode.h
#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_Base.h"
#include "MyAnimNode.h"
#include "MyAnimGraphNode.generated.h"

UCLASS()
class MOCAP_HARDWARE_API UMyAnimGraphNode : public UAnimGraphNode_Base
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Settings")
    FMyAnimNode Node;

    // Override the GetNodeTitle function to provide a title for the node
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override
    {
        return FText::FromString(TEXT("My Mocap Animation Node"));
    }

protected:
    virtual void CreateOutputPins() override;
};

