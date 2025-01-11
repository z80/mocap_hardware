// Fill out your copyright notice in the Description page of Project Settings.


#include "MocapComponent.h"

// To be able to use UE_LOG.
#include "Engine/GameEngine.h"
#include "Rigs/RigHierarchyContainer.h"
#include "Animation/AnimInstanceProxy.h"

//const float UMocapComponent::CharacterIdRetryTimeInterval = 2.0;

// Convert transformation ref. frame from mocap sensors right reference frame to Unreal Engine left reference frame.
static FMatrix MocapToUnreal(const FQuat& Q);

static void ConvertLocalToWorldTransforms(const FPoseSnapshot& PoseSnapshot, USkeletalMeshComponent* SkeletalMeshComponent, TArray<FMocapBoneData>& BoneData);
static bool GetGroundDistances(AActor* Actor, TArray<FMocapBoneData>& BoneData, float & HeightOverGround);
static FVector GetDisplacement(TArray<FMocapBoneData>& BoneData, float alpha);

// Sets default values for this component's properties
UMocapComponent::UMocapComponent()
{
	V_batt = 0.0;
}

UMocapComponent::~UMocapComponent()
{

}


// Called when the game starts
void UMocapComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	//RetryTimeLeft = CharacterIdRetryTimeInterval;
	UMocapStreamer * MocapStreamer = UMocapStreamer::GetMocapStreamer();
	Handle_DataEvent = MocapStreamer->Data_Event.AddUObject( this, &UMocapComponent::OnRotationData);


	// Get the owner of this component
	AActor* Owner = GetOwner();
	if (Owner)
	{
		// Find the skeletal mesh component attached to the same actor
		USkeletalMeshComponent* SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
		if (SkeletalMeshComponent)
		{
			UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
			if (AnimInstance)
			{
				FPoseSnapshot PoseSnapshot;
				AnimInstance->SnapshotPose(PoseSnapshot);
				for (const FTransform& BoneTransform : PoseSnapshot.LocalTransforms)
				{
					// Process each bone transform
				}
			}
		}
	}
}

void UMocapComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UMocapStreamer * MocapStreamer = UMocapStreamer::GetMocapStreamer();

	MocapStreamer->Data_Event.Remove(Handle_DataEvent);
}


// Called every frame
void UMocapComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//RetryTimeLeft -= DeltaTime;
	//if (RetryTimeLeft <= 0.0f)
	//{
	//	RetryTimeLeft += CharacterIdRetryTimeInterval;
	//}
}

void UMocapComponent::LockInitialTransforms( TSubclassOf<UControlRig> ControlRigClass )
{
	ControlConfigs.Empty();

	if (ControlRigClass)
	{
		ControlRigInstance = NewObject<UControlRig>(this, ControlRigClass);
		if (ControlRigInstance)
		{
			// Access the hierarchy
			URigHierarchy* Hierarchy = ControlRigInstance->GetHierarchy();
			if (Hierarchy)
			{
				const int Qty = SensorMapping.Num();
				for (int i = 0; i < Qty; i++)
				{
					const FControlMapping& Mapping = SensorMapping[i];
					const FName& ControlName = Mapping.ControlName;
					// Assuming you know the names of the controls
					FRigElementKey ControlKey(ControlName, ERigElementType::Control);
					// Check if the control exists
					if (Hierarchy->Contains(ControlKey))
					{
						FTransform InitialTransform = Hierarchy->GetInitialGlobalTransform(ControlKey);

						FControlData ControlData;
						ControlData.InitialControl = InitialTransform.ToMatrixNoScale();
						ControlConfigs.Add( TPair<FName, FControlData>(ControlName, ControlData) );

						// Do something with the initial transform
						UE_LOG(LogTemp, Log, TEXT("Initial Transform: %s"), *InitialTransform.ToString());
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("Control %s does not exist"), *ControlKey.Name.ToString());
					}
				}
			}
		}
	}

	for (const TPair<int, FQuat>& Pair : MocapData)
	{
		const int SensorIndex = Pair.Key;
		const FQuat InvSensorQuat = Pair.Value.Inverse();
		InvMocapData.Add( TPair<int, FQuat>(SensorIndex, InvSensorQuat) );
	}

}

float UMocapComponent::GetBatteryVoltage()
{
	return V_batt;
}

FTransform UMocapComponent::GetTransform(const FName& ControlName)
{
	const FTransform* Transform = ResultData.Find(ControlName);
	if (Transform == nullptr)
		return FTransform();

	return *Transform;
}

