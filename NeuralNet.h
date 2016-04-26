// NeuralNet.h
// Travis Payton
// Feb, 11, 2010
//
/* This is the header for a Neural Network (NN) class
   the NN should be self learning and will adjust its weights
   over time due to evolutionary process of natural selection */

#ifndef NEURALNET_H_INCLUDED
#define NEURALNET_H_INCLUDED

#include "helper.h"

// Set up the Neural Net

// Neuron holds a single Neuron which knows its weights and its number of inputs
struct Neuron {

	// holds the number of inputs to the neuron
	int m_NumInputs;

	// holds the weights for each input
	vector<float> m_vecWeight;

	// Ctor
	Neuron (int NumInputs) : m_NumInputs(NumInputs+1)
	{
		for (int i = 0; i < NumInputs+1; ++i)
		{/* adding an additional weight on purpose for the bias*/
			//set up the weights with an initial random value from -1.0 -> 1.0
			m_vecWeight.push_back(randfloat(-1.0,1.0));
		}
	}
}; // end struct Neuron

// NeuronLayer, this is a layer of neurons
struct NeuronLayer
{
	// holds the number of neurons in this layer
	int m_NumNeurons;

	// holds the layer of neurons
	vector<Neuron> m_vecNeurons;

	// Ctor
	NeuronLayer(int NumNeurons, int
NumInputsPerNeuron):m_NumNeurons(NumNeurons)
	{
		 for (int i=0; i<NumNeurons; ++i)
			 m_vecNeurons.push_back(Neuron(NumInputsPerNeuron));
	}
}; // end struct NeuronLayer

class NeuralNet
{
private:
	int m_NumInputs;
	int m_NumOutputs;
	int m_NumHiddenLayers;
	vector<int> m_NeuronsPerHiddenLyr;

	// storage for each layer of neurons including the output layer
	vector<NeuronLayer>m_vecLayers;

public:
		//ctor
		
		// creates the NN
		void CreateNet(int iNumInputs, int iNumOutputs, int iNumHidden,
			vector<int> &iNeuronsPerHiddenLayer)
		{
			m_NumInputs = iNumInputs;
			m_NumOutputs = iNumOutputs;
			m_NumHiddenLayers = iNumHidden;
			m_NeuronsPerHiddenLyr= iNeuronsPerHiddenLayer;
			
			// Create the layers of the network
			if(m_NumHiddenLayers > 0)
			{
				// create first hidden layer
				m_vecLayers.push_back(NeuronLayer(m_NeuronsPerHiddenLyr[0],m_NumInputs));
				for (int i = 0; i < m_NumHiddenLayers-1; ++i)
					m_vecLayers.push_back(NeuronLayer(m_NeuronsPerHiddenLyr[i+1],m_NeuronsPerHiddenLyr[i]));
				// create output layer
				m_vecLayers.push_back(NeuronLayer(m_NumOutputs,m_NeuronsPerHiddenLyr[m_NeuronsPerHiddenLyr.size()-1]));
			}
			else
			{ // create output layer
				m_vecLayers.push_back(NeuronLayer(m_NumOutputs,m_NumInputs));
			}
		}

		// gets the weights from the NN
		vector<float> GetWeights()const
		{
			// make a vector that whill hold the weights
			vector <float> weights;

			// do it for each layer
			for (int i = 0; i<m_NumHiddenLayers + 1; ++i)
				//for each Neuron
				for (int j = 0; j < m_vecLayers[i].m_NumNeurons; ++j)
					// for each weight
					for (int k = 0; k < m_vecLayers[i].m_vecNeurons[j].m_NumInputs; ++k)
					 weights.push_back(m_vecLayers[i].m_vecNeurons[j].m_vecWeight[k]);

			return weights;
		}

		// returns the total number of weights in the net
		int GetNumberOfWeights()const
		{
			int weights = 0;
			// do it for each layer
			for (int i = 0; i<m_NumHiddenLayers + 1; ++i)
				//for each Neuron
				for (int j = 0; j < m_vecLayers[i].m_NumNeurons; ++j)
					// for each weight
					for (int k = 0; k < m_vecLayers[i].m_vecNeurons[j].m_NumInputs; ++k)
					 weights++;

			return weights;
		}

		// replaces the weights with new ones
		void PutWeights(vector<float> &weights)
		{
			int weight = 0;
			// do it for each layer
			for (int i = 0; i<m_NumHiddenLayers + 1; ++i)
				//for each Neuron
				for (int j = 0; j < m_vecLayers[i].m_NumNeurons; ++j)
					// for each weight
					for (int k = 0; k < m_vecLayers[i].m_vecNeurons[j].m_NumInputs; ++k)
						m_vecLayers[i].m_vecNeurons[j].m_vecWeight[k] = weights[weight++];

			return;
		}

		float Sigmoid(float netinput, float response)
		{ return (1 / (1 + exp(-netinput / response)));}

		vector<float> Update(vector<float> &inputs)
		{

			vector<float> tempinputs = inputs; // Don't want to change the inputs vector
			//stores the resultant outputs from each layer
			vector<float>outputs;
			int cWeight = 0;

			// make sure we have the right number of inputs
			if((int)tempinputs.size() != m_NumInputs)
			{
				return outputs; // return an empty vector if its wrong
			}

			// get the weights for each layer
			for (int i = 0; i<m_NumHiddenLayers + 1; ++i)
			{
				if (i>0)
				{
					tempinputs = outputs;
				}

				outputs.clear();
				cWeight = 0;

				// for each neuron sum the inputs * corresponding weights.
				// and run it through the sigmoud function to get output

				for (int j=0;j<m_vecLayers[i].m_NumNeurons;++j)
				{
					float netinput = 0;
					int NumInputs = m_vecLayers[i].m_vecNeurons[j].m_NumInputs;
					// for each weight

					for (int k = 0; k<NumInputs - 1; ++k)
					{
						//sum the weights * inputs
						netinput += m_vecLayers[i].m_vecNeurons[j].m_vecWeight[k]*tempinputs[cWeight++];
					}

					// add in the bias
					netinput += m_vecLayers[i].m_vecNeurons[j].m_vecWeight[NumInputs-1]*-1;

					outputs.push_back(Sigmoid(netinput, 1));
					cWeight = 0;
				}
			}
			return outputs;
		}

}; // end Class NeuralNet


#endif // NEURALNET_H_INCLUDED
