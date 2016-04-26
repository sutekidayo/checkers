//board.cpp
//Matthew Chandler
//2-27-10

//Class definitions for Checkers board.
//Stores data in 3 32-bit ints (board layout)
//Contains methods for generating moves, moving, and evaluating a position (done by NeuralNet)

#include <iomanip>
#include <limits>
#include "board.h"
#include "dblookup.h"

#ifdef _OPENMP
#include <omp.h>
#endif

//UNCOMMENT FOR TESTING ONLY
//#define NEGATE_CHOOSE_MOVE_VAL
//#define PRINT_CHOICES


//table of possible legal moves from a given position
int move_table[32][8] =
{
	{4, 5, -1, 9,		-1, -1, -1, -1},
	{5, 6, 8, 10,		-1, -1, -1, -1},
	{6, 7, 9, 11,		-1, -1, -1, -1},
	{7, -1, 10, -1,		-1, -1, -1, -1},
	{-1, 8, -1, 13,		-1, 0, -1, -1},
	{8, 9, 12, 14,		0, 1, -1, -1},
	{9, 10, 13, 15,		1, 2, -1, -1},
	{10, 11, 14, -1,	2, 3, -1, -1},
	{12, 13, -1, 17,	4, 5, -1, 1},
	{13, 14, 16, 18,	5, 6, 0, 2},
	{14, 15, 17, 19,	6, 7, 1, 3},
	{15, -1, 18, -1,	7, -1,2, -1},
	{-1, 16, -1, 21,	-1, 8, -1, 5},
	{16, 17, 20, 22,	8, 9, 4, 6},
	{17, 18, 21, 23,	9, 10, 5, 7},
	{18, 19, 22, -1,	10, 11, 6, -1},
	{20, 21, -1, 25,	12, 13, -1, 9},
	{21, 22, 24, 26,	13, 14, 8, 10},
	{22, 23, 25, 27,	14, 15, 9, 11},
	{23, -1, 26, -1,	15, -1,10, -1},
	{-1, 24, -1, 29,	-1, 16, -1, 13},
	{24, 25, 28, 30,	16, 17, 12, 14},
	{25, 26, 29, 31,	17, 18, 13, 15},
	{26, 27, 30, -1,	18, 19, 14, -1},
	{28, 29, -1,-1,		20, 21, -1, 17},
	{29, 30, -1,-1,		21, 22, 16, 18},
	{30, 31, -1,-1,		22, 23, 17, 19},
	{31, -1, -1,-1,		23, -1,18, -1},
	{-1,-1,-1,-1,		-1, 24, -1, 21},
	{-1,-1,-1,-1,		24, 25, 20, 22},
	{-1,-1,-1,-1,		25, 26, 21, 23},
	{-1,-1,-1,-1,		26, 27, 22, -1}
};



