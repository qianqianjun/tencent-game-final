#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIBase.generated.h"

UCLASS()
class SECOND_API AAIBase : public ACharacter
{
	GENERATED_BODY()
public:
	AAIBase();
	// 添加一把枪
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=Mesh)
	USkeletalMeshComponent* Gun;
	// 添加子弹生成处
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=Mesh)
	USceneComponent* BulletLocation;
	// 添加开火动画
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=GamePlay)
	UAnimMontage* FireAnimation;
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
};
