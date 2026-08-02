#include "search.h"
#include "bitops.h"
#include "cstdlib"

using namespace std;

// array of piece values to use for evaluation
const int pieceValues[] = { 100, 300, 300, 500, 900, 0, -100, -300, -300, -500, -900, 0, 0 };

// evaluation function
int Search::evaluate(Chessboard& board)
{
	// get the bitboards from the board state
	uint64_t* bitboards = board.getState()->bitboards;

	// loop through all bitboards and accumulate piece scores
	int score = 0;
	for (int i = 0; i < 12; ++i)
		score += popcount(bitboards[i]) * pieceValues[i];

	// return the score (positive if white is ahead, negative if black is ahead)
	return score;
}

// negamax search function
int Search::negamax(Chessboard& board, int depth, int alpha, int beta)
{
	++nodesSearched;

	// depth limit
	if (depth <= 0)
	{
		int terminal = board.isTerminal();
		if (terminal == WHITE || terminal == BLACK) return -100000 - depth;
		if (terminal == DRAW) return 0;
		return board.turn() == WHITE ? evaluate(board) : -evaluate(board);
	}

	// draws
	if (board.softDraw()) return 0;

	// generate moves
	Move moves[218];
	int numMoves;
	board.pseudoMoves(moves, numMoves);

	// determine and return the best score
	bool terminal = true;
	for (int i = 0; i < numMoves; ++i)
	{
		// try the move to see if it is legal
		board.move(moves[i]);
		if (board.turn() == WHITE ? !board.isAttacked(board.blackKingSquare(), WHITE) : !board.isAttacked(board.whiteKingSquare(), BLACK))
		{
			// if the move is legal, score it and update alpha
			terminal = false;
			int score = -negamax(board, depth - 1, -beta, -alpha);
			if (score > alpha) alpha = score;
		}
		board.undo();

		// alpha-beta pruning
		if (alpha >= beta) break;
	}

	if (terminal)
	{
		if (board.turn() == WHITE ? board.isAttacked(board.whiteKingSquare(), BLACK) : board.isAttacked(board.blackKingSquare(), WHITE))
			return -100000 - depth;
		return 0;
	}

	return alpha;
}

// function to find the best move in a given position
Move Search::bestMove(Chessboard& board, int depth)
{
	nodesSearched = 0;

	// if the state is terminal, return a garbage move
	if (board.isTerminal())
	{
		Move fallback = { 0, 0, EMPTY };
		return fallback;
	}

	// generate moves
	Move moves[218];
	int numMoves;
	board.pseudoMoves(moves, numMoves);

	// find the best move
	Move bestMove = moves[0];
	int alpha = -10000000;
	for (int i = 0; i < numMoves; ++i)
	{
		// figure out how good the move is
		board.move(moves[i]);
		if (board.turn() == WHITE ? !board.isAttacked(board.blackKingSquare(), WHITE) : !board.isAttacked(board.whiteKingSquare(), BLACK))
		{
			int score = -negamax(board, depth - 1, -10000000, -alpha);
			if (score > alpha)
			{
				alpha = score;
				bestMove = moves[i];
			}
		}
		board.undo();
	}

	// return the move that resulted in the best score
	return bestMove;
}

uint64_t Search::getNodesSearched()
{
	return nodesSearched;
}
