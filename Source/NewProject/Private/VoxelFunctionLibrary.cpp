// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelFunctionLibrary.h"

FIntVector UVoxelFunctionLibrary::WorldToBlockPosition(const FVector& Position)
{
	return FIntVector(Position) / 100;
}

FIntVector UVoxelFunctionLibrary::WorldToLocalBlockPosition(const FVector& Position, const FIntVector Size)
{
	const auto ChunkPos = WorldToChunkPosition(Position, Size);
	
	auto Result = WorldToBlockPosition(Position) - ChunkPos * Size;

	// Negative Normalization
	if (ChunkPos.X < 0) Result.X--;
	if (ChunkPos.Y < 0) Result.Y--;
	if (ChunkPos.Z < 0) Result.Z--;

	return Result;
}

FIntVector UVoxelFunctionLibrary::WorldToChunkPosition(const FVector& Position, const FIntVector Size)
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