TArray<int> UMocapComponent::GetAvailableIndices()
{
	TArray<int> Ret;
	for (const TPair<int, FQuat> & Pair : MocapData)
	{
		const int SensorIndex = Pair.Key;
		Ret.Add(SensorIndex);
	}
	return Ret;
}



void UMocapComponent::OnRotationData( const FRotationData & RotationData )
{
	V_batt = RotationData.BatteryVoltage;
	for (const TPair<int, FQuat> & Pair : RotationData.Quats )
	{
		const int MocapIndex = Pair.Key;
		const FQuat Quat = Pair.Value;
		MocapData.Add( TTuple<int, FQuat>( MocapIndex, Quat ) );
	}

	UpdateResults();
}

void UMocapComponent::UpdateResults()
{
	ResultData.Empty();

	const int SensorsQty = SensorMapping.Num();
	for (int i = 0; i < SensorsQty; i++)
	{
		const FControlMapping& CtrlMapping = SensorMapping[i];
		const int SensorIndex   = CtrlMapping.MocapIndex;
		const FName ControlName = CtrlMapping.ControlName;

		const FQuat* MocapQuat = MocapData.Find(SensorIndex);
		if (MocapQuat == nullptr)
			continue;

		const FControlData* ControlData = ControlConfigs.Find(ControlName);
		if (ControlData == nullptr)
			continue;

		const FQuat SensorQuat = *MocapQuat;

		const FQuat * InvMocapQuat = InvMocapData.Find(SensorIndex);
		const FQuat   InvInitSensorQuat = (InvMocapQuat == nullptr) ? FQuat::Identity : *InvMocapQuat;

		const FQuat SensorRelQuat = InvInitSensorQuat * SensorQuat;

		const FMatrix A_Unreal  = MocapToUnreal(SensorRelQuat);
		const FMatrix A_Initial = ControlData->InitialControl;
		const FMatrix A_Total   = A_Initial * A_Unreal;

		const FTransform Transform_Total(A_Total);
		ResultData.Add( TPair< FName, FTransform >( ControlName, Transform_Total ) );
	}
}




FMatrix MocapToUnreal(const FQuat& Q)
{
	//const double w = Q.W;
	//const double x = Q.X;
	//const double y = Q.Y;
	//const double z = Q.Z;

	//const double m00 = 1.0 - 2.0 * (y*y + z*z);
	//const double m01 = 2.0 * (x * y - w * z);
	//const double m02 = 2.0 * (x * z + w * y);

	//const double m10 = 2.0 * (x * y + w * z);
	//const double m11 = 1.0 - 2.0 * (x * x + z * z);
	//const double m12 = 2.0 * (y * z - w * x);

	//const double m20 = 2.0 * (x * z - w * y);
	//const double m21 = 2.0 * (y * z + w * x);
	//const double m22 = 1.0 - 2.0 * (x * x + y * y);

	//const FMatrix A_Mocap(
	//	FPlane(m00, m01, m02, 0.0),
	//	FPlane(m10, m11, m12, 0.0),
	//	FPlane(m20, m21, m22, 0.0),
	//	FPlane(0.0, 0.0, 0.0, 1.0) // This row is typically used for translation in a 4x4 matrix
	//);

	const FMatrix A_Mocap = Q.ToMatrix();

	// Technically, it soule contain 0.01 instead of 1.0.
	// But for pure rotations it doesn't matter.
	const FMatrix UnrealToMocap(
		FPlane(1.0,  0.0, 0.0,  0.0),
		FPlane(0.0, -1.0, 0.0,  0.0),
		FPlane(0.0,  0.0, 1.0,  0.0),
		FPlane(0.0,  0.0, 0.0,  1.0) // This row is typically used for translation in a 4x4 matrix
	);

	// Because the matrix is symmetric, transformation back and forth are the same.
	const FMatrix MocapToUnreal = UnrealToMocap;

	const FMatrix A_Unreal = MocapToUnreal * A_Mocap * UnrealToMocap;

	return A_Unreal;
}


