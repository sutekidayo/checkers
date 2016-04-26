#include "checkersplayer.h"

//-----------------------------------constructor-------------------------
//
//-----------------------------------------------------------------------
CheckersPlayer::CheckersPlayer(int iNumInputs, int iNumOutputs, int iNumHidden,
			vector<int> &iNeuronsPerHiddenLayer)
                            
{
	Brain.CreateNet(iNumInputs, iNumOutputs, iNumHidden, iNeuronsPerHiddenLayer);
  
}

