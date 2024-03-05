// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Chunk/Chunk.h"
#include "GameFramework/Actor.h"
#include "ChunkManager.generated.h"

UCLASS()
class NEWPROJECT_API AChunkManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AChunkManager();

	UPROPERTY(EditAnywhere, Category="Chunk")
	FIntVector Size = FIntVector(16, 16, 128);

	UPROPERTY(EditInstanceOnly, Category="Chunk")
	TObjectPtr<UMaterialInterface> Material;

	UPROPERTY(EditInstanceOnly, Category="Chunk")
	TObjectPtr<UMaterialInterface> WaterMaterial;

	UPROPERTY(EditInstanceOnly, Category="Chunk")
	UStaticMesh* Water;

	UPROPERTY(EditAnywhere, Category="Chunk")
	float Frequency = 0.03f;

	UPROPERTY(EditAnywhere, Category="Chunk")
	float WaterLevel = 44.5;

	UPROPERTY(EditAnywhere, Category="Chunk")
	int Seed = rand();

	UPROPERTY(EditAnywhere, Category="World")
	int DrawDistance = 5;

	UFUNCTION(BlueprintCallable, Category="Chunk")
	void ModifyVoxel(const FVector Position, const FVector Normal, EBlock Block);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
private:

	UPROPERTY()
	TMap<FIntVector, AChunk*> Chunks = TMap<FIntVector, AChunk*>();

	UFUNCTION(BlueprintPure, Category="Voxel")
	static FIntVector WorldToBlockPosition(const FVector& Position);
	
	UFUNCTION(BlueprintPure, Category="Voxel")
	FIntVector WorldToLocalBlockPosition(FIntVector ChunkPos, const FVector& Position) const;

	UFUNCTION(BlueprintPure, Category="Voxel")
	FIntVector WorldToChunkPosition(const FVector& Position) const;
	
	void GenerateWorld();
};
