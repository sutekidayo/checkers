//network.h
//4-3-10
//Matthew Chandler

#include "network.h"

bool error = false;


//convert board to network format
std::string board_to_network(const Board & board)
{
	std::string ret;
	for(int i=0; i<32; i++)
	{
		int num = (2*(i/4))*4+3 -i;
		bool red_flag=false, black_flag=false, king_flag=false;
		red_flag = ((board.red>>num) & 1);
		black_flag = ((board.black>>num) & 1);
		king_flag = ((board.king>>num) & 1);

		if((!red_flag && !black_flag))
			ret.push_back('_');
		if(red_flag)
		{
			if(king_flag)
				ret.push_back('W');
			else
				ret.push_back('w');
		}
		if(black_flag)
		{
			if(king_flag)
				ret.push_back('B');
			else
				ret.push_back('b');
		}
	}
	return ret;
}

//convert network format to board
Board network_to_board(const std::string & buff)
{
	Board ret;
	if(buff.size() !=32)
	{
		error = true;
		std::cerr<<"Error: recived wrong board size"<<std::endl;
		return ret;

	}
	for(int i=31; i>=0; i--)
	{
		int num = (2*(i/4))*4+3 -i;
		ret.red<<=1;
		ret.black<<=1;
		ret.king<<=1;

		switch(buff[num])
		{
		case '_':
			break;
		case 'W':
			ret.red |= 1u;
			ret.king |= 1u;
			break;
		case 'w':
			ret.red |= 1u;
			break;
		case 'B':
			ret.black |= 1u;
			ret.king |= 1u;
			break;
		case 'b':
			ret.black |= 1u;
			break;
		default:
			std::cerr<<"Error: Illegal char in string"<<std::endl;
			error = true;
			return ret;
			break;
		}
	}

	return ret;
}


//convert difference between boards to move
std::vector<int> boards_to_move(bool red_player, const Board & old_board, const Board & new_board)
{
	int num_pieces; bool forced_capture;
	std::vector<vector<int> > moves = old_board.gen_moves(red_player,forced_capture,num_pieces);
	for(int i = 0; i<(int)moves.size(); i++)
	{
		Board tmp = old_board;
		tmp.move(moves[i]);
		if(new_board == tmp)
			return moves[i];
	}
	error = true;
	return moves[0];
}

bool error_check()
{
	if(error)
	{
		error = false;
		return true;
	}
	return false;
}
