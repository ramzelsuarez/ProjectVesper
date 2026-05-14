// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "VesperHUD.generated.h"


class UVesperOverlay;

UCLASS()
class PROJECTVESPER_API AVesperHUD : public AHUD
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY (EditDefaultsOnly, Category = Vesper)
	TSubclassOf<class UVesperOverlay> VesperOverlayClass;
	
	UPROPERTY()
	UVesperOverlay* VesperOverlay;
};
