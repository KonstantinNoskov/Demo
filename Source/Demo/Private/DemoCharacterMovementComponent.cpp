// Fill out your copyright notice in the Description page of Project Settings.


#include "Demo/Public/DemoCharacterMovementComponent.h"

#include "Demo/Public/DemoCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"


// Debug Helper Macro
#if 1
float MacroDuration = 2.f;
#define SLOG(x, c) GEngine->AddOnScreenDebugMessage(-1, MacroDuration ? MacroDuration : -1.f, FColor::c, FString::x);
#define FLOG(x, c) GEngine->AddOnScreenDebugMessage(-1, MacroDuration ? MacroDuration : -1.f, FColor::c, FString::SanitizeFloat(x));
#define POINT(x, c) DrawDebugPoint(GetWorld(), x, 10, FColor::c, !MacroDuration, MacroDuration);
#define LINE(x1, x2, c) DrawDebugLine(GetWorld(), x1, x2, FColor::c, !MacroDuration, MacroDuration);
#define CAPSULE(x, c) DrawDebugCapsule(GetWorld(), x, CapHH(), CapR(), FQuat::Identity, FColor::c, !MacroDuration, MacroDuration);

#else
#define SLOG(x, c)
#define FLOG(x, c)
#define POINT(x, c)
#define LINE(x1, x2, c)
#define CAPSULE(x, c)

#endif

UDemoCharacterMovementComponent::UDemoCharacterMovementComponent()
{
	NavAgentProps.bCanCrouch = true;
}

void UDemoCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	DemoCharacterOwner = Cast<ADemoCharacter>(GetOwner());
	SpringArmComponent = Cast<USpringArmComponent>(DemoCharacterOwner->GetSpringArmComponent());
	
	//DashStartDelegate->BindUObject(this, &UDemo3MovementComponent::PerformDash);
};

