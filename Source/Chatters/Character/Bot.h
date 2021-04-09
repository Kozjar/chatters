// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BotController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/StaticMeshComponent.h"
#include "../UI/Widgets/BotNameWidget.h"
#include "Bot.generated.h"

UCLASS()
class CHATTERS_API ABot : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UPROPERTY(VisibleAnywhere, Category="Bot")
		FString DisplayName = FString();

	uint32 ID;

	UPROPERTY(VisibleAnywhere, Category = "Bot")
		int32 HealthPoints;

	UPROPERTY(VisibleAnywhere, Category = "Bot")
		int32 MaxHealthPoints;

	bool GetIsAlive();

	ABotController* GetAIController();

	void Init(FString NewName, uint32 NewID);

	void ApplyDamage(int32 Damage);

	float GetHeathValue();
private:
	bool bAlive = true;

	void SetOutfit();

	void MoveToRandomLocation();

	bool bMovingToRandomLocation = false;

	FVector RandomLocationTarget;

	UBotNameWidget* GetNameWidget();

public:
	static ABot* CreateBot(UWorld* World, FString NameToSet, uint32 IDToSet, TSubclassOf<ABot> Subclass);
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		USkeletalMeshComponent* HeadMesh;

	/** UI Widget with display name and health points */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UWidgetComponent* NameWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* HatMesh;
};