//board.h
//Matthew Chandler
//2-27-10

//Class declaration for Checkers board.
//Stores data in 3 32-bit ints (board layout)
//Contains methods for generating moves, moving, and evaluating a position (done by Neural Net)
#ifndef BOARD_H
#define BOARD_H

#define MAX_HASH_SIZE 10000	//max num of entries per hash table
#define SIZE_OF_DB 7        //Number of pieces in the Endgmae database

#include <iostream>
#include <vector>
#include <string>
#include <boost/functional/hash.hpp>
//#ifdef WIN32
	#include <unordered_map>
//#else
	//#include <tr1/unordered_map>
//#endif

#include "NeuralNet.h"

using std::vector;
using std::string;


class Board
{
public:
	typedef std::tr1::unordered_map <Board, float ,boost::hash<Board> > hash_table;
public:

	Board(unsigned int red_bits=0x00000FFFu, unsigned int black_bits=0xFFF00000u, unsigned int king_bits=0x00000000u)
	:red(red_bits),black(black_bits),king(king_bits)
	{}

	//dctor and copy ctor not needed because no dynamically declared mem used

	void set(unsigned int red_bits=0x00000FFFu, unsigned int black_bits=0xFFF00000u, unsigned int king_bits=0u)
	{
		red = red_bits;
		black = black_bits;
		king = king_bits;
	}

	bool operator == (const Board & a) const
	{
		return red==a.red && black==a.black && king == a.king;
	}

	//hash function (using boost)
	friend std::size_t hash_value(const Board & a);//hash function

	//recursive function to generate possible jumps
	//Inputs:start position, piece jumped by last move, color, king status, a bitmap to pieces that have been killed by previous jumps
	//a vector of int vectors to store legal moves, and an int vector containing oves that brought us to this point
	void gen_jumps(int start, int kill_ind, bool red_flag, bool king_flag, unsigned int killed,
		vector<vector<int> > & main_moves, vector<int> & this_move) const;

	//move generator
	//Input: color of player to generate for
	//Returns a vector of int vectors containing all possible moves
	//format is:
	//{{start,[intermediate, ..., intermediate], end},
	//	...
	//  start,[intermediate, ..., intermediate], end}}
	vector<vector<int> > gen_moves(bool red_player, bool & forced_capture, int & num_pieces) const;

	//Input: vector of board locations { start, [intermediate, ..., intermediate,] end}
	//assumes move passed to it is legal. NO CHECKING IS PERFORMED
	void move(const vector<int> & locations);

	//chooses a move
	//does a combination of alpha-beta pruning minimax search, hash-table lookup,
	//	endgame DB lookup, and NeuralNet evaluation to find the optimal move for a given position.
	//Inputs:
	//red_player: boolean true if the player is red, false if black
	//net: the NeuralNet to be used for evaluating boards
	//seen_hash: the hash table associated with the NeuralNet to speed up lookups of previously seen boards
	//				boards evaluated in this call to choose_move will be added to this hash table
	//num_plies: the number of plies to search, where 1 ply is a move and a response
	//returns a move in this format: { start, [intermediate, ..., intermediate,] end}
	vector<int> choose_move(bool red_player, NeuralNet & net, hash_table & seen_hash, int num_plies) const;

	//THIS FUNCTION FOR TESTING ONLY
	vector<int> choose_move_piece_diff(bool red_player, NeuralNet & net, hash_table & seen_hash, int num_plies) const;
	

	//verifies that a given move is legal for the current board
	//Inputs
	//move, whose format is { start, [intermediate, ..., intermediate,] end}
	//red_player: boolean true if the player is red, false if black
	bool verify_move(const vector<int> & move, bool red_player) const;

	//function to see if it is possible for a piece to be captured by either side
	bool any_captures() const;

	//Calculates the difference in pieces to get a good estimate of current winner
	float piece_differential(const bool red_player) const;

	//Calls the NeuralNet to evaluate this board
	//Evaluates the board for Red's advantage
	//Prereqs: Must have generated moves(for red), must have initialized end-game DB.
	float evaluate(vector<vector<int> > & moves, NeuralNet & Net) const;

	//check to see if a player (color denoted by red_player input) has won
	//returns
	//-1 for loss
	//0 for neither win nor loss
	//1 for win
	//note that it does not attempt to determine draws.
	int check_win_loss(bool red_player, int num_moves) const;

	//print the board to the given output stream (assumes cout by default)
	void print(std::ostream & stream =std::cout) const;

	unsigned int red , black, king; //compressed board data
};

//function to dump a given hashtable to a string
std::string dump_hash(const Board::hash_table & hash);
//function to load up a hash table given a string in the same format as returned from dump_hash
void load_hash(Board::hash_table & hash, const std::string & in);

#endif
