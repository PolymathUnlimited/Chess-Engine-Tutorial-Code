#pragma once

#include "chessboard.h"

class Search
{
private:
	uint64_t nodesSearched;

	// board evaluation function
	int evaluate(Chessboard& board);

	// minimax search function
	int minimax(Chessboard& board, int depth);

public:
	// function to get the best move from a given position
	Move bestMove(Chessboard& board, int depth);

	// returns the number of nodes searched during the last bestMove call
	uint64_t getNodesSearched();
};
