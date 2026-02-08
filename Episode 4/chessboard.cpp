#include "chessboard.h"
#include "bitops.h"
#include <math.h>

using namespace std;

uint8_t Chessboard::getPiece(uint8_t square)
{
	uint64_t mask = uint64_t(1) << square;
	for (int i = 0; i < 12; ++i)
		if (stateStack[stackIndex].bitboards[i] & mask) return i;
	return EMPTY;
}

void Chessboard::setPiece(uint8_t piece, uint8_t square)
{
	uint64_t mask = uint64_t(1) << square;
	for (int i = 0; i < 12; ++i)
		if (i == piece) stateStack[stackIndex].bitboards[i] |= mask;
		else stateStack[stackIndex].bitboards[i] &= ~mask;
}

void Chessboard::move(const Move& move)
{
	// push a new state onto the state stack
	stateStack[stackIndex + 1] = stateStack[stackIndex++];

	// get information about the move
	uint64_t fromBoard = uint64_t(1) << move.from;
	uint64_t toBoard = uint64_t(1) << move.to;
	uint64_t moveBoard = fromBoard | toBoard;

	// move a piece
	uint8_t movedPiece = EMPTY;
	for (int i = stateStack[stackIndex].turn == WHITE ? 0 : 6; i < (stateStack[stackIndex].turn == WHITE ? 6 : 12); ++i)
	{
		if (stateStack[stackIndex].bitboards[i] & fromBoard)
		{
			stateStack[stackIndex].bitboards[i] ^= moveBoard;
			movedPiece = i;
			break;
		}
	}

	// capture a piece
	for (int i = stateStack[stackIndex].turn == WHITE ? 6 : 0; i < (stateStack[stackIndex].turn == WHITE ? 11 : 5); ++i)
	{
		if (stateStack[stackIndex].bitboards[i] & toBoard)
		{
			stateStack[stackIndex].bitboards[i] ^= toBoard;
			break;
		}
	}

	// en-passant captures
	if (toBoard == stateStack[stackIndex].passantTarget)
	{
		if (movedPiece == WHITE_PAWN) stateStack[stackIndex].bitboards[BLACK_PAWN] ^= stateStack[stackIndex].passantTarget << 8;
		else if (movedPiece == BLACK_PAWN) stateStack[stackIndex].bitboards[WHITE_PAWN] ^= stateStack[stackIndex].passantTarget >> 8;
	}

	// set en-passant target
	if ((movedPiece == WHITE_PAWN || movedPiece == BLACK_PAWN) && abs(move.from - move.to) == 16) stateStack[stackIndex].passantTarget = uint64_t(1) << int((move.from + move.to) * 0.5);
	else stateStack[stackIndex].passantTarget = 0;


	// castling
	if (movedPiece == WHITE_KING)
	{
		if (moveBoard == 0x5000000000000000) stateStack[stackIndex].bitboards[WHITE_ROOK] ^= 0xa000000000000000;
		else if (moveBoard == 0x1400000000000000) stateStack[stackIndex].bitboards[WHITE_ROOK] ^= 0x0900000000000000;
	}
	else if (movedPiece == BLACK_KING)
	{
		if (moveBoard == 0x0000000000000050) stateStack[stackIndex].bitboards[BLACK_ROOK] ^= 0x00000000000000a0;
		else if (moveBoard == 0x0000000000000014) stateStack[stackIndex].bitboards[BLACK_ROOK] ^= 0x0000000000000009;
	}

	// update castling rights
	if (moveBoard & 0x9000000000000000) stateStack[stackIndex].WKC = false;
	if (moveBoard & 0x1100000000000000) stateStack[stackIndex].WQC = false;
	if (moveBoard & 0x0000000000000090) stateStack[stackIndex].BKC = false;
	if (moveBoard & 0x0000000000000011) stateStack[stackIndex].BQC = false;

	// promotions
	if (move.promotion != EMPTY)
	{
		stateStack[stackIndex].bitboards[stateStack[stackIndex].turn == WHITE ? WHITE_PAWN : BLACK_PAWN] ^= toBoard;
		stateStack[stackIndex].bitboards[move.promotion] ^= toBoard;
	}

	// switch turns
	if (stateStack[stackIndex].turn == WHITE) stateStack[stackIndex].turn = BLACK;
	else stateStack[stackIndex].turn = WHITE;
}

void Chessboard::undo()
{
	if (stackIndex > 0) --stackIndex;
}

Chessboard::Chessboard()
{
	// allocate memory for the state stack
	stateStack = new BoardState[1000];
	stackIndex = 0;

	// set up initial board state
	stateStack[0].bitboards[WHITE_PAWN] = 0x00ff000000000000;
	stateStack[0].bitboards[WHITE_KNIGHT] = 0x4200000000000000;
	stateStack[0].bitboards[WHITE_BISHOP] = 0x2400000000000000;
	stateStack[0].bitboards[WHITE_ROOK] = 0x8100000000000000;
	stateStack[0].bitboards[WHITE_QUEEN] = 0x0800000000000000;
	stateStack[0].bitboards[WHITE_KING] = 0x1000000000000000;
	stateStack[0].bitboards[BLACK_PAWN] = 0x000000000000ff00;
	stateStack[0].bitboards[BLACK_KNIGHT] = 0x0000000000000042;
	stateStack[0].bitboards[BLACK_BISHOP] = 0x0000000000000024;
	stateStack[0].bitboards[BLACK_ROOK] = 0x0000000000000081;
	stateStack[0].bitboards[BLACK_QUEEN] = 0x0000000000000008;
	stateStack[0].bitboards[BLACK_KING] = 0x0000000000000010;

	stateStack[0].WKC = true;
	stateStack[0].WQC = true;
	stateStack[0].BKC = true;
	stateStack[0].BQC = true;

	stateStack[0].passantTarget = 0;

	stateStack[0].turn = WHITE;

	// generate attack boards
	for (int x1 = 0; x1 < 8; ++x1)
		for (int y1 = 0; y1 < 8; ++y1)
		{
			uint8_t squareIndex = x1 + 8 * y1;

			// knight attacks
			knightAttacks[squareIndex] = 0;
			if (x1 < 7 && y1 < 6) knightAttacks[squareIndex] |= uint64_t(1) << (x1 + 1 + 8 * (y1 + 2));
			if (x1 < 6 && y1 < 7) knightAttacks[squareIndex] |= uint64_t(1) << (x1 + 2 + 8 * (y1 + 1));
			if (x1 < 7 && y1 > 1) knightAttacks[squareIndex] |= uint64_t(1) << (x1 + 1 + 8 * (y1 - 2));
			if (x1 > 1 && y1 < 7) knightAttacks[squareIndex] |= uint64_t(1) << (x1 - 2 + 8 * (y1 + 1));
			if (x1 < 6 && y1 > 0) knightAttacks[squareIndex] |= uint64_t(1) << (x1 + 2 + 8 * (y1 - 1));
			if (x1 > 0 && y1 < 6) knightAttacks[squareIndex] |= uint64_t(1) << (x1 - 1 + 8 * (y1 + 2));
			if (x1 > 0 && y1 > 1) knightAttacks[squareIndex] |= uint64_t(1) << (x1 - 1 + 8 * (y1 - 2));
			if (x1 > 1 && y1 > 0) knightAttacks[squareIndex] |= uint64_t(1) << (x1 - 2 + 8 * (y1 - 1));

			// diagonal attack rays
			diagonalRays[squareIndex][0] = 0;
			for (int x2 = x1 + 1, y2 = y1 + 1; x2 < 8 && y2 < 8; ++x2, ++y2) diagonalRays[squareIndex][0] |= uint64_t(1) << (x2 + 8 * y2);
			diagonalRays[squareIndex][1] = 0;
			for (int x2 = x1 + 1, y2 = y1 - 1; x2 < 8 && y2 >= 0; ++x2, --y2) diagonalRays[squareIndex][1] |= uint64_t(1) << (x2 + 8 * y2);
			diagonalRays[squareIndex][2] = 0;
			for (int x2 = x1 - 1, y2 = y1 - 1; x2 >= 0 && y2 >= 0; --x2, --y2) diagonalRays[squareIndex][2] |= uint64_t(1) << (x2 + 8 * y2);
			diagonalRays[squareIndex][3] = 0;
			for (int x2 = x1 - 1, y2 = y1 + 1; x2 >= 0 && y2 < 8; --x2, ++y2) diagonalRays[squareIndex][3] |= uint64_t(1) << (x2 + 8 * y2);

			// cardinal attack rays
			cardinalRays[squareIndex][0] = 0;
			for (int x2 = x1 + 1; x2 < 8; ++x2) cardinalRays[squareIndex][0] |= uint64_t(1) << (x2 + 8 * y1);
			cardinalRays[squareIndex][1] = 0;
			for (int x2 = x1 - 1; x2 >= 0; --x2) cardinalRays[squareIndex][1] |= uint64_t(1) << (x2 + 8 * y1);
			cardinalRays[squareIndex][2] = 0;
			for (int y2 = y1 + 1; y2 < 8; ++y2) cardinalRays[squareIndex][2] |= uint64_t(1) << (x1 + 8 * y2);
			cardinalRays[squareIndex][3] = 0;
			for (int y2 = y1 - 1; y2 >= 0; --y2) cardinalRays[squareIndex][3] |= uint64_t(1) << (x1 + 8 * y2);

			// king attacks
			kingAttacks[squareIndex] = 0;
			if (x1 < 7 && y1 < 7) kingAttacks[squareIndex] |= uint64_t(1) << (x1 + 1 + 8 * (y1 + 1));
			if (x1 < 7) kingAttacks[squareIndex] |= uint64_t(1) << (x1 + 1 + 8 * y1);
			if (x1 < 7 && y1 > 0) kingAttacks[squareIndex] |= uint64_t(1) << (x1 + 1 + 8 * (y1 - 1));
			if (y1 < 7) kingAttacks[squareIndex] |= uint64_t(1) << (x1 + 8 * (y1 + 1));
			if (y1 > 0) kingAttacks[squareIndex] |= uint64_t(1) << (x1 + 8 * (y1 - 1));
			if (x1 > 0 && y1 < 7) kingAttacks[squareIndex] |= uint64_t(1) << (x1 - 1 + 8 * (y1 + 1));
			if (x1 > 0) kingAttacks[squareIndex] |= uint64_t(1) << (x1 - 1 + 8 * y1);
			if (x1 > 0 && y1 > 0) kingAttacks[squareIndex] |= uint64_t(1) << (x1 - 1 + 8 * (y1 - 1));
		}
}

Chessboard::~Chessboard()
{
	delete[] stateStack;
}
