// Copyright Epic Games, Inc. All Rights Reserved.

#include "DemoCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Demo/Public/DemoCharacterMovementComponent.h"


//////////////////////////////////////////////////////////////////////////
// ADemoCharacter

ADemoCharacter::ADemoCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UDemoCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{	

	DemoCharacterMovementComponent = Cast<UDemoCharacterMovementComponent>(GetCharacterMovement());
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character rotates in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate
	
	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	SpringArmComponent->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ADemoCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ADemoCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/** Scroll Zoom */
	float AdjustOffset = DemoCharacterMovementComponent->GetCrouchZoomOffset();
	DesiredArmLength = ScrollZoomOffset + AdjustOffset;
	// Делает отдаление камеры более гладким 
	SpringArmComponent->TargetArmLength = FMath::FInterpTo(SpringArmComponent->TargetArmLength, DesiredArmLength, DeltaTime, CameraZoomSpeed);	
	
}

/** Луч игнорирует коллизию с игроком */
FCollisionQueryParams ADemoCharacter::GetIgnoreCharacterParams()
{
	FCollisionQueryParams Params;

	TArray<AActor*> CharacterChildren;
	GetAllChildActors(CharacterChildren);
	Params.AddIgnoredActors(CharacterChildren);
	Params.AddIgnoredActor(this);

	return Params;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ADemoCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Sprinting/Walking 
		EnhancedInputComponent->BindAction(Sprint, ETriggerEvent::Triggered, DemoCharacterMovementComponent, &UDemoCharacterMovementComponent::SprintPressed);
		EnhancedInputComponent->BindAction(Sprint, ETriggerEvent::Completed, DemoCharacterMovementComponent, &UDemoCharacterMovementComponent::SprintReleased);
		EnhancedInputComponent->BindAction(Walk, ETriggerEvent::Triggered, DemoCharacterMovementComponent, &UDemoCharacterMovementComponent::WalkToggle);

		// Slide
		EnhancedInputComponent->BindAction(Slide, ETriggerEvent::Triggered, DemoCharacterMovementComponent, &UDemoCharacterMovementComponent::SlidePressed);
		EnhancedInputComponent->BindAction(Slide, ETriggerEvent::Completed, DemoCharacterMovementComponent, &UDemoCharacterMovementComponent::SlideReleased);
		
		// Crouch
		EnhancedInputComponent->BindAction(DemoCrouch, ETriggerEvent::Triggered, DemoCharacterMovementComponent, &UDemoCharacterMovementComponent::CrouchPressed);

		// Dash
		EnhancedInputComponent->BindAction(Dash, ETriggerEvent::Triggered, DemoCharacterMovementComponent, &UDemoCharacterMovementComponent::DashPressed);
		EnhancedInputComponent->BindAction(Dash, ETriggerEvent::Completed, DemoCharacterMovementComponent, &UDemoCharacterMovementComponent::DashReleased);
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ADemoCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ADemoCharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADemoCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADemoCharacter::Look);

		// Zoom
		//PlayerInputComponent->BindAxis("CameraZoom", this, &ADemoCharacter::CameraZoom);
		EnhancedInputComponent->BindAction(Zoom, ETriggerEvent::Triggered, this, &ADemoCharacter::AdjustScrollZoom);
	}

}

void ADemoCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}
void ADemoCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
		
		if (!GetVelocity().Length())
		{
			float TurnAxis = LookAxisVector.X;
			
			if (TurnAxis > .3f)
			{
				bTurnLeft = false;
				bTurnRight = true;
				UE_LOG(LogClass,Warning,TEXT("TurnRight"));
			} 
			else if (TurnAxis < -.3f)
			{
				bTurnRight = false;
				bTurnLeft = true;
				UE_LOG(LogClass,Warning,TEXT("TurnLeft"));
			}
			else
			{
				bTurnRight = false;
				bTurnLeft = false;
				UE_LOG(LogClass,Warning,TEXT("Not Turning"));
			}
		}

		/*float DeltaRotation = (GetControlRotation() - GetActorRotation()).Yaw;
		UE_LOG(LogClass,Warning,TEXT("DeltaRotation: %f"), DeltaRotation);

		if (DeltaRotation > 90.f)
		{
			bTurnLeft = false;
			bTurnRight = true;
			GetCharacterMovement()->bUseControllerDesiredRotation = true;
			GetCharacterMovement()->bOrientRotationToMovement = false;
		}
		else if (DeltaRotation < -90.f)
		{
			bTurnRight = false;
			bTurnLeft = true;
			GetCharacterMovement()->bUseControllerDesiredRotation = true;
			GetCharacterMovement()->bOrientRotationToMovement = false;
		}
		else if (DeltaRotation == 0)
		{
			bTurnRight = false;
			bTurnLeft = false;
			GetCharacterMovement()->bUseControllerDesiredRotation = false;
			GetCharacterMovement()->bOrientRotationToMovement = false;
		}*/
	}
}

void ADemoCharacter::Jump()
{
	DemoCharacterMovementComponent->bWantsToCrouch = false;
	bPressedDemoJump = true;
	
	Super::Jump();
	
	bPressedJump = false;
}
void ADemoCharacter::StopJumping()
{
	bPressedDemoJump = false;
	Super::StopJumping();
}

// Zoom 
void ADemoCharacter::AdjustScrollZoom(const FInputActionValue& value)
{	
	float ZoomValue = value.Get<float>();
	
	if (ZoomValue)
	{
		ScrollZoomOffset = FMath::Clamp(ZoomValue * ZoomStep + GetSpringArmComponent()->TargetArmLength, MinZoomDistance, MaxZoomDistance);
	}
		
}







