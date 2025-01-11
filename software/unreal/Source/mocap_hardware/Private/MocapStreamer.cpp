
#include "MocapStreamer.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Misc/Char.h"

UMocapStreamer* UMocapStreamer::Instance = nullptr;

UMocapStreamer::UMocapStreamer()
	: UWebsocketWrapper()
{

}

UMocapStreamer::~UMocapStreamer()
{

}

UMocapStreamer* UMocapStreamer::GetMocapStreamer()
{
	if (!Instance)
	{
		Instance = NewObject<UMocapStreamer>();
		Instance->AddToRoot();
	}
	return Instance;
}

void UMocapStreamer::ParseMessageReceived(const FString& Message)
{
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	if ( FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid() )
	{
		FRotationData RotationData;
		RotationData.BatteryVoltage  = JsonObject->GetNumberField( TEXT("v_batt") );

		const TSharedPtr<FJsonObject>* KeyBObject;
		if (JsonObject->TryGetObjectField(TEXT("quats"), KeyBObject))
		{
			//for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : (*KeyBObject)->Values)
			for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : (*KeyBObject)->Values)
			{
				const int SensorIndex = FCString::Atoi( *Pair.Key );
				TSharedPtr<FJsonObject> InnerObject = Pair.Value->AsObject();
				if (InnerObject.IsValid())
				{
					const float w = InnerObject->GetNumberField( TEXT("w") );
					const float x = InnerObject->GetNumberField( TEXT("x") );
					const float y = InnerObject->GetNumberField( TEXT("y") );
					const float z = InnerObject->GetNumberField( TEXT("z") );

					FQuat Quat(x, y, z, w);
					RotationData.Quats.Add( TTuple<int, FQuat>( SensorIndex, Quat ) );

					//UE_LOG(LogTemp, Warning, TEXT("Inner Object - a: %d, b: %d"), InnerA, InnerB);
				}
			}
		}
		Data_Event.Broadcast(RotationData);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON"));
	}
}

