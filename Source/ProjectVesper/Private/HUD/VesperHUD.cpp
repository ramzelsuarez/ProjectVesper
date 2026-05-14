// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/VesperHUD.h"
#include "HUD/VesperOverlay.h"

void AVesperHUD::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* Controller = World->GetFirstPlayerController();
		if (Controller && VesperOverlayClass)
		{
			VesperOverlay = CreateWidget<UVesperOverlay>(Controller, VesperOverlayClass);
			VesperOverlay->AddToViewport();
		}
	}
}
