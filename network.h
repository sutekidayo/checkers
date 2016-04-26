//network.h
//4-3-10
//Matthew Chandler

#ifndef __NETWORK_H__
#define __NETWORK_H__
#include "board.h"
#include <string>

//convert board to network format
std::string board_to_network(const Board & board);

//convert network format to board
Board network_to_board(const std::string & buff);

//convert difference between boards to move
std::vector<int> boards_to_move(bool red_player, const Board & old_board, const Board & new_board);

bool error_check();


#endif
