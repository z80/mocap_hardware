// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ControlRigComponent.h"

#include "Animation/AnimInstance.h"
#include "AnimNode_ControlRig.h"
#include "ControlRig.h"

#include "MocapStreamer.h"


#include "MocapComponent.generated.h"


// Mocap data configuration. It describes which sensor reading belongs to which 
// control in the control rig.
USTRUCT(BlueprintType)
struct FControlMapping
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ControlName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int   MocapIndex;
};

// This is what is obtained from the control rig.
struct FControlData
{
	FMatrix InitialControl;
};

struct FMocapBoneData
{
	FName      Name;
	FTransform Previous;
	FTransform Current;
	float      Height;
	bool       bIsRoot;
};



UCLASS( Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MOCAP_HARDWARE_API UMocapComponent: public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMocapComponent();
	virtual ~UMocapComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category = "Settings")
	TArray<FControlMapping> SensorMapping;

	// It seems there is no other way to read initial transforms other than 
	// to create an instance of the same control rig.
	UFUNCTION(BlueprintCallable, Category = Character)
	void LockInitialTransforms(TSubclassOf<UControlRig> ControlRigClass);

	UPROPERTY()
	UControlRig* ControlRigInstance;

	UFUNCTION(BlueprintCallable, Category = Character)
	float GetBatteryVoltage();

	UFUNCTION(BlueprintCallable, Category = Character)
	FTransform GetTransform( const FName & ControlName );

	UFUNCTION(BlueprintCallable, Category = Character)
	TArray<int> GetAvailableIndices();

	FDelegateHandle Handle_DataEvent;
	void OnRotationData( const FRotationData & RotationData );



private:
	void UpdateResults();

	float                     V_batt;
	TMap<FName, FControlData> ControlConfigs;
	TMap<int, FQuat>          MocapData;
	TMap<int, FQuat>          InvMocapData;
	TMap<FName, FTransform>   ResultData;

	// Data for extracting toot motion.
	UControlRig* ControlRigAsset;
	TArray<FMocapBoneData> BoneData;

	//float RetryTimeLeft;
	//static const float CharacterIdRetryTimeInterval;
};
