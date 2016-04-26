// tournament.cpp
// Travis Payton
// 03 - 20 - 10

// This code is what trains the Neural Nets
// It will play each Neural Net against 5 other random Neural Nets
// Each checkersplayer's fitness level will increase 1 for each game won
// And it will decrease 2 for each game lost.  It will also decrease 1 for each game drawn.
//    (We Want Winners!!)
// The mutation then is performed by calling the genetic algorithm code found in genalg.h and genalg.cpp
// It also outputs the current generations info csv .nn file
// This file contains the weights, fitness, and other necissary files so
// that training can be continued on at a later date if neccissary.
// And also incase you want to look at how the generations have been changing over time

#include <fstream>
#include "helper.h"
#include "board.h"
#include "checkersplayer.h"
#include "genalg.h"
using std::cout;
using std::cin;
using std::cerr;
#include "dblookup.h"
#ifdef _OPENMP
#include <omp.h>
#endif

#define NUM_PLY 2
#define NUM_HIDDEN_LAYERS 2
const int SIZE_OF_HIDDEN_LAYERS [NUM_HIDDEN_LAYERS] = {80,20};
#define GRAPH_IT true
const int pop_size = 18;
int gamescompleted;
int roundgamescompleted;
// CreateFileName
// Uses stringstreams to create a dynamic file name based on which generation it is
std::string CreateFileName ( const std::string &basename, int number, const string &ending)
{
	std::ostringstream result;
	result << basename << number << ending;
	return result.str();
}

// CreateCSV
// This function takes the info on the generation and out puts into a CSV file
// Pre Conditions:  NONE
// Post Conditions:  The data is stored in a file of the specified name

void CreateCSV (vector <CheckersPlayer> &generation, int genNumber)
{
	std::ofstream csvfile;
	csvfile.open(CreateFileName("generation",genNumber,".nn").c_str());
	for (int i = 0; i < generation.size(); ++i)
	{
		vector <float> tmpweights = generation[i].Lobotomy();
		for (int j = 0; j < tmpweights.size(); ++j)
		{
			//and now for crazy pointer magic to store the bits of a float reather than risk losing precision
			const float * ptr = &(tmpweights[j]);
			csvfile<<(*(unsigned int *)ptr) << " , ";
		}
		csvfile << "\n";
	}
	csvfile.close();
}

void CreateCSV (CheckersPlayer &BEST, int genNumber)
{
	std::ofstream csvfile;
	csvfile.open(CreateFileName("BEST",genNumber,".nn").c_str());
	vector <float> tmpweights = BEST.Lobotomy();
	for (int j = 0; j < tmpweights.size(); ++j)
		{
			//and now for crazy pointer magic to store the bits of a float reather than risk losing precision
			const float * ptr = &(tmpweights[j]);
			csvfile<<(*(unsigned int *)ptr) << " , ";
		}
	csvfile.close();
	csvfile.open(CreateFileName("BEST",genNumber,".hash").c_str());
	csvfile << dump_hash(BEST.seen_it);
	csvfile.close();

}

