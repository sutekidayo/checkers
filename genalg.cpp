#include "genalg.h"



GenAlg::GenAlg(vector <CheckersPlayer> &pop,
                 float	MutRat,
                 float	CrossRat) :	m_iPopSize(pop.size()),
                                      m_dMutationRate(MutRat),
										                  m_dCrossoverRate(CrossRat),
										                  m_iChromoLength(pop[0].GetNumberOfWeights()),
										                  m_dTotalFitness(0),
										                  m_cGeneration(0),
										                  m_iFittestGenome(0),
										                  m_dBestFitness(0),
										                  m_dWorstFitness(99999999),
										                  m_dAverageFitness(0)
{
	//initialise population with chromosomes consisting of random
	//weights and all fitnesses set to zero
	for (int i=0; i<m_iPopSize; ++i)
	{
		m_vecPop.push_back(SGenome(pop[i].Lobotomy(),pop[i].Fitness()));
	}
}

void GenAlg::Reload(vector <CheckersPlayer> &pop) 
{
    for (int i = 0; i < pop.size(); ++i)
	{
		m_vecPop[i].dFitness = pop[i].Fitness();
		m_vecPop[i].vecWeights = pop[i].Lobotomy();
	}
}
//---------------------------------Mutate--------------------------------
//
//	mutates a chromosome by perturbing its weights by an amount not 
//	greater than CParams::dMaxPerturbation
//-----------------------------------------------------------------------
void GenAlg::Mutate(vector<float> &chromo)
{
	//traverse the chromosome and mutate each weight dependent
	//on the mutation rate
	for (int i=0; i<chromo.size(); ++i)
	{
		//do we perturb this weight?
		if (randfloat(0.0,1.0) < m_dMutationRate)
		{
			//add or subtract a small value to the weight
			chromo[i] += (randfloat(-1.0,1.0) * .3);
		}
	}
}

//----------------------------------GetChromoRoulette()------------------
//
//	returns a chromo based on roulette wheel sampling
//
//-----------------------------------------------------------------------
SGenome GenAlg::GetChromoRoulette()
{
	//generate a random number between 0 & total fitness count
	float Slice = (float)(randfloat(0.0,1.0) * m_dTotalFitness);
	
	//this will be set to the chosen chromosome
	SGenome TheChosenOne;

	//go through the chromosones adding up the fitness so far
	float FitnessSoFar = 0;
	
	for (int i=0; i<m_iPopSize; ++i)
	{
		FitnessSoFar += m_vecPop[i].dFitness;
		
		//if the fitness so far > random number return the chromo at 
		//this point
		if (FitnessSoFar >= Slice)
		{
			TheChosenOne = m_vecPop[i];

      		break;
		}
		
	}
	return TheChosenOne;
	
}
	
//-------------------------------------Crossover()-----------------------
//	
//  given parents and storage for the offspring this method performs
//	crossover according to the GAs crossover rate
//-----------------------------------------------------------------------
void GenAlg::Crossover(const vector<float> &mum,
                        const vector<float> &dad,
                        vector<float>       &baby1,
                        vector<float>       &baby2)
{
	//just return parents as offspring dependent on the rate
	//or if parents are the same
	if ( (randfloat(0.0,1.0) > m_dCrossoverRate) || (mum == dad)) 
	{
		baby1 = mum;
		baby2 = dad;

		return;
	}

	//determine a crossover point
	int cp = randint(0, m_iChromoLength - 1);

	//create the offspring
	for (int i=0; i<cp; ++i)
	{
		baby1.push_back(mum[i]);
		baby2.push_back(dad[i]);
	}

	for (int i=cp; i<mum.size(); ++i)
	{
		baby1.push_back(dad[i]);
		baby2.push_back(mum[i]);
	}
	
	
	return;
}

//-----------------------------------Epoch()-----------------------------
//
//	takes a population of chromosones and runs the algorithm through one
//	 cycle.
//	Returns a new population of chromosones.
//
//-----------------------------------------------------------------------
vector<SGenome> GenAlg::Epoch(vector<SGenome> &old_pop)
{
  //assign the given population to the classes population
  m_vecPop = old_pop;

  //reset the appropriate variables
  Reset();

  //sort the population (for scaling and elitism)
  sort(m_vecPop.begin(), m_vecPop.end());

  //calculate best, worst, average and total fitness
	CalculateBestWorstAvTot();
  
  //create a temporary vector to store new chromosones
	vector <SGenome> vecNewPop;

	//Now to add a little elitism we shall add in some copies of the
	//fittest genomes. Make sure we add an EVEN number or the roulette
  //wheel sampling will crash
	GrabNBest(6, 1, vecNewPop);
	
	//now we enter the GA loop
	
	//repeat until a new population is generated
	while (vecNewPop.size() < m_iPopSize)
	{
		//grab two chromosones
		SGenome mum = GetChromoRoulette();
		SGenome dad = GetChromoRoulette();

		//create some offspring via crossover
		vector<float>		baby1, baby2;

		Crossover(mum.vecWeights, dad.vecWeights, baby1, baby2);

		//now we mutate
		Mutate(baby1);
		Mutate(baby2);

		//now copy into vecNewPop population
		vecNewPop.push_back(SGenome(baby1, 0));
		vecNewPop.push_back(SGenome(baby2, 0));
	}

	//finished so assign new pop back into m_vecPop
	m_vecPop = vecNewPop;

	return m_vecPop;
}


//-------------------------GrabNBest----------------------------------
//
//	This works like an advanced form of elitism by inserting NumCopies
//  copies of the NBest most fittest genomes into a population vector
//--------------------------------------------------------------------
void GenAlg::GrabNBest(int	            NBest,
                        const int	      NumCopies,
                        vector<SGenome>	&Pop)
{
  //add the required amount of copies of the n most fittest 
	//to the supplied vector
	int done = 0;
	while(done <NBest)
	{
		for (int i=0; i<NumCopies; ++i)
		{  // push back NBEST of the elite starting from the most elite
			Pop.push_back(m_vecPop[(m_iPopSize - 1) - done]); 
	  	}
		++done;
	}
}

//-----------------------CalculateBestWorstAvTot-----------------------	
//
//	calculates the fittest and weakest genome and the average/total 
//	fitness scores
//---------------------------------------------------------------------
void GenAlg::CalculateBestWorstAvTot()
{
	m_dTotalFitness = 0;
	
	float HighestSoFar = 0;
	float LowestSoFar  = 9999999;
	
	for (int i=0; i<m_iPopSize; ++i)
	{
		//update fittest if necessary
		if (m_vecPop[i].dFitness > HighestSoFar)
		{
			HighestSoFar	 = m_vecPop[i].dFitness;
			
			m_iFittestGenome = i;

			m_dBestFitness	 = HighestSoFar;
		}
		
		//update worst if necessary
		if (m_vecPop[i].dFitness < LowestSoFar)
		{
			LowestSoFar = m_vecPop[i].dFitness;
			
			m_dWorstFitness = LowestSoFar;
		}
		
		m_dTotalFitness	+= m_vecPop[i].dFitness;
		
		
	}//next chromo
	
	m_dAverageFitness = m_dTotalFitness / m_iPopSize;
}

//-------------------------Reset()------------------------------
//
//	resets all the relevant variables ready for a new generation
//--------------------------------------------------------------
void GenAlg::Reset()
{
	m_dTotalFitness		= 0;
	m_dBestFitness		= 0;
	m_dWorstFitness		= 9999999;
	m_dAverageFitness	= 0;
}

