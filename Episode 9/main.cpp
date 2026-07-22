#include <iostream>
#include <sstream>
#include <GL/freeglut.h>
#include "chessboard.h"
#include "search.h"

using namespace std;

// global object for the game state
Chessboard game;

// global search object to run the engine
Search search;

// global variables for window width and height
int width = 640;
int height = 480;

// global variables for board drawing parameters
double board_x = 0.5 * (width - height);
double board_y = 0;
double board_size = height;

double square_size = board_size * 0.9 * 0.125;
double square_x = board_x + board_size * 0.05;
double square_y = board_y + board_size * 0.05;

// global variables to keep track of the selected square
int select_x = -1;
int select_y = -1;

// global varibales to keep track of promotions
int promote_x = -1;
int promote_y = -1;

// global variabls to keep track of AI moves
bool undo = false;
uint8_t AI_turn = BLACK;
int AI_depth = 7;

// stack of previous moves
Move moveStack[1000];
int moveIndex = 0;

// function to draw a chess piece
void drawPiece(int x, int y, int size, int piece)
{
	glBegin(GL_POLYGON);
	glColor3f(1, 1, 1);

	size = abs(size);

	switch (piece)
	{
	case BLACK_PAWN:
		glColor3f(0, 0, 0);
	case WHITE_PAWN:
		glVertex2i(x - size / 16, y);
		glVertex2i(x - size / 6, y + size / 3);
		glVertex2i(x + size / 6, y + size / 3);
		glVertex2i(x + size / 16, y);
		glVertex2i(x + size / 8, y - size / 16);
		glVertex2i(x + size / 8, y - size / 6);
		glVertex2i(x + size / 16, y - size / 4);
		glVertex2i(x - size / 16, y - size / 4);
		glVertex2i(x - size / 8, y - size / 6);
		glVertex2i(x - size / 8, y - size / 16);
		break;
	case BLACK_KNIGHT:
		glColor3f(0, 0, 0);
	case WHITE_KNIGHT:
		glVertex2i(x + size / 8, y);
		glVertex2i(x + size / 8, y + size / 3);
		glVertex2i(x - size / 4, y + size / 3);
		glVertex2i(x - size / 8, y - size / 3);
		glVertex2i(x - size / 16, y - size / 4);
		glVertex2i(x + size / 12, y - size / 4);
		glVertex2i(x + size / 3, y - size / 5);
		glVertex2i(x + size / 3, y);
		break;
	case BLACK_BISHOP:
		glColor3f(0, 0, 0);
	case WHITE_BISHOP:
		glVertex2i(x - size / 6, y + size / 3);
		glVertex2i(x - size / 8, y + size / 3.5);
		glVertex2i(x - size / 5, y + size / 6);
		glVertex2i(x - size / 5, y);
		glVertex2i(x - size / 32, y - size / 3);
		glVertex2i(x - size / 24, y - size / 3);
		glVertex2i(x - size / 24, y - size / 2.7);
		glVertex2i(x + size / 24, y - size / 2.7);
		glVertex2i(x + size / 24, y - size / 3);
		glVertex2i(x + size / 32, y - size / 3);
		glVertex2i(x + size / 5, y - size / 32);
		glVertex2i(x - size / 8, y + size / 4);
		glVertex2i(x + size / 5, y + size / 16);
		glVertex2i(x + size / 5, y + size / 6);
		glVertex2i(x + size / 8, y + size / 3.5);
		glVertex2i(x + size / 6, y + size / 3);
		break;
	case BLACK_ROOK:
		glColor3f(0, 0, 0);
	case WHITE_ROOK:
		glVertex2i(x, y + size / 3);
		glVertex2i(x - size / 4, y + size / 3);
		glVertex2i(x - size / 4, y + size / 4);
		glVertex2i(x - size / 6, y + size / 4);
		glVertex2i(x - size / 6, y);
		glVertex2i(x - size / 5, y);
		glVertex2i(x - size / 5, y - size / 5);
		glVertex2i(x - size / 8, y - size / 5);
		glVertex2i(x - size / 8, y - size / 10);
		glVertex2i(x - size / 24, y - size / 10);
		glVertex2i(x - size / 24, y - size / 5);
		glVertex2i(x + size / 24, y - size / 5);
		glVertex2i(x + size / 24, y - size / 10);
		glVertex2i(x + size / 8, y - size / 10);
		glVertex2i(x + size / 8, y - size / 5);
		glVertex2i(x + size / 5, y - size / 5);
		glVertex2i(x + size / 5, y);
		glVertex2i(x + size / 6, y);
		glVertex2i(x + size / 6, y + size / 4);
		glVertex2i(x + size / 4, y + size / 4);
		glVertex2i(x + size / 4, y + size / 3);
		break;
	case BLACK_KING:
		glColor3f(0, 0, 0);
	case WHITE_KING:
		glEnd();
		glBegin(GL_QUADS);
		glVertex2i(x - size / 16, y - size / 2.5);
		glVertex2i(x + size / 16, y - size / 2.5);
		glVertex2i(x + size / 16, y);
		glVertex2i(x - size / 16, y);
		glVertex2i(x - size / 8, y - size / 3.5);
		glVertex2i(x + size / 8, y - size / 3.5);
		glVertex2i(x + size / 8, y - size / 3);
		glVertex2i(x - size / 8, y - size / 3);
		glEnd();
		glBegin(GL_POLYGON);
		goto continue_draw;
	case BLACK_QUEEN:
		glColor3f(0, 0, 0);
	case WHITE_QUEEN:
	continue_draw:
		glVertex2i(x - size / 4, y + size / 3);
		glVertex2i(x - size / 3, y - size / 6);
		glVertex2i(x - size / 5, y - size / 7);
		glVertex2i(x - size / 16, y - size / 5);
		glVertex2i(x - size / 16, y - size / 3.5);
		glVertex2i(x + size / 16, y - size / 3.5);
		glVertex2i(x + size / 15, y - size / 5);
		glVertex2i(x + size / 5, y - size / 7);
		glVertex2i(x + size / 3, y - size / 6);
		glVertex2i(x + size / 4, y + size / 3);
		break;
	default:
		break;
	}
	glEnd();
}