//recursive function to generate possible jumps
//Inputs:start position, piece jumped by last move, color, king status, a bitmap to pieces that have been killed by previous jumps
//a vector of int vectors to store legal moves, and an int vector containing oves that brought us to this point
void Board::gen_jumps(int start, int kill_ind, bool red_flag, bool king_flag, unsigned int killed,
	vector<vector<int> > & main_moves, vector<int> & this_move) const
{
	killed |= 1<<kill_ind;//mark the piece we jumped dead for this and subsequent jump checking
	bool flag = false;
	if(red_flag || king_flag)//moving down
	{
		if(move_table[start][2]!=-1)//SW
		{
			if(((red_flag?black:red)>>move_table[start][0] &1u) && !(killed>>move_table[start][0] &1u)
				&& !(red>>move_table[start][2] &1u) && !(black>>move_table[start][2] &1u))
			{
				//push this position and look in next position for more possible jumps
				this_move.push_back(move_table[start][2]);
				flag = true;//mark that a jump was found
				gen_jumps(move_table[start][2],move_table[start][0],red_flag,king_flag,killed,main_moves,this_move);
				this_move.pop_back();
			}
		}
		if(move_table[start][3]!=-1)//SE
		{
			if(((red_flag?black:red)>>move_table[start][1] &1u) && !(killed>>move_table[start][1] &1u)
				&& !(red>>move_table[start][3] &1u) && !(black>>move_table[start][3] &1u))
			{
				//push this position and look in next position for more possible jumps
				this_move.push_back(move_table[start][3]);
				flag = true;//mark that a jump was found
				gen_jumps(move_table[start][3],move_table[start][1],red_flag,king_flag,killed,main_moves,this_move);
				this_move.pop_back();
			}
		}
	}
	if(!red_flag || king_flag)//up
	{
		if(move_table[start][6]!=-1)//NW
		{
			if(((red_flag?black:red)>>move_table[start][4] &1u) && !(killed>>move_table[start][4] &1u)
				&& !(red>>move_table[start][6] &1u) && !(black>>move_table[start][6] &1u))
			{
				//push this position and look in next position for more possible jumps
				this_move.push_back(move_table[start][6]);
				flag = true;//mark that a jump was found
				gen_jumps(move_table[start][6],move_table[start][4],red_flag,king_flag,killed,main_moves,this_move);
				this_move.pop_back();
			}
		}
		if(move_table[start][7]!=-1)//NE
		{
			if(((red_flag?black:red)>>move_table[start][5] &1u) && !(killed>>move_table[start][5] &1u)
				&& !(red>>move_table[start][7] &1u) && !(black>>move_table[start][7] &1u))
			{
				//push this position and look in next position for more possible jumps
				this_move.push_back(move_table[start][7]);
				flag = true;//mark that a jump was found
				gen_jumps(move_table[start][7],move_table[start][5],red_flag,king_flag,killed,main_moves,this_move);
				this_move.pop_back();
			}
		}
	}
	//if no further jumps were found, add this move to the list
	if(!flag)
		main_moves.push_back(this_move);
	return;
}
//move generator
//Input: color of player to generate for
//Returns a vector of int vectors containing all possible moves
//format is:
//{{start,[intermediate, ..., intermediate], end},
//	...
//  start,[intermediate, ..., intermediate], end}}
//returns if capture is forced and the current # of pieces by reference in the arguments
//	(for evaluation function)
vector<vector<int> > Board::gen_moves(bool red_player, bool & forced_capture, int & num_pieces) const
{
	num_pieces = 0;
	//create storage for moves
	vector <vector<int> > moves;
	vector <int> one_move;
	moves.reserve(50);
	one_move.reserve(20);
	bool forced_jump = false, non_jumps_cleared = false; //flags for forcing a capture
	for(int i=0; i< 32; i++)
	{
		bool red_flag=false, black_flag=false, king_flag=false;
		red_flag = ((red>>i) & 1);
		black_flag = ((black>>i) & 1);

		if(red_flag || black_flag)
			num_pieces++;

		if((red_player && black_flag) | (!red_player && red_flag) | (!red_flag && !black_flag))
			continue;
		//now i is the location of a piece of the current player
		king_flag = ((king>>i) & 1);

		bool SW =false, SE =false, NW =false, NE =false, //availible move flags
			 jump_SW =false, jump_SE =false, jump_NW =false, jump_NE =false;

		if(red_player || king_flag) //moving down
		{
			if(move_table[i][0] !=-1) //check for availible move SW
			{
				if(!(((red_player?black:red) >> move_table[i][0]) & 1u))//check for opposing piece
				{
					if(!(((red_player?red:black) >> move_table[i][0]) & 1u))//check for friendly
						SW = true;
				}
				else if(move_table[i][2]!=-1 &&
					!(((red_player?black:red) >> move_table[i][2]) & 1u) &&
					!(((red_player?red:black) >>move_table[i][2]) & 1u))//check to see if we can jump
				{
					jump_SW = true;
					forced_jump = true;
				}
			}

			if(move_table[i][1] !=-1) //check for availible move SE
			{
				if(!(((red_player?black:red) >> move_table[i][1]) & 1u))//check for opposing piece
				{
					if(!(((red_player?red:black) >> move_table[i][1]) & 1u))//check for friendly
						SE = true;
				}
				else if(move_table[i][3]!=-1 &&
					!(((red_player?black:red) >> move_table[i][3]) & 1u) &&
					!(((red_player?red:black) >>move_table[i][3]) & 1u))//check to see if we can jump
				{
					jump_SE = true;
					forced_jump = true;
				}
			}
		}

		if(!red_player || king_flag) //moving up
		{
			if(move_table[i][4] !=-1) //check for availible move NW
			{
				if(!(((red_player?black:red) >> move_table[i][4]) & 1u))//check for opposing piece
				{
					if(!(((red_player?red:black) >> move_table[i][4]) & 1u))//check for friendly
						NW = true;
				}
				else if(move_table[i][6]!=-1 &&
					!(((red_player?black:red)>> move_table[i][6]) & 1u) &&
					!(((red_player?red:black) >>move_table[i][6]) & 1u))//check to see if we can jump
				{
					jump_NW = true;
					forced_jump = true;
				}
			}

			if(move_table[i][5] !=-1) //check for availible move NE
			{
				if(!(((red_player?black:red) >> move_table[i][5]) & 1u))//check for opposing piece
				{
					if(!(((red_player?red:black) >> move_table[i][5]) & 1u))//check for friendly
						NE = true;
				}
				else if(move_table[i][7]!=-1 &&
					!(((red_player?black:red) >> move_table[i][7]) & 1u) &&
					!(((red_player?red:black) >>move_table[i][7]) & 1u))//check to see if we can jump
				{
					jump_NE = true;
					forced_jump = true;
				}
			}
		}
		//now we have determined which directions and jumps are possible
		//first calculate multiple jumps and store to main vector
		if(forced_jump)
		{
			if(!non_jumps_cleared)
			{
				moves.clear();
				non_jumps_cleared = true;
			}
			if(jump_SW)
			{
				one_move.push_back(i);
				one_move.push_back(move_table[i][2]);
				gen_jumps(move_table[i][2],move_table[i][0],red_flag,king_flag,0u,moves,one_move);
				one_move.clear();
			}
			if(jump_SE)
			{
				one_move.push_back(i);
				one_move.push_back(move_table[i][3]);
				gen_jumps(move_table[i][3],move_table[i][1],red_flag,king_flag,0u,moves,one_move);
				one_move.clear();
			}
			if(jump_NW)
			{
				one_move.push_back(i);
				one_move.push_back(move_table[i][6]);
				gen_jumps(move_table[i][6],move_table[i][4],red_flag,king_flag,0u,moves,one_move);
				one_move.clear();
			}
			if(jump_NE)
			{
				one_move.push_back(i);
				one_move.push_back(move_table[i][7]);
				gen_jumps(move_table[i][7],move_table[i][5],red_flag,king_flag,0u,moves,one_move);
				one_move.clear();
			}
		}
		//if a jump is not forced, we store non-jump moves
		else
		{
			if(SW)
			{
				one_move.push_back(i);
				one_move.push_back(move_table[i][0]);
				moves.push_back(one_move);
				one_move.clear();

			}
			if(SE)
			{
				one_move.push_back(i);
				one_move.push_back(move_table[i][1]);
				moves.push_back(one_move);
				one_move.clear();
			}
			if(NW)
			{
				one_move.push_back(i);
				one_move.push_back(move_table[i][4]);
				moves.push_back(one_move);
				one_move.clear();
			}
			if(NE)
			{
				one_move.push_back(i);
				one_move.push_back(move_table[i][5]);
				moves.push_back(one_move);
				one_move.clear();
			}
		}
	}
	forced_capture = forced_jump;
	return moves;
}

