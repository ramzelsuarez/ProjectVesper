// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/VesperCharacter.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Math/RotationMatrix.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GroomComponent.h"
#include "Components/AttributeComponent.h"
#include "Item.h"
#include "Weapons/Weapon.h"
#include "Animation/AnimMontage.h"
#include "HUD/VesperHUD.h"
#include "HUD/VesperOverlay.h"
#include "Interfaces/PickupInterface.h"
#include "Items/Soul.h"
#include "Items/Treasure.h"
#include "Items/HealthPickup.h"
#include "Enemy/Enemy.h"
#include "Kismet/GameplayStatics.h"

AVesperCharacter::AVesperCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->bUsePawnControlRotation = false;
	ViewCamera->SetupAttachment(CameraBoom);

	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("head");

	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("head");
}

void AVesperCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateGameplayCameraTransition(DeltaTime);
	UpdateLockOn(DeltaTime);

	if (bIsSprinting && (!IsUnoccupied() || !Attributes || Attributes->GetStamina() <= 0.f))
	{
		StopSprint();
	}

	if (Attributes && VesperOverlay)
	{
		if (bIsSprinting && Attributes->GetStamina() > 0.f && IsUnoccupied())
		{
			Attributes->UseStamina(SprintStaminaDrainRate * DeltaTime);

			if (Attributes->GetStamina() <= 0.f)
			{
				StopSprint();
			}
		}
		else
		{
			Attributes->RegenStamina(DeltaTime);
		}

		VesperOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
}
void AVesperCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &AVesperCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AVesperCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AVesperCharacter::Jump);
		EnhancedInputComponent->BindAction(EKeyAction, ETriggerEvent::Triggered, this, &AVesperCharacter::EKeyPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AVesperCharacter::Attack);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &AVesperCharacter::Dodge);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AVesperCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AVesperCharacter::StopSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Canceled, this, &AVesperCharacter::StopSprint);
		EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &AVesperCharacter::TogglePauseMenu);
		EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started, this, &AVesperCharacter::ToggleLockOn);
	}
}

float AVesperCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	SetHUDHealth();

	return DamageAmount;
}

void AVesperCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);

	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Attributes && Attributes->GetHealthPercent() > 0.f)
	{
		ActionState = EActionState::EAS_HitReaction;
	}
}

void AVesperCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;

	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingWeapon)
	{
		ShowPickupPrompt(OverlappingWeapon->GetWeaponName());
	}
	else
	{
		HidePickupPrompt();
	}
}

void AVesperCharacter::AddSouls(ASoul* Soul)
{
	if (Attributes && VesperOverlay)
	{
		Attributes->AddSouls(Soul->GetSouls());
		VesperOverlay->SetSouls(Attributes->GetSouls());
	}
}

void AVesperCharacter::AddGold(ATreasure* Treasure)
{
	if (Attributes && VesperOverlay)
	{
		Attributes->AddGold(Treasure->GetGold());
		VesperOverlay->SetGold(Attributes->GetGold());
	}
}

