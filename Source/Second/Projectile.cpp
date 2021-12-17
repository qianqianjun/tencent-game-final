// Write by qianqianjun

#include "Projectile.h"
AProjectile::AProjectile()
{
	CollisionComp=CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	// CollisionComp->OnComponentHit.AddDynamic(this,&AProjectile::OnHit);

	// 设置玩家无法跨上去
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable,0.f));
	CollisionComp->CanCharacterStepUpOn=ECB_No;
	// 设置根节点
	RootComponent=CollisionComp;
	
	// 设置初始的移动
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent=CollisionComp;
	ProjectileMovement->InitialSpeed=1500.f;
	ProjectileMovement->MaxSpeed=1500.f;
	ProjectileMovement->bRotationFollowsVelocity=true;
	ProjectileMovement->bShouldBounce=true;
	// 创建静态网格体
	Mesh=CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh1");
	Mesh->SetupAttachment(CollisionComp);
	// 设置三秒后自动销毁
	InitialLifeSpan=3.f;
}
