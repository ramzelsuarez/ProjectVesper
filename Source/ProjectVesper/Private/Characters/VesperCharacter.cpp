// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/VesperCharacter.h"

AVesperCharacter::AVesperCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AVesperCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AVesperCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AVesperCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

