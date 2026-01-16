// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Enum/USAEnums.h"
#include "GameplayTagContainer.h"

//#include 

#include "USAStructs.generated.h"

// USA Attack

// attack information
USTRUCT(BlueprintType)
struct FAttackTraceInfo
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Trace Attack Info: Setting Type")
	bool bIsDirectToTarget = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Trace Attack Info: Setting Type")
	bool bIsUsingSigleTrace = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Trace Attack Info: Setting Type")
	bool bIsPinnedLocation = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Trace Attack Info: Setting Type")
	bool bIsPinnedRotation = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Trace Attack Info: Setting Type")
	TSubclassOf<UDamageType> AttackDamageType;

	//
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Trace Attack Info: Setting Type")
	
	TObjectPtr <class UNiagaraSystem> AttackHitNiagaraSystemObject;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Trace Attack Info: Setting Type")
	bool bIsAttackHitNiagaraOffset = true;

//float AttackHitNiagaraSystemObjectRandomRatioX = 0.0f;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Trace Attack Info: Setting Type")
	//float AttackHitNiagaraSystemObjectRandomRatioY = 0.0f;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Trace Attack Info: Setting Type")
	//float AttackHitNiagaraSystemObjectRandomRatioZ = 0.0f;

	//

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Attack Info: Setting Trace")
	FVector OffsetTraceLocation = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Attack Info: Setting Trace")
	FVector OffsetTraceEndLocation = FVector::ZeroVector;

	//

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Attack Info: Setting Trace")
	float AttackTraceRadius = 100.0f;

	//

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Attack Info: Setting Damage")
	float AttackDamage = -1.0f;

	//

// timing
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Attack Info: Setting Timing")
	float AttackTime = -1.0f;

	//

// Variables used in Attack Component

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Attack Info: Setting End Condition")
	float AttackDuration = -1.0f;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Attack Info: Setting End Condition")
	//float AttackTraceDuration = -1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Attack Info: Setting End Condition")
	FGameplayTag AttackEndGameplayTag_Added;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Attack Info: Setting End Condition")
	FGameplayTag AttackEndGameplayTag_Removed;

	//

};

// Data that manages attack information
USTRUCT(BlueprintType)
struct FAttackTraceInfos
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Attack Data")
	TArray <FAttackTraceInfo> AttackTraceInfos;

};

// Data utilized by USA Character Attack Component
USTRUCT(BlueprintType)
struct FAttackTraceSceneInfo
{
	GENERATED_BODY()

public:
	FAttackTraceSceneInfo();

	FAttackTraceSceneInfo
	(UWorld* InWorld, const FAttackTraceInfo& InDefault, AActor* InSource,
		const FVector& InLocation,
		const FVector& InFoward, const FVector& InRight, const FVector& InUp);

// Use after copying
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trace Attack Scene Info")
	FAttackTraceInfo DefaultAttackTraceInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trace Attack Info")
	float StartAttackTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trace Attack Info")
	AActor* AttackSourceActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trace Attack Info")
	TArray <AActor*> CheckedActorList;

	//

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trace Attack Info")
	FVector AttackForwardDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trace Attack Info")
	FVector AttackRightDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trace Attack Info")
	FVector AttackUpDirection;

	//

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trace Attack Info")
	FVector AttackLocation;

public:

	void DoAttackTrace(UWorld* InWorld, float DeltaTime);

	//bool GetIsAttackTraceEnded(UWorld* InWorld);

	bool GetIsAttackEnded(UWorld* InWorld);
};

// ========================================================================================


USTRUCT(BlueprintType)
struct FSpawnActorDetail
{
	GENERATED_BODY()

	public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Actor Detail")
	TSubclassOf<AActor> DesiredActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Actor Detail")
	float SpawnTime = -1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Actor Detail")
	bool bIsDirectToTarget = false;

};


USTRUCT(BlueprintType)
struct FSpawnActorData
{
	GENERATED_BODY()

	public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Actor Data")
	TArray <FSpawnActorDetail> SpawnActorDetails;

};




// ========================================================================================

