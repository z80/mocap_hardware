// Fill out your copyright notice in the Description page of Project Settings.


#include "WebsocketWrapper.h"
#include "WebSocketsModule.h"

UWebsocketWrapper * UWebsocketWrapper::Instance = nullptr;

// Sets default values for this component's properties
UWebsocketWrapper::UWebsocketWrapper()
{
}

UWebsocketWrapper::~UWebsocketWrapper()
{
}

UWebsocketWrapper * UWebsocketWrapper::GetWebsocketWrapperInstance()
{
	if (!Instance)
	{
		Instance = NewObject<UWebsocketWrapper>();
		Instance->AddToRoot();
	}
	return Instance;
}


void UWebsocketWrapper::Connect(const FString& address)
{
	const FString ServerURL      = address;
	const FString ServerProtocol = TEXT("ws");
	Socket = FWebSocketsModule::Get().CreateWebSocket(ServerURL, ServerProtocol);

	// Registering callbacks.
	Socket->OnConnected().AddUObject(this, &UWebsocketWrapper::OnConnected);
	Socket->OnConnectionError().AddUObject(this, &UWebsocketWrapper::OnConnectionError);
	Socket->OnMessage().AddUObject(this, &UWebsocketWrapper::OnMessageReceived);

	Socket->Connect();
}

void UWebsocketWrapper::Close()
{
	if (Socket.IsValid())
	{
		Socket->Close();
		Socket.Reset();
	}
}

bool UWebsocketWrapper::IsConnected()
{
	if (!Socket.IsValid())
		return false;

	const bool ret = Socket->IsConnected();
	return ret;
} 

void UWebsocketWrapper::Send(const FString& Data)
{
	if (!Socket.IsValid())
	{
		UE_LOG(WebsocketWrapperLogCategory, Warning, TEXT("Tried to send data. But socket wasn't created and/or conneteted.") );
		return;
	}
	Socket->Send(Data);
}

// Callbacks.
void UWebsocketWrapper::OnConnected()
{
}

void UWebsocketWrapper::OnConnectionError(const FString& Error)
{
}

void UWebsocketWrapper::OnMessageReceived(const FString& Message)
{
	ParseMessageReceived(Message);
}

void UWebsocketWrapper::ParseMessageReceived(const FString& Message)
{

}




