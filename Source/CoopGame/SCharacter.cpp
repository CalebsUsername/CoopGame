// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SWeapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "CoopGame.h"
#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"


// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	XValueSensitivity = 30;
	YValueSensitivity = 30;

	CameraAttachSocketName = TEXT("CameraSocket");
	WeaponAttachSocketName = TEXT("WeaponSocket");

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, CameraAttachSocketName);

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComponent"));

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComp->SetupAttachment(CameraBoom);
	CameraComp->bConstrainAspectRatio = true;

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	ADSightFOV = 65.f;
	AimDownSightsSpeed = 20.f;
	IsShooting = false;
}


// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// GetMesh()->HideBoneByName(TEXT("weapon_r"), EPhysBodyOp::PBO_None);
	DefaultFOV = CameraComp->FieldOfView;
	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);

	if(HasAuthority())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (StarterWeaponClass)
		{
			CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if(CurrentWeapon)
			{
				CurrentWeapon->SetOwner(this);
				CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
			}
		}
	}
}


// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float TargetFOV = bIsADSight ? ADSightFOV : DefaultFOV;
	float CurrentFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, AimDownSightsSpeed);

	CameraComp->SetFieldOfView(CurrentFOV);
}


// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookRight", this, &ASCharacter::LookRight);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::ToggleCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);

	PlayerInputComponent->BindAction("Reload", IE_Released, this, &ASCharacter::Reload);

	PlayerInputComponent->BindAction("AimDownSights", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("AimDownSights", IE_Released, this, &ASCharacter::EndZoom);
}


// Character Control
void ASCharacter::MoveForward(float Value) 
{
	AddMovementInput(GetActorForwardVector() * Value);
}


void ASCharacter::MoveRight(float Value) 
{
	AddMovementInput(GetActorRightVector() * Value);
}


void ASCharacter::LookUp(float AxisValue) 
{
	AddControllerPitchInput(AxisValue * YValueSensitivity * GetWorld()->GetDeltaSeconds());
}


void ASCharacter::LookRight(float AxisValue) 
{
	AddControllerYawInput(AxisValue * XValueSensitivity * GetWorld()->GetDeltaSeconds());
}


void ASCharacter::ToggleCrouch() 
{
	if (GetCharacterMovement())
	{
		if (GetCharacterMovement()->bWantsToCrouch == true)
		{
			
			GetCharacterMovement()->bWantsToCrouch = false;
		}
		else
		{
			GetCharacterMovement()->bWantsToCrouch = true;
		}
	}
}


void ASCharacter::BeginZoom() 
{
	bIsADSight = true;
	// CameraComp->SetRelativeRotation(FRotator{5.f, -5.f, 0.f});
	// CameraBoom->SocketOffset = FVector{0.f, 20.f, 5.f};
}


void ASCharacter::EndZoom() 
{
	bIsADSight = false;
	// CameraComp->SetRelativeRotation(FRotator{0.f});
	// CameraBoom->SocketOffset = FVector{0.f, 15.f, 10.f};
}

void ASCharacter::StartFire() 
{
	if(CurrentWeapon)
	{
		if (CurrentWeapon->CurrentMagSize > 0 && !IsReloading)
		{
			IsShooting = true;
			CurrentWeapon->StartFire();
		}
	}
}

void ASCharacter::StopFire() 
{
	if(CurrentWeapon)
	{
		IsShooting = false;
		CurrentWeapon->EndFire();
	}
}

void ASCharacter::Reload() 
{
	if(CurrentWeapon && !IsShooting)
	{
		if (CurrentWeapon->CurrentMagSize < CurrentWeapon->BaseMagazineSize)
		{
			IsReloading = true;
			GetWorldTimerManager().SetTimer(TH_Reloading, this, &ASCharacter::EndReload, 2.1, false);
		}

		else
		{
			IsReloading = false;
			EndReload();
		}
	}
}

void ASCharacter::EndReload() 
{
	if(CurrentWeapon)
	{
		CurrentWeapon->OnReload();
		IsReloading = false;
		GetWorldTimerManager().ClearTimer(TH_Reloading);
	}
}

void ASCharacter::OnHealthChanged(USHealthComponent* HealthComponent, float Health, float HealthDelta, 
								  const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser) 
{
	if(Health <= 0 && !bDied)
	{
		bDied = true;
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DetachFromControllerPendingDestroy();
		SetLifeSpan(10.f);
	}
}

void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);
  DOREPLIFETIME(ASCharacter, CurrentWeapon); // Replicates the Variable to every machine
  DOREPLIFETIME(ASCharacter, bDied);
  DOREPLIFETIME(ASCharacter, bIsADSight);
  DOREPLIFETIME(ASCharacter, IsReloading);
  DOREPLIFETIME(ASCharacter, IsShooting);
}