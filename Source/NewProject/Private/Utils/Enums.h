#pragma once

enum class EDirection
{
	Forward,
	Right,
	Back,
	Left,
	Up,
	Down
};

UENUM(BlueprintType)
enum class EBlock : uint8
{
	Null,
	Air,
	Stone,
	Dirt,
	Grass,
	Sand,
	Water,
	Snow,
	Wood
};
