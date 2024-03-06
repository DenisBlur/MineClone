// Fill out your copyright notice in the Description page of Project Settings.


#include "Chunk.h"

#include "BlockEntity.h"
#include "ProceduralMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Utils/Enums.h"
#include "Utils/FastNoiseLite.h"

// Sets default values
AChunk::AChunk()
{
	PrimaryActorTick.bCanEverTick = false;

	//Create Mesh in Actor 
	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");

	//Init Noises
	Noise = new FastNoiseLite();
	Temperature = new FastNoiseLite;
	Moisture = new FastNoiseLite;
	Altitude = new FastNoiseLite;
	CaveNoise = new FastNoiseLite;

	//Init Blocks
	BlocksNew = TMap<FIntVector, EBlock>();

	SetRootComponent(Mesh);
}

// Called when the game starts or when spawned
void AChunk::BeginPlay()
{
	Super::BeginPlay();

	//Setup Noises start
	Noise->SetSeed(Seed);
	Noise->SetFrequency(Frequency);
	Noise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	Noise->SetFractalType(FastNoiseLite::FractalType_FBm);
	Temperature->SetFractalOctaves(20);

	Temperature->SetSeed(Seed);
	Temperature->SetFrequency(Frequency);
	Temperature->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	Temperature->SetFractalType(FastNoiseLite::FractalType_FBm);
	Temperature->SetFractalOctaves(20);

	Moisture->SetSeed(Seed);
	Moisture->SetFrequency(Frequency);
	Moisture->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	Moisture->SetFractalType(FastNoiseLite::FractalType_FBm);
	Moisture->SetFractalOctaves(20);

	Altitude->SetSeed(Seed);
	Altitude->SetFrequency(Frequency / 2);
	Altitude->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	Altitude->SetFractalType(FastNoiseLite::FractalType_FBm);
	Altitude->SetFractalOctaves(20);

	CaveNoise->SetSeed(Seed);
	CaveNoise->SetFrequency(0.01);
	CaveNoise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	CaveNoise->SetFractalType(FastNoiseLite::FractalType_PingPong);
	CaveNoise->SetFractalOctaves(2);
	//Setup Noises end


	//Generate Blocks (Biomes) and GenerateMesh
	GenerateBiomes();
	GenerateMesh();
	ApplyMesh();
}


void AChunk::GenerateBiomes()
{
	const auto Location = GetActorLocation();
	for (int x = 0; x < Size.X; x++)
	{
		for (int y = 0; y < Size.Y; y++)
		{
			int RiverHole;
			int Mountain;

			const float X_Pos = (x * 100 + Location.X) / 100;
			const float Y_Pos = (y * 100 + Location.Y) / 100;

			const auto Alt = 2 * abs(Altitude->GetNoise(X_Pos, Y_Pos));
			const auto Temp = 2 * abs(Temperature->GetNoise(X_Pos, Y_Pos));
			const auto Moist = 2 * abs(Moisture->GetNoise(X_Pos, Y_Pos));

			int Height = FMath::Clamp(FMath::RoundToInt((Noise->GetNoise(X_Pos, Y_Pos) + 1) * Size.Z / 2), 1,
			                          Size.Z);


			if (Alt <= 0.14)
			{
				RiverHole = round((0.14 - Alt) * 300);
			}
			else
			{
				RiverHole = 0;
			}

			if (Alt > 0.8)
			{
				Mountain = abs(round((0.8 - Alt) * 300));
			}
			else
			{
				Mountain = 0;
			}

			Height = Height - RiverHole;
			Height = Height + Mountain;

			if (Height == 0)
			{
				Height = 5;
			}

			for (int z = 0; z < Size.Z; z++)
			{
				if (constexpr int StartStone = 5; z < Height - StartStone)
				{
					BlocksNew.Add(FIntVector(x, y, z), EBlock::Stone);
					continue;
				}

				if (z <= Height - 1)
				{
					if (Alt < 0.1)
					{
						BlocksNew.Add(FIntVector(x, y, z), EBlock::Stone);
						if (WaterLevel > Height)
						{
							int dif = WaterLevel - Height;
							for (int i = 0; i < dif; i++)
							{
								BlocksNew.Add(FIntVector(x, y, Height + i), EBlock::Water);
							}
						}
					}
					else if (Alt >= 0.1 && Alt <= 0.15)
					{
						BlocksNew.Add(FIntVector(x, y, z), EBlock::Sand);
						BlocksNew.Add(FIntVector(x, y, static_cast<int>(WaterLevel)), EBlock::Water);
						if (Height < WaterLevel)
						{
							const int dif = WaterLevel - Height;
							for (int i = 0; i < dif; i++)
							{
								BlocksNew.Add(FIntVector(x, y, Height + i), EBlock::Water);
							}
						}
						else
						{
							BlocksNew.Add(FIntVector(x, y, WaterLevel), EBlock::Water);
						}
					}
					else if (Alt >= 0.15 && Alt <= 0.8)
					{
						if (Moist >= 0 && Moist <= 0.9 and Temp >= 0.2 && Temp <= 0.6)
						{
							BlocksNew.Add(FIntVector(x, y, z), z == Height - 1 ? EBlock::Grass : EBlock::Dirt);
						}
						else if (Temp > 0.6 && Moist < 0.4)
						{
							BlocksNew.Add(FIntVector(x, y, z), EBlock::Sand);
						}
						else if (Moist > 0.9)
						{
							BlocksNew.Add(FIntVector(x, y, z), EBlock::Dirt);
						}
						else
						{
							BlocksNew.Add(FIntVector(x, y, z), z == Height - 1 ? EBlock::Grass : EBlock::Dirt);
						}
					}
					else if (Alt >= 0.8 && Alt <= 0.95)
					{
						BlocksNew.Add(FIntVector(x, y, z), EBlock::Water);
					}
					else
					{
						BlocksNew.Add(FIntVector(x, y, z), EBlock::Snow);
					}
				}
				else
				{
					if (!BlocksNew.Find(FIntVector(x, y, z)))
					{
						BlocksNew.Add(FIntVector(x, y, z), EBlock::Air);
					}
				}
			}
		}
	}
}