/** Client/Server setups */
#pragma region Client/Server

	bool UDemoCharacterMovementComponent::FSavedMove_Demo::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
	{
		FSavedMove_Demo* NewDemo3Move = static_cast<FSavedMove_Demo*>(NewMove.Get());
	
		if (Saved_bWantsToSprint != NewDemo3Move->Saved_bWantsToSprint) return false;
		if (Saved_bWantsToWalk != NewDemo3Move->Saved_bWantsToWalk)		return false;
		/*if (Saved_bWantsToDash != NewDemo3Move->Saved_bWantsToDash)		return false;
		if (Saved_bWantsToSlide != NewDemo3Move->Saved_bWantsToSlide)	return false;*/
		
		return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
	}
	void UDemoCharacterMovementComponent::FSavedMove_Demo::Clear()
	{
		FSavedMove_Character::Clear();
	
		Saved_bWantsToSprint	 = 0;
		Saved_bWantsToWalk		 = 0;
		/*Saved_bWantsToDash		 = 0;
		Saved_bWantsToSlide		 = 0;*/
		Saved_bPressedDemoJump = 0;
	}
	uint8 UDemoCharacterMovementComponent::FSavedMove_Demo::GetCompressedFlags() const
	{
		uint8 Result = Super::GetCompressedFlags();
	
		if (Saved_bWantsToSprint)		Result	|= FLAG_Sprint;
		//if (Saved_bWantsToWalk )		Result	|= FLAG_Walk;
		//if (Saved_bWantsToDash )		Result	|= FLAG_Dash;
		//f (Saved_bWantsToSlide )		Result	|= FLAG_Slide;
		if (Saved_bPressedDemoJump )	Result	|= FLAG_JumpPressed;
		
		
		return Result;
	}
	
	void UDemoCharacterMovementComponent::FSavedMove_Demo::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
	{
		FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);
	
		UDemoCharacterMovementComponent* DemoMovementComponent = Cast<UDemoCharacterMovementComponent>(C->GetCharacterMovement());
	
		Saved_bWantsToSprint = DemoMovementComponent->Safe_bWantsToSprint;
		Saved_bWantsToWalk = DemoMovementComponent->Safe_bWantsToWalk;
		Saved_bPrevWantsToCrouch = DemoMovementComponent->Safe_bPrevWantsToCrouch;
		Saved_bPressedDemoJump = DemoMovementComponent->DemoCharacterOwner->bPressedDemoJump;
		
		//Saved_bWantsToWalk = DemoMovementComponent->Safe_bWantsToWalk;
		//Saved_bWantsToDash = DemoMovementComponent->Safe_bWantsToDash;
		//Saved_bWantsToSlide = DemoMovementComponent->Safe_bWantsToSlide;
	
		Saved_bPressedDemoJump = DemoMovementComponent->DemoCharacterOwner->bPressedDemoJump;
		Saved_bHadAnimRootMotion = DemoMovementComponent->Safe_bHadAnimRootMotion;
		Saved_bTransitionFinished = DemoMovementComponent->Safe_bTransitionFinished;
		
	}
	void UDemoCharacterMovementComponent::FSavedMove_Demo::PrepMoveFor(ACharacter* C)
	{
		FSavedMove_Character::PrepMoveFor(C);
	
		UDemoCharacterMovementComponent* DemoMovementComponent = Cast<UDemoCharacterMovementComponent>(C->GetCharacterMovement());
	
		DemoMovementComponent->Safe_bWantsToSprint = Saved_bWantsToSprint;
		DemoMovementComponent->Safe_bWantsToWalk = Saved_bWantsToWalk;
		DemoMovementComponent->Safe_bPrevWantsToCrouch =Saved_bPrevWantsToCrouch;
		
		//Demo3MovementComponent->Safe_bWantsToDash = Saved_bWantsToDash;
		//Demo3MovementComponent->Safe_bWantsToSlide = Saved_bWantsToSlide;
		
		DemoMovementComponent->DemoCharacterOwner->bPressedDemoJump = Saved_bPressedDemoJump; 
		DemoMovementComponent->Safe_bHadAnimRootMotion = Saved_bHadAnimRootMotion;
		DemoMovementComponent->Safe_bTransitionFinished = Saved_bTransitionFinished;
		
	}
	UDemoCharacterMovementComponent::FNetworkPredictionData_Client_Demo::FNetworkPredictionData_Client_Demo(const UCharacterMovementComponent& ClientMovement)
		: Super(ClientMovement)
	{
		
	}
	FSavedMovePtr UDemoCharacterMovementComponent::FNetworkPredictionData_Client_Demo::AllocateNewMove()
	{
		return FSavedMovePtr(new FSavedMove_Demo());
	}
	
	FNetworkPredictionData_Client* UDemoCharacterMovementComponent::GetPredictionData_Client() const
	{
		check(PawnOwner != nullptr)
	
		if (ClientPredictionData == nullptr)
		{
			UDemoCharacterMovementComponent* MutableThis = const_cast<UDemoCharacterMovementComponent*>(this);
	
			MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Demo(*this);
			MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
			MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
		}
		return ClientPredictionData;
	}
	
	// Переворачиваем флаги наших кастомных MovementMode
	void UDemoCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
	{
		Super::UpdateFromCompressedFlags(Flags);
	
		Safe_bWantsToSprint = (Flags & FSavedMove_Demo::FLAG_Sprint)	!= 0;
		//Safe_bWantsToWalk	= (Flags & FSavedMove_Demo::FLAG_Walk)	!= 0;
		/*Safe_bWantsToDash	= (Flags & FSavedMove_Demo::FLAG_Dash)		!= 0;
		Safe_bWantsToSlide	= (Flags & FSavedMove_Demo::FLAG_Slide)	!= 0;*/
	}

#pragma endregion

