
#include "AIBase.h"

AAIBase::AAIBase()
{
	PrimaryActorTick.bCanEverTick = true;
	// 创建武器骨骼体
	Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gun"));
	Gun->SetOnlyOwnerSee(false);
	Gun->bCastDynamicShadow=false;
	Gun->SetupAttachment(RootComponent);
	// 创建子弹生成位置
	BulletLocation=CreateDefaultSubobject<USceneComponent>("MuzzleLocation");
	BulletLocation->SetupAttachment(Gun);
	BulletLocation->SetRelativeLocation(FVector(0.0f,48.f,11.f));
	BulletLocation->SetRelativeRotation(FRotator(0,90,0));
}

void AAIBase::BeginPlay()
{
	Super::BeginPlay();
	// 将枪放到角色的手上。
	Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
		TEXT("GripPoint"));
}

void AAIBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAIBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
}

