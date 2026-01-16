// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Sekiro/SekiroShurikenProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Tag/USAGameplayTags.h"

ASekiroShurikenProjectile::ASekiroShurikenProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComponent->InitSphereRadius(5.0f);
	CollisionComponent->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComponent->OnComponentHit.AddDynamic(this, &ASekiroShurikenProjectile::OnHit);
	RootComponent = CollisionComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.2f;

	InitialLifeSpan = 3.0f;
}

void ASekiroShurikenProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void ASekiroShurikenProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != GetOwner())
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OtherActor))
		{
			if (UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent())
			{
				// Apply Damage Event
				FGameplayEventData Payload;
				Payload.Instigator = GetOwner();
				Payload.Target = OtherActor;
				Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("GameplayEvent.Combat.Hit"));
				
				// We can pass damage info in the payload or apply a GE here
				// For now, let's use the Event system we've set up for Hit Reactions
				TargetASC->HandleGameplayEvent(Payload.EventTag, &Payload);
				
				UE_LOG(LogTemp, Log, TEXT("Shuriken hit %s!"), *OtherActor->GetName());
			}
		}
		
		Destroy();
	}
}
