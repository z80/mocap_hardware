
#pragma once

#include "CoreMinimal.h"
#include "WebsocketWrapper.h"

#include "MocapStreamer.generated.h"

USTRUCT()
struct FRotationData
{
	GENERATED_BODY()

	float BatteryVoltage;

	TMap<int, FQuat> Quats;
};



UCLASS()
class MOCAP_HARDWARE_API UMocapStreamer : public UWebsocketWrapper
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMocapStreamer();
	virtual ~UMocapStreamer();

public:
	UFUNCTION(BlueprintCallable, Category = "Singleton")
	static UMocapStreamer * GetMocapStreamer();

	// Here need to define the data type returned "FData".
	DECLARE_EVENT_OneParam(UMocapStreamer, FDataSignature, const FRotationData& );
	FDataSignature  Data_Event;

private:
	static UMocapStreamer* Instance;

	void ParseMessageReceived(const FString& Message) override;
};