/** Movement Pipeline */
#pragma region  Movement Pipeline

	void UDemoCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation,
		const FVector& OldVelocity)
	{	
		Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
		
		if (MovementMode == MOVE_Walking)
		{
			// Sprint / Walk
			if (Safe_bWantsToSprint && CanSprint())
			{
				MaxWalkSpeed = Sprint_MaxSpeed;
			}
			else if (Safe_bWantsToWalk)
			{
				MaxWalkSpeed = Walk_MaxSpeed; 
			}
			else
			{
				MaxWalkSpeed = DefaultMoveSpeed;
			}
	
			// Dash
			DashCurrentCount = DashMaxCount;
		}
	}
	
	void UDemoCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode,
		uint8 PreviousCustomMode)
	{
		Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	
		/** Slide */
		if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Slide)
		{
			ExitSlide();	
		}
		if (IsCustomMovementMode(CMOVE_Slide))
		{
			EnterSlide();	
		}
		
		/** Dash */
		// Count reset. Uncomment if you want to use root-motion Dash. 
		/*if (MovementMode == MOVE_Walking)
		{
			DashCurrentCount = DashMaxCount;
		}*/
	}
	
	void UDemoCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
	{
		// Crouch Zoom
		AdjustCrouchZoom = IsCrouching() && !IsCustomMovementMode(CMOVE_Slide) && Velocity.Length() > 0 ? CrouchZoomOffset : 0;
		
		// Slide
		if (MovementMode == MOVE_Walking)
		{
			FHitResult PotentialSurface;
			
			if (/*Velocity.SizeSquared() > pow(SlideMinSpeed, 2) &&*/ GetStandOnSurface(PotentialSurface))
			{
				// Проверка крутизны склона. Запрещаем делать слайд вверх по склону.
				bool bUpSlope;
				float UpSlopeCurrentCos = PotentialSurface.Normal | UpdatedComponent->GetForwardVector().GetSafeNormal();
				float UpSlopeTresholdCos = FMath::Cos(FMath::DegreesToRadians(SlideSlopeAngle + 90.f));
				bUpSlope = UpSlopeCurrentCos < UpSlopeTresholdCos ? true : false;
	
				// Проверка стоит ли персонаж на ступеньках.
				FHitResult StairsHit;
				FVector StairsHitStart = UpdatedComponent->GetComponentLocation() + FVector::DownVector * (CapHH() - 10.f);
				FVector StairsHitEnd = StairsHitStart + UpdatedComponent->GetForwardVector() * CapR() * 2;
				FName ProfileName = TEXT("BlockAll");
				
				GetWorld()->LineTraceSingleByProfile(StairsHit, StairsHitStart, StairsHitEnd, ProfileName, DemoCharacterOwner->GetIgnoreCharacterParams());
				bool bOnStairs = (StairsHit.Normal | StairsHitEnd.GetSafeNormal2D()) >= 0 ? false : true;
				
				LINE(StairsHitStart, StairsHitEnd, Orange) // <-- Debug LineTrace 
				
				/** Если находимся на земле, спринтуем, позволяет таймер на слайд, не стоим на ступеньках и нажимаем клавишу слайда,
				 * то запускаем слайд.
				 */
				if(IsMovingOnGround() && Safe_bWantsToSlide && Safe_bWantsToSprint && GetWorld()->GetTimeSeconds() - SlideExitTime >= SlideCooldownDuration
				&& !bOnStairs)
				{
					EnterSlide();
				}
				
				/** Также, если склон достаточно крутой, входим слайд принудительно. */
				float CurrentSurfaceAngle = FVector::UpVector | PotentialSurface.Normal;
				if (CurrentSurfaceAngle < FMath::Cos(FMath::DegreesToRadians(SlideMinSurfaceAngle)) && !bUpSlope)
				{	
					EnterSlide();
				}
				
			}
		}
		
		if (IsCustomMovementMode(CMOVE_Slide) && !bWantsToCrouch)
		{
			ExitSlide();
		}
		
		// Dash
		bool bAuthProxy = CharacterOwner->HasAuthority() && !CharacterOwner->IsLocallyControlled();
		if (Safe_bWantsToDash && IsCanDash())
		{
			if (!bAuthProxy || GetWorld()->GetTimeSeconds() - DashStartTime > AuthDashCooldownDuration)
			{
				PerformDash();
				Safe_bWantsToDash = false;
				bDashing = true;
	
				//Proxy_bDashStart = true;
				//Proxy_bDashStart = !Proxy_bDashStart;
			}
		}
		
		// Lean
		//GetLeanAngle(DeltaSeconds);

		// Try to Mantle
		if (DemoCharacterOwner->bPressedDemoJump)
		{
			if(TryMantle())
			{
				DemoCharacterOwner->StopJumping();
			}
			else
			{
				SLOG(Printf(TEXT("Failed Mantle, Reverting to jump")), Yellow)
				DemoCharacterOwner->bPressedDemoJump = false;
				CharacterOwner->bPressedJump = true;
				CharacterOwner->CheckJumpInput(DeltaSeconds);
				bOrientRotationToMovement = true;
			}
		}
		
		Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
	}
	void UDemoCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
	{
		Super::PhysCustom(deltaTime, Iterations);
		
		switch (CustomMovementMode)
		{
		case CMOVE_Slide:
			PhysSlide(deltaTime,Iterations);
			Acceleration = FVector::ZeroVector; // <-- Костыль устраняет глич анимации при ускорении в разных направлениях во время слайда
			break;
		default:
			UE_LOG(LogClass, Warning, TEXT("Invalid Movement Mode!"));		
		}
	}
	void UDemoCharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
	{
		Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
	}

