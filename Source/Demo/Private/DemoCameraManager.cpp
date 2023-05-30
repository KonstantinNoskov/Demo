// Fill out your copyright notice in the Description page of Project Settings.


#include "DemoCameraManager.h"
#include "DemoCharacter.h"
#include "DemoCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

ADemoCameraManager::ADemoCameraManager()
{
}

void ADemoCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	Super::UpdateViewTarget(OutVT, DeltaTime);
	
	// Crouch Camera
	if (ADemoCharacter* DemoCharacter = Cast<ADemoCharacter>(GetOwningPlayerController()->GetPawn()))
	{
		UDemoCharacterMovementComponent* DMC = DemoCharacter->GetDemoCMC();
		FVector TargetCrouchOffset = FVector(
			0,
			0,
			DMC->GetCrouchedHalfHeight() - DemoCharacter->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

		FVector Offset = FMath::Lerp(FVector::ZeroVector, TargetCrouchOffset, FMath::Clamp(CrouchBlendTime/CrouchBlendDuration, 0.f, 1.f));

		if (DMC->IsCrouching())
		{
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime + DeltaTime, 0.f, CrouchBlendDuration);
			Offset -= TargetCrouchOffset;
		}
		else
		{
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime - DeltaTime, 0.f, CrouchBlendDuration);
		}

		if (DMC->IsMovingOnGround())
		{
			OutVT.POV.Location += Offset;
		}
	}
}