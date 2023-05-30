// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DemoCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DemoCharacterMovementComponent.generated.h"

/**
 * 
 */

// Classes
class ADemoCharacter;
/*class UEnhancedInputComponent;*/
class USpringArmComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDashStartDelegate);

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None		UMETA(Hidden),
	CMOVE_Sprint	UMETA(DisplayName = "Sprint"),
	CMOVE_Walk		UMETA(DisplayName = "Walk"),
	CMOVE_Dash		UMETA(DisplayName = "Dash"),
	CMOVE_Slide		UMETA(DisplayName = "Slide"),
	CMOVE_MAX		UMETA(Hidden)
};

UCLASS()
class DEMO_API UDemoCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	UDemoCharacterMovementComponent();
	
	UPROPERTY(Transient) ADemoCharacter* DemoCharacterOwner;
	UPROPERTY(Transient) USpringArmComponent* SpringArmComponent;
	
// Client/Server
#pragma region Client/Server
	
	class FSavedMove_Demo : public FSavedMove_Character
	{
		typedef FSavedMove_Character Super;

	public:
		enum CompressedFlags
		{
			FLAG_Sprint		= 0x10,
			//FLAG_Walk		= 0x20,
			FLAG_Dash		= 0x40,
			FLAG_Slide		= 0x80
		};
		
		uint8 Saved_bWantsToSprint:1;
		uint8 Saved_bWantsToWalk:1;
		uint8 Saved_bPressedDemoJump:1;
		uint8 Saved_bPrevWantsToCrouch:1;

		uint8 Saved_bHadAnimRootMotion:1;
		uint8 Saved_bTransitionFinished:1;
		
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
		
	};
	
	class FNetworkPredictionData_Client_Demo : public FNetworkPredictionData_Client_Character
	{
		
	public:
		FNetworkPredictionData_Client_Demo(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;
	};

public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	
protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

#pragma endregion

// Movement Pipeline
#pragma region Movement Pipeline
	
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;
	
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

#pragma endregion

/** Helper functions */
#pragma region Helper functions
	
public:
	UFUNCTION(BlueprintPure) bool IsMovementMode(EMovementMode InMovementMode) const;
	UFUNCTION(BlueprintPure) bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;
	UFUNCTION(BlueprintPure) float GetCrouchZoomOffset() const;
	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;
	
	UCapsuleComponent* Cap() const;
	float CapHH() const;
	float CapR() const;
	
	
#pragma endregion 
	
// Parameters
#pragma region Params
	
protected:
	virtual void InitializeComponent() override;

	// Walk / Sprint
	UPROPERTY(EditDefaultsOnly, Category=Walk) float Walk_MaxSpeed = 200.f;
	UPROPERTY(EditDefaultsOnly, Category=Sprint) float Sprint_MaxSpeed = 750.f;
	float DefaultMoveSpeed = MaxWalkSpeed;
	float MoveSpeed = MaxWalkSpeed;
	
	// Crouch
	UPROPERTY(EditDefaultsOnly, Category=Crouch) float CrouchZoomOffset = 200.f;
	float AdjustCrouchZoom;
	
	// Dash
	UPROPERTY(EditDefaultsOnly, Category=Dash) float DashImpulse = 1000.f;
	UPROPERTY(EditDefaultsOnly, Category=Dash) float DashCooldownDuration = 1.f;
	UPROPERTY(EditDefaultsOnly, Category=Dash) float AuthDashCooldownDuration=.9f;
	UPROPERTY(EditDefaultsOnly, Category=Dash) UAnimMontage* DashMontage;
	UPROPERTY(EditDefaultsOnly, Category=Dash, meta=(ClampMin = "1")) uint8 DashMaxCount = 1;
	UPROPERTY(EditDefaultsOnly, Category=Dash) UAnimationAsset* DashAnimationAsset;
	
	uint8 DashCurrentCount = DashMaxCount;
	float DashStartTime;
	bool bDashing;
	
	// Slide
	UPROPERTY(EditDefaultsOnly, Category=Slide) float SlideImpulse = 500.f;
	UPROPERTY(EditDefaultsOnly, Category=Slide) float SlideMinSpeed = 350.f;
	UPROPERTY(EditDefaultsOnly, Category=Slide) float SlideMaxSpeed = 900.f;
	UPROPERTY(EditDefaultsOnly, Category=Slide) float SlideGravityForce = 5000.f;
	UPROPERTY(EditDefaultsOnly, Category=Slide) float SlideFriction = 1.3f;
	UPROPERTY(EditDefaultsOnly, Category=Slide) float SlideCooldownDuration = 1.f;
	
	/** Регулирует максимальный угол склона, по которому можно совершать слайд стоя к нему лицом:
	 *	 - 0 - 90, где 0 - Слайд только по абсолютно ровной поверхности, 90 - Cлайд вдоль отвесной стены. */
	UPROPERTY(EditDefaultsOnly, Category=Slide, meta=(ClampMin="0", ClampMax="90")) float SlideSlopeAngle = 45.f;

	/** Регулирует угол склона, после которого игрок принудительно войдет в слайд:
	 *	 - 0 - 90, где 0 - Принудительный слайд на практически ровной поверхности, 90 - Принудительный слайд только по отвесной стене. */
	UPROPERTY(EditDefaultsOnly, Category=Slide, meta=(ClampMin="0", ClampMax="90")) float SlideMinSurfaceAngle = 45.f;
	
	// Mantle
	UPROPERTY(EditDefaultsOnly, Category=Mantle) float MantleCheckDistance = 200;
	UPROPERTY(EditDefaultsOnly, Category=Mantle) float MantleReachHeight = 50;
	UPROPERTY(EditDefaultsOnly, Category=Mantle) float MinMantleDepth = 30;
	UPROPERTY(EditDefaultsOnly, Category=Mantle) float MantleMinWallSteepnessAngle = 45;
	UPROPERTY(EditDefaultsOnly, Category=Mantle) float MantleMaxAlignmentAngle = 45;
	UPROPERTY(EditDefaultsOnly, Category=Mantle) float MantleMaxSurfaceAngle = 45;
	
	UPROPERTY(EditDefaultsOnly, Category=Mantle) UAnimMontage* TallMantleMontage;
	UPROPERTY(EditDefaultsOnly, Category=Mantle) UAnimMontage* TransitionTallMantleMontage;
	UPROPERTY(EditDefaultsOnly, Category=Mantle) UAnimMontage* ProxyTallMantleMontage;
	
	UPROPERTY(EditDefaultsOnly, Category=Mantle) UAnimMontage* ShortMantleMontage;
	UPROPERTY(EditDefaultsOnly, Category=Mantle) UAnimMontage* TransitionShortMantleMontage;
	UPROPERTY(EditDefaultsOnly, Category=Mantle) UAnimMontage* ProxyShortMantleMontage;
	
	bool bTallMantle;
	bool Safe_bTransitionFinished;
	
	TSharedPtr<FRootMotionSource_MoveToForce> TransitionRMS;
	UPROPERTY(Transient) UAnimMontage* TransitionQueuedMontage;
	
	// Lean
	UPROPERTY(EditDefaultsOnly, Category=Lean) float LeanMultiplier = 0.f;
	UPROPERTY(EditDefaultsOnly, Category=Lean) float LeanSpeed = 10.f;
	
	float TransitionQueuedMontageSpeed;
	int TransitionRMS_ID;
	
	// Safe Flags
	bool Safe_bWantsToSprint;
	bool Safe_bWantsToWalk;
	bool Safe_bWantsToSlide;
	bool Safe_bWantsToDash;

	bool Safe_bHadAnimRootMotion;
	bool Safe_bPrevWantsToCrouch;
	
	// Timers
	FTimerHandle TimerHandle_SlideCooldown;
	FTimerHandle TimerHandle_DashCooldown;
	
	// Replication
	UPROPERTY(ReplicatedUsing=OnRep_DashStart)   bool Proxy_bDashStart;
	UPROPERTY(ReplicatedUsing=OnRep_ShortMantle) bool Proxy_bShortMantle;
	UPROPERTY(ReplicatedUsing=OnRep_TallMantle)  bool Proxy_bTallMantle;

public:

	// Delegates;
	FDashStartDelegate* DashStartDelegate;

#pragma endregion


// Mantle
#pragma region Mantle
	bool TryMantle();
	FVector GetMantleStartLocation(FHitResult FrontHit, FHitResult SurfaceHit, bool bTallMantle) const;

#pragma endregion 
	
// Input Interface
#pragma region Input Interface 

	public:
	// Sprint/Walk 
		UFUNCTION(BlueprintCallable) void SprintPressed();
		UFUNCTION(BlueprintCallable) void SprintReleased();
		UFUNCTION(BlueprintCallable) void WalkToggle();
	
		bool CanSprint();
		
	// Crouch 
		UFUNCTION(BlueprintCallable) void CrouchPressed();
		
	// Dash 
	#pragma region Dash
	
	UFUNCTION(BlueprintCallable) void DashPressed();
	UFUNCTION(BlueprintCallable) void DashReleased();

	/*UFUNCTION(BlueprintPure) bool IsDashing() const; 
	UFUNCTION(BlueprintPure) int GetDashCount() const {return DashCurrentCount;}*/
	
	bool IsCanDash() const;
	void OnDashCooldown();
	void PerformDash();

	#pragma endregion
	
	// Slide 
	#pragma region Slide
		UFUNCTION(BlueprintCallable) void SlidePressed();
		UFUNCTION(BlueprintCallable) void SlideReleased();
	
		float SlideExitTime;
		
	public:
		void EnterSlide();
	
	private:
		void ExitSlide();
		void PhysSlide(float deltaTime, int32 Iterations);
		bool GetStandOnSurface(FHitResult& Hit);
		
	
	#pragma endregion
	
		
		// Lean 
		FRotator LastFrameRotation;
		float LeanAngle;
		UFUNCTION(BlueprintPure) float GetLeanAngle(float DeltaSeconds, FRotator& LastFrameRotator);
		
	#pragma endregion

// Proxy Replication
#pragma region Proxy Replication
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
private:
	UFUNCTION() void OnRep_DashStart();
	UFUNCTION() void OnRep_ShortMantle();
	UFUNCTION() void OnRep_TallMantle();

#pragma endregion 
	
};