#pragma endregion

// Helper functions 
#pragma region Helper functions

	bool UDemoCharacterMovementComponent::IsMovementMode(EMovementMode InMovementMode) const
	{
		return MovementMode == InMovementMode;
	}
	bool UDemoCharacterMovementComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
	{
		return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
	}
	bool UDemoCharacterMovementComponent::IsMovingOnGround() const
	{
		return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Slide);
	}
	bool UDemoCharacterMovementComponent::CanCrouchInCurrentState() const
	{
		return Super::CanCrouchInCurrentState() && IsMovingOnGround();
	}
	bool UDemoCharacterMovementComponent::GetStandOnSurface(FHitResult& Hit)
	{
		FVector LineTraceStart = UpdatedComponent->GetComponentLocation();
		FVector LineTraceEnd = LineTraceStart + CapHH() * 3.f * FVector::DownVector;
		FName ProfileName = TEXT("BlockAll");
		
		LINE(LineTraceStart, LineTraceEnd, Red) 
		
		return GetWorld()->LineTraceSingleByProfile(Hit, LineTraceStart, LineTraceEnd, ProfileName, DemoCharacterOwner->GetIgnoreCharacterParams());
	}

	float UDemoCharacterMovementComponent::GetCrouchZoomOffset() const
	{
		return AdjustCrouchZoom;
	}
	float UDemoCharacterMovementComponent::CapHH() const 
		{
			return DemoCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		}
	float UDemoCharacterMovementComponent::CapR() const 
		{
			return DemoCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
		}
	float UDemoCharacterMovementComponent::GetLeanAngle(float DeltaTime, FRotator& LastFrameRotator)
	{
		FRotator ActorRotator = DemoCharacterOwner->GetActorRotation();
		LeanAngle = FMath::FInterpTo(LeanAngle, (LastFrameRotation - ActorRotator).Yaw / DeltaTime / LeanMultiplier,
			DeltaTime, LeanSpeed) * -1.f;
		
		LastFrameRotator = ActorRotator;
		
		return LeanAngle;
	}



#pragma endregion