void UpdateGraph (vector<CheckersPlayer> &population, int genNumber)
{
	float totalfitness = 0;
		for (int i =0; i < population.size(); ++i)
		 	totalfitness += population[i].Fitness();
	  CheckersPlayer Best = population[population.size()-1], Worst = population[0];
	  cout << "\nBest: "<< population[population.size()-1].Fitness() << "		Worst: "<< population[0].Fitness() << "		Average: " << totalfitness/ pop_size;
	  // update GRAPH
      std::ofstream graphfile;
	  if (genNumber > 0 )
		  graphfile.open(CreateFileName("CHART",1,".csv").c_str(), std::ios::app); // just append onto it.
	  else
		graphfile.open(CreateFileName("CHART",genNumber,".csv").c_str());
	  // Play Best and Worst against a piece counter and store the score
	  Best.fitnessLevel = 0;
	  Worst.fitnessLevel = 0;
	  Board game;
	  bool red_player = true;
	  bool forced_capture = false;
	  int num_pieces = 0;
	  int numturns = 0;  // if it gets past 100 -- its a draw
	  bool switch_sides = false;
	  while(true)
	  {
	      vector<vector<int> > moves = game.gen_moves(red_player,forced_capture,num_pieces);
	      int result = game.check_win_loss(red_player, (int)moves.size()); //check for win/loss
	      if(result == 1) //current player won
	      {
	          if(red_player)
			  {
				  if(!switch_sides)
	              	Best.Win(game, red_player);
				  else
					Best.Loss(game,red_player);
			  }
	          else
			  {
				  if(!switch_sides)
			  	  	Best.Loss(game,red_player);
				  else
					Best.Win(game, red_player);
			  }
			  if (switch_sides)
			  {
				cout << "\n Played 2 round against Piece Counter, Fitness : " << Best.Fitness();
			  	break;
			  }
			  else 
			  {
				cout << "\n Played 1 round against Piece Counter, Fitness : " << Best.Fitness();
				 switch_sides = true;
				game.set();
				numturns = 0;
				continue;
			  }
				
	      }
	      if(result == -1) //current player lost
	      {
	          if(red_player)
			  {
	              if(!switch_sides)
				  	Best.Loss(game, red_player);
				  else
					  Best.Win(game,red_player);
			  }
	          else
			  {
	             if(!switch_sides)
				 	Best.Win(game, red_player);
				 else
					Best.Loss(game,red_player);
			  }

			  if (switch_sides)
			  {
				cout << "\n Played 2 round against Piece Counter, Fitness : " << Best.Fitness();
			  	break;
			  }
			  else 
			  {
				cout << "\n Played 1 round against Piece Counter, Fitness : " << Best.Fitness();
				switch_sides = true;
				game.set();
				numturns = 0;
				continue;
			  }
	      }
		  if (!switch_sides)
		  {
	      if (red_player)
	          game.move(game.choose_move(red_player, Best.Brain, Best.seen_it,NUM_PLY));
	      else
	          game.move(game.choose_move_piece_diff(red_player, Best.Brain, Best.seen_it, NUM_PLY));
		  }
		  else
		  {
			  if (red_player)
	        	game.move(game.choose_move_piece_diff(red_player, Best.Brain, Best.seen_it, NUM_PLY));  
	      	  else
	        	game.move(game.choose_move(red_player, Best.Brain, Best.seen_it,NUM_PLY));  
		  }
	      numturns++;
		   if(numturns >=300) //check for a draw
	          if (switch_sides)
			  {
				cout << "\n Played 2 round against Piece Counter DRAW, Fitness : " << Best.Fitness();
			  	break;
			  }
			  else 
			  {
				cout << "\n Played 1 round against Piece Counter--DRAW, Fitness : " << Best.Fitness();
				switch_sides = true;
				game.set();
				numturns = 0;
				continue;
			  }
	      red_player = !red_player; //change players
	  }

	  graphfile << "Generation: " << genNumber << " , " << Best.Fitness() << "\n";	  
	  graphfile.close();
		

}

// readFromFile
//  reads weights from a file into a CheckersPlayer's Brain
//  return == false if the read failed
//  return == true if the read succeeded
// Does not throw (No-throw Guarantee)
//  ***********  !!!!  MUST READ BACK FLOATS FROM A FILE SAVED WITH CreateCSV FROM THE SAME TYPE OF NEURAL NET !!!! ***********************

bool readFromFile(vector <CheckersPlayer> &population, int genNumber) {
	//load the file
	std::ifstream file(CreateFileName("generation",genNumber,".nn").c_str());
	int playernum = 0;

	// file does not exist, or didn't open
	if (!file) {
		std::cerr<<"ERROR: Cannot read from file \n";
        return false;}
    // go until the end of the file is reached
	vector <float> tmpweights;
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

		++playernum;



    }
	population[playernum].PutWeights(tmpweights);
	return true; //read completed without errors
}

// playGame
// This function plays a game between 2 players
// and adjusts their fitness accordingly
void playGame(CheckersPlayer &player1, CheckersPlayer &player2)
{
  Board game;
  bool red_player = true;
  bool forced_capture = false;
  int num_pieces = 0;
  int numturns = 0;  // if it gets past 100 -- its a draw
  while(true)
  {
      vector<vector<int> > moves = game.gen_moves(red_player,forced_capture,num_pieces);
      int result = game.check_win_loss(red_player, (int)moves.size()); //check for win/loss
      if(result == 1) //current player won
      {
          if(red_player)
		  {
              player1.Win(game, red_player);
			  if (player2.Fitness() > 0)
				  player2.Loss(game,red_player);
		  }
          else
		  {
              player2.Win(game, red_player);
		  	  if (player1.Fitness() > 0)
				  player2.Loss(game,red_player);
		  }
          return;
      }
      if(result == -1) //current player lost
      {
          if(red_player)
		  {
              player2.Win(game, red_player);
			  if (player1.Fitness() > 0)
				  player2.Loss(game,red_player);
		  }
          else
		  {
              player1.Win(game, red_player);
			  if (player2.Fitness() > 0)
				  player2.Loss(game,red_player);
		  }
          return;
      }

      if (red_player)
          game.move(game.choose_move(red_player, player1.Brain, player1.seen_it,NUM_PLY));
      else
          game.move(game.choose_move(red_player, player2.Brain, player2.seen_it,NUM_PLY));
      numturns++;
	   if(numturns >=300) //check for a draw
          break;
      red_player = !red_player; //change players
  }
}
// This function plays a match between 2 players
// Changes each player's performance according to their performance
void playMatch (CheckersPlayer &player1, CheckersPlayer &player2)
{
	playGame(player1, player2);
	++roundgamescompleted;
	++gamescompleted;
	if (roundgamescompleted % 4 == 0)
	{ // print a status update
		//cout << "\nRound is " << 100*((float)roundgamescompleted / ((float)pop_size / 2)) << "% complete";
		cout << "\nGeneration is " << 100*((float)gamescompleted / (((float)pop_size / 2)*(pop_size - 1))) <<"% complete";
	}




}

