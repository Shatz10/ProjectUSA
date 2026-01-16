// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/USAJellyEffectComponent.h"

#include "Components/MeshComponent.h"
#include "Components/CapsuleComponent.h"

#include "Data/USAJellyEffectData.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Kismet/KismetMathLibrary.h"

#include "Curves/CurveVector.h"

// Sets default values for this component's properties
UUSAJellyEffectComponent::UUSAJellyEffectComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	bIsPlayingJellyEffect = false;
}


// Called when the game starts
void UUSAJellyEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	bIsPlayingJellyEffect = false;
}


// Called every frame
void UUSAJellyEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickJellyEffect();
	TickJellyEffectByGravity();

	TickJellyEffectFinal();
}

void UUSAJellyEffectComponent::SetJellySceneComponent(ACharacter* InCharacter, USceneComponent* InComponent)
{
	JellySceneComponent = InComponent;

	if (InCharacter != nullptr
		&& InCharacter->GetClass() != nullptr
		&& InCharacter->GetClass()->GetDefaultObject<ACharacter>() != nullptr
		&& InCharacter->GetClass()->GetDefaultObject<ACharacter>()->GetMesh() != nullptr)
	{
		StartMeshLocation = InCharacter->GetClass()->GetDefaultObject<ACharacter>()->GetMesh()->GetRelativeLocation();
		StartMeshRotation = InCharacter->GetClass()->GetDefaultObject<ACharacter>()->GetMesh()->GetRelativeRotation();
		StartMeshScale = InCharacter->GetClass()->GetDefaultObject<ACharacter>()->GetMesh()->GetRelativeScale3D();
	}
	else
	{
		StartMeshLocation = FVector::ZeroVector;
		StartMeshRotation = FRotator::ZeroRotator;
		StartMeshScale = FVector::OneVector;
	}
}

void UUSAJellyEffectComponent::SetMeshStartLocation(FVector InVector)
{
	StartMeshLocation = InVector;
}

void UUSAJellyEffectComponent::PlayJellyEffect(UUSAJellyEffectData* InJellyEffectData)
{
	//Jelly data validity judgment
	if (IsValid(InJellyEffectData) == false)
	{
		return;
	}

	//Confirm the existence of jelly effect target
	if (IsValid(GetJellySceneComponent()) == false)
	{
		return;
	}

	//Jelly Data and Startup Settings
	CurrentJellyEffectData = InJellyEffectData;
	bIsPlayingJellyEffect = true;

	//Set start and end time
	PlayJellyEffectTime = GetWorld()->GetTimeSeconds();
	EndJellyEffectTime = PlayJellyEffectTime + CurrentJellyEffectData->GetJellyEffectTime();

	//Perform jelly effect (first tick -> 0.0f)
	CurrentJellyEffectLocation = CurrentJellyEffectData->GetLocationVectorByRatio(0.0f);
	CurrentJellyEffectRotation = FRotator::MakeFromEuler(CurrentJellyEffectData->GetRotationVectorByRatio(0.0f));
	CurrentJellyEffectScale = CurrentJellyEffectData->GetScaleVectorByRatio(0.0f);
}

void UUSAJellyEffectComponent::StopJellyEffect()
{
	bIsPlayingJellyEffect = false;

	if (GetJellySceneComponent() == false)
	{
		return;
	}

	GetJellySceneComponent()->SetRelativeLocation(StartMeshLocation);
	GetJellySceneComponent()->SetRelativeRotation(StartMeshRotation);
	GetJellySceneComponent()->SetRelativeScale3D(StartMeshScale);
}

void UUSAJellyEffectComponent::TickJellyEffect()
{
	//Determine if you are playing
	if (bIsPlayingJellyEffect)
	{
		//Reset current jelly effect scale
		CurrentJellyEffectScale = FVector::OneVector;

		//If there is no data, it is not performed after initialization.
		if (IsValid(CurrentJellyEffectData) == false)
		{
			bIsPlayingJellyEffect = false;

			return;
		}

		//If all the time passes
		float CurrentJellyEffectTime = GetWorld()->GetTimeSeconds();
		if (CurrentJellyEffectTime > EndJellyEffectTime)
		{
			//Performing last-minute joint effects (1.0f)
			CurrentJellyEffectLocation
				= CurrentJellyEffectData->GetLocationVectorByRatio(1.0f);
			CurrentJellyEffectRotation
				= FRotator::MakeFromEuler
				(CurrentJellyEffectData->GetRotationVectorByRatio(1.0f));
			CurrentJellyEffectScale
				= CurrentJellyEffectData->GetScaleVectorByRatio(1.0f);

			//If you don't keep the last effect
			//Stop Tick operation by setting bIsPlayingJellyEffect to false
			if (CurrentJellyEffectData->GetJellyKeepLastEffect() == false)
			{
				bIsPlayingJellyEffect = false;
			}

			return;
		}

		//Calculate progress percentage based on current time
		float CurrentJellyEffectRatio 
			= (CurrentJellyEffectTime - PlayJellyEffectTime) 
			/ (EndJellyEffectTime - PlayJellyEffectTime);

		//Perform jelly effect
		CurrentJellyEffectLocation 
			= CurrentJellyEffectData->GetLocationVectorByRatio(CurrentJellyEffectRatio);
		CurrentJellyEffectRotation 
			= FRotator::MakeFromEuler
			(CurrentJellyEffectData->GetRotationVectorByRatio(CurrentJellyEffectRatio));
		CurrentJellyEffectScale 
			= CurrentJellyEffectData->GetScaleVectorByRatio(CurrentJellyEffectRatio);
	}
}

void UUSAJellyEffectComponent::TickJellyEffectByGravity()
{
	if (bIsUsingJellyEffectByGravity == false)
	{
		CurrentJellyEffectGravityScale = FVector::OneVector;

		return;
	}

	if (GetJellySceneComponent() == nullptr)
	{
		return;
	}

	ACharacter* Character = Cast <ACharacter>(GetOwner());
	if (Character == nullptr
		|| Character->GetCharacterMovement() == nullptr)
	{
		return;
	}

	if (ScaleByGravityRatio == nullptr)
	{
		return;
	}

	float CurrentRatio = Character->GetCharacterMovement()->Velocity.Z / MaxGravityForScale;
	CurrentRatio = FMath::Abs(CurrentRatio);
	CurrentRatio = FMath::Clamp(CurrentRatio, 0.0f, 1.0f);
	
	CurrentJellyEffectGravityScale = ScaleByGravityRatio->GetVectorValue(CurrentRatio);
}

void UUSAJellyEffectComponent::TickJellyEffectFinal()
{
	if (GetJellySceneComponent() == false)
	{
		return;
	}

	GetJellySceneComponent()->SetRelativeLocation(StartMeshLocation + CurrentJellyEffectLocation + CurrentJellyEffectCapsuleOffsetLocation);
	GetJellySceneComponent()->SetRelativeRotation(StartMeshRotation + CurrentJellyEffectRotation);
	GetJellySceneComponent()->SetRelativeScale3D((StartMeshScale * CurrentJellyEffectScale) * CurrentJellyEffectGravityScale);

	CurrentJellyEffectLocation = FVector::ZeroVector;
	CurrentJellyEffectRotation = FRotator::ZeroRotator;
	CurrentJellyEffectScale = FVector::OneVector;
}
