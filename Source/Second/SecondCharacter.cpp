// Write by qianqianjun

#include "SecondCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

ASecondCharacter::ASecondCharacter()
{
	PrimaryActorTick.bCanEverTick=true;
	bReplicates=true;
	
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;   

	// Configure character movement
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 250.0f;
	CameraBoom->SetRelativeLocation(FVector(0,0,90));
	

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; 
	FollowCamera->SetRelativeRotation(FRotator(0,-15,0));
	FollowCamera->SetRelativeLocation(FVector(0,30,0));

	// 创建武器骨骼体
	Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gun"));
	Gun->SetOnlyOwnerSee(false);
	Gun->bCastDynamicShadow=false;
	Gun->SetupAttachment(RootComponent);

	// 创建背后的武器
	Cannon=CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BackGun"));
	Cannon->SetOnlyOwnerSee(false);
	Cannon->bCastDynamicShadow=false;
	Cannon->SetupAttachment(RootComponent);

	// 创建子弹生成位置
	BulletLocation=CreateDefaultSubobject<USceneComponent>("MuzzleLocation");
	BulletLocation->SetupAttachment(Gun);
	BulletLocation->SetRelativeLocation(FVector(0.0f,48.f,11.f));
	BulletLocation->SetRelativeRotation(FRotator(0,90,0));

	// 创建炮弹生成位置
	Bomb=CreateDefaultSubobject<USceneComponent>("BombLocation");
	Bomb->SetupAttachment(Cannon);
	Bomb->SetRelativeLocation(FVector(0.0f,48.f,11.f));
	Bomb->SetRelativeRotation(FRotator(0,90,0));
	
	// 旋转相关的设置
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	CameraBoom->bUsePawnControlRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation=true;

	// 模式设置
	AimMode=false; // 默认不是瞄准模式
	IsSpeedUp=false; // 默认是行走
	MaxWalkSpeed=500.f;
	IsGunOnHand=true;
}
// 绑定输入
void ASecondCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	// 跳跃
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	// 角色移动
	PlayerInputComponent->BindAxis("MoveForward", this, &ASecondCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASecondCharacter::MoveRight);

	// 角色开火
	PlayerInputComponent->BindAction("Fire",IE_Pressed,this,&ASecondCharacter::ThrowBomb);
	// 角色视角转换
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASecondCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASecondCharacter::LookUpAtRate);

	PlayerInputComponent->BindTouch(IE_Pressed, this, &ASecondCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ASecondCharacter::TouchStopped);
	// 角色切换模式
	PlayerInputComponent->BindAction("SwitchMode",IE_Pressed,this,&ASecondCharacter::SwitchMode);
	// 设置跑步速度，实现走路和快跑的切换
	PlayerInputComponent->BindAction("SpeedUp",IE_Pressed,this,&ASecondCharacter::SpeedUp);
	PlayerInputComponent->BindAction("SpeedUp",IE_Released,this,&ASecondCharacter::SpeedUp);
}

void ASecondCharacter::BeginPlay()
{
	Super::BeginPlay();
	// 将枪放到角色的手上。
	Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
		TEXT("GripPoint"));
	// 将大炮放在后背的插槽上。
	Cannon->AttachToComponent(GetMesh(),FAttachmentTransformRules(EAttachmentRule::SnapToTarget,true),
		TEXT("BackWeapon"));
}

void ASecondCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

// 抛掷炸弹或者手雷等抛射物
void ASecondCharacter::ThrowBomb()
{
	CreateBombInServer();
}

void ASecondCharacter::CreateBombInServer_Implementation()
{
	// 生成抛射物
	if(ProjectileClass!=nullptr)
	{
		UWorld* const World=GetWorld();
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		World->SpawnActor<AProjectile>(ProjectileClass,Bomb->GetComponentTransform(),ActorSpawnParameters);
	}
	// 如果有动画，则播放开火动画
	if(FireAnimation!=nullptr)
	{
		UAnimInstance* AnimInstance=GetMesh()->GetAnimInstance();
		if(AnimInstance!=nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation,1.f);
		}
	}
	// 如果有音效，则播放开火音效
	if(FireSound!=nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this,FireSound,GetActorLocation());
	}
}

void ASecondCharacter::MulticastBombToClient_Implementation()
{
	
}

void ASecondCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void ASecondCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

// 设置角色的加速函数
void ASecondCharacter::SpeedUp()
{
	SpeedUpInServer();
}

void ASecondCharacter::SpeedUpInServer_Implementation()
{
	MulticastSpeedUp();
}

void ASecondCharacter::MulticastSpeedUp_Implementation()
{
	if(IsSpeedUp)
	{
		GetCharacterMovement()->MaxWalkSpeed=MaxWalkSpeed/2;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed=MaxWalkSpeed;
	}
	IsSpeedUp=!IsSpeedUp;
}

void ASecondCharacter::SwitchMode()
{
	SwitchModeOnServer();
}

void ASecondCharacter::SwitchModeOnServer_Implementation()
{
	MulticastModeToClient();
}

void ASecondCharacter::MulticastModeToClient_Implementation()
{
	bUseControllerRotationYaw=AimMode?bUseControllerRotationYaw:false;
	AimMode = !AimMode;
}

void ASecondCharacter::SwitchWeapon(UAnimMontage* Anim,FName Section1,FName Section2)
{
	SwitchWeaponOnServer(Anim,Section1,Section2);
}

void ASecondCharacter::SwitchWeaponOnServer_Implementation(UAnimMontage* Anim,FName Section1,FName Section2)
{
	MontageMulticast(Anim,Section1);
	MulticastWeaponToClient();
	MontageMulticast(Anim,Section2);
}

void ASecondCharacter::MulticastWeaponToClient_Implementation()
{
	if(IsGunOnHand)
	{
		// 将枪放到角色的后背上。
		Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
			TEXT("BackWeapon"));
		// 将来大炮放在角色手上。
		Cannon->AttachToComponent(GetMesh(),FAttachmentTransformRules(EAttachmentRule::SnapToTarget,true),
			TEXT("GripPoint"));
	}
	else
	{
		Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
			TEXT("GripPoint"));
		Cannon->AttachToComponent(GetMesh(),FAttachmentTransformRules(EAttachmentRule::SnapToTarget,true),
			TEXT("BackWeapon"));
	}
	IsGunOnHand=!IsGunOnHand;
}

// 根据玩家的状态，设置旋转相关的设置
void ASecondCharacter::SetRotationSetting()
{
	SetRotationSettingOnServer();
}

void ASecondCharacter::SetRotationSettingOnServer_Implementation()
{
	MulticastRotationSettingToClient();
}

void ASecondCharacter::MulticastRotationSettingToClient_Implementation()
{
	if(AimMode)
	{
		const float Speed=GetVelocity().Size();
		const bool Value=Speed>0;
		bUseControllerRotationYaw=Value;
		GetCharacterMovement()->bOrientRotationToMovement=!Value;
	}
}

void ASecondCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASecondCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ASecondCharacter::MontageMulticast_Implementation(UAnimMontage* Anim,FName StartSection=NAME_None)
{
	PlayAnimMontage(Anim,1.f,StartSection);
}

void ASecondCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ASecondCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