static void ConvertLocalToWorldTransforms(const FPoseSnapshot& PoseSnapshot, USkeletalMeshComponent* SkeletalMeshComponent, TArray<FMocapBoneData> & BoneData)
{
	// Ensure the skeletal mesh component and pose snapshot are valid
	if (!SkeletalMeshComponent || PoseSnapshot.LocalTransforms.Num() == 0)
	{
		return;
	}

	// Get the skeleton associated with the skeletal mesh component
	USkeleton* Skeleton = SkeletalMeshComponent->GetSkinnedAsset()->GetSkeleton();
	if (!Skeleton)
	{
		return;
	}

	// Component transform but without translation.
	FTransform ComponentTransform = SkeletalMeshComponent->GetComponentTransform();
	ComponentTransform.SetTranslation(FVector::Zero());

	// Array to store world transforms
	const int CurrentQty = BoneData.Num();
	const int NeededQty = PoseSnapshot.LocalTransforms.Num();
	const bool NewData = (CurrentQty != NeededQty);
	if (NewData)
	{
		BoneData.SetNum(NeededQty);
		for (int32 ind = 0; ind < NeededQty; ind++)
		{
			const FName Name = PoseSnapshot.BoneNames[ind];
			BoneData[ind].Name = Name;
		}
	}

	// Iterate through each bone
	for (int32 BoneIndex = 0; BoneIndex < PoseSnapshot.LocalTransforms.Num(); ++BoneIndex)
	{
		// Get the local transform of the current bone
		const FTransform& LocalTransform = PoseSnapshot.LocalTransforms[BoneIndex];

		// Get the parent bone index
		int32 ParentBoneIndex = Skeleton->GetReferenceSkeleton().GetParentIndex(BoneIndex);

		// If the bone has a parent, combine the local transform with the parent's world transform
		const bool bIsRoot = (ParentBoneIndex == INDEX_NONE);
		BoneData[BoneIndex].bIsRoot = bIsRoot;
		if (!bIsRoot)
		{
			BoneData[BoneIndex].Current = LocalTransform * BoneData[ParentBoneIndex].Current;
		}
		else
		{
			// If the bone is a root bone, its world transform is the same as its local transform
			BoneData[BoneIndex].Current = LocalTransform * ComponentTransform;
		}
	}

	if (NewData)
	{
		for (int32 BoneIndex = 0; BoneIndex < PoseSnapshot.LocalTransforms.Num(); ++BoneIndex)
		{
			BoneData[BoneIndex].Previous = BoneData[BoneIndex].Current;
		}
	}

	// Now WorldTransforms array contains the world space transforms of all bones
	//for (int32 BoneIndex = 0; BoneIndex < WorldTransforms.Num(); ++BoneIndex)
	//{
	//	UE_LOG(LogTemp, Log, TEXT("Bone %d World Transform: %s"), BoneIndex, *WorldTransforms[BoneIndex].ToString());
	//}
}

static bool GetGroundDistances(AActor * Actor, TArray<FMocapBoneData>& BoneData, float& HeightOverGround)
{
	UWorld* World = Actor->GetWorld();
	if (!World)
	{
		return false;
	}

	// Update all heights.
	// Also, search for the smallest height for all the bones except the root one.
	float MinHeight = 0.0;
	bool bIsAssigned = false;
	for (FMocapBoneData& OneBoneData : BoneData)
	{
		const FVector Start = OneBoneData.Current.GetLocation() + FVector(0, 0, 1.0);
		const FVector End = Start - FVector(0, 0, 1000.0); // Cast ray 1000 units downward

		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(Actor);

		bool bHit = World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams);

		if (bHit)
		{
			OneBoneData.Height = (Start - HitResult.Location).Size();
			if (((!bIsAssigned) || (MinHeight > OneBoneData.Height)) && (!OneBoneData.bIsRoot))
			{
				MinHeight = OneBoneData.Height;
				bIsAssigned = true;
			}
			//UE_LOG(LogTemp, Log, TEXT("Bone at %s is %.2f units above the ground."), *Start.ToString(), DistanceToGround);

			// Optionally, draw the debug line
			DrawDebugLine(World, Start, HitResult.Location, FColor::Green, false, 1, 0, 1);
		}
		else
		{
			OneBoneData.Height = 1000.0;
			//UE_LOG(LogTemp, Warning, TEXT("No ground detected below bone at %s."), *Start.ToString());

			// Optionally, draw the debug line
			DrawDebugLine(World, Start, End, FColor::Red, false, 1, 0, 1);
		}
	}

	HeightOverGround = MinHeight;

	return bIsAssigned;
}

static FVector GetDisplacement(TArray<FMocapBoneData>& BoneData, float alpha)
{
	FVector AllDisplacements = FVector::Zero();
	double AllWeights = 0.0;
	for (FMocapBoneData& OneBoneData : BoneData)
	{
		double Weight = FMath::Exp(-alpha * OneBoneData.Height);
		FVector Displacement = OneBoneData.Current.GetLocation() - OneBoneData.Previous.GetLocation();

		OneBoneData.Previous = OneBoneData.Current;

		Displacement.Z = 0.0;
		Displacement *= Weight;

		AllDisplacements += Displacement;
		AllWeights += Weight;
	}

	AllDisplacements /= AllWeights;

	return AllDisplacements;
}








