// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Weapon.h"
#include "Characters/VesperCharacter.h"

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AVesperCharacter* VesperCharacter = Cast<AVesperCharacter>(OtherActor);
	if (VesperCharacter)
	{
		FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
		ItemMesh->AttachToComponent(VesperCharacter->GetMesh(), TransformRules, FName("RightHandSocket"));
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnSphereEndOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}
