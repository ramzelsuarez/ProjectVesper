// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VesperCharacter.generated.h"

UCLASS()
class PROJECTVESPER_API AVesperCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AVesperCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

};