// function to update the screen
void draw()
{
	// clear the buffer
	glClear(GL_COLOR_BUFFER_BIT);

	// draw the board background
	glColor3f(0.3, 0.15, 0.1);
	glBegin(GL_QUADS);
	glVertex2f(board_x, board_y);
	glVertex2f(board_x + board_size, board_y);
	glVertex2f(board_x + board_size, board_y + board_size);
	glVertex2f(board_x, board_y + board_size);
	glEnd();

	// draw the board squares
	glBegin(GL_QUADS);
	for (int x = 0; x < 8; ++x)
		for (int y = 0; y < 8; ++y)
		{
			if ((x + y) % 2) glColor3f(0.05, 0.3, 0.05);
			else glColor3f(0.7, 0.65, 0.55);

			glVertex2f(square_x + square_size * x, square_y + square_size * y);
			glVertex2f(square_x + square_size * (x + 1), square_y + square_size * y);
			glVertex2f(square_x + square_size * (x + 1), square_y + square_size * (y + 1));
			glVertex2f(square_x + square_size * x, square_y + square_size * (y + 1));
		}
	glEnd();

	// highlight the selected square
	if (select_x >= 0 && select_y >= 0 && select_x < 8 && select_y < 8)
	{
		glColor3f(0, 1, 0);
		glBegin(GL_QUADS);

		glVertex2f(square_x + square_size * select_x, square_y + square_size * select_y);
		glVertex2f(square_x + square_size * (select_x + 1), square_y + square_size * select_y);
		glVertex2f(square_x + square_size * (select_x + 1), square_y + square_size * (select_y + 1));
		glVertex2f(square_x + square_size * select_x, square_y + square_size * (select_y + 1));

		glEnd();
	}

	// show legal moves
	if (select_x < 8 && select_x >= 0 && select_y < 8 && select_y >= 0)
	{
		for (int x = 0; x < 8; ++x)
			for (int y = 0; y < 8; ++y)
			{
				Move move;
				move.from = select_x + 8 * select_y;
				move.to = x + 8 * y;
				move.promotion = EMPTY;

				Move promotion = move;
				promotion.promotion = game.turn() == WHITE ? WHITE_QUEEN : BLACK_QUEEN;

				if (game.isLegal(move) || game.isLegal(promotion))
				{
					glColor3f(0, 1, 1);
					glBegin(GL_QUADS);
					glVertex2f(square_x + square_size * x, square_y + square_size * y);
					glVertex2f(square_x + square_size * x, square_y + square_size * (y + 1));
					glVertex2f(square_x + square_size * (x + 1), square_y + square_size * (y + 1));
					glVertex2f(square_x + square_size * (x + 1), square_y + square_size * y);
					glEnd();
				}
			}
	}

	// show checks and terminal states
	uint8_t terminal = game.isTerminal();
	if (terminal == DRAW || game.isAttacked(game.whiteKingSquare(), BLACK))
	{
		int x = game.whiteKingSquare() % 8;
		int y = game.whiteKingSquare() / 8;

		if (terminal == BLACK) glColor3f(1, 0, 0);
		else if (terminal == DRAW) glColor3f(0, 0, 1);
		else glColor3f(1, 0, 1);

		glBegin(GL_QUADS);
		glVertex2f(square_x + square_size * x, square_y + square_size * y);
		glVertex2f(square_x + square_size * x, square_y + square_size * (y + 1));
		glVertex2f(square_x + square_size * (x + 1), square_y + square_size * (y + 1));
		glVertex2f(square_x + square_size * (x + 1), square_y + square_size * y);
		glEnd();
	}
	if (terminal == DRAW || game.isAttacked(game.blackKingSquare(), WHITE))
	{
		int x = game.blackKingSquare() % 8;
		int y = game.blackKingSquare() / 8;

		if (terminal == WHITE) glColor3f(1, 0, 0);
		else if (terminal == DRAW) glColor3f(0, 0, 1);
		else glColor3f(1, 0, 1);

		glBegin(GL_QUADS);
		glVertex2f(square_x + square_size * x, square_y + square_size * y);
		glVertex2f(square_x + square_size * x, square_y + square_size * (y + 1));
		glVertex2f(square_x + square_size * (x + 1), square_y + square_size * (y + 1));
		glVertex2f(square_x + square_size * (x + 1), square_y + square_size * y);
		glEnd();
	}

	// show the previous move
	glLineWidth(abs(square_size) * 0.15);
	glColor3f(0, 1, 0);
	glBegin(GL_LINES);
	glVertex2f(square_x + square_size * (moveStack[moveIndex - 1].from % 8 + 0.5), square_y + square_size * (moveStack[moveIndex - 1].from / 8 + 0.5));
	glVertex2f(square_x + square_size * (moveStack[moveIndex - 1].to % 8 + 0.5), square_y + square_size * (moveStack[moveIndex - 1].to / 8 + 0.5));
	glEnd();

	// draw the chess pieces
	for (int x = 0; x < 8; ++x)
		for (int y = 0; y < 8; ++y)
		{
			uint8_t square = x + y * 8;
			drawPiece(square_x + square_size * (0.5 + x), square_y + square_size * (0.5 + y), square_size, game.getPiece(square));
		}

	// draw promotion UI elements
	if (promote_y == 0)
	{
		glBegin(GL_QUADS);

		glColor3f(1, 1, 1);
		glVertex2f(square_x + square_size * (promote_x - 0.1), square_y - square_size * 0.1);
		glVertex2f(square_x + square_size * (promote_x + 1.1), square_y - square_size * 0.1);
		glVertex2f(square_x + square_size * (promote_x + 1.1), square_y + square_size * 4.1);
		glVertex2f(square_x + square_size * (promote_x - 0.1), square_y + square_size * 4.1);


		glColor3f(0, 0, 0);
		glVertex2f(square_x + square_size * promote_x, square_y);
		glVertex2f(square_x + square_size * (promote_x + 1), square_y);
		glVertex2f(square_x + square_size * (promote_x + 1), square_y + square_size * 4);
		glVertex2f(square_x + square_size * promote_x, square_y + square_size * 4);

		glEnd();

		drawPiece(square_x + square_size * (0.5 + promote_x), square_y + square_size * 0.5, square_size, WHITE_QUEEN);
		drawPiece(square_x + square_size * (0.5 + promote_x), square_y + square_size * 1.5, square_size, WHITE_ROOK);
		drawPiece(square_x + square_size * (0.5 + promote_x), square_y + square_size * 2.5, square_size, WHITE_BISHOP);
		drawPiece(square_x + square_size * (0.5 + promote_x), square_y + square_size * 3.5, square_size, WHITE_KNIGHT);
	}
	if (promote_y == 7)
	{
		glBegin(GL_QUADS);

		glColor3f(0, 0, 0);
		glVertex2f(square_x + square_size * (promote_x - 0.1), square_y + square_size * 8.1);
		glVertex2f(square_x + square_size * (promote_x + 1.1), square_y + square_size * 8.1);
		glVertex2f(square_x + square_size * (promote_x + 1.1), square_y + square_size * 3.9);
		glVertex2f(square_x + square_size * (promote_x - 0.1), square_y + square_size * 3.9);


		glColor3f(1, 1, 1);
		glVertex2f(square_x + square_size * promote_x, square_y + square_size * 8);
		glVertex2f(square_x + square_size * (promote_x + 1), square_y + square_size * 8);
		glVertex2f(square_x + square_size * (promote_x + 1), square_y + square_size * 4);
		glVertex2f(square_x + square_size * promote_x, square_y + square_size * 4);

		glEnd();

		drawPiece(square_x + square_size * (0.5 + promote_x), square_y + square_size * 7.5, square_size, BLACK_QUEEN);
		drawPiece(square_x + square_size * (0.5 + promote_x), square_y + square_size * 6.5, square_size, BLACK_ROOK);
		drawPiece(square_x + square_size * (0.5 + promote_x), square_y + square_size * 5.5, square_size, BLACK_BISHOP);
		drawPiece(square_x + square_size * (0.5 + promote_x), square_y + square_size * 4.5, square_size, BLACK_KNIGHT);
	}

	// update the display by swapping buffers
	glutSwapBuffers();
}

