// checkersplayer.h
// Travis Payton
// 3 - 19 - 2010

// This class holds all the information to create a checkers playing Neural Net work
// Also has accessor functions for easy access to the Neural Net that controls it.


#ifndef CHECKERSPLAYER_H_INCLUDED
#define CHECKERSPLAYER_H_INCLUDED

#include "NeuralNet.h"
#include "board.h"

class CheckersPlayer
{

	public:
		// CheckersPlayer's brain
		NeuralNet Brain;
		
		
		// holds the hash table for this Neural Net
		// it's like its short term memory!
		Board::hash_table seen_it;

		
		// The CheckersPlayer's fitness score
		float fitnessLevel;

	public:


	CheckersPlayer(int iNumInputs, int iNumOutputs, int iNumHidden,
			vector<int> &iNeuronsPerHiddenLayer)
	{
		Brain.CreateNet(iNumInputs, iNumOutputs, iNumHidden, iNeuronsPerHiddenLayer);
		fitnessLevel = 0;
	}

  

	//-------------------accessor functions

	void Win(Board &game, bool &red_player)
	{	
		int num_red= 0, num_black=0;
		for(int i=0; i< 32; i++)
		{
			bool red_flag=false, black_flag=false;
			red_flag = ((game.red>>i) & 1);
			black_flag = ((game.black>>i) & 1);
			if(red_flag)
				num_red++;
			if(black_flag)
				num_black++;
		}
		if (red_player) // playing as red
				fitnessLevel += .2*num_red; // add in .2 * how many pieces remaining
		else
				fitnessLevel += .2*num_black;
			
	}
	
	void Loss(Board & game, bool & red_player)
	{
		int num_red= 0, num_black=0;
		for(int i=0; i< 32; i++)
		{
			bool red_flag=false, black_flag=false;
			red_flag = ((game.red>>i) & 1);
			black_flag = ((game.black>>i) & 1);
			if(red_flag)
				num_red++;
			if(black_flag)
				num_black++;
		}
		if (red_player) // playing as red
				fitnessLevel -= 1 - .2*num_black; // subtract .2 * how many pieces left for opponent ... 
		else
				fitnessLevel -= 1 - .2*num_red;  

		if (fitnessLevel < 0)
			fitnessLevel = 0; // can't go below 0 to make the GenAlg work.
			
	}

	float Fitness()const{return fitnessLevel;}
  
  	void PutWeights(vector<float> &w){Brain.PutWeights(w);}

  	int GetNumberOfWeights()const{return Brain.GetNumberOfWeights();}

  	vector <float> Lobotomy () const {return Brain.GetWeights();}

	//overload '<' used for sorting
	friend bool operator<(const CheckersPlayer& lhs, const CheckersPlayer& rhs)
	{
		return (lhs.fitnessLevel < rhs.fitnessLevel);
	}

};



#endif // CHECKERSPLAYER_H_INCLUDED