// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "../Core/ChattersGameInstance.h"
#include "../Misc/MathHelper.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerPawnController.h"

APlayerPawn* APlayerPawn::Singleton = nullptr;

// Sets default values
APlayerPawn::APlayerPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	this->SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	this->SetRootComponent(this->SphereCollision);

	this->CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	this->CameraBoom->SetupAttachment(this->SphereCollision);
	
	this->Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	this->Camera->SetupAttachment(this->CameraBoom);

	this->MovementComponent = CreateDefaultSubobject<UPawnMovementComponent, UFloatingPawnMovement>(TEXT("MovementComponent"));
	this->MovementComponent->UpdatedComponent = this->SphereCollision;

	this->AIControllerClass = APlayerPawnController::StaticClass();
}

// Called when the game starts or when spawned
void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	
	this->LastZoomValue = this->DefaultAttachedZoom;
}

// Called every frame
void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	this->CachedCameraLocation = this->Camera->GetComponentLocation();

	if (this->bReady)
	{
		this->UpdateBotNicknameWidgets();
	}

	if (this->bAttachedToBot)
	{
		this->SetActorRotation(FRotator(0.0f));
	}

}

void APlayerPawn::AttachToBot(ABot* Bot)
{
	if (Bot)
	{
		if (this->BotToAttach)
		{
			this->BotToAttach->bPlayerAttached = false;
		}

		this->BotToAttach = Bot;
		this->BotToAttach->bPlayerAttached = true;

		FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepRelative, true);
		this->AttachToActor(this->BotToAttach, TransformRules);

		if (!this->bAttachedToBot)
		{
			if (this->Controller)
			{
				this->Controller->SetControlRotation(FRotator(0.0f, 0.0f, 0.0f));
			}

			if (this->CameraBoom)
			{
				this->CameraBoom->TargetArmLength = this->LastZoomValue;

				FRotator BotRotation = Bot->GetActorRotation();

				this->CameraBoom->SetRelativeRotation(FRotator(-30.0f, BotRotation.Yaw, 0.0f));
			}
			
		}
		else
		{
			if (this->CameraBoom)
			{
				FRotator CameraBoomRotation = this->CameraBoom->GetRelativeRotation();
				CameraBoomRotation.Roll = 0.0f;
				this->CameraBoom->SetRelativeRotation(CameraBoomRotation);
			}
		}

		/** Check is bot visible from camera */

		FRotator RotationToSet = this->FindNewAcceptableCameraRotation(this->CameraBoom->GetRelativeRotation());
		this->CameraBoom->SetRelativeRotation(RotationToSet);

		this->bAttachedToBot = true;
		this->SetSpectatorMenuVisibiliy(true);
	}
}

void APlayerPawn::DetachFromBot()
{
	if (this->bAttachedToBot)
	{
		this->bAttachedToBot = false;

		if (this->BotToAttach)
		{
			FVector CameraLocation = this->BotToAttach->GetActorLocation();
			FRotator CameraRotation = this->BotToAttach->GetActorRotation();

			this->BotToAttach->bPlayerAttached = false;

			this->BotToAttach = nullptr;

			if (this->Camera)
			{
				CameraLocation = this->Camera->GetComponentLocation();;
				CameraRotation = this->Camera->GetComponentRotation();
			}

			CameraRotation.Roll = 0.0f;

			if (this->CameraBoom)
			{
				this->CameraBoom->TargetArmLength = 0.0f;
				this->CameraBoom->SetRelativeRotation(FRotator(0.0f));
			}

			this->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

			this->TeleportTo(CameraLocation, CameraRotation);

			if (this->Controller)
			{
				this->Controller->SetControlRotation(CameraRotation);

			}
		}
		this->SetSpectatorMenuVisibiliy(false);
	}
}

float APlayerPawn::GetDistanceFromCamera(FVector Location)
{
	FVector const CameraLocation = this->GetCameraLocation();
	return FVector::Dist(CameraLocation, Location);
}

void APlayerPawn::UpdateBotNicknameWidgets()
{
	auto* GameSessionObject = this->GetGameSession();

	TArray<ABot*> Bots = GameSessionObject->Bots;

	if (!Bots.Num())
	{
		return;
	}

	if (!this->Camera)
	{
		return;
	}

	for (int32 i = 0; i < Bots.Num(); i++)
	{
		ABot* Bot = Bots[i];

		if (!Bot)
		{
			continue;
		}

		FVector const CameraLocation = this->GetCameraLocation();
		FVector const BotLocation = Bot->GetActorLocation();
		float const DistanceFromBot = FVector::Dist(CameraLocation, BotLocation);

		auto* BotNameWidgetComponent = Bot->NameWidgetComponent;

		if (!BotNameWidgetComponent)
		{
			continue;
		}

		auto* BotNameWidget = Cast<UBotNameWidget>(BotNameWidgetComponent->GetWidget());

		if (BotNameWidget)
		{
			float const ClampedDistance = FMath::Clamp(DistanceFromBot, BotNameWidget->MinDistanceToScale, BotNameWidget->MaxDistanceToScale);
			float const DistanceAlpha = (ClampedDistance - BotNameWidget->MinDistanceToScale) / (BotNameWidget->MaxDistanceToScale - BotNameWidget->MinDistanceToScale);

			float NewWidgetSize = FMath::Lerp(BotNameWidget->MaxWrapperScale, BotNameWidget->MinWrapperScale, DistanceAlpha);

			BotNameWidget->UpdateSize(NewWidgetSize);


			float const MinOpacityDistance = BotNameWidget->MaxOpacityDistance.GetLowerBoundValue();
			float const MaxOpacityDistance = BotNameWidget->MaxOpacityDistance.GetUpperBoundValue();

			float const ClampedOpacityDistnace = FMath::Clamp(DistanceFromBot, MinOpacityDistance, MaxOpacityDistance);
			float const OpacityValue = 1.0f - UKismetMathLibrary::NormalizeToRange(ClampedOpacityDistnace, MinOpacityDistance, MaxOpacityDistance);
			

			BotNameWidget->UpdateOpacity(OpacityValue);

		}

	}

}