// function to handle mouse input
void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		undo = false;

		// get the board coordinates of the click
		int click_x = floor((x - square_x) / square_size);
		int click_y = floor((y - square_y) / square_size);

		// if we need to promote, then promote a piece
		if (promote_y == 0)
		{
			if (click_x == promote_x)
			{
				uint8_t promotion = EMPTY;
				if (click_y == 0) promotion = WHITE_QUEEN;
				else if (click_y == 1) promotion = WHITE_ROOK;
				else if (click_y == 2) promotion = WHITE_BISHOP;
				else if (click_y == 3) promotion = WHITE_KNIGHT;

				if (promotion != EMPTY)
				{
					moveStack[moveIndex - 1].promotion = promotion;
					game.undo();
					game.move(moveStack[moveIndex - 1]);

					promote_x = -1;
					promote_y = -1;
				}
			}
		}
		else if (promote_y == 7)
		{
			if (click_x == promote_x)
			{
				uint8_t promotion = EMPTY;
				if (click_y == 7) promotion = BLACK_QUEEN;
				else if (click_y == 6) promotion = BLACK_ROOK;
				else if (click_y == 5) promotion = BLACK_BISHOP;
				else if (click_y == 4) promotion = BLACK_KNIGHT;

				if (promotion != EMPTY)
				{
					moveStack[moveIndex - 1].promotion = promotion;
					game.undo();
					game.move(moveStack[moveIndex - 1]);

					promote_x = -1;
					promote_y = -1;
				}
			}
		}
		// otherwise, if we have a piece selected, then move the piece (if able)
		else if (select_x < 8 && select_x >= 0 && select_y < 8 && select_y >= 0 && game.getPiece(select_x + 8 * select_y) != EMPTY)
		{
			if (click_x >= 0 && click_x < 8 && click_y >= 0 && click_y < 8)
			{
				uint8_t from = select_x + 8 * select_y;
				uint8_t to = click_x + 8 * click_y;

				moveStack[moveIndex].from = from;
				moveStack[moveIndex].to = to;
				moveStack[moveIndex].promotion = EMPTY;

				Move promotionMove = moveStack[moveIndex];
				promotionMove.promotion = game.turn() == WHITE ? WHITE_QUEEN : BLACK_QUEEN;

				if (game.isLegal(moveStack[moveIndex]) || game.isLegal(promotionMove))
				{
					game.move(moveStack[moveIndex++]);

					// check to see if we need to promote a pawn
					if (click_y == 7 && game.getPiece(to) == BLACK_PAWN || click_y == 0 && game.getPiece(to) == WHITE_PAWN)
					{
						promote_x = click_x;
						promote_y = click_y;
					}
					else
					{
						promote_x = -1;
						promote_y = -1;
					}

					select_x = -1;
					select_y = -1;
				}
				else
				{
					select_x = click_x;
					select_y = click_y;
				}
			}
		}
		// otherwise, select the square that was clicked
		else
		{
			select_x = click_x;
			select_y = click_y;
		}
	}
}

