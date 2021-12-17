// Write by qianqianjun

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Projectile.generated.h"

UCLASS()
class SECOND_API AProjectile : public AActor
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=Projectile)
	USphereComponent* CollisionComp;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=Movement)
	UProjectileMovementComponent* ProjectileMovement;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=Mesh)
	UStaticMeshComponent* Mesh;
	AProjectile();

};