//Input: vector of board locations { start, [intermediate, ..., intermediate,] end}
//assumes move passed to it is legal. NO CHECKING IS PERFORMED
void Board::move(const vector<int> & locations)
{
	int num_locs = (int)locations.size();
	bool red_flag=false, king_flag=false;
	//generate piece identity
	red_flag = ((red>>locations[0]) & 1);
	king_flag = ((king>>locations[0]) & 1);
	int start = locations[0], fin=0;
	int this_row = start/4, next_row=0;
	//
	for(int i=0; i<num_locs-1; i++)
	{
		fin = locations[i+1];
		next_row = fin/4;
		if((next_row > this_row + 1) | (next_row < this_row - 1))//jump
		{
			//determine which opposing piece to remove
			int remove_ind=0;
			if(fin - start == 7)
			{
				remove_ind = move_table[start][0];
			}
			else if(fin - start == 9)
			{
				remove_ind = move_table[start][1];
			}
			else if(fin - start == -9)
			{
				remove_ind = move_table[start][4];
			}
			else if(fin - start == -7)
			{
				remove_ind = move_table[start][5];
			}
			//remove the piece
			if(red_flag)
			{
				black &= ~(1u << remove_ind);
				king &= ~(1u << remove_ind);
			}
			else
			{
				red &= ~(1u << remove_ind);
				king &= ~(1u << remove_ind);
			}
		}
		//move to next part of the move
		this_row = next_row;
		start = fin;
	}
	//move active piece from start location to end
	if(red_flag)
	{
		red |= (1u << fin);
		red &= ~(1u << locations[0]);
		if(king_flag)
		{
			king |= (1u << fin);
			king &= ~(1u << locations[0]);
		}
	}
	else
	{
		black |= (1u << fin);
		black &= ~(1u << locations[0]);
		if(king_flag)
		{
			king |= (1u << fin);
			king &= ~(1u << locations[0]);
		}
	}

	if(!king_flag && ((red_flag && next_row == 7) || (!red_flag && next_row ==0))) // check for promotion
		king |= 1u << locations[num_locs -1];
}

