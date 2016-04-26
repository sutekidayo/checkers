
#include "GUI.h"
#include "dblookup.h"
#include "checkersplayer.h"
#include <fstream>
#include <sstream>
#include <iostream>

Board b;
// CreateFileName
// Uses stringstreams to create a dynamic file name based on which generation it is
std::string CreateFileName ( const std::string &basename, int number, const string &ending)
{
	std::ostringstream result;
	result << basename << number << ending;
	return result.str();
}

bool readFromFile(CheckersPlayer &Best, int genNumber) {
	//load the file
	std::ifstream file(CreateFileName("BEST",genNumber,".nn"));
	
	
	// file does not exist, or didn't open
	if (!file) {
		std::cerr<<"ERROR: Cannot read from file \n";
        return false;}

	vector <float> tmpweights;
    // go until the end of the file is reached
    while(!file.eof()) {
        
		std::string line;
        unsigned int float_bits;
        // if read error, not due to EOF
		if (!file && !file.eof()) return false;
		
		while(getline(file, line, ','))
		{
			//more crazy pointer magic to convert the bits to a float
			file >> float_bits;
			unsigned int * float_bits_addr = &float_bits;
			float score = *(float *)float_bits_addr;
			tmpweights.push_back(score);
		}
		
		
  	}
	Best.PutWeights(tmpweights);
	if (tmpweights == Best.Lobotomy())
			std::cout << "\nLoaded Successfully!";
	else
		std::cout <<"\nError! Weights not loaded successfully!";
	std::ifstream hash(CreateFileName("BEST",genNumber,".hash"));
	if (!hash)
		return false;
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
	//drawMove(0);
	DrawCheckerPieces(b);
	glPopMatrix();

	glutSwapBuffers();
	glutPostRedisplay();


}

void myIdle()
{
	
	static bool red_player = true;
	static bool loadnet = true;
	static bool first_run = false;
	static bool configure = true;
	static vector<int> hidden_layers;
	if (configure)
	{
	hidden_layers.push_back(40);
	hidden_layers.push_back(10);
	hidden_layers.push_back(20);
	hidden_layers.push_back(10);
	configure = false;
	}
	static CheckersPlayer NN1(54, 1, 2, hidden_layers);
	static CheckersPlayer NN2(54, 1, 2, hidden_layers);
	if (loadnet)
	{
		// This is where you load the net! pass it the CheckersPlayer you want to load,
		// and then the generation from whence you want to load it.
		if(!readFromFile(NN2,327))
			std::cerr<< "ERROR LOADING FILE!!" << std::endl;
		//if(!readFromFile(NN2,100))
			//std::cerr<< "ERROR LOADING FILE!!" << std::endl;
		loadnet = false;
		NN2.seen_it.clear();
	}
	
	bool forced_capture;
	int num_pieces;
	if(!first_run)
	{
		vector< vector<int> > moves = b.gen_moves(red_player,forced_capture, num_pieces);
		if (b.check_win_loss(red_player,(int)moves.size()) == 0) //keep playing
		{
			if (red_player)
			{
				b.move(b.choose_move(red_player, NN1.Brain, NN1.seen_it,3));
			}
			else
				b.move(b.choose_move(red_player, NN2.Brain, NN2.seen_it,3));

			red_player = !red_player; // change turns
		}
	}

	glutPostRedisplay();
}


int main(int argc, char ** argv)
{
	srand(time(0));
	char * mesg = new char[2048]; //message buffer for the DB initialization
	db_init(64,mesg);			  //Initialize the endgame DB. First param is cache size in MB
	std::cout << mesg;
	delete mesg;				  //delete the buffer
	GUIinit(argc, argv);
	glutDisplayFunc(myDisplay);
	glutIdleFunc(myIdle);
	glutPostRedisplay();
	glutMainLoop();
	return 0;
}
