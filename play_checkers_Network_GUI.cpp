#include <iostream>
#include "GUI.h"
#include "dblookup.h"
#include "checkersplayer.h"
#include <osl/socket.h>
#include <osl/socket.cpp>
#include "network.h"
#include "timer.h"
#include <fstream>
#include <limits>
#include <iomanip>
#include <string>
using std::cout;
using std::cin;
using std::endl;
using std::string;

//MACROS! Because scrolling is stupid.
#define NUM_PLY 4

#define NUM_HIDDEN_LAYERS_1 2
const int SIZE_OF_HIDDEN_LAYERS_1[NUM_HIDDEN_LAYERS_1] = {80,20};
#define LOAD_NET_1 true
#define LOAD_HASH_1 true
#define USE_PIECE_DIFF_1 false
#define PAUSE falsee
#define PAUSE_TIME 500
#define CHECK_DRAW false
#define MOVES_TILL_DRAW 200


Board game;//(0x56000000u,0x08000000u,0x5e000000u);

// CreateFileName
// Uses stringstreams to create a dynamic file name based on which generation it is
std::string CreateFileName ( const std::string &basename, int number, const string &ending)
{
	std::ostringstream result;
	result << basename << number << ending;
	return result.str();
}

bool readFromFile(CheckersPlayer & Best, int genNumber) {
	//load the file
	std::ifstream file(CreateFileName("BEST",genNumber,".nn").c_str());


	// file does not exist, or didn't open
	if (!file) {
		std::cerr<<"ERROR: Cannot read from file \n";
        return false;}
    // go until the end of the file is reached
	vector <float> tmpweights;
    while(!file.eof()) {
		std::string line;

        // if read error, not due to EOF
		if (!file && !file.eof()) return false;

		while(getline(file, line, ','))
		{
			std::istringstream floatme(line);
			float floated;
			unsigned int float_bits;
			unsigned int * float_bit_ptr;
			floatme >> float_bits;
			float_bit_ptr = & float_bits;
			floated = *((float *)float_bit_ptr);
			tmpweights.push_back(floated);
		}
  	}
	Best.PutWeights(tmpweights);

	std::ifstream hash(CreateFileName("BEST",genNumber,".hash").c_str());
	if (!hash)
	{
		std::cerr<<"ERROR: Cannot read from file \n";
		return false;
	}
	while(!hash.eof()){
		std::string line;

		if(!hash && !hash.eof()) return false;

		while(getline(hash,line))
		{
			load_hash(Best.seen_it, line);
		}
	}

	return true; //read completed without errors
}



// Glut Display function
void myDisplay(void)
{
	glClearColor(0.2,0.2,0.2,0.0); // background color
	glClear(GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT);
	//glEnable(GL_DEPTH_TEST);
	//glDepthMask(GL_TRUE);
	//glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_BLEND);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(-1, -1.,0.);
	glScalef(.25,.25,0.);
	DrawCheckerBoard();
	DrawCheckerPieces(game);
	glPopMatrix();

	glutSwapBuffers();
	glutPostRedisplay();


}