//recursive alpha-beta pruning minimax search. to be used with choose_move
//Inputs:
//node: current board
//red_player: boolean true if the player is red, false if black
//depth: remaining depth to search
//alpha: alpha value
//beta: beta value
//net: the NeuralNet to be used for evaluating boards
//seen_hash: the hash table associated with the NeuralNet to speed up lookups of previously seen boards
//				boards evaluated in this call to alphabeta will be added to this hash table
//returns the score for current board
float alphabeta(const Board & node, bool red_player, int depth, float alpha, float beta, NeuralNet & net, Board::hash_table & seen_hash) // beta represents previous player best choice - doesn't want it if alpha would worsen it
{
	Board::hash_table::iterator it = seen_hash.find(node);//search to see if this has been seen in the database before
	if(it != seen_hash.end())
		return red_player?it->second:-it->second;
	bool forced_capture = false;
	int num_pieces =0;
	vector<vector<int> > moves = node.gen_moves(red_player,forced_capture,num_pieces);

	int win_loss = node.check_win_loss(red_player, (int)moves.size());
	if(depth ==0 || win_loss!=0)
	{
		if(win_loss ==1)
			return 5.0f + depth ; //replace with win value
		else if(win_loss ==-1)
			return -5.0f - depth; //replace with lose value

		//query the endgame DB if we are able to.
		if(num_pieces <=SIZE_OF_DB && !node.any_captures()) //Look it up in the endgame DB (DB does not work for forced capture situations)
		{
			//convert to format used by DB
			POSITION tmp;//struct to hold board data
			tmp.bm = 0x00000000u;
			tmp.wm = 0x00000000u;

			tmp.bk = 0x00000000u;
			tmp.wk = 0x00000000u;
			tmp.color = red_player?DB_WHITE:DB_BLACK;
			//conversion between Board and DB:
			//Red --> white
			//black --> black
			unsigned int red_men=(node.red &(~node.king)), black_men=(node.black &(~node.king)),
				red_king=(node.red & node.king), black_king=(node.black & node.king);

			for(int i=0; i<32; i+=4)
			{
				tmp.wm<<=4;
				tmp.wm |= (red_men >> i) & 0xFu;
				tmp.bm<<=4;
				tmp.bm |= (black_men >> i) & 0xFu;
				tmp.wk<<=4;
				tmp.wk |= (red_king >> i) & 0xFu;
				tmp.bk<<=4;
				tmp.bk |= (black_king >> i) & 0xFu;
			}
			int result = 0;
			#pragma omp critical
			{
				result = dblookup(&tmp,0);
			}
			switch(result)
			{
				case DB_WIN:
					return 4.0f + depth; //Or whatever win value should be
				case DB_LOSS:
					return -4.0f - depth;
				case DB_DRAW:
					return -1.0f;
				default: //either not looked up, or unknown
					break;
			}
		}

		if(!red_player)
		{
			vector<vector<int> > eval_moves = node.gen_moves(true,forced_capture,num_pieces);
			return -node.evaluate(eval_moves,net);
		}
		else
		{
			return node.evaluate(moves,net);
		}
	}
	vector<Board> child (moves.size(),node);
	if(moves.size() ==1)
	{
		child[0].move(moves[0]);
		alpha = std::max(alpha, -alphabeta(child[0], !red_player, depth, -beta, -alpha, net,seen_hash));
	}
	else
	{
		for(int i=0; i<(int)moves.size(); i++)
		{
			child[i].move(moves[i]);
			alpha = std::max(alpha, -alphabeta(child[i], !red_player, depth-1, -beta, -alpha, net,seen_hash));
			if (beta<=alpha)
			{
				break; // Beta cut-off
			}
		}
	}
	return alpha;
}

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
vector<int> Board::choose_move(bool red_player, NeuralNet & net, hash_table & seen_hash, int num_plies) const
{
	bool forced_capture = false;
	int num_pieces =0;
	vector< vector<int> > moves = gen_moves(red_player,forced_capture,num_pieces);
	vector<Board> child(moves.size(),*this);
	float best_move = -1000000.0f; //replace with infinity?
	int best_move_ind = 0;
	if((int)moves.size() >1)
	{
		for(int i=0; i<(int)moves.size(); i++)
		{
			child[i].move(moves[i]);
			float a = -alphabeta(child[i],!red_player,num_plies * 2-1,-1000000.0f,-best_move,net,seen_hash);//replace with infinity
			#ifdef NEGATE_CHOOSE_MOVE_VAL
				a= -a;
			#endif
			if( a > best_move)
			{
				best_move = a;
				best_move_ind = i;
			}
			#ifdef PRINT_CHOICES
				std::cout<<"move #"<<i<<" ";
				for(int j=0; j<(int)moves[i].size(); j++)
					std::cout<<moves[i][j]<<", ";
				std::cout<<"score: "<<a<<std::endl;
			#endif
			if(seen_hash.size() >= MAX_HASH_SIZE)
			{
				seen_hash.erase(seen_hash.begin());//modify to delete a range?
			}

			seen_hash.insert(std::make_pair(child[i],red_player?a:-a));
			}
	}
	#ifdef PRINT_CHOICES
		std::cout<<"chose move #"<<best_move_ind<<": ";
		for(int j=0; j<(int)moves[best_move_ind].size(); j++)
			std::cout<<moves[best_move_ind][j]<<", ";
		std::cout<<std::endl;
	#endif
	return moves[best_move_ind];
}

//FOR TESTING ONLY
//recursive alpha-beta pruning minimax search. to be used with choose_move_piece_diff
//Inputs:
//node: current board
//red_player: boolean true if the player is red, false if black
//depth: remaining depth to search
//alpha: alpha value
//beta: beta value
//net: the NeuralNet to be used for evaluating boards (not actually used)
//seen_hash: the hash table associated with the NeuralNet to speed up lookups of previously seen boards
//				boards evaluated in this call to alphabeta will be added to this hash table
//returns the score for current board
float alphabeta_piece_diff(const Board & node, bool red_player, int depth, float alpha, float beta, NeuralNet & net, Board::hash_table & seen_hash) // beta represents previous player best choice - doesn't want it if alpha would worsen it
{
	int num_pieces =0;
	bool forced_capture =false;
	vector<vector<int> > moves = node.gen_moves(red_player,forced_capture,num_pieces);

	int win_loss = node.check_win_loss(red_player, (int)moves.size());
	if(depth ==0 || win_loss!=0)
	{
		if(win_loss ==1)
			return 5.0f + depth; //replace with win value
		else if(win_loss ==-1)
			return -5.0f -depth; //replace with lose value

		if(!red_player)
		{
			//vector<vector<int> > eval_moves = node.gen_moves(true,forced_capture,num_pieces);
			return node.piece_differential(false);//node.evaluate(eval_moves,net);
		}
		else
		{
			return node.piece_differential(true);//node.evaluate(moves,net);
		}
	}

	//query the endgame DB if we are able to.
	if(num_pieces <=SIZE_OF_DB && !node.any_captures()) //Look it up in the endgame DB (DB does not work for forced capture situations)
	{
		//convert to format used by DB
		POSITION tmp;//struct to hold board data
		tmp.bm = 0x00000000u;
		tmp.wm = 0x00000000u;

		tmp.bk = 0x00000000u;
		tmp.wk = 0x00000000u;
		tmp.color = red_player?DB_WHITE:DB_BLACK;
		//conversion between Board and DB:
		//Red --> white
		//black --> black
		unsigned int red_men=(node.red &(~node.king)), black_men=(node.black &(~node.king)),
			red_king=(node.red & node.king), black_king=(node.black & node.king);

		for(int i=0; i<32; i+=4)
		{
			tmp.wm<<=4;
			tmp.wm |= (red_men >> i) & 0xFu;
			tmp.bm<<=4;
			tmp.bm |= (black_men >> i) & 0xFu;
			tmp.wk<<=4;
			tmp.wk |= (red_king >> i) & 0xFu;
			tmp.bk<<=4;
			tmp.bk |= (black_king >> i) & 0xFu;
		}
		int result = 0;
		#pragma omp critical
		{
			result = dblookup(&tmp,0);
		}
		switch(result)
		{
			case DB_WIN:
				return 4.0f + depth; //Or whatever win value should be
			case DB_LOSS:
				return -4.0f -depth ;
			case DB_DRAW:
				return -1.0f;
			default: //either not looked up, or unknown
				break;
		}
	}
	vector<Board> child (moves.size(),node);
	if(moves.size() ==1)
	{
		child[0].move(moves[0]);
		alpha = std::max(alpha, -alphabeta_piece_diff(child[0], !red_player, depth, -beta, -alpha, net,seen_hash));
	}
	else{
		for(int i=0; i<(int)moves.size(); i++)
		{
			child[i].move(moves[i]);
			alpha = std::max(alpha, -alphabeta_piece_diff(child[i], !red_player, depth-1, -beta, -alpha, net,seen_hash));
			if (beta<=alpha)
			{
				break; // Beta cut-off
			}
		}
	}
	return alpha;
}

