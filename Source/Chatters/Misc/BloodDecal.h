// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DecalComponent.h"
#include "Misc.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Curves/CurveFloat.h"
#include "BloodDecal.generated.h"

UCLASS()
class CHATTERS_API ABloodDecal : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABloodDecal();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY()
		AActor* BotOwner = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USceneComponent* DefaultSceneRoot = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UDecalComponent* Decal = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UMaterialInterface* DecalMaterialBase = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UMaterialInstanceDynamic* DecalMaterial = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UCurveFloat* ScaleCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UCurveFloat* OpacityCurve = nullptr;

	float LifeTime = 0.0f;

	void DestroyDecal();
};
