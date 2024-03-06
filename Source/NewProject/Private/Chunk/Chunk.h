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

	//Base setting for Chunk Size chunk (Default 16 x 16 x 128) set from ChunkManager
	FIntVector Size;

	//Base setting set spawn water from Level set from ChunkManager
	float WaterLevel;

	//Noise setting set from ChunkManager
	float Frequency;
	int Seed;
	
	//Block material set from ChunkManager
	UPROPERTY()
	TObjectPtr<UMaterialInterface> Material;

	//Change block type 
	UFUNCTION(BlueprintCallable, Category="Chunk")
	void ModifyVoxel(const FIntVector Position, EBlock Block);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	//Main mesh
	TObjectPtr<UProceduralMeshComponent> Mesh;

	//All noise for Height and Biomes
	TObjectPtr<FastNoiseLite> Noise;
	TObjectPtr<FastNoiseLite> Temperature;
	TObjectPtr<FastNoiseLite> Moisture;
	TObjectPtr<FastNoiseLite> Altitude;
	TObjectPtr<FastNoiseLite> CaveNoise;
	
	//Have MeshData Vertices Triangles and more
	FChunkMeshData MeshData;

	//Map have Position block in Chunk and have Type Block (Air, Dirt, Grass, Stone, .....)
	TMap<FIntVector, EBlock> BlocksNew = TMap<FIntVector, EBlock>();

	//Count VertexCount
	int VertexCount = 0;

	//Its not work :(
	void GenerateTrees();

	//Generate blocks with biomes use five Noises
	void GenerateBiomes();

	//Old generate blocks, use only one Noise* 
	void GenerateBlocks();

	//Applay MeshData to Mesh*
	void ApplyMesh() const;

	//Clear MeshData and Vertex Count
	void ClearMesh();

	//Generate Greedy Mesh
	void GenerateMesh();

	//Create Quad
	void CreateQuad(FMask Mask, const int Width, const int Height, FIntVector AxisMask, FIntVector V1, FIntVector V2,
	                FIntVector V3, FIntVector V4);
	
	//Get Block by (int) Vector ID 
	EBlock GetBlock(FIntVector Index) const;

	//Check Mask
	static bool CompareMask(FMask M1, FMask M2);

	//Get Texture Index for material
	static int GetTextureIndex(EBlock Block, const FVector& Normal);
	static int GetTextureOverlay(EBlock Block, const FVector& Normal);

	void CheckBlockPhysic(FIntVector Position, EBlock Block);
	void SpawnEntityBlock(FIntVector Position);
};