//FOR TESTING ONLY!!
//chooses a move
//does a combination of alpha-beta pruning minimax search, hash-table lookup,
//	endgame DB lookup, and piece differeintial evaluation to find the optimal move for a given position.
//Inputs:
//red_player: boolean true if the player is red, false if black
//net: the NeuralNet to be used for evaluating boards
//seen_hash: the hash table associated with the NeuralNet to speed up lookups of previously seen boards
//				boards evaluated in this call to choose_move will be added to this hash table
//num_plies: the number of plies to search, where 1 ply is a move and a response
//returns a move in this format: { start, [intermediate, ..., intermediate,] end}
vector<int> Board::choose_move_piece_diff(bool red_player, NeuralNet & net, hash_table & seen_hash, int num_plies) const
{
	bool forced_capture = false;
	int num_pieces =0;
	vector< vector<int> > moves = gen_moves(red_player,forced_capture,num_pieces);
	vector<Board> child(moves.size(),*this);
	float best_move = -1000000.0f; //replace with infinity?
	int best_move_ind = 0;
	if((int)moves.size() >1)
	{
		for(int i=0; i<(int)moves.size(); i++)
		{
			child[i].move(moves[i]);
			float a = -alphabeta_piece_diff(child[i],!red_player,num_plies * 2-1,-1000000.0f,-best_move,net,seen_hash);//replace with infinity
			#ifdef NEGATE_CHOOSE_MOVE_VAL
				a= -a;
			#endif
			if( a > best_move)
			{
				best_move = a;
				best_move_ind = i;
			}
			#ifdef PRINT_CHOICES
				std::cout<<"move #"<<i<<" ";
				for(int j=0; j<(int)moves[i].size(); j++)
					std::cout<<moves[i][j]<<", ";
				std::cout<<"score: "<<a<<std::endl;
			#endif
			if(seen_hash.size() >= MAX_HASH_SIZE)
			{
				seen_hash.erase(seen_hash.begin());//modify to delete a range?
			}

			seen_hash.insert(std::make_pair(child[i],red_player?a:-a));
			}
	}
	#ifdef PRINT_CHOICES
		std::cout<<"chose move #"<<best_move_ind<<": ";
		for(int j=0; j<(int)moves[best_move_ind].size(); j++)
			std::cout<<moves[best_move_ind][j]<<", ";
		std::cout<<std::endl;
	#endif
	return moves[best_move_ind];
}



bool Board::any_captures() const
{
	bool red_flag=false, black_flag=false, king_flag=false;
	for(int i=0; i< 32; i++)
	{
		red_flag = ((red>>i) & 1);
		black_flag = ((black>>i) & 1);
		king_flag = ((king>>i) & 1);
		if(!red_flag && !black_flag)
			continue;
		if(red_flag)
		{
			if(move_table[i][0]!=-1 && move_table[i][2]!=-1 &&((black >> move_table[i][0]) & 1u) &&
					!((red >> move_table[i][2]) & 1u) && !((black >> move_table[i][2]) & 1u))
				return true;
			if(move_table[i][1]!=-1 && move_table[i][3]!=-1 &&((black >> move_table[i][1]) & 1u) &&
					!((red >> move_table[i][3]) & 1u) && !((black >> move_table[i][3]) & 1u))
				return true;
			if(king_flag)
			{
				if(move_table[i][4]!=-1 && move_table[i][6]!=-1 &&((black >> move_table[i][4]) & 1u) &&
						!((red >> move_table[i][6]) & 1u) && !((black >> move_table[i][6]) & 1u))
					return true;
				if(move_table[i][5]!=-1 && move_table[i][7]!=-1 &&((black >> move_table[i][5]) & 1u) &&
						!((red >> move_table[i][7]) & 1u) && !((black >> move_table[i][7]) & 1u))
					return true;
			}

		}
		else
		{
			if(move_table[i][4]!=-1 && move_table[i][6]!=-1 &&((red >> move_table[i][4]) & 1u) &&
					!((red >> move_table[i][6]) & 1u) && !((black >> move_table[i][6]) & 1u))
				return true;
			if(move_table[i][5]!=-1 && move_table[i][7]!=-1 &&((red >> move_table[i][5]) & 1u) &&
					!((red >> move_table[i][7]) & 1u) && !((black >> move_table[i][7]) & 1u))
				return true;
			if(king_flag)
			{
				if(move_table[i][0]!=-1 && move_table[i][2]!=-1 &&((red >> move_table[i][0]) & 1u) &&
						!((red >> move_table[i][2]) & 1u) && !((black >> move_table[i][2]) & 1u))
					return true;
				if(move_table[i][1]!=-1 && move_table[i][3]!=-1 &&((red >> move_table[i][1]) & 1u) &&
						!((red >> move_table[i][3]) & 1u) && !((black >> move_table[i][3]) & 1u))
					return true;
			}
		}
	}
	return false;
}


