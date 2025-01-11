
#include "MyAnimGraphNode.h"
#include "AnimationGraphSchema.h"

void UMyAnimGraphNode::CreateOutputPins()
{
    const UAnimationGraphSchema* Schema = GetDefault<UAnimationGraphSchema>();

    // Create the default pose output pin
    CreatePin(EGPD_Output, Schema->PC_Struct, FPoseLink::StaticStruct(), TEXT("Pose"));

    // Create the additional text output pin
    CreatePin(EGPD_Output, Schema->PC_String, TEXT("Text Data"));
}