void AChunk::GenerateBlocks()
{
	const auto Location = GetActorLocation();

	for (int x = 0; x < Size.X; x++)
	{
		for (int y = 0; y < Size.Y; y++)
		{
			const float Xpos = (x * 100 + Location.X) / 100;
			const float Ypos = (y * 100 + Location.Y) / 100;

			const int Height = FMath::Clamp(FMath::RoundToInt((Noise->GetNoise(Xpos, Ypos) + 1) * Size.Z / 2), 0,
			                                Size.Z);

			for (int z = 0; z < Size.Z; z++)
			{
				if (z < Height - 3) BlocksNew.Add(FIntVector(x, y, z), EBlock::Stone);
				else if (z < Height - 1) BlocksNew.Add(FIntVector(x, y, z), EBlock::Dirt);
				else if (z == Height - 1) BlocksNew.Add(FIntVector(x, y, z), EBlock::Grass);
				else BlocksNew.Add(FIntVector(x, y, z), EBlock::Air);
			}
		}
	}
}

void AChunk::ApplyMesh() const
{
	Mesh->SetMaterial(0, Material);
	Mesh->CreateMeshSection(
		0,
		MeshData.Vertices,
		MeshData.Triangles,
		MeshData.Normals,
		MeshData.UV0,
		MeshData.Colors,
		TArray<FProcMeshTangent>(),
		true);
}

void AChunk::ClearMesh()
{
	VertexCount = 0;
	MeshData.Clear();
}

