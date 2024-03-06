// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkManager.h"

#include "Chunk/Chunk.h"
#include "Kismet/GameplayStatics.h"
#include "Utils/Enums.h"


// Sets default values
AChunkManager::AChunkManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AChunkManager::ModifyVoxel(const FVector Position, const FVector Normal, EBlock Block)
{
	if (
		const auto ChunkPos = WorldToChunkPosition(Position + (Normal * 10));
		Chunks.Find(FIntVector(ChunkPos.X, ChunkPos.Y, 0)))
	{
		const FIntVector Result = WorldToLocalBlockPosition(ChunkPos, Position) + FIntVector(Normal);
		Chunks[FIntVector(ChunkPos.X, ChunkPos.Y, 0)]->ModifyVoxel(Result, Block);
	}
}

// Called when the game starts or when spawned
void AChunkManager::BeginPlay()
{
	Super::BeginPlay();

	GenerateWorld();
}

void AChunkManager::GenerateWorld()
{
	for (int X = -DrawDistance; X < DrawDistance; X++)
	{
		for (int Y = -DrawDistance; Y < DrawDistance; Y++)
		{
			auto Transform = FTransform(FRotator::ZeroRotator, FVector(X * Size.X * 100, Y * Size.Y * 100, 0),
			                            FVector::OneVector);
			const auto SpawnChunk = GetWorld()->SpawnActorDeferred<AChunk>(AChunk::StaticClass(), Transform, this);
			SpawnChunk->Frequency = Frequency;
			SpawnChunk->Size = Size;
			SpawnChunk->Material = Material;
			SpawnChunk->Seed = Seed;
			SpawnChunk->WaterLevel = WaterLevel;

			Chunks.Add(FIntVector(X, Y, 0), SpawnChunk);

			UGameplayStatics::FinishSpawningActor(SpawnChunk, Transform);
		}
	}
}

FIntVector AChunkManager::WorldToBlockPosition(const FVector& Position)
{
	return FIntVector(Position) / 100;
}

FIntVector AChunkManager::WorldToLocalBlockPosition(FIntVector ChunkPos, const FVector& Position) const
{;
	auto Result = WorldToBlockPosition(Position) - ChunkPos * Size;

	// Negative Normalization
	if (ChunkPos.X < 0) Result.X--;
	if (ChunkPos.Y < 0) Result.Y--;
	if (ChunkPos.Z < 0) Result.Z--;

	return Result;
}

FIntVector AChunkManager::WorldToChunkPosition(const FVector& Position) const
{
	FIntVector Result;

	const int FactorX = Size.X * 100;
	const int FactorZ = Size.Z * 100;
	const auto IntPosition = FIntVector(Position);

	if (IntPosition.X < 0) Result.X = static_cast<int>(Position.X / FactorX) - 1;
	else Result.X = static_cast<int>(Position.X / FactorX);

	if (IntPosition.Y < 0) Result.Y = static_cast<int>(Position.Y / FactorX) - 1;
	else Result.Y = static_cast<int>(Position.Y / FactorX);

	if (IntPosition.Z < 0) Result.Z = static_cast<int>(Position.Z / FactorZ) - 1;
	else Result.Z = static_cast<int>(Position.Z / FactorZ);

	return Result;
}
