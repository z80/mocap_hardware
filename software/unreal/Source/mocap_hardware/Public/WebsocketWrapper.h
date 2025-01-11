// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "IWebSocket.h"

#include "WebsocketWrapper.generated.h"

/**
 * 
 */

UCLASS()
class MOCAP_HARDWARE_API UWebsocketWrapper : public UObject
{
	GENERATED_BODY()
	
public:
	// Sets default values for this component's properties
	UWebsocketWrapper();
	virtual ~UWebsocketWrapper();

public:
	UFUNCTION(BlueprintCallable, Category = "Singleton")
	static UWebsocketWrapper * GetWebsocketWrapperInstance();

private:
	static UWebsocketWrapper * Instance;

public:
	UFUNCTION(BlueprintCallable, Category= Character)
	void Connect( const FString & address=TEXT("ws://127.0.0.1:8765"));

	UFUNCTION(BlueprintCallable, Category = Character)
	void Close();

	UFUNCTION(BlueprintCallable, Category= Character)
	bool IsConnected();

	UFUNCTION(BlueprintCallable, Category = Character)
	void Send(const FString& Data);

	// Callbacks.
	void OnConnected();
	void OnConnectionError(const FString& Error);
	void OnMessageReceived(const FString& Message);



private:
	TSharedPtr<IWebSocket> Socket;

	virtual void ParseMessageReceived(const FString& Message);
};

DEFINE_LOG_CATEGORY_STATIC( WebsocketWrapperLogCategory, Log, All );

