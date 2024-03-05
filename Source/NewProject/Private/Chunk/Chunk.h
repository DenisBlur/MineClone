// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utils/ChunkMeshData.h"
#include "Chunk.generated.h"

enum class EBlock : uint8;
class FastNoiseLite;
class UProceduralMeshComponent;

UCLASS()
class AChunk : public AActor
{
	GENERATED_BODY()

	struct FMask
	{
		EBlock Block;
		int Normal;
	};

public:
	AChunk();

	FIntVector Size = FIntVector(16, 16, 128);

	float WaterLevel = 44.5;
	int Seed;
	float Frequency;
	bool HasWater = false;
	
	UPROPERTY()
	UStaticMesh* Water;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> Material;
	UPROPERTY()
	TObjectPtr<UMaterialInterface> WaterMaterial;
	
	UFUNCTION(BlueprintCallable, Category="Chunk")
	void ModifyVoxel(const FIntVector Position, EBlock Block);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void GenerateTrees();
	void GenerateBiomes();

private:
	
	TObjectPtr<UProceduralMeshComponent> Mesh;

	TObjectPtr<FastNoiseLite> Noise;
	TObjectPtr<FastNoiseLite> Temperature;
	TObjectPtr<FastNoiseLite> Moisture;
	TObjectPtr<FastNoiseLite> Altitude;
	TObjectPtr<FastNoiseLite> CaveNoise;

	UPROPERTY()
	UStaticMeshComponent* StaticMeshComponent;
	
	FChunkMeshData MeshData;

	TMap<FIntVector, EBlock> BlocksNew = TMap<FIntVector, EBlock>();
	
	int VertexCount = 0;

	void GenerateBlocks();

	void ApplyMesh() const;

	void ClearMesh();
	
	void GenerateMesh();

	void CreateQuad(FMask Mask, const int Width, const int Height, FIntVector AxisMask, FIntVector V1, FIntVector V2,
	                FIntVector V3, FIntVector V4);

	int GetBlockIndex(int X, int Y, int Z) const;

	EBlock GetBlock(FIntVector Index) const;

	static bool CompareMask(FMask M1, FMask M2);
	static int GetTextureIndex(EBlock Block, const FVector& Normal);
	static int GetTextureOverlay(EBlock Block, const FVector& Normal);
};