//verifies that a given move is legal for the current board
//Inputs
//move, whose format is { start, [intermediate, ..., intermediate,] end}
//red_player: boolean true if the player is red, false if black
bool Board::verify_move(const vector<int> & move, bool red_player) const
{
	bool dummy_bool =false;
	int dummy_int = 0;
	vector<vector<int> > all_moves = gen_moves(red_player,dummy_bool, dummy_int);
	for(int i=0; i<(int)all_moves.size(); i++)
	{
		if(move == all_moves[i])
			return true;
	}
	return false;
}

//Calculates the difference in pieces to get a good estimate of current winner
float Board::piece_differential(bool red_player) const
{
	float piece_diff =0.0f;
	bool red_flag=false, black_flag=false, king_flag=false;
	for(int i=0; i< 32; i++)
	{
		red_flag = ((red>>i) & 1);
		black_flag = ((black>>i) & 1);
		king_flag = ((king>>i) & 1);
		if(!red_flag && !black_flag)
			continue;
		if(red_flag)
		{
			if(king_flag)
				piece_diff +=1.3f;
			else
				piece_diff +=1.0f;
		}
		else
		{
			if(king_flag)
				piece_diff -=1.3f;
			else
				piece_diff -=1.0f;
		}
	}
	if(red_player)
		return piece_diff;
	else
		return -piece_diff;

}

