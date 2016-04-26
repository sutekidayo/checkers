// GenAlg.h
// Travis Payton
// This code is to update / mutate the weights of the Neural Nets based on genetic algorithms
// This is just a modified CGenAlg.h written in 2002 by Mat Buckland for his smart sweepers tutorial

#ifndef MUTATE_H
#define MUTATE_H


#include <algorithm>
#include <iostream>
#include <fstream>
#include "helper.h"
#include "checkersplayer.h"

//-----------------------------------------------------------------------
//
//	create a structure to hold each genome
//-----------------------------------------------------------------------
struct SGenome
{
	vector <float>	vecWeights;

	float          dFitness;

	SGenome():dFitness(0){}
	SGenome(float fitness) :dFitness(fitness){};
	SGenome( vector <float> w, float f): vecWeights(w), dFitness(f){}

	//overload '<' used for sorting
	friend bool operator<(const SGenome& lhs, const SGenome& rhs)
	{
		return (lhs.dFitness < rhs.dFitness);
	}
};

//-----------------------------------------------------------------------
//
//	the genetic algorithm class
//-----------------------------------------------------------------------
class GenAlg
{
private:
	
	//this holds the entire population of chromosomes
	vector <SGenome>	 m_vecPop;

	//size of population
	int m_iPopSize;
	
	//amount of weights per chromo
	int m_iChromoLength;

	//total fitness of population
	float m_dTotalFitness;

	//best fitness this population
	float m_dBestFitness;

	//average fitness
	float m_dAverageFitness;

	//worst
	float m_dWorstFitness;

	//keeps track of the best genome
	int		m_iFittestGenome;

	//probability that a chromosones bits will mutate.
	//Try figures around 0.05 to 0.3 ish
	float m_dMutationRate;

	//probability of chromosones crossing over bits
	//0.7 is pretty good
	float m_dCrossoverRate;

	//generation counter
	int	  m_cGeneration;


	void    Crossover(const vector<float> &mum,
                    const vector<float> &dad,
                    vector<float>       &baby1,
                    vector<float>       &baby2);
	

	void    Mutate(vector<float> &chromo);
	
	SGenome GetChromoRoulette();

	//use to introduce elitism
 	void	  GrabNBest(int	            NBest,
	                  const int       NumCopies,
	                  vector<SGenome>	&vecPop);


	void	  CalculateBestWorstAvTot();

	void	  Reset();



public:
	// ctor	
	GenAlg(vector <CheckersPlayer> &pop,
			   float	MutRat,
			   float	CrossRat);

	
	void Reload(vector <CheckersPlayer> &pop); 
	
	//this runs the GA for one generation.
	vector<SGenome>	Epoch(vector<SGenome> &old_pop);


	//-------------------accessor methods
	vector<SGenome>	GetChromos()const{return m_vecPop;}
	float			    AverageFitness()const{return m_dTotalFitness / m_iPopSize;}
	float		    	BestFitness()const{return m_dBestFitness;}

	
};

#endif