int main ()
{
	srand(time(NULL));
	char * mesg = new char[2048];
	db_init(64,mesg);
	cout << mesg << std::endl;
	db_infostring(mesg);
	cout << mesg << std::endl;
	delete mesg;

	int genNumber = 0;
	bool load = false;
	std::cout << "Would you like to load a generation? ('Y' to load/ anything else to start at generation 0)";
	char answer;
	std::cin >> answer;
	if (answer == 'y' || answer == 'Y')
		load = true;
	vector<int> hidden_layers(NUM_HIDDEN_LAYERS);
	for(int i=0; i<NUM_HIDDEN_LAYERS; i++)
	{
		hidden_layers[i] = SIZE_OF_HIDDEN_LAYERS[i];
	}
	vector <CheckersPlayer> population;
	// Create Population of Neural Nets
	for( int i = 0; i<pop_size;++i)
	{
		CheckersPlayer player(54,1,NUM_HIDDEN_LAYERS,hidden_layers);
		population.push_back(player);
	}

	if (load)
	{
		cout << "What is the generation number you want to load?";
		cin >> genNumber;
		if(!readFromFile(population, genNumber))
		{
			cerr << "\nERROR: File didn't load successfully";
			return 0;
		}

		cout << "\nLoaded successfully!";

	}
	else
		CreateCSV(population, genNumber); // starting over!!


	while (true)
	{
		cout << "\n Training Generation " << genNumber << std::endl;
		gamescompleted = 0;
		roundgamescompleted = 0;
		int round = 1;
		int player2 = pop_size-1; // population is 0 -> pop_size - 1
		int nextplayer=0;
		vector<int> games;

		// Set up and play round 1;
		for(int player1 = 0; player1 < pop_size / 2; ++player1)
		{
			if (player1 == player2)
			{
				++player1;
				--player2;
				continue; // don't play yourself *hence pop_size - 1 rounds*
			}
			games.push_back(player1);  // stores it as player1 vs player 2 for cycling through the round robin game
			games.push_back(player2); // in the later rounds

			--player2;
		}
		#pragma omp parallel for
		for (int i = 0; i < games.size()-1; i+=2)
			playMatch(population[games[i]],population[games[i+1]]);


		roundgamescompleted = 0;
		cout << "\nRound " << round << " completed! " << pop_size - round - 1 << " rounds to go\n";

		++round;
		while (round < pop_size) // play the rest of the round robin tournament
		{

			vector <int> previous = games;

			// set up the rest of the round based on previous round
			for (int player = 0; player < pop_size; ++player)
			{
				if (player == 0)
				{
					// set up the first three players
					games[1] = previous[3];
					games[2] = previous[1];
					player+=3;
				}
				else if (player == pop_size - 2)
				{ // set up the last 2 players
					games[player] = previous[player - 2];
					games[player+1] = previous[player];
					break; // finished scheduling the tournament
				}
				// match 3 -> second of last match
				if (player % 2 == 0)
					games[player] = previous[player - 2];
				else
					games[player] = previous[player + 2];

			}

			if (round % 2 == 0)
			{
			#pragma omp parallel for
			for (int i = 0; i < games.size()-1; i+=2)
				playMatch(population[games[i+1]], population[games[i]]);
			}
			else
			{
			#pragma omp parallel for
			for (int i = 0; i < games.size()-1; i+=2)
				playMatch(population[games[i]], population[games[i+1]]);

			}
			cout << "\nRound " << round << " completed! " << pop_size - round - 1 << " rounds to go\n";
			++round;
			roundgamescompleted = 0;
		}

		cout << "\nFinished training this generation! Saving info";

		std::sort(population.begin(),population.end());
		cout<< "\nPopulation: " << population.size() << "\n Sorted!\n";
		
		CreateCSV(population[population.size()-1],genNumber);
		if(GRAPH_IT)
			UpdateGraph(population,genNumber);
		
		genNumber++;
		//Update the  GA with current population
		GenAlg pGA(population, .03, .7);

		//storage for the population of genomes for genetic algorithm
		vector<SGenome> PopulationGenomes = pGA.GetChromos();

		cout << "\nMutating population";



		//run the GA to create a new population
		PopulationGenomes = pGA.Epoch(PopulationGenomes);

		cout << "\nMutated!!";

		CreateCSV(population, genNumber);
		// put the new *hopefully* better Genomes back into the population
		for (int i=0; i<pop_size; ++i)
		{

			population[i].fitnessLevel = 0.0;
			if ( i > 5)
			{ // GenAlg keeps the top six, no need to reset their cache and redo their weights
			population[i].seen_it.clear();
			population[i].PutWeights(PopulationGenomes[i].vecWeights);
			}
		}

		games.clear();
	}


	return 0;
}