// function to handle keyboard input
void keydown(unsigned char key, int x, int y)
{
	if (key == 27) glutLeaveMainLoop(); // exit when the user presses esc
	if (key == ' ') // flip the board if the user presses the spacebar
	{
		square_x += 8 * square_size;
		square_y += 8 * square_size;
		square_size *= -1;
	}
	if (key == '\b') // undo if the user presses backspace
	{
		if (moveIndex > 0)
		{
			game.undo();
			undo = true;
			--moveIndex;
		}
	}
}

// timer function
void timer(int value)
{
	// update the display every 16ms
	glutPostRedisplay();
	glutTimerFunc(16, timer, NULL);

	// if the AI needs to move, then have the AI move
	if (!game.isTerminal() && game.turn() == AI_turn && !undo && promote_y != 0 && promote_y != 7)
	{
		draw();
		time_t start_t = clock();
		moveStack[moveIndex++] = search.bestMove(game, AI_depth);
		time_t end_t = clock();
		game.move(moveStack[moveIndex - 1]);

		double elapsed_t = end_t - start_t;
		elapsed_t /= CLOCKS_PER_SEC;

		cout << "nodes searched: " << search.getNodesSearched() << " (" << search.getNodesSearched() / elapsed_t << " nps)" << endl;
	}
}

