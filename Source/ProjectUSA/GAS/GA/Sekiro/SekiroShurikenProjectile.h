// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SekiroShurikenProjectile.generated.h"

/**
 * Projectile actor for the Shuriken prosthetic tool.
 */
UCLASS()
class PROJECTUSA_API ASekiroShurikenProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ASekiroShurikenProjectile();

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	class UProjectileMovementComponent* ProjectileMovement;

	/** Sphere collision */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	class USphereComponent* CollisionComponent;

	/** Static mesh for the shuriken */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	class UStaticMeshComponent* MeshComponent;

	/** Damage dealt by the shuriken */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Damage = 10.f;

	/** Posture damage dealt by the shuriken */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float PostureDamage = 5.f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

};