void APlayerPawn::SetSpectatorMenuVisibiliy(bool bVisible)
{
	auto* GameSessionObject = this->GetGameSession();

	if (GameSessionObject)
	{
		auto* SessionWidget = GameSessionObject->GetSessionWidget();

		if (SessionWidget)
		{
			if (bVisible && this->bAttachedToBot && this->BotToAttach)
			{
				SessionWidget->UpdateSpectatorBotName(this->BotToAttach->DisplayName);
				SessionWidget->UpdateSpectatorBotHealth(this->BotToAttach->HealthPoints);
				SessionWidget->UpdateSpectatorBotKills(this->BotToAttach->Kills);
				SessionWidget->SpectatorNicknameColor = this->BotToAttach->GetTeamColor();
			}

			SessionWidget->SetSpectatorWidgetVisibility(bVisible);
		}
	}
}

UChattersGameSession* APlayerPawn::GetGameSession()
{
	if (!this->GameSession)
	{
		auto* GameInstance = UChattersGameInstance::Get();

		if (!GameInstance || GameInstance->GetIsInMainMenu())
		{
			return nullptr;
		}

		this->GameSession = GameInstance->GetGameSession();
	}

	return this->GameSession;
}

FVector APlayerPawn::GetAttachedCameraWorldLocation(float Distance, FRotator CameraRotation)
{
	CameraRotation.Pitch *= -1.0f;
	CameraRotation.Yaw += 180.0f;
	FVector CameraVector = CameraRotation.Vector() * Distance;
	
	FVector BotLocation = FVector(0.0f);

	if (this->BotToAttach)
	{
		BotLocation = this->BotToAttach->GetActorLocation();
	}

	return BotLocation + CameraVector;
}

bool APlayerPawn::IsBotVisibleFromCamera(float Distance, FRotator CameraRotation)
{
	if (!this->BotToAttach)
	{
		return false;
	}

	FVector CameraLocation = this->GetAttachedCameraWorldLocation(Distance, CameraRotation);

	FVector BotLocation = this->BotToAttach->GetActorLocation();
	 
	FHitResult HitResult;
	this->GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, BotLocation, ECollisionChannel::ECC_Camera);

	//FVector TraceEndLocation = HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd;

	//DrawDebugLine(GetWorld(), CameraLocation, TraceEndLocation, FColor::Red, false, 7.0f);

	auto* HitActor = HitResult.GetActor();

	if (!(HitResult.bBlockingHit && HitActor == this->BotToAttach))
	{
		return false;
	}

	/** Trace from bot to camera */

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this->BotToAttach);
	this->GetWorld()->LineTraceSingleByChannel(HitResult, BotLocation, CameraLocation, ECollisionChannel::ECC_Camera, Params);

	/** Return as visible if there's no collision between bot and camera */
	return !HitResult.bBlockingHit;
}

FRotator APlayerPawn::FindNewAcceptableCameraRotation(FRotator StartRotation)
{
	FRotator RotationToSet = StartRotation;
	float StartYawRotation = FMath::Fmod(RotationToSet.Yaw + 360.0f, 360.0f);

	const float CheckVisibilityAngleAddition = 360.0f / 20.0f;
	const int32 MaxChecks = int32(360.0f / CheckVisibilityAngleAddition);

	float Distance = this->CameraBoom->TargetArmLength;

	for (int32 i = 0; i < MaxChecks; i++)
	{
		float YawRotation = StartYawRotation + (CheckVisibilityAngleAddition * i);
		YawRotation = FMath::Fmod(YawRotation + 360.0f, 360.0f);

		FRotator RotationToCheck = StartRotation;
		RotationToCheck.Yaw = YawRotation;

		/** Check is bot visible from camera */
		bool bVisible = this->IsBotVisibleFromCamera(Distance, RotationToCheck);

		if (bVisible)
		{
			RotationToSet = RotationToCheck;
			break;
		}
	}

	return RotationToSet;
}

APlayerPawn* APlayerPawn::Get()
{
	return APlayerPawn::Singleton;
}

void APlayerPawn::Init()
{
	this->bReady = true;
	APlayerPawn::Singleton = this;
}


FVector APlayerPawn::GetCameraLocation()
{
	return this->CachedCameraLocation;
}

void APlayerPawn::RespawnAttachedBot()
{
	if (this->bAttachedToBot && this->BotToAttach)
	{
		this->BotToAttach->RespawnAtRandomPlace();
	}
}
