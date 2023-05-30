// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "DemoCharacter.generated.h"

/** Classes */
class UDemoCharacterMovementComponent;

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

UCLASS(config=Game)
class ADemoCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	/** Movement Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = MovementComponent, meta = (AllowPrivateAccess = "true"))
	UDemoCharacterMovementComponent* DemoCharacterMovementComponent;
	
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArmComponent;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

/** Inputs */
#pragma region Inputs
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Camera Zoom */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Zoom;

	/** Sprint/Walk */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Sprint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Walk;

	/** Crouch */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DemoCrouch;
	
	/** Slide */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Slide;

	/** Dash */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Dash;

#pragma endregion

/** Parameters */
#pragma region  Params

	// Camera Zoom
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=CameraZoom) float ZoomStep = 200.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=CameraZoom) float MaxZoomDistance = 1000.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=CameraZoom) float MinZoomDistance = 150.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=CameraZoom) float CameraZoomSpeed = 2.5f;
	
	float DesiredArmLength;
	float ScrollZoomOffset = 300.f;

#pragma endregion

	
public:
	ADemoCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);
	
	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	// Camera Zoom
	void AdjustScrollZoom(const FInputActionValue& value);

public:
	virtual void Jump() override;
	virtual void StopJumping() override;

protected:
	
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();
	virtual void Tick(float DeltaTime) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetSpringArmComponent() const { return SpringArmComponent; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** Returns MovementComponent subobject **/
	FORCEINLINE UDemoCharacterMovementComponent* GetDemoCMC() const { return DemoCharacterMovementComponent; }

	UPROPERTY(BlueprintReadOnly, Category = Input)
	bool bTurnRight;
	UPROPERTY(BlueprintReadOnly, Category = Input)
	bool bTurnLeft;

	bool bPressedDemoJump;
	
	FCollisionQueryParams GetIgnoreCharacterParams();

	
	
};