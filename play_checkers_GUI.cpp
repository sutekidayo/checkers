#include "dblookup.h"
#include "checkersplayer.h"
#include "timer.h"
#include <fstream>
#include <limits>
#include <iomanip>

//MACROS! Because scrolling is stupid.
#define NUM_PLY 4

#define NUM_HIDDEN_LAYERS_1 2
const int SIZE_OF_HIDDEN_LAYERS_1[NUM_HIDDEN_LAYERS_1] = {80,20};
#define LOAD_NET_1 true
#define KEEP_NET_1 true
#define NET_FILE_1 281 //THIS IS SPARTA!
#define LOAD_HASH_1 true
#define USE_PIECE_DIFF_1 false

#define NUM_HIDDEN_LAYERS_2 2
const int SIZE_OF_HIDDEN_LAYERS_2[NUM_HIDDEN_LAYERS_2] = {80,20};
#define LOAD_NET_2 true
#define KEEP_NET_2 true
#define NET_FILE_2 281
#define LOAD_HASH_2 true
#define USE_PIECE_DIFF_2 false


#define RED_FIRST true
#define PAUSE false
#define PAUSE_TIME 500
#define VS_RED false
#define VS_BLACK true

#include "matt_GUIvers2.h"

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
		std::cerr<<"ERROR: Cannot read from file"<<std::endl;
        return false;}
    // go until the end of the file is reached
	vector <float> tmpweights;
    while(!file.eof()) {
		std::string line;

        // if read error, not due to EOF
		if (!file && !file.eof()) return false;

		while(getline(file, line, ','))
		{
			if(file.eof()) break;
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
	std:: vector<float> mattstemp = Best.Lobotomy();
	if (tmpweights == Best.Lobotomy())
	{
		std::cout << "Loaded BEST"<<genNumber<<" Successfully!"<<std::endl;
	}
	else
	{
		std::cout <<"Error! Weights not loaded successfully!"<<std::endl;
	}

	std::ifstream hash(CreateFileName("BEST",genNumber,".hash").c_str());
	if (!hash)
	{
		std::cerr<<"ERROR: Cannot read hashtable from file"<<std::endl;
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
	glDisable(GL_DEPTH_TEST);
    glLoadIdentity();
	if (menu)
	{
    	glClearColor(0., 0., 0., 0.);
		glClear(GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		glPushMatrix();
		// Draw Menu
    	drawMenu();
		glPopMatrix();
	}
	else
	{
		glClearColor(0.9,0.9,0.9,0.0); // background color
		glClear(GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glTranslatef(-1, -1.,0.);
		glScalef(.25,.25,0.);
		DrawCheckerPieces();
		if (!hum_move.empty())
			for (int i = 0; i < (int)possible.size(); ++i)
				drawMove(possible[i]);

		glPopMatrix();
	}
	glutSwapBuffers();
	glutPostRedisplay();


}

void myIdle()
{
	static bool loadnet = true;
	static bool first_run = true;
	static vector<int> hidden_layers_1(NUM_HIDDEN_LAYERS_1);
	for(int i=0; i<NUM_HIDDEN_LAYERS_1; i++)
	{
		hidden_layers_1[i] = SIZE_OF_HIDDEN_LAYERS_1[i];
	}
	static vector<int> hidden_layers_2(NUM_HIDDEN_LAYERS_2);
	for(int i=0; i<NUM_HIDDEN_LAYERS_2; i++)
	{
		hidden_layers_2[i] = SIZE_OF_HIDDEN_LAYERS_2[i];
	}
	static CheckersPlayer NN1(54, 1, NUM_HIDDEN_LAYERS_1, hidden_layers_1);
	static CheckersPlayer NN2(54, 1, NUM_HIDDEN_LAYERS_2, hidden_layers_2);
	if (loadnet)
	{
		// This is where you load the net! pass it the CheckersPlayer you want to load,
		// and then the generation from whence you want to load it.
		if(LOAD_NET_1)
			readFromFile(NN1,NET_FILE_1);
		if(LOAD_NET_2)
		readFromFile(NN2,NET_FILE_2);

		if(!LOAD_HASH_1)
			NN1.seen_it.clear();
		else
			std::cout<<"red loaded hash of size: "<<NN1.seen_it.size()<<std::endl;
		if(!LOAD_HASH_2)
			NN2.seen_it.clear();
		else
			std::cout<<"black loaded hash of size: "<<NN2.seen_it.size()<<std::endl;
		loadnet = false;

	}
	bool forced_capture;
	int num_pieces;
	static int num_moves =0;
	static int red_points =0, black_points =0, draws =0;
	if(first_run)
	{
		red_player = RED_FIRST;
		first_run = false;
	}
	else
	{
		//b.print();
		vector< vector<int> > moves = b.gen_moves(red_player,forced_capture, num_pieces);
		vector<int> one_move;
		if (b.check_win_loss(red_player,(int)moves.size()) == 0 && num_moves < 100) //keep playing
		{
			if (red_player && !VS_BLACK)
			{
				if(!USE_PIECE_DIFF_1)
					one_move=b.choose_move(red_player, NN1.Brain, NN1.seen_it,NUM_PLY);
				else
					one_move=b.choose_move_piece_diff(red_player, NN1.Brain, NN1.seen_it,NUM_PLY);
				vector< vector<int> > my_moves =b.gen_moves(red_player,forced_capture, num_pieces);
				//std::cout<<"Evaluation of current Board by Red: "<<std::setprecision(std::numeric_limits<double>::digits10)<<b.evaluate(my_moves,NN1.Brain)<<std::endl;
				//std::cout<<dump_hash(NN1.seen_it)<<std::endl;
				b.move(one_move);
				num_moves++;
				red_player = !red_player; // change turns
			}
			else if (!red_player && !VS_RED)
			{
				if(!USE_PIECE_DIFF_2)
					one_move=b.choose_move(red_player, NN2.Brain, NN2.seen_it,NUM_PLY);
				else
					one_move=b.choose_move_piece_diff(red_player, NN2.Brain, NN2.seen_it,NUM_PLY);
				vector< vector<int> > my_moves =b.gen_moves(red_player,forced_capture, num_pieces);
				//std::cout<<"Evaluation of current Board by Black: "<<-b.evaluate(my_moves,NN2.Brain)<<std::endl;
				//std::cout<<dump_hash(NN2.seen_it)<<std::endl;
				b.move(one_move);
				num_moves++;
				red_player = !red_player; // change turns
			}

		}
		else
		{
			b.set();
			vector <float> weights1(NN1.GetNumberOfWeights(),randfloat(-5,5));
			vector <float> weights2(NN2.GetNumberOfWeights(),randfloat(-5,5));
			num_moves=0;
			if(b.check_win_loss(red_player,(int)moves.size()) == 1)
			{
				if(red_player)
				{
					red_points++;
				}
				else
				{
					black_points++;
				}
			}
			else if(b.check_win_loss(red_player,(int)moves.size()) == -1)
			{
				if(red_player)
				{
					black_points++;
				}
				else
				{
					red_points++;
				}
			}
			else
			{
				draws++;
			}
			#if defined WIN32
				system("cls");
			#else
				char esc = 0x1B;
				std::cout << esc << "[2J";	// clear console
				std::cout << esc << "[H";	// reset cursor to the home position
			#endif
			std::cout<<"Score:\nRed: "<<red_points<<" Black: "<<black_points<<" Draw: "<<draws<<std::endl;
			red_player = RED_FIRST;

			if(!KEEP_NET_1)
			{
				NN1.PutWeights(weights1);
				NN1.seen_it.clear();
			}
			if(!KEEP_NET_2)
			{
				NN2.PutWeights(weights2);
				NN2.seen_it.clear();
			}
		}
	}
	//if (PAUSE)
		//timer_pause(PAUSE_TIME);

	glutPostRedisplay();
}


int main(int argc, char ** argv)
{
	std::cout<<"Score:\nRed: "<<0<<" Black: "<<0<<" Draw: "<<0<<std::endl;
	srand(time(0));
	char * mesg = new char[2048]; //message buffer for the DB initialization
	db_init(64,mesg);			  //Initialize the endgame DB. First param is cache size in MB
	delete mesg;				  //delete the buffer
	GUIinit(argc, argv);
	glutDisplayFunc(myDisplay);
	glutIdleFunc(myIdle);
	glutPostRedisplay();
	glutMainLoop();
	return 0;
}