void myIdle()
{
	static bool red_player = false;
	static bool are_we_red = true;
	static bool loadnet = true;
	static bool first_run = false;
	static vector<int> hidden_layers_1(NUM_HIDDEN_LAYERS_1);
	static SOCKET sock;
	static string ip_addr;
	static int port = 0;
	for(int i=0; i<NUM_HIDDEN_LAYERS_1; i++)
	{
		hidden_layers_1[i] = SIZE_OF_HIDDEN_LAYERS_1[i];
	}
	static CheckersPlayer NN1(54, 1, NUM_HIDDEN_LAYERS_1, hidden_layers_1);
	if (loadnet)
	{
		// This is where you load the net! pass it the CheckersPlayer you want to load,
		// and then the generation from whence you want to load it.
		if(LOAD_NET_1)
		{
			cout<<"Enter file number to load"<<endl;
			int file_no;
			cin>>file_no;
			cin.ignore();
			readFromFile(NN1,file_no);
		}
		if(!LOAD_HASH_1)
			NN1.seen_it.clear();
		else
			cout<<"Loaded hash of size: "<<NN1.seen_it.size()<<endl;
		loadnet = false;

		//Set up a socket connected to the server
		cout<<"Enter IP to connect to:"<<endl;
		getline(cin,ip_addr);
		cout<<"Enter port # ";

		cin>>port;
		sock = skt_connect(skt_lookup_ip(ip_addr.c_str()),port,5);
		string color= skt_recv_string(sock,"\n");
		if(color == "black")
			are_we_red = false;
		else if(color == "white")
			are_we_red = true;
		else
		{
			std::cerr<<"Error: did not receive side to play"<<endl;
			exit(0);
		}
		cout<<"Playing as "<<(are_we_red?"red":"black")<<". Waiting to begin"<<endl;

		//wait for game to start (starting board will be sent)
		string ignore_me;
		skt_select1(sock,0);
		ignore_me = skt_recv_string(sock,"\n\r");


	}
	bool forced_capture;
	int num_pieces;
	static int num_moves =0;
	if(!first_run)
	{
		//game.print();
		vector< vector<int> > moves = game.gen_moves(red_player,forced_capture, num_pieces);
		vector<int> one_move;
		if (game.check_win_loss(red_player,(int)moves.size()) == 0 && (!CHECK_DRAW || num_moves < MOVES_TILL_DRAW)) //keep playing
		{
			if(red_player != are_we_red)
			{
				//recive move
				//cout<<"waiting for Data"<<endl;
				skt_select1(sock,0);
				//cout<<"Recieving move..."<<endl;
				std::string net_move = skt_recv_string(sock,"\n\r");
				//cout<<(red_player?"Red":"Black")<<" sent "<<endl<<net_move<<endl;
				//cout<<"Move Received!"<<endl;
				//convert to temporary board
				Board tmp = network_to_board(net_move);
				if(error_check())
				{
					std::cerr<<"Opponent sent invalid board"<<std::endl;
					skt_sendN(sock,"invalid\n",13);
					skt_close(sock);
					exit(0);
				}
				//calculate move
				std::vector<int> move = boards_to_move(red_player,game,tmp);
				if(error_check())
				{
					std::cerr<<"Opponent made illegal move"<<std::endl;
					skt_sendN(sock,"invalid\n",13);
					skt_close(sock);
					exit(0);
				}
				//make move
				game.move(move);
				cout<<(red_player?"Red":"Black")<<" moved ";
				for(int i=0; i<(int)move.size()-1; i++)
					cout<<move[i]<<"-";
				cout<<move[move.size()-1]<<endl;
			}
			if (red_player == are_we_red)
			{
				if(!USE_PIECE_DIFF_1)
					one_move=game.choose_move(red_player, NN1.Brain, NN1.seen_it,NUM_PLY);
				else
					one_move=game.choose_move_piece_diff(red_player, NN1.Brain, NN1.seen_it,NUM_PLY);
				//vector< vector<int> > my_moves =game.gen_moves(red_player,forced_capture, num_pieces);
				//cout<<"Evaluation of current Board by Red: "<<std::setprecision(std::numeric_limits<double>::digits10)<<game.evaluate(my_moves,NN1.Brain)<<std::endl;


				game.move(one_move);
				std::string net_move = board_to_network(game);
				net_move.push_back('\n');
				//cout<<"Sending move..."<<endl;
				//cout<<(red_player?"Red":"Black")<<" sent "<<endl<<net_move<<endl;
				skt_sendN(sock,net_move.c_str(),net_move.size());
				//cout<<"Move Sent"<<endl;
				cout<<(red_player?"Red":"Black")<<" moved ";
				for(int i=0; i<(int)one_move.size()-1; i++)
					cout<<one_move[i]<<"-";
				cout<<one_move[one_move.size()-1]<<endl;

			}
			num_moves++;
			red_player = !red_player; // change turns
		}
		else
		{
			if(game.check_win_loss(red_player,(int)moves.size()) == 1)
			{
				if(red_player)
				{
					cout<<"Red wins"<<endl;
					//send red wins
					skt_sendN(sock,"white wins\n",11);
				}
				else
				{
					cout<<"Black wins"<<endl;
					//send black wins
					skt_sendN(sock,"black wins\n",11);
				}
			}
			else if(game.check_win_loss(red_player,(int)moves.size()) == -1)
			{
				if(red_player)
				{
					cout<<"Black wins"<<endl;
					//send black wins
					skt_sendN(sock,"black wins\n",11);
				}
				else
				{
					cout<<"Red wins"<<endl;
					//send red wins
					skt_sendN(sock,"white wins\n",11);
				}
			}
			else
			{
				cout<<"Draw"<<endl;
				//send draw
				skt_sendN(sock,"declared a draw\n",16);
			}
			skt_close(sock);
			exit(0);

		}
	}
	glutPostRedisplay();
}

void exiter()
{
	std::cout<<"Press Enter to exit"<<std::endl;
	std::cin.ignore();
}

int main(int argc, char ** argv)
{
	srand(time(0));
	char * mesg = new char[2048]; //message buffer for the DB initialization
	db_init(64,mesg);			  //Initialize the endgame DB. First param is cache size in MB
	delete mesg;				  //delete the buffer
	atexit(exiter);
	GUIinit(argc, argv);
	glutDisplayFunc(myDisplay);
	glutIdleFunc(myIdle);
	glutPostRedisplay();
	glutMainLoop();
	return 0;
}