void AChunk::GenerateMesh()
{
	for (int Axis = 0; Axis < 3; ++Axis)
	{
		// 2 Perpendicular axis
		const int Axis1 = (Axis + 1) % 3;
		const int Axis2 = (Axis + 2) % 3;

		const int MainAxisLimit = Size[Axis];
		const int Axis1Limit = Size[Axis1];
		const int Axis2Limit = Size[Axis2];

		auto DeltaAxis1 = FIntVector::ZeroValue;
		auto DeltaAxis2 = FIntVector::ZeroValue;

		auto ChunkItr = FIntVector::ZeroValue;
		auto AxisMask = FIntVector::ZeroValue;

		AxisMask[Axis] = 1;

		TArray<FMask> Mask;
		Mask.SetNum(Axis1Limit * Axis2Limit);

		// Check each slice of the chunk
		for (ChunkItr[Axis] = -1; ChunkItr[Axis] < MainAxisLimit;)
		{
			int N = 0;

			// Compute Mask
			for (ChunkItr[Axis2] = 0; ChunkItr[Axis2] < Axis2Limit; ++ChunkItr[Axis2])
			{
				for (ChunkItr[Axis1] = 0; ChunkItr[Axis1] < Axis1Limit; ++ChunkItr[Axis1])
				{
					const auto CurrentBlock = GetBlock(ChunkItr);
					const auto CompareBlock = GetBlock(ChunkItr + AxisMask);

					const bool CurrentBlockOpaque = CurrentBlock != EBlock::Air;
					const bool CompareBlockOpaque = CompareBlock != EBlock::Air;

					if (CurrentBlockOpaque == CompareBlockOpaque)
					{
						Mask[N++] = FMask{EBlock::Null, 0};
					}
					else if (CurrentBlockOpaque)
					{
						Mask[N++] = FMask{CurrentBlock, 1};
					}
					else
					{
						Mask[N++] = FMask{CompareBlock, -1};
					}
				}
			}

			++ChunkItr[Axis];
			N = 0;

			// Generate Mesh From Mask
			for (int j = 0; j < Axis2Limit; ++j)
			{
				for (int i = 0; i < Axis1Limit;)
				{
					if (Mask[N].Normal != 0)
					{
						const auto CurrentMask = Mask[N];
						ChunkItr[Axis1] = i;
						ChunkItr[Axis2] = j;

						int Width;

						for (Width = 1; i + Width < Axis1Limit && CompareMask(Mask[N + Width], CurrentMask); ++Width)
						{
						}

						int Height;
						bool Done = false;

						for (Height = 1; j + Height < Axis2Limit; ++Height)
						{
							for (int k = 0; k < Width; ++k)
							{
								if (CompareMask(Mask[N + k + Height * Axis1Limit], CurrentMask)) continue;

								Done = true;
								break;
							}

							if (Done) break;
						}

						DeltaAxis1[Axis1] = Width;
						DeltaAxis2[Axis2] = Height;

						CreateQuad(
							CurrentMask,
							Width,
							Height,
							AxisMask,
							ChunkItr,

							ChunkItr + DeltaAxis1,
							ChunkItr + DeltaAxis2,
							ChunkItr + DeltaAxis1 + DeltaAxis2
						);

						DeltaAxis1 = FIntVector::ZeroValue;
						DeltaAxis2 = FIntVector::ZeroValue;

						for (int l = 0; l < Height; ++l)
						{
							for (int k = 0; k < Width; ++k)
							{
								Mask[N + k + l * Axis1Limit] = FMask{EBlock::Null, 0};
							}
						}

						i += Width;
						N += Width;
					}
					else
					{
						i++;
						N++;
					}
				}
			}
		}
	}
}

void AChunk::CreateQuad(FMask Mask, const int Width, const int Height, FIntVector AxisMask, FIntVector V1,
                        FIntVector V2, FIntVector V3, FIntVector V4)
{
	const auto Normal = FVector(AxisMask * Mask.Normal);

	const int R = GetTextureOverlay(Mask.Block, Normal);
	const int G = R == 255 ? 0 : 1;
	const int B = Mask.Block == EBlock::Snow ? 255 : 0;
	const int A = GetTextureIndex(Mask.Block, Normal);

	const auto Color = FColor(R, G, B, A);

	MeshData.Vertices.Add(FVector(V1) * 100);
	MeshData.Vertices.Add(FVector(V2) * 100);
	MeshData.Vertices.Add(FVector(V3) * 100);
	MeshData.Vertices.Add(FVector(V4) * 100);

	MeshData.Triangles.Add(VertexCount);
	MeshData.Triangles.Add(VertexCount + 2 + Mask.Normal);
	MeshData.Triangles.Add(VertexCount + 2 - Mask.Normal);
	MeshData.Triangles.Add(VertexCount + 3);
	MeshData.Triangles.Add(VertexCount + 1 - Mask.Normal);
	MeshData.Triangles.Add(VertexCount + 1 + Mask.Normal);

	MeshData.Colors.Append({
		Color,
		Color,
		Color,
		Color
	});

	if (Normal.X == 1 || Normal.X == -1)
	{
		MeshData.UV0.Append({
			FVector2D(Width, Height),
			FVector2D(0, Height),
			FVector2D(Width, 0),
			FVector2D(0, 0),
		});
	}
	else
	{
		MeshData.UV0.Append({
			FVector2D(Height, Width),
			FVector2D(Height, 0),
			FVector2D(0, Width),
			FVector2D(0, 0),
		});
	}


	MeshData.Normals.Add(Normal);
	MeshData.Normals.Add(Normal);
	MeshData.Normals.Add(Normal);
	MeshData.Normals.Add(Normal);

	VertexCount += 4;
}