void AVesperCharacter::AddHealth(AHealthPickup* HealthPickup)
{
	if (Attributes && VesperOverlay)
	{
		Attributes->Heal(HealthPickup->GetHealthAmount());
		VesperOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}

void AVesperCharacter::BeginPlay()
{
	Super::BeginPlay();

	Tags.Add(FName("EngageableTarget"));

	AddInputMappingContext();
	ApplyMenuCameraPose();
}

void AVesperCharacter::Move(const FInputActionValue& Value)
{
	if (bIsInMenu) return;
	if (ActionState != EActionState::EAS_Unoccupied) return;

	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVector.Y);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void AVesperCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void AVesperCharacter::StartSprint()
{
	if (bIsInMenu) return;
	if (!Attributes || !IsUnoccupied() || Attributes->GetStamina() <= 0.f) return;

	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void AVesperCharacter::StopSprint()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AVesperCharacter::ApplyMenuCameraPose()
{
	AController* LocalController = GetController();
	if (!LocalController || !CameraBoom) return;

	const FRotator ActorRot = GetActorRotation();

	const FRotator MenuRot(
		MenuPitch,
		ActorRot.Yaw + 180.f,
		0.f
	);

	LocalController->SetControlRotation(MenuRot);

	CameraBoom->TargetArmLength = MenuArmLength;

	FVector SocketOffset = CameraBoom->SocketOffset;
	SocketOffset.Z = MenuSocketOffsetZ;
	CameraBoom->SocketOffset = SocketOffset;
}

void AVesperCharacter::StartGameplayCameraTransition()
{
	AController* LocalController = GetController();
	if (!LocalController || !CameraBoom || bCameraTransitionActive) return;

	bCameraTransitionActive = true;
	CameraTransitionElapsed = 0.f;

	CameraTransitionStartRotation = LocalController->GetControlRotation();
	CameraTransitionStartArmLength = CameraBoom->TargetArmLength;
	CameraTransitionStartSocketOffsetZ = CameraBoom->SocketOffset.Z;

	const FRotator ActorRot = GetActorRotation();

	CameraTransitionTargetRotation = FRotator(
		0.f,
		ActorRot.Yaw,
		0.f
	);
}

void AVesperCharacter::UpdateGameplayCameraTransition(float DeltaTime)
{
	if (!bCameraTransitionActive) return;

	AController* LocalController = GetController();
	if (!LocalController || !CameraBoom)
	{
		bCameraTransitionActive = false;
		return;
	}

	CameraTransitionElapsed += DeltaTime;

	const float Alpha = FMath::Clamp(CameraTransitionElapsed / CameraTransitionDuration, 0.f, 1.f);
	const float EasedAlpha = FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.f);

	FVector SocketOffset = CameraBoom->SocketOffset;
	SocketOffset.Z = FMath::Lerp(CameraTransitionStartSocketOffsetZ, GameplaySocketOffsetZ, EasedAlpha);
	CameraBoom->SocketOffset = SocketOffset;

	CameraBoom->TargetArmLength = FMath::Lerp(CameraTransitionStartArmLength, GameplayArmLength, EasedAlpha);

	const FRotator NewRotation = FMath::Lerp(
		CameraTransitionStartRotation,
		CameraTransitionTargetRotation,
		EasedAlpha
	);

	LocalController->SetControlRotation(NewRotation);

	if (Alpha >= 1.f)
	{
		bCameraTransitionActive = false;
		bIsInMenu = false;

		InitializeVesperOverlay();
	}
}

void AVesperCharacter::AddInputMappingContext()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
	ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

	if (Subsystem)
	{
		Subsystem->AddMappingContext(VesperContext, 0);
	}
}

void AVesperCharacter::EKeyPressed()
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingWeapon)
	{
		const FString WeaponName = OverlappingWeapon->GetWeaponName();

		if (EquippedWeapon)
		{
			EquippedWeapon->Destroy();
		}

		EquipWeapon(OverlappingWeapon);
		HidePickupPrompt();
		ShowPickupNotification(FString::Printf(TEXT("Obtained: %s"), *WeaponName));
	}
	else
	{
		if (CanDisarm())
		{
			Disarm();
		}
		else if (CanArm())
		{
			Arm();
		}
	}
}

void AVesperCharacter::Attack()
{
	if (bIsInMenu) return;
	Super::Attack();
	if (CanAttack())
	{
		PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
}

void AVesperCharacter::EquipWeapon(AWeapon* Weapon)
{
	Weapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	EquippedWeapon = Weapon;
	OverlappingItem = nullptr;
}

void AVesperCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void AVesperCharacter::DodgeEnd()
{
	Super::DodgeEnd();

	ActionState = EActionState::EAS_Unoccupied;
}

bool AVesperCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

bool AVesperCharacter::CanDisarm()
{
	return ActionState == EActionState::EAS_Unoccupied && 
		CharacterState != ECharacterState::ECS_Unequipped;
}

bool AVesperCharacter::CanArm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_Unequipped &&
		EquippedWeapon;
}