//Calls the NeuralNet to evaluate this board
//Evaluates the board for Red's advantage
//Prereqs: Must have generated moves, must have initialized end-game DB.
float Board::evaluate(vector<vector<int> > & moves, NeuralNet & Net) const
{	//need to have a flag if any kings for each side
	//individual piece counts
	//need to determine how to pass to NN

	vector <float> inputs(54,0);

	/*
	input scheme
	0-31:	board layout
	32:		piece differential
	33:		ADV
	34:		APEX
	35:		BACK
	36:		CENT
	37:		CNTR
	38:		CRAMP
	39:		DIA
	40:		DIAV
	41:		DYKE
	42:		EXPOS
	43:		GAP
	44:		GUARD
	45:		HOLE
	46:		KCENT
	47:		NODE
	48:		OREO
	49:		POLE
	50:		BACK2
	51:		OREO2
	52:		GUARD2
	53:		Number of availible moves
	*/

	char board_layout[8][8] = //Kings do not matter here. either ' ', 'R', or 'B'
	{
		{'*','*','*','*','*','*','*','*'},
		{'*','*','*','*','*','*','*','*'},
		{'*','*','*','*','*','*','*','*'},
		{'*','*','*','*','*','*','*','*'},
		{'*','*','*','*','*','*','*','*'},
		{'*','*','*','*','*','*','*','*'},
		{'*','*','*','*','*','*','*','*'},
		{'*','*','*','*','*','*','*','*'}
	};
	char king_layout[8][8] = //Locations of kings. either ' ', 'R', or 'B'
	{
		{'*','*','*','*','*','*','*','*'},
		{'*','*','*','*','*','*','*','*'},
		{'*','*','*','*','*','*','*','*'},
		{'*','*','*','*','*','*','*','*'},
		{'*','*','*','*','*','*','*','*'},
		{'*','*','*','*','*','*','*','*'},
		{'*','*','*','*','*','*','*','*'},
		{'*','*','*','*','*','*','*','*'}
	};
	bool any_red_king = false, any_black_king = false;
	//expand board into float inputs
	bool red_flag = false, black_flag =false, king_flag = false;
	for(int i=0; i<32; i++)
	{
		int row = i/4, column = (i % 4) * 2 - (row % 2) + 1;
		red_flag = ((red>>i) & 1);
		black_flag = ((black>>i) & 1);
		king_flag = ((king>>i) & 1);
		if(red_flag)
		{
			board_layout[row][column] = 'R';
			if(king_flag)
			{
				any_red_king = true;
				king_layout[row][column] = 'R';
				inputs[i] = 1.3f;
				inputs[32] += 1.3f;

			}
			else
			{
				inputs[i] = 1.0f;
				inputs[32] += 1.0f;
			}
		}
		else if(black_flag)
		{
			board_layout[row][column] = 'B';
			if(king_flag)
			{
				any_black_king = true;
				king_layout[row][column] = 'B';
				inputs[i] = -1.3f;
				inputs[32] -= 1.3f;
			}
			else
			{
				inputs[i] = -1.0f;
				inputs[32] -=1.0f;
			}
			//ADV
			if(i>=8 && i<=15)
				inputs[33] -=1.0f;
			if(i>=16 && i<=23)
				inputs[33] +=1.0f;
		}
		else
		{
			board_layout[row][column] =' ';
			king_layout[row][column] =' ';
			inputs[i] = 0.0f;
		}
	}
	for(int i=1; i<7; i++) //loops for DYKE/EXPOS/GAP/HOLE/NODE/POLE calcs
	{
		for(int j=1; j<7; j++)
		{
			if(board_layout[i][j] == '*') continue; //skip non-playing squares
			char mid = board_layout[i][j], NW = board_layout[i-1][j-1], NE = board_layout[i-1][j+1],
				SW = board_layout[i+1][j-1], SE = board_layout[i+1][j+1];
			if(mid == ' ')
			{
				//GAP (part 1)
				if((NW == 'B' && SE == 'B') || (NE == 'B' && SW == 'B'))
					inputs[43] +=1.0f;
				//HOLE
				if(((NW == 'B') + (NE == 'B') + (SW == 'B') + (SE == 'B'))  >= 3)
					inputs[45] += 1.0f;
			}
			else if(mid == 'B')
			{
				//DYKE
				if(NW == 'B' && SE == 'B')
					inputs[41] +=1.0f;
				if(NE == 'B' && SW == 'B')
					inputs[41] +=1.0f;
				//EXPOS
				if((NW == ' ' && SE == ' ') || (NE == ' ' && SW == ' '))
					inputs[42] +=1.0f;
				//NODE
				if(((NW == ' ') + (NE == ' ') + (SW == ' ') + (SE == ' '))  >= 3)
					inputs[47] +=1.0f;
				//POLE
				if((NW == ' ') && (NE == ' ') && (SW == ' ') && (SE == ' '))
					inputs[49] +=1.0f;

			}

		}
	}

	//GAP (part 2)
	for(int i=1; i<=7; i+=2)//top row
	{
		if(board_layout[0][i] == ' ')
		{
			if(board_layout[1][i-1] == 'B' || ((i<7) && board_layout[1][i+1] == 'B'))
				inputs[43] +=1.0f;

		}
	}
	for(int i=0; i<=6; i+=2)//bottom row
	{
		if(board_layout[7][i] == ' ')
		{
			if(board_layout[6][i+1] == 'B' || ((i>0) && board_layout[6][i-1] == 'B'))
				inputs[43] +=1.0f;

		}
	}
	for(int i=1; i<=5; i+=2)
	{
		if(board_layout[i][0] == ' ')
		{
			if(board_layout[i-1][1] == 'B' || board_layout[i+1][1] == 'B')
				inputs[43] +=1.0f;

		}
	}
	for(int i=2; i<=6; i+=2)
	{
		if(board_layout[i][7] == ' ')
		{
			if(board_layout[i-1][6] == 'B' || board_layout[i+1][6] == 'B')
				inputs[43] +=1.0f;

		}
	}

	//loops for CENT, CNTR, KCENT
	for(int i=2; i<=5; i++)
	{
		for(int j=2; j<=5; j++)
		{
			if(board_layout[i][j] == '*')
				continue;
			//CENT
			if(board_layout[i][j] == 'B')
				inputs[36] += 1.0f;
			//CNTR part 1
			if(board_layout[i][j] == 'R')
				inputs[37] += 1.0f;
			//KCENT
			if(king_layout[i][j] == 'B')
				inputs[46] += 1.0f;
		}
	}
	//CNTR (part 2)
	int * mobile_pieces = new int[moves.size()];
	int num_unique = 0;
	//check for unique pieces that can move into the center area
	for(int i=0; i<(int)moves.size(); i++)
	{
		//check to see if the piece has been counted already
		if(num_unique != 0 && mobile_pieces[num_unique-1] == moves[i][0])
			continue;
		int move_to = moves[i][moves[i].size()-1], move_from = moves[i][0];
		if((move_to == 9 || move_to ==10  || move_to ==13 || move_to ==14 ||
			move_to ==17 || move_to ==18 || move_to ==21 || move_to ==22) &&
			!(move_from == 9 || move_from ==10  || move_from ==13 || move_from ==14 ||
			move_from ==17 || move_from ==18 || move_from ==21 || move_from ==22))
		{
			inputs[37] += 1.0f;
			mobile_pieces[num_unique++] = move_from;
		}
	}
	delete [] mobile_pieces;

	bool back_flag = false;
	//APEX
	if(!any_red_king && !any_black_king &&
			(board_layout[1][4] == 'R' || board_layout[6][3] == 'R') &&
			(board_layout[1][4] != 'B' && board_layout[6][3] != 'B'))
		inputs[34] = 1.0f;
	//BACK
	if(!any_red_king && board_layout[7][2] == 'B' && board_layout[7][6] == 'B')
	{
		inputs[35] = 1.0f;
		back_flag = true;
	}

	//BACK2
	bool back2_flag = false;

	if(board_layout[0][1]=='R' && board_layout[0][5]=='R')
	{
		inputs[50] = 1.0f;
		back2_flag = true;
	}
	//CRAMP
	if(board_layout[4][7] == 'B' && (board_layout[5][6] == 'B' || board_layout[4][5] == 'B') &&
			(board_layout[1][6] == 'R' && board_layout[2][5] == 'R' &&
			board_layout[2][7] == 'R' && board_layout[3][6] == 'R'))
		inputs[38] = 2.0f;
	//DIA and DIAV (part 1)
	for(int i=0; i<=6; i++)
	{
		if(board_layout[i][i+1] == 'B' || board_layout[i+1][i] == 'B')
		{
			inputs[39] += 1.0f;
			inputs[40] += 1.5f;
		}
	}
	//DIAV (part 2)
	for(int i=0; i<=4; i++)
	{
		if(board_layout[i][i+3] == 'B' || board_layout[i+3][i] == 'B')
			inputs[40] += 1.0f;
	}
	for(int i=0; i<=2; i++)
	{
		if(board_layout[i][i+5] == 'B' || board_layout[i+5][i] == 'B')
			inputs[40] += 0.5f;
	}
	bool oreo_flag =false;
	//OREO
	if(board_layout[6][3] == 'B' && board_layout[7][2] == 'B' && board_layout[7][4] == 'B')
	{
		oreo_flag = true;
		if(!any_black_king)
			inputs[48] = 1.0f;
	}
	//OREO2
	bool oreo2_flag =false;
	if(board_layout[0][3] == 'R' && board_layout[0][5] == 'R' && board_layout[1][4] == 'R')
	{
		oreo2_flag = true;
		inputs[51] =1.0f;
	}
	//GUARD
	if(!any_red_king && (back_flag || oreo_flag))
		inputs[44] = 1.0f;

	//GUARD2
	if(back2_flag && oreo2_flag)
		inputs[52] = 1.0f;

	inputs[53] = (float)moves.size();

	vector<float> output = Net.Update(inputs);
	return output[0];
}


