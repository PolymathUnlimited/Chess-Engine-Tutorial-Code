#pragma once

#include <cstdint>

#define WHITE_PAWN 0
#define WHITE_KNIGHT 1
#define WHITE_BISHOP 2
#define WHITE_ROOK 3
#define WHITE_QUEEN 4
#define WHITE_KING 5
#define BLACK_PAWN 6
#define BLACK_KNIGHT 7
#define BLACK_BISHOP 8
#define BLACK_ROOK 9
#define BLACK_QUEEN 10
#define BLACK_KING 11
#define EMPTY 12

class Chessboard
{
private:
	// bitboards (one for each piece type)
	std::uint64_t bitboards[12];

public:
	// methods to get and set pieces
	std::uint8_t getPiece(std::uint8_t square);
	void setPiece(std::uint8_t piece, std::uint8_t square);

	// default constructor
	Chessboard();
};