void AVesperCharacter::Disarm()
{
	PlayEquipMontage(FName("Unequipped"));
	CharacterState = ECharacterState::ECS_Unequipped;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void AVesperCharacter::Arm()
{
	PlayEquipMontage(FName("Equip"));
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void AVesperCharacter::AttachWeaponToBack()
{

	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void AVesperCharacter::AttachWeaponToHand()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void AVesperCharacter::PlayEquipMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

void AVesperCharacter::Die_Implementation()
{
	Super::Die_Implementation();

	GetCharacterMovement()->StopMovementImmediately();

	Tags.Add(FName("Dead"));
	ActionState = EActionState::EAS_Dead;
	DisableMeshCollision();

	FTimerHandle GameOverTimer;
	GetWorldTimerManager().SetTimer(
		GameOverTimer,
		this,
		&AVesperCharacter::ShowGameOverScreen,
		2.0f
	);
}

bool AVesperCharacter::HasEnoughStamina()
{
	return Attributes && Attributes->GetStamina() > Attributes->GetDodgeCost();
}

bool AVesperCharacter::IsOccupied()
{
	return ActionState != EActionState::EAS_Unoccupied;
}

void AVesperCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void AVesperCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void AVesperCharacter::ToggleLockOn()
{
	if (bIsLockedOn)
	{
		ClearLockOn();
		return;
	}

	LockedTarget = FindNearestEnemy();

	if (LockedTarget)
	{
		bIsLockedOn = true;
		CombatTarget = LockedTarget;

		LockedTarget->ShowLockOnWidget();
	}
}

AEnemy* AVesperCharacter::FindNearestEnemy()
{
	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemy::StaticClass(), Enemies);

	AEnemy* ClosestEnemy = nullptr;
	float ClosestDistance = LockOnRadius;

	for (AActor* Actor : Enemies)
	{
		AEnemy* Enemy = Cast<AEnemy>(Actor);
		if (!Enemy || Enemy->ActorHasTag(FName("Dead"))) continue;

		const float Distance = FVector::Dist(GetActorLocation(), Enemy->GetActorLocation());

		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestEnemy = Enemy;
		}
	}

	return ClosestEnemy;
}

void AVesperCharacter::UpdateLockOn(float DeltaTime)
{
	if (!bIsLockedOn || !LockedTarget) return;

	if (LockedTarget->ActorHasTag(FName("Dead")) ||
		FVector::Dist(GetActorLocation(), LockedTarget->GetActorLocation()) > LockOnRadius)
	{
		ClearLockOn();
		return;
	}

	CombatTarget = LockedTarget;
}

void AVesperCharacter::ClearLockOn()
{
	if (LockedTarget)
	{
		LockedTarget->HideLockOnWidget();
	}

	bIsLockedOn = false;
	LockedTarget = nullptr;
	CombatTarget = nullptr;
}

void AVesperCharacter::InitializeVesperOverlay()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController) return;

	AVesperHUD* VesperHUD = Cast<AVesperHUD>(PlayerController->GetHUD());
	if (!VesperHUD) return;

	VesperOverlay = VesperHUD->GetVesperOverlay();
	if (VesperOverlay && Attributes)
	{
		VesperOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
		VesperOverlay->SetStaminaBarPercent(1.f);
		VesperOverlay->SetGold(0);
		VesperOverlay->SetSouls(0);
	}
}

void AVesperCharacter::SetHUDHealth()
{
	if (VesperOverlay && Attributes)
	{
		VesperOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}

void AVesperCharacter::Dodge()
{
	if (bIsInMenu) return;
	if (IsOccupied() || !HasEnoughStamina()) return;

	PlayDodgeMontage();
	ActionState = EActionState::EAS_Dodge;
	if (Attributes && VesperOverlay)
	{
		Attributes->UseStamina(Attributes->GetDodgeCost());
		VesperOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
}

void AVesperCharacter::Jump()
{
	if (bIsInMenu) return;
	if (IsUnoccupied())
	{
		Super::Jump();
	}
}

bool AVesperCharacter::IsUnoccupied()
{
	return ActionState == EActionState::EAS_Unoccupied;
}

