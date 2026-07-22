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

// minimax search function
int Search::minimax(Chessboard& board, int depth, int alpha, int beta)
{
	++nodesSearched;

	// check for terminal states
	int terminal = board.isTerminal();
	if (terminal)
	{
		if (terminal == WHITE) return 100000 + depth;
		if (terminal == BLACK) return -100000 - depth;
		return 0;
	}

	// depth limit
	if (depth <= 0) return evaluate(board);

	// generate moves
	Move moves[218];
	int numMoves;
	board.pseudoMoves(moves, numMoves);

	// determine and return the best score
	if (board.turn() == WHITE)
	{
		// if it is white's turn, try to maximize the score
		int bestScore = -1000000;

		// loop through all moves
		for (int i = 0; i < numMoves; ++i)
		{
			board.move(moves[i]);
			if (!board.isAttacked(board.whiteKingSquare(), BLACK))
			{
				// if the move is legal, score it and update best score
				int score = minimax(board, depth - 1, alpha, beta);
				if (score > bestScore) bestScore = score;
			}
			board.undo();

			// alpha-beta pruning
			if (bestScore > alpha) alpha = bestScore;
			if (alpha >= beta) break;
		}
		return bestScore;
	}
	else
	{
		// if it is black's turn, try to minimize the score
		int bestScore = 1000000;

		// loop through all moves
		for (int i = 0; i < numMoves; ++i)
		{
			board.move(moves[i]);
			if (!board.isAttacked(board.blackKingSquare(), WHITE))
			{
				// if the move is legal, score it and update best score
				int score = minimax(board, depth - 1, alpha, beta);
				if (score < bestScore) bestScore = score;
			}
			board.undo();

			// alpha-beta pruning
			if (bestScore < beta) beta = bestScore;
			if (beta <= alpha) break;
		}
		return bestScore;
	}
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
	if (board.turn() == WHITE)
	{
		// if it is white's turn, try to maximize the score
		int bestScore = -10000000;

		// loop through moves
		for (int i = 0; i < numMoves; ++i)
		{
			board.move(moves[i]);
			if (!board.isAttacked(board.whiteKingSquare(), BLACK))
			{
				// if the move is legal, score it and update bestScore and bestMove
				int score = minimax(board, depth - 1, bestScore, 10000000);
				if (score > bestScore)
				{
					bestScore = score;
					bestMove = moves[i];
				}
			}
			board.undo();
		}
	}
	else
	{
		// if it is black's turn, try to minimize the score
		int bestScore = 10000000;

		// loop through moves
		for (int i = 0; i < numMoves; ++i)
		{
			board.move(moves[i]);
			if (!board.isAttacked(board.blackKingSquare(), WHITE))
			{
				// if the move is legal, score it and update bestScore and bestMove
				int score = minimax(board, depth - 1, -10000000, bestScore);
				if (score < bestScore)
				{
					bestScore = score;
					bestMove = moves[i];
				}
			}
			board.undo();
		}
	}

	// return the move that resulted in the best score
	return bestMove;
}

uint64_t Search::getNodesSearched()
{
	return nodesSearched;
}