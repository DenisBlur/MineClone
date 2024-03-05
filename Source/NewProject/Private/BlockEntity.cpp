// Fill out your copyright notice in the Description page of Project Settings.


#include "BlockEntity.h"

#include "Chunk/Chunk.h"
#include "Kismet/GameplayStatics.h"
#include "Utils/Enums.h"
#include "World/ChunkManager.h"

// Sets default values
ABlockEntity::ABlockEntity()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Block = CreateDefaultSubobject<UStaticMeshComponent>("Block");

	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT("/Game/MainGame/Cube.Cube"));
	if(UStaticMesh* Asset = MeshAsset.Object; Asset != nullptr)
	{
		Block->SetStaticMesh(Asset);
	}

	Block->SetSimulatePhysics(true);
	Block->BodyInstance.bLockXRotation  = true;
	Block->BodyInstance.bLockYRotation   = true;
	Block->BodyInstance.bLockZRotation   = true;
	
}

// Called when the game starts or when spawned
void ABlockEntity::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABlockEntity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FHitResult OutHit;

	const FVector StartVector = FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z - 51);
	FVector EndVector = (FVector::DownVector * 10) + StartVector;

	FCollisionQueryParams QueryParams;

	DrawDebugLine(GetWorld(), StartVector, EndVector, FColor::Green, false, 1, 0, 1);

	if(GetWorld()->LineTraceSingleByChannel(OutHit, StartVector, EndVector, ECC_Visibility, QueryParams))
	{
		if(OutHit.bBlockingHit && Cast<AChunk>(OutHit.GetActor()))
		{
			TObjectPtr<AChunkManager> ChunkManager;
			ChunkManager = Cast<AChunkManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AChunkManager::StaticClass()));
			ChunkManager->ModifyVoxel(OutHit.Location - OutHit.Normal, OutHit.Normal, EBlock::Sand);
			Destroy();
		} 
	}

}