EBlock AChunk::GetBlock(FIntVector Index) const
{
	//UE_LOG(LogTemp, Warning, TEXT("Block position : %s"), *Index.ToString());

	if (Index.X >= Size.X || Index.Y >= Size.Y || Index.Z >= Size.Z) return EBlock::Air;
	if (Index.X < 0 || Index.Y < 0 || Index.Z < 0) return EBlock::Air;

	try
	{
		return BlocksNew[Index];
	}
	catch (...)
	{
		UE_LOG(LogTemp, Warning, TEXT("Error block position : %s"), *Index.ToString());
	}

	UE_LOG(LogTemp, Warning, TEXT("Do find block : %s"), *Index.ToString());

	return EBlock::Air;
}

bool AChunk::CompareMask(FMask M1, FMask M2)
{
	return M1.Block == M2.Block && M1.Normal == M2.Normal;
}

int AChunk::GetTextureIndex(const EBlock Block, const FVector& Normal)
{
	switch (Block)
	{
	case EBlock::Grass:
		{
			if (Normal == FVector::UpVector) return 0;
			return 1;
		}
	case EBlock::Dirt: return 2;
	case EBlock::Stone: return 3;
	case EBlock::Sand: return 4;
	case EBlock::Water: return 5;
	case EBlock::Snow:
		{
			if (Normal == FVector::UpVector) return 0;
			return 1;
		}
	case EBlock::Wood:
		{
			if (Normal == FVector::UpVector) return 6;
			return 7;
		}
	default: return 255;
	}
}

int AChunk::GetTextureOverlay(const EBlock Block, const FVector& Normal)
{
	switch (Block)
	{
	case EBlock::Grass:
		{
			if (Normal == FVector::UpVector) return 0;
			return 1;
		}
	case EBlock::Snow:
		{
			if (Normal == FVector::UpVector) return 0;
			return 1;
		}
	default: return 255;
	}
}

void AChunk::ModifyVoxel(const FIntVector Position, const EBlock Block)
{
	if (BlocksNew.Find(Position))
	{
		CheckBlockPhysic(Position, Block);
		ClearMesh();
		GenerateMesh();
		ApplyMesh();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Block not find: %s"), *Position.ToString())
	}
}

void AChunk::CheckBlockPhysic(const FIntVector Position, const EBlock Block)
{
	if (
		const auto DownVector = Position + static_cast<FIntVector>(FVector::DownVector);
		BlocksNew.Find(DownVector)
		&& BlocksNew[DownVector] == EBlock::Air
		&& Block == EBlock::Sand)
	{
		SpawnEntityBlock(Position);
	}
	else
	{
		BlocksNew[Position] = Block;
		if (Block == EBlock::Air)
		{
			auto LocalPosition = Position;
			for (int i = Position.Z; i < Size.Z; i++)
			{
				if (
					auto UpVector = LocalPosition + static_cast<FIntVector>(FVector::UpVector);
					BlocksNew.Find(UpVector)
					&& BlocksNew[UpVector] == EBlock::Sand)
				{
					LocalPosition = UpVector;
					BlocksNew[FIntVector(LocalPosition)] = EBlock::Air;

					SpawnEntityBlock(LocalPosition);
				}
			}
		}
	}
}

void AChunk::SpawnEntityBlock(const FIntVector Position)
{
	const auto Transform = FTransform(FRotator::ZeroRotator,
	                                  FVector(
		                                  (GetActorLocation().X + Position.X * 100) + 50,
		                                  (GetActorLocation().Y + Position.Y * 100) + 50,
		                                  (GetActorLocation().Z + Position.Z * 100) + 50),
	                                  FVector::OneVector);
	const auto SpawnEntity = GetWorld()->SpawnActorDeferred<ABlockEntity>(
		ABlockEntity::StaticClass(), Transform, this);

	UGameplayStatics::FinishSpawningActor(SpawnEntity, Transform);
}