// function to handle window resizing
void resize(int w, int h)
{
	// set global width and height
	width = w;
	height = h;

	// recalculate board parameters
	bool isFlipped = square_size < 0;
	if (width > height)
	{
		board_size = height;
		board_x = 0.5 * (width - height);
		board_y = 0;
	}
	else
	{
		board_size = width;
		board_x = 0;
		board_y = 0.5 * (height - width);
	}

	square_size = board_size * 0.9 * 0.125;
	square_x = board_x + board_size * 0.05;
	square_y = board_y + board_size * 0.05;

	if (isFlipped) keydown(' ', 0, 0);

	// reset the viewport
	glViewport(0, 0, width, height);
	glLoadIdentity();
	gluOrtho2D(0, width, height, 0);
}

// function to implement the Universal Chess Interface
void UCI()
{
	while (true)
	{
		string command = "";
		getline(cin, command);

		if (command == "uci")
		{
			cout << "id name Tutorial Engine\n";
			cout << "id author Polymath Unlimited\n";
			cout << "uciok\n";
		}
		if (command.substr(0, 7) == "isready")
		{
			cout << "readyok\n";
		}
		if (command.substr(0, 10) == "ucinewgame")
		{
			game.newGame();
		}
		if (command.substr(0, 8) == "position")
		{
			// set up a stringstream
			stringstream ss;
			ss.str(command.substr(9));

			// read starting position
			string posString = "";
			string str = "";
			while (ss >> str)
			{
				if (str == "moves") break;
				posString += str + " ";
			}

			// set up the position
			if (posString.substr(0, 8) == "startpos")
			{
				// starting position is a new game
				game.newGame();
			}
			else if (posString.substr(0, 3) == "fen")
			{
				// starting position specified by fen string
				game.setPosition(posString.substr(4));
			}

			// make moves from the given position
			string moveString = "";
			while (ss >> moveString)
			{
				Move move;
				int x1 = moveString[0] - 'a';
				int y1 = 7 - moveString[1] + '1';
				int x2 = moveString[2] - 'a';
				int y2 = 7 - moveString[3] + '1';

				move.from = x1 + 8 * y1;
				move.to = x2 + 8 * y2;
				move.promotion = EMPTY;

				if (moveString.length() > 4)
				{
					switch (moveString[4])
					{
					case 'q':
						move.promotion = game.turn() == WHITE ? WHITE_QUEEN : BLACK_QUEEN;
						break;
					case 'r':
						move.promotion = game.turn() == WHITE ? WHITE_ROOK : BLACK_ROOK;
						break;
					case 'b':
						move.promotion = game.turn() == WHITE ? WHITE_BISHOP : BLACK_BISHOP;
						break;
					case 'n':
						move.promotion = game.turn() == WHITE ? WHITE_KNIGHT : BLACK_KNIGHT;
						break;
					default:
						break;
					}
				}
				game.move(move);
			}
		}
		if (command.substr(0, 2) == "go")
		{
			// set up a stringstream
			stringstream ss;
			ss.str(command.substr(3));

			// get search depth
			int engineDepth = 0;
			string token = "";
			while (ss >> token)
				if (token == "depth") ss >> engineDepth;

			// search for the best move
			Move bestMove = search.bestMove(game, engineDepth);

			// store the move in a string
			int x1 = bestMove.from % 8;
			int y1 = bestMove.from / 8;
			int x2 = bestMove.to % 8;
			int y2 = bestMove.to / 8;

			string moveString = "";
			moveString.push_back(x1 + 'a');
			moveString.push_back(7 + '1' - y1);
			moveString.push_back(x2 + 'a');
			moveString.push_back(7 + '1' - y2);

			// handle promotions
			switch (bestMove.promotion)
			{
			case WHITE_QUEEN:
			case BLACK_QUEEN:
				moveString.push_back('q');
				break;
			case WHITE_ROOK:
			case BLACK_ROOK:
				moveString.push_back('r');
				break;
			case WHITE_BISHOP:
			case BLACK_BISHOP:
				moveString.push_back('b');
				break;
			case WHITE_KNIGHT:
			case BLACK_KNIGHT:
				moveString.push_back('n');
				break;
			default:
				break;
			}

			// print the move to the console
			cout << "bestmove " + moveString + "\n";
		}
		if (command.substr(0, 4) == "quit")
		{
			exit(0);
		}
	}
}

// main function
int main(int argc, char** argv)
{
	cout << "select uci or gui: ";
	string mode = "";
	getline(cin, mode);
	if (mode == "uci")
	{
		cout << "id name Tutorial Engine\n";
		cout << "id author Polymath Unlimited\n";
		cout << "uciok\n";

		UCI();
	}

	// initialize OpenGL
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	// create a window
	glutInitWindowSize(width, height);
	glutCreateWindow("Chess Tutorial");

	// set the clear color
	glClearColor(0, 0, 0, 1);

	// set up a 2D projection
	gluOrtho2D(0, width, height, 0);

	// register callback functions
	glutDisplayFunc(draw);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keydown);
	glutTimerFunc(16, timer, NULL);
	glutReshapeFunc(resize);

	// swap perspectives if the engine is playing white
	if (AI_turn == WHITE) keydown(' ', 0, 0);

	// enter GLUT's main rendering loop
	glutMainLoop();

	// end program
	return 0;
}