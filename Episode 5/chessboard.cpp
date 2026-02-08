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

uint8_t Chessboard::turn()
{
	return stateStack[stackIndex].turn;
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

void Chessboard::pseudoMoves(Move* moves, int& numMoves)
{
	numMoves = 0;

	// construct some bitboards
	uint64_t whiteBoard = 0;
	uint64_t blackBoard = 0;
	for (int i = 0; i < 6; ++i) whiteBoard |= stateStack[stackIndex].bitboards[i];
	for (int i = 6; i < 12; ++i) blackBoard |= stateStack[stackIndex].bitboards[i];
	uint64_t board = whiteBoard | blackBoard;

	// white moves
	if (stateStack[stackIndex].turn == WHITE)
	{
		// pawn moves
		uint64_t bb = (stateStack[stackIndex].bitboards[WHITE_PAWN] >> 8) & ~board;
		uint64_t doubles = ((bb & 0x0000ff0000000000) >> 8) & ~board;
		uint8_t square;
		while (bb)
		{
			square = lsbIndex(bb);
			bb &= bb - 1;
			uint8_t origin = square + 8;
			if (square < 8)
			{
				moves[numMoves++] = { origin, square, WHITE_QUEEN };
				moves[numMoves++] = { origin, square, WHITE_ROOK };
				moves[numMoves++] = { origin, square, WHITE_BISHOP };
				moves[numMoves++] = { origin, square, WHITE_KNIGHT };
			}
			else moves[numMoves++] = { origin, square, EMPTY };
		}
		while (doubles)
		{
			square = lsbIndex(doubles);
			doubles &= doubles - 1;
			uint8_t origin = square + 16;
			moves[numMoves++] = { origin, square, EMPTY };
		}

		// pawn attacks
		bb = ((stateStack[stackIndex].bitboards[WHITE_PAWN] & 0xfefefefefefefefe) >> 9) & (blackBoard | stateStack[stackIndex].passantTarget);
		while (bb)
		{
			square = lsbIndex(bb);
			bb &= bb - 1;
			uint8_t origin = square + 9;
			if (square < 8)
			{
				moves[numMoves++] = { origin, square, WHITE_QUEEN };
				moves[numMoves++] = { origin, square, WHITE_ROOK };
				moves[numMoves++] = { origin, square, WHITE_BISHOP };
				moves[numMoves++] = { origin, square, WHITE_KNIGHT };
			}
			else moves[numMoves++] = { origin, square, EMPTY };
		}
		bb = ((stateStack[stackIndex].bitboards[WHITE_PAWN] & 0x7f7f7f7f7f7f7f7f) >> 7) & (blackBoard | stateStack[stackIndex].passantTarget);
		while (bb)
		{
			square = lsbIndex(bb);
			bb &= bb - 1;
			uint8_t origin = square + 7;
			if (square < 8)
			{
				moves[numMoves++] = { origin, square, WHITE_QUEEN };
				moves[numMoves++] = { origin, square, WHITE_ROOK };
				moves[numMoves++] = { origin, square, WHITE_BISHOP };
				moves[numMoves++] = { origin, square, WHITE_KNIGHT };
			}
			else moves[numMoves++] = { origin, square, EMPTY };
		}

		// knight moves
		bb = stateStack[stackIndex].bitboards[WHITE_KNIGHT];
		while (bb)
		{
			square = lsbIndex(bb);
			bb &= bb - 1;

			uint64_t knightMoves = knightAttacks[square] & ~whiteBoard;
			uint8_t target;
			while (knightMoves)
			{
				target = lsbIndex(knightMoves);
				knightMoves &= knightMoves - 1;
				moves[numMoves++] = { square, target, EMPTY };
			}
		}

		// diagonalMoves
		bb = stateStack[stackIndex].bitboards[WHITE_BISHOP] | stateStack[stackIndex].bitboards[WHITE_QUEEN];
		while (bb)
		{
			square = lsbIndex(bb);
			bb &= bb - 1;

			uint64_t diagonalMoves = 0;
			for (int i = 0; i < 4; ++i)
			{
				diagonalMoves |= diagonalRays[square][i];
				uint64_t blockers = board & diagonalRays[square][i];
				if (blockers)
				{
					uint8_t blockerIndex;
					if (i == 0 || i == 3) blockerIndex = lsbIndex(blockers);
					else blockerIndex = msbIndex(blockers);
					diagonalMoves &= ~diagonalRays[blockerIndex][i];
				}
			}
			diagonalMoves &= ~whiteBoard;

			uint8_t target;
			while (diagonalMoves)
			{
				target = lsbIndex(diagonalMoves);
				diagonalMoves &= diagonalMoves - 1;
				moves[numMoves++] = { square, target, EMPTY };
			}
		}

		// cardinal moves
		bb = stateStack[stackIndex].bitboards[WHITE_ROOK] | stateStack[stackIndex].bitboards[WHITE_QUEEN];
		while (bb)
		{
			square = lsbIndex(bb);
			bb &= bb - 1;

			uint64_t cardinalMoves = 0;
			for (int i = 0; i < 4; ++i)
			{
				cardinalMoves |= cardinalRays[square][i];
				uint64_t blockers = board & cardinalRays[square][i];
				if (blockers)
				{
					uint8_t blockerIndex;
					if (i == 0 || i == 2) blockerIndex = lsbIndex(blockers);
					else blockerIndex = msbIndex(blockers);
					cardinalMoves &= ~cardinalRays[blockerIndex][i];
				}
			}
			cardinalMoves &= ~whiteBoard;

			uint8_t target;
			while (cardinalMoves)
			{
				target = lsbIndex(cardinalMoves);
				cardinalMoves &= cardinalMoves - 1;
				moves[numMoves++] = { square, target, EMPTY };
			}
		}

		// king moves
		square = lsbIndex(stateStack[stackIndex].bitboards[WHITE_KING]);
		bb = kingAttacks[square] & ~whiteBoard;
		while (bb)
		{
			uint8_t target = lsbIndex(bb);
			bb &= bb - 1;
			moves[numMoves++] = { square, target, EMPTY };
		}

		// castling
		if (stateStack[stackIndex].WKC && !(board & 0x6000000000000000) && !isAttacked(60, BLACK) && !isAttacked(61, BLACK)) moves[numMoves++] = { 60, 62, EMPTY };
		if (stateStack[stackIndex].WQC && !(board & 0x0e00000000000000) && !isAttacked(60, BLACK) && !isAttacked(59, BLACK)) moves[numMoves++] = { 60, 58, EMPTY };
	}

	// black moves
	else
	{
		// pawn moves
		uint64_t bb = (stateStack[stackIndex].bitboards[BLACK_PAWN] << 8) & ~board;
		uint64_t doubles = ((bb & 0x0000000000ff0000) << 8) & ~board;
		uint8_t square;
		while (bb)
		{
			square = lsbIndex(bb);
			bb &= bb - 1;
			uint8_t origin = square - 8;
			if (square >= 56)
			{
				moves[numMoves++] = { origin, square, BLACK_QUEEN };
				moves[numMoves++] = { origin, square, BLACK_ROOK };
				moves[numMoves++] = { origin, square, BLACK_BISHOP };
				moves[numMoves++] = { origin, square, BLACK_KNIGHT };
			}
			else moves[numMoves++] = { origin, square, EMPTY };
		}
		while (doubles)
		{
			square = lsbIndex(doubles);
			doubles &= doubles - 1;
			uint8_t origin = square - 16;
			moves[numMoves++] = { origin, square, EMPTY };
		}

		// pawn attacks
		bb = ((stateStack[stackIndex].bitboards[BLACK_PAWN] & 0xfefefefefefefefe) << 7) & (whiteBoard | stateStack[stackIndex].passantTarget);
		while (bb)
		{
			square = lsbIndex(bb);
			bb &= bb - 1;
			uint8_t origin = square - 7; if (square >= 56)
			{
				moves[numMoves++] = { origin, square, BLACK_QUEEN };
				moves[numMoves++] = { origin, square, BLACK_ROOK };
				moves[numMoves++] = { origin, square, BLACK_BISHOP };
				moves[numMoves++] = { origin, square, BLACK_KNIGHT };
			}
			else moves[numMoves++] = { origin, square, EMPTY };
		}
		bb = ((stateStack[stackIndex].bitboards[BLACK_PAWN] & 0x7f7f7f7f7f7f7f7f) << 9)& (whiteBoard | stateStack[stackIndex].passantTarget);
		while (bb)
		{
			square = lsbIndex(bb);
			bb &= bb - 1;
			uint8_t origin = square - 9; if (square >= 56)
			{
				moves[numMoves++] = { origin, square, BLACK_QUEEN };
				moves[numMoves++] = { origin, square, BLACK_ROOK };
				moves[numMoves++] = { origin, square, BLACK_BISHOP };
				moves[numMoves++] = { origin, square, BLACK_KNIGHT };
			}
			else moves[numMoves++] = { origin, square, EMPTY };
		}

		// knight moves
		bb = stateStack[stackIndex].bitboards[BLACK_KNIGHT];
		while (bb)
		{
			square = lsbIndex(bb);
			bb &= bb - 1;

			uint64_t knightMoves = knightAttacks[square] & ~blackBoard;
			uint8_t target;
			while (knightMoves)
			{
				target = lsbIndex(knightMoves);
				knightMoves &= knightMoves - 1;
				moves[numMoves++] = { square, target, EMPTY };
			}
		}

		// diagonalMoves
		bb = stateStack[stackIndex].bitboards[BLACK_BISHOP] | stateStack[stackIndex].bitboards[BLACK_QUEEN];
		while (bb)
		{
			square = lsbIndex(bb);
			bb &= bb - 1;

			uint64_t diagonalMoves = 0;
			for (int i = 0; i < 4; ++i)
			{
				diagonalMoves |= diagonalRays[square][i];
				uint64_t blockers = board & diagonalRays[square][i];
				if (blockers)
				{
					uint8_t blockerIndex;
					if (i == 0 || i == 3) blockerIndex = lsbIndex(blockers);
					else blockerIndex = msbIndex(blockers);
					diagonalMoves &= ~diagonalRays[blockerIndex][i];
				}
			}
			diagonalMoves &= ~blackBoard;

			uint8_t target;
			while (diagonalMoves)
			{
				target = lsbIndex(diagonalMoves);
				diagonalMoves &= diagonalMoves - 1;
				moves[numMoves++] = { square, target, EMPTY };
			}
		}

		// cardinal moves
		bb = stateStack[stackIndex].bitboards[BLACK_ROOK] | stateStack[stackIndex].bitboards[BLACK_QUEEN];
		while (bb)
		{
			square = lsbIndex(bb);
			bb &= bb - 1;

			uint64_t cardinalMoves = 0;
			for (int i = 0; i < 4; ++i)
			{
				cardinalMoves |= cardinalRays[square][i];
				uint64_t blockers = board & cardinalRays[square][i];
				if (blockers)
				{
					uint8_t blockerIndex;
					if (i == 0 || i == 2) blockerIndex = lsbIndex(blockers);
					else blockerIndex = msbIndex(blockers);
					cardinalMoves &= ~cardinalRays[blockerIndex][i];
				}
			}
			cardinalMoves &= ~blackBoard;

			uint8_t target;
			while (cardinalMoves)
			{
				target = lsbIndex(cardinalMoves);
				cardinalMoves &= cardinalMoves - 1;
				moves[numMoves++] = { square, target, EMPTY };
			}
		}

		// king moves
		square = lsbIndex(stateStack[stackIndex].bitboards[BLACK_KING]);
		bb = kingAttacks[square] & ~blackBoard;
		while (bb)
		{
			uint8_t target = lsbIndex(bb);
			bb &= bb - 1;
			moves[numMoves++] = { square, target, EMPTY };
		}

		// castling
		if (stateStack[stackIndex].BKC && !(board & 0x0000000000000060) && !isAttacked(4, WHITE) && !isAttacked(5, WHITE)) moves[numMoves++] = { 4, 6, EMPTY };
		if (stateStack[stackIndex].BQC && !(board & 0x000000000000000e) && !isAttacked(4, WHITE) && !isAttacked(3, WHITE)) moves[numMoves++] = { 4, 2, EMPTY };
	}
}

bool Chessboard::isLegal(const Move& move)
{
	Move moves[218];
	int numMoves;
	pseudoMoves(moves, numMoves);

	for (int i = 0; i < numMoves; ++i)
		if (move == moves[i])
		{
			this->move(moves[i]);
			bool illegal = turn() == WHITE ? isAttacked(blackKingSquare(), WHITE) : isAttacked(whiteKingSquare(), BLACK);
			undo();
			if (!illegal) return true;
		}

	return false;
}

bool Chessboard::isAttacked(uint8_t square, uint8_t color)
{
	if (color == BLACK)
	{
		uint64_t board = 0;
		for (int i = 0; i < 12; ++i) board |= stateStack[stackIndex].bitboards[i];

		// cardinally attacked
		uint64_t attackers = stateStack[stackIndex].bitboards[BLACK_QUEEN] | stateStack[stackIndex].bitboards[BLACK_ROOK];
		for (int i = 0; i < 4; ++i)
		{
			uint64_t blockers = cardinalRays[square][i] & board;
			if (blockers & attackers)
			{
				uint8_t blockerIndex;
				if (i == 0 || i == 2) blockerIndex = lsbIndex(blockers);
				else blockerIndex = msbIndex(blockers);
				if (attackers & (uint64_t(1) << blockerIndex)) return true;
			}
		}

		// diagonally attacked
		attackers = stateStack[stackIndex].bitboards[BLACK_QUEEN] | stateStack[stackIndex].bitboards[BLACK_BISHOP];
		for (int i = 0; i < 4; ++i)
		{
			uint64_t blockers = diagonalRays[square][i] & board;
			if (blockers & attackers)
			{
				uint8_t blockerIndex;
				if (i == 0 || i == 3) blockerIndex = lsbIndex(blockers);
				else blockerIndex = msbIndex(blockers);
				if (attackers & (uint64_t(1) << blockerIndex)) return true;
			}
		}

		// attacked by a knight
		if (knightAttacks[square] & stateStack[stackIndex].bitboards[BLACK_KNIGHT]) return true;

		// attacked by a pawn
		if ((((stateStack[stackIndex].bitboards[BLACK_PAWN] & 0xfefefefefefefefe) << 7) | ((stateStack[stackIndex].bitboards[BLACK_PAWN] & 0x7f7f7f7f7f7f7f7f) << 9)) & (uint64_t(1) << square)) return true;
	
		// attacked by a king
		if (kingAttacks[square] & stateStack[stackIndex].bitboards[BLACK_KING]) return true;
	}
	else if (color == WHITE)
	{
		uint64_t board = 0;
		for (int i = 0; i < 12; ++i) board |= stateStack[stackIndex].bitboards[i];

		// cardinally attacked
		uint64_t attackers = stateStack[stackIndex].bitboards[WHITE_QUEEN] | stateStack[stackIndex].bitboards[WHITE_ROOK];
		for (int i = 0; i < 4; ++i)
		{
			uint64_t blockers = cardinalRays[square][i] & board;
			if (blockers & attackers)
			{
				uint8_t blockerIndex;
				if (i == 0 || i == 2) blockerIndex = lsbIndex(blockers);
				else blockerIndex = msbIndex(blockers);
				if (attackers & (uint64_t(1) << blockerIndex)) return true;
			}
		}

		// diagonally attacked
		attackers = stateStack[stackIndex].bitboards[WHITE_QUEEN] | stateStack[stackIndex].bitboards[WHITE_BISHOP];
		for (int i = 0; i < 4; ++i)
		{
			uint64_t blockers = diagonalRays[square][i] & board;
			if (blockers & attackers)
			{
				uint8_t blockerIndex;
				if (i == 0 || i == 3) blockerIndex = lsbIndex(blockers);
				else blockerIndex = msbIndex(blockers);
				if (attackers & (uint64_t(1) << blockerIndex)) return true;
			}
		}

		// attacked by a knight
		if (knightAttacks[square] & stateStack[stackIndex].bitboards[WHITE_KNIGHT]) return true;

		// attacked by a pawn
		if ((((stateStack[stackIndex].bitboards[WHITE_PAWN] & 0xfefefefefefefefe) >> 9) | ((stateStack[stackIndex].bitboards[WHITE_PAWN] & 0x7f7f7f7f7f7f7f7f) >> 7)) & (uint64_t(1) << square)) return true;

		// attacked by a king
		if (kingAttacks[square] & stateStack[stackIndex].bitboards[WHITE_KING]) return true;
	}

	return false;
}

uint8_t Chessboard::whiteKingSquare()
{
	return lsbIndex(stateStack[stackIndex].bitboards[WHITE_KING]);
}

uint8_t Chessboard::blackKingSquare()
{
	return lsbIndex(stateStack[stackIndex].bitboards[BLACK_KING]);
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