// Struct to manage InputID, InputAction, and GameplayAbility
USTRUCT(BlueprintType)
struct FUSAGameplayAbilityHandle
{
	GENERATED_BODY()

	public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Ability Handle Info")
	int32 InputID = -1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Ability Handle Info")
	class UInputAction* InputAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Ability Handle Info")
	TSubclassOf<class UGameplayAbility> GameplayAbility;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	//FName GameplayTag;

	//FORCEINLINE TObjectPtr <class UInputAction> GetInputAction() { return InputAction;}
};

// ========================================================================================

USTRUCT(BlueprintType)
struct FUSACharacterCapsuleInfo
{
	GENERATED_BODY()

	public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Capsule Info")
	float CapsuleOriginalHalfHeight = 90.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Capsule Info")
	float CapsuleHaflHeight = 90.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Capsule Info")
	float CapsuleRadius = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Capsule Info")
	EUSACharacterCapsulePivot CapsulePivot = EUSACharacterCapsulePivot::Bottom;

public:
	void RenewCharacterCapsule(class ACharacter* InCharacter, class USpringArmComponent* InSpringArmComponent = nullptr);
	void RenewCharacterCapsuleSize(class ACharacter* InCharacter);
	void RenewCharacterCapsuleLocation(class ACharacter* InCharacter, class USpringArmComponent* InSpringArmComponent = nullptr);

	//

	void RenewJellyEffectMeshLocation(class UUSAJellyEffectComponent* InJellyEffect);
};


// ========================================================================================

USTRUCT(BlueprintType)
struct FUSACharacterAttributeSetInfo
{
	GENERATED_BODY()

	public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character AttributeSet Info")
	float StartCurrentHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character AttributeSet Info")
	float StartMaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character AttributeSet Info")
	float StartBaseArmor = 10.0f;

public:
	void RenewUSACharacterAttributeSetData(class UAbilitySystemComponent* InASC);

};


// ========================================================================================

USTRUCT(BlueprintType)
struct FUSAGameplayTagInputInfo
{
	GENERATED_BODY()

	public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Input GameplayTag Info")
	FGameplayTag InputGameplayTag_Pressed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Input GameplayTag Info")
	FGameplayTag InputGameplayTag_Holding;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Input GameplayTag Info")
	FGameplayTag InputGameplayTag_Released;
};

// ========================================================================================

// Set movement value

USTRUCT(BlueprintType)
struct FCharacterMovementWalkInfo
{
	GENERATED_BODY()

	public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement Info")
	float MaxWalkSpeed = 500;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement Info")
	FRotator RotationRate = FRotator(0, 500, 0);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement Info")
	float MaxAcceleration = 2000;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement Info")
	float GroundFriction = 8;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement Info")
	float BrakingDecelerationWalking = 2000;

public:

	void RenewCharacterMovementInfo(class UCharacterMovementComponent* InMovementComponent);
};

// ========================================================================================

// Animation playback structure

USTRUCT(BlueprintType)
struct FPlayAnimMontageData
{
	GENERATED_BODY()

	public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Montage Info")
	TObjectPtr <UAnimMontage> AnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Montage Info")
	float AnimMontageRate = 1.0f;

	//

	//FPlayAnimMontageSectionDetail StartAnimMontageSectionDetail;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Montage Info")
	FName StartAnimMontageSectionName = NAME_None;

	//

	//TArray <FPlayAnimMontageSectionDetail> MiddleAnimMontageSectionDetails;
	// <GameplayTag, SectionName>
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Montage Info")
	TMap<FGameplayTag, FName> AnimMontageSectionMapByGameplayTagAdded;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Montage Info")
	TMap<FGameplayTag, FName> AnimMontageSectionMapByGameplayTagRemoved;


	//

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Montage Info")
	//bool bHasEndSection = false;

	//FPlayAnimMontageSectionDetail EndAnimMontageSectionDetail;
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Montage Info")
	//FName EndAnimMontageSectionName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Montage Info")
	bool bIsStopWhenFinished= true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Montage Info")
	TObjectPtr <UAnimMontage> EndAnimMontage;

};