// Input Interface 
#pragma region Input Interface

	// Sprint/Walk 
	void UDemoCharacterMovementComponent::SprintPressed()
	{
		Safe_bWantsToSprint = true;
	}
	void UDemoCharacterMovementComponent::SprintReleased()
	{	
		Safe_bWantsToSprint = false;
	}
	void UDemoCharacterMovementComponent::WalkToggle()
	{
		Safe_bWantsToWalk = !Safe_bWantsToWalk;
		//SLOG(FString::Printf(TEXT("WantsToWalk: %hhd"), Safe_bWantsToWalk), Green);
	}
	bool UDemoCharacterMovementComponent::CanSprint()
{
		return MovementMode == MOVE_Walking && (UpdatedComponent->GetForwardVector() | Velocity.GetSafeNormal2D()) > 0;
}

	// Crouch  
	void UDemoCharacterMovementComponent::CrouchPressed()
	{
		bWantsToCrouch = IsWalking() ? bWantsToCrouch = !bWantsToCrouch : false;
	}
	
	// Slide 
	#pragma region Slide
	
	void UDemoCharacterMovementComponent::SlidePressed()
	{
		Safe_bWantsToSlide = true;
	}
	void UDemoCharacterMovementComponent::SlideReleased()
	{
		Safe_bWantsToSlide = false;
	}
	
	void UDemoCharacterMovementComponent::EnterSlide() 
	{
		bWantsToCrouch = true;
		Velocity += Velocity.GetSafeNormal() * SlideImpulse + (FVector::DownVector * (SlideImpulse / 1.9));
		SetMovementMode(MOVE_Custom, CMOVE_Slide);
	
		//SLOG(FString::Printf(TEXT("EnterSlide!")), Yellow);
	}
	void UDemoCharacterMovementComponent::ExitSlide()
	{
		bWantsToCrouch = false;
		Safe_bWantsToSlide = false;
		
		// Поворачиваем капсуль прямо перед собой по завершении слайда
		FQuat NewRotation = FRotationMatrix::MakeFromXZ(UpdatedComponent->GetForwardVector().GetSafeNormal2D(), FVector::UpVector).ToQuat();
		FHitResult Hit;
		SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, true, Hit);
	
		// Возвращаемся в режим Walking по завершении слайд
		SetMovementMode(MOVE_Walking);
	
		// Сохраняем время окончания слайда для таймера, предотвращающего слишком частые слайды.
		SlideExitTime = GetWorld()->GetTimeSeconds();
	}
	
	void UDemoCharacterMovementComponent::PhysSlide(float deltaTime, int32 Iterations)
	{
		if (deltaTime < MIN_TICK_TIME) { return; }
	
		RestorePreAdditiveRootMotionVelocity();
		
		FHitResult SurfaceHit;
		 
		if (!GetStandOnSurface(SurfaceHit) || Velocity.SizeSquared() < pow(SlideMinSpeed, 2))
		{
			ExitSlide();
			StartNewPhysics(deltaTime, Iterations);
			return;
		}
	
		// Surface Gravity
		Velocity += SlideGravityForce * FVector::DownVector * deltaTime;
	
		// Strafe
		if (FMath::Abs(FVector::DotProduct(Acceleration.GetSafeNormal(), UpdatedComponent->GetRightVector())) > .5)
		{
			Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector());
		}
		else
		{
			Acceleration = FVector::ZeroVector;
		}
	
		// Calc Velocity
		if(!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
		{
			CalcVelocity(deltaTime, SlideFriction, true, GetMaxBrakingDeceleration());
		}
	
		// Perform Move
		Iterations++;
		bJustTeleported = false;
	
		FVector OldLocation = UpdatedComponent->GetComponentLocation();
		FQuat OldRotation = UpdatedComponent->GetComponentRotation().Quaternion();
		FHitResult Hit(1.f);
		FVector Adjusted = Velocity * deltaTime;
		FVector VelPlaneDir = FVector::VectorPlaneProject(Velocity, SurfaceHit.Normal).GetSafeNormal();
		FQuat NewRotation = FRotationMatrix::MakeFromXZ(VelPlaneDir, SurfaceHit.Normal).ToQuat();
		SafeMoveUpdatedComponent(Adjusted, NewRotation, true, Hit);
		
		// Скользим вдоль стенки
		if (Hit.Time < 1.f)
		{
			HandleImpact(Hit, deltaTime, Adjusted);
			SlideAlongSurface(Adjusted,(1.f - Hit.Time), Hit.Normal, Hit, true);
		}
	
		FHitResult NewSurfaceHit;
		if (!GetStandOnSurface(NewSurfaceHit) || Velocity.SizeSquared() < pow(SlideMinSpeed, 2))
		{
			ExitSlide();
		}
	
		// Update Outgoing Velocity & Acceleration
		if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
		{
			Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime; //v = ds / dt
		}
		
	}
	
	#pragma endregion
	
	// Dash 
	#pragma region Dash
	
	void UDemoCharacterMovementComponent::DashPressed()
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime - DashStartTime >= DashCooldownDuration)
		{
			Safe_bWantsToDash = true; 
		}
		else 
		{
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_DashCooldown, this, &UDemoCharacterMovementComponent::OnDashCooldown,
			DashCooldownDuration - (CurrentTime - DashStartTime));
		}
	}
	void UDemoCharacterMovementComponent::DashReleased()
	{
		Safe_bWantsToDash = false;
	}
	
	bool UDemoCharacterMovementComponent::IsCanDash() const
	{
		return DashCurrentCount && (IsWalking() && !IsCrouching() || IsFalling());
	}
	void UDemoCharacterMovementComponent::OnDashCooldown()
	{
		Safe_bWantsToDash = true;
	}
	
	void UDemoCharacterMovementComponent::PerformDash()
	{
		bDashing = true;
		DashCurrentCount -= DashCurrentCount ? 1 : 0;
		
		DashStartTime = GetWorld()->GetTimeSeconds();
		
		//Phys implemintation for non-rootmotion only.
		FVector DashDirection = (Acceleration.IsNearlyZero() ? UpdatedComponent->GetForwardVector() : Acceleration).GetSafeNormal2D();
		Velocity += DashImpulse * (DashDirection + FVector::UpVector * .1f);
	
		FQuat NewRotation = FRotationMatrix::MakeFromXZ(DashDirection, FVector::UpVector).ToQuat();
		FHitResult Hit;
		SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, false, Hit);
		
		/*SetMovementMode(MOVE_Flying);
		CharacterOwner->PlayAnimMontage(DashMontage);
		//DashStartDelegate.Broadcast();*/
		
		UE_LOG(LogClass, Warning, TEXT("PerformDash!"));
	}
	
	#pragma endregion

	// Mantle
	#pragma region Mantle

	bool UDemoCharacterMovementComponent::TryMantle()
	{
		
		SLOG(Printf(TEXT("Tried to Mantle")), Yellow)
		
		if (!(IsMovementMode(MOVE_Walking) && !IsCrouching()) && !IsMovementMode(MOVE_Falling)) return false;

		// Helper Variables
		FVector BaseLoc = UpdatedComponent->GetComponentLocation() + FVector::DownVector * CapHH();
		FVector Fwd = UpdatedComponent->GetForwardVector().GetSafeNormal2D();
		auto Params = DemoCharacterOwner->GetIgnoreCharacterParams();
		float MaxHeight = CapHH() * 2 + MantleReachHeight;
	
		float CosFrontSlope = FMath::Cos(FMath::DegreesToRadians(MantleMinWallSteepnessAngle));
		float CosFrontAngle = FMath::Cos(FMath::DegreesToRadians(MantleMaxAlignmentAngle));
		float CosSurfaceAngle = FMath::Cos(FMath::DegreesToRadians(MantleMaxSurfaceAngle));


		SLOG(Printf(TEXT("Starting Mantle Attempt")), Yellow)


		// Check Front Face 
	FHitResult FrontHit;
		float CheckDistance = FMath::Clamp(Velocity | Fwd, CapR() + 30, MantleCheckDistance);
		FVector FrontStart = BaseLoc + FVector::UpVector * (MaxStepHeight - 1);

		for (int i = 0; i < 6; i++)
		{
			LINE(FrontStart, FrontStart + Fwd * CheckDistance, Red)
			if (GetWorld()->LineTraceSingleByProfile(FrontHit, FrontStart, FrontStart + Fwd * CheckDistance, "BlockAll", Params)) break;
			FrontStart += FVector::UpVector * (2.f * CapHH() - (MaxStepHeight - 1)) / 5;
		
		}
		if (!FrontHit.IsValidBlockingHit()) return false;
	
		float CosWallSteepnessAngle = FrontHit.Normal | FVector::UpVector;
	
		if (FMath::Abs(CosWallSteepnessAngle) > CosFrontSlope || (Fwd | -FrontHit.Normal) < CosFrontAngle)
		{
		
			/*FLOG(FMath::Abs(CosWallSteepnessAngle), Red)
			SLOG("Current COS:", Red)
	
			FLOG(CosFrontSlope, Purple)
			SLOG("Treshold COS:", Purple)*/
		
			return false;
		}
		else
		{
			POINT(FrontHit.Location, Green)
			LINE(FrontStart, FrontStart + Fwd * CheckDistance, Green)

			/*FLOG(CosFrontSlope, Purple)
			SLOG("Treshold COS:", Purple)
			
			FLOG(CosWallSteepnessAngle, Green)
			SLOG("Current COS:", Green)*/
		
		}

		
		return false;
	}
	
	FVector UDemoCharacterMovementComponent::GetMantleStartLocation(FHitResult FrontHit, FHitResult SurfaceHit, bool bTallMantle) const
	{
		return FVector::ZeroVector;
	}

	#pragma endregion

	

#pragma endregion

// Proxy Replication
#pragma region Proxy Replication

void UDemoCharacterMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	}

void UDemoCharacterMovementComponent::OnRep_DashStart()
	{
	}
	
void UDemoCharacterMovementComponent::OnRep_ShortMantle()
	{
	}
	
void UDemoCharacterMovementComponent::OnRep_TallMantle()
	{
	}

#pragma endregion 