//check to see if a player (color denoted by red_player input) has won
//returns
//-1 for loss
//0 for neither win nor loss
//1 for win
//note that it does not attempt to determine draws.
int Board::check_win_loss(bool red_player, int num_moves) const
{
	if(num_moves == 0)
		return -1;
	int num_red = 0, num_black = 0;
	for(int i=0; i< 32; i++)
	{
		bool red_flag=false, black_flag=false;
		red_flag = ((red>>i) & 1);
		black_flag = ((black>>i) & 1);

		if(red_flag)
			num_red++;
		if(black_flag)
			num_black++;
	}
	if(num_black >0 && num_red>0)
		return 0;
	if(num_black ==0)
	{
		if(red_player)
			return 1;
		else
			return 1;
	}
	if(num_red ==0)
	{
		if(red_player)
			return -1;
		else
			return 1;
	}
	return 0;
}
//print the board to the given output stream (assumes cout by default)
void Board::print(std::ostream & stream) const
{
	int row=0;
	stream<<"\t\tRed"<<std::endl;
	stream<<"    0   1   2   3   4   5   6   7"<<std::endl;
	stream<<"  +---+---+---+---+---+---+---+---+"<<std::endl;
	bool red_flag=false, black_flag=false, king_flag=false;
	for(int i=0; i< 32; i++)
	{
		string disp = "  ";
		red_flag = ((red>>i) & 1);
		black_flag = ((black>>i) & 1);
		king_flag = ((king>>i) & 1);
		if(red_flag)
		{
			if(king_flag)
				disp = "RK";
			else
				disp = "R ";
		}
		if(black_flag)
		{
			if(king_flag)
				disp = "BK";
			else
				disp = "B ";
		}

		if(i%4 == 0)
			stream<<row<<" ";
		if(row %2 == 0)
			stream<<"|###| "<<disp;
		else
			stream<<"| "<<disp<<"|###";

		if(i%4 == 3)
		{
			row++;
			stream<<"|"<<std::endl<<"  +---+---+---+---+---+---+---+---+"<<std::endl;
		}
	}
	stream<<"\t\tBlack"<<std::endl;
}

//hash function
std::size_t hash_value(const Board & a)
{
	std::size_t seed = 0;
	boost::hash_combine(seed, a.red);
	boost::hash_combine(seed, a.black);
	boost::hash_combine(seed, a.king);
	return seed;
}

//function to dump a given hashtable to a string
std::string dump_hash(const Board::hash_table & hash)
{
	std::ostringstream out;
	int i = 0;
	for(Board::hash_table::const_iterator it = hash.begin(); it != hash.end(); it++)
	{
		out<<(it->first.red)<<","<<(it->first.black)<<","<<(it->first.king)<<",";

		//and now for crazy pointer magic to store the bits of a float reather than risk losing precision
		const float * ptr = &(it->second);
		out<<(*(unsigned int *)ptr);
		if(i<(int)hash.size() -1)
			out<<";";
		i++;
	}
	return out.str();
}

//function to load up a hash table given a string in the same format as returned from dump_hash
void load_hash(Board::hash_table & hash, const std::string & in)
{
	std::istringstream in_stream(in);
	while(!in_stream.eof())
	{
		unsigned int red = 0u, black = 0u, king = 0u;
		unsigned int score_bits;
		unsigned int * score_bits_addr;
		float score = 0.0f;

		std::string tmp_str;
		std::getline(in_stream,tmp_str,';');
		std::istringstream tmp_stream(tmp_str);

		std::string token_str;
		std::getline(tmp_stream,token_str,',');
		std::istringstream token_stream(token_str);
		token_stream>>red;

		std::getline(tmp_stream,token_str,',');
		token_stream.clear();
		token_stream.str(token_str);
		token_stream>>black;

		std::getline(tmp_stream,token_str,',');
		token_stream.clear();
		token_stream.str(token_str);
		token_stream>>king;

		std::getline(tmp_stream,token_str);
		token_stream.clear();
		token_stream.str(token_str);
		token_stream>>score_bits;
		//more crazy pointer magic to convert the bits to a float
		score_bits_addr = & score_bits;
		score = *(float *)score_bits_addr;


		Board tmp_board(red,black,king);
		token_stream.clear();
		hash.insert(std::make_pair(tmp_board,score));
	}
}
