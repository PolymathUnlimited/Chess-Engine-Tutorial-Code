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

#define WHITE 13
#define BLACK 14
#define DRAW 15

struct Move
{
	// move coordinates
	std::uint8_t from;
	std::uint8_t to;

	// used to specify promotions
	std::uint8_t promotion;

	// operator overload to check equality
	bool operator== (const Move& rhs) const
	{
		return from == rhs.from && to == rhs.to && promotion == rhs.promotion;
	}
};

struct BoardState
{
	// bitboards for each piece type
	std::uint64_t bitboards[12];

	// castling rights
	bool WKC, BKC, WQC, BQC;

	// bitboard to represent en-passant target
	std::uint64_t passantTarget;

	// whose turn it is
	std::uint8_t turn;

	// half-move counter (for 50 move rule)
	std::uint8_t halfMoves;

	// zobrist hash
	std::uint64_t hash;
};

class Chessboard
{
private:
	// a stack of board states
	BoardState* stateStack;
	int stackIndex;

	// attack bitboards
	uint64_t knightAttacks[64];
	uint64_t diagonalRays[64][4];
	uint64_t cardinalRays[64][4];
	uint64_t kingAttacks[64];

	// hashes
	std::uint64_t boardHash[12][64];
	std::uint64_t passantHash[64];
	std::uint64_t turnHash;
	std::uint64_t WKChash;
	std::uint64_t WQChash;
	std::uint64_t BKChash;
	std::uint64_t BQChash;

public:
	// zobrist hashing function
	std::uint64_t hash();

	// methods to get and set pieces
	std::uint8_t getPiece(std::uint8_t square);
	void setPiece(std::uint8_t piece, std::uint8_t square);

	// method to see whose turn it is
	std::uint8_t turn();

	// methods to make and undo moves
	void move(const Move& move);
	void undo();

	// move generation methods
	void pseudoMoves(Move* moves, int& numMoves);
	bool isLegal(const Move& move);
	bool isAttacked(uint8_t square, uint8_t color);
	uint8_t whiteKingSquare();
	uint8_t blackKingSquare();
	
	// terminal state detection
	bool softDraw();
	std::uint8_t isTerminal();

	// default constructor
	Chessboard();

	// destructor
	~Chessboard();
};
