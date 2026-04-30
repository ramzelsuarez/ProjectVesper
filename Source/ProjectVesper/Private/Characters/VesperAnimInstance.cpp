// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/VesperAnimInstance.h"
#include "Characters/VesperCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UVesperAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	VesperCharacter = Cast<AVesperCharacter>(TryGetPawnOwner());
	if (VesperCharacter)
	{
		VesperCharacterMovement = VesperCharacter->GetCharacterMovement();
	}
}

void UVesperAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (VesperCharacterMovement)
	{
		GroundSpeed = UKismetMathLibrary::VSizeXY(VesperCharacterMovement->Velocity);
		IsFalling = VesperCharacterMovement->IsFalling();
		CharacterState = VesperCharacter->GetCharacterState();
	}
}
