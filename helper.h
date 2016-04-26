// helper.h
// Travis Payton
// This header contains all of the useful helper functions and headers that are needed in the Checkers Program

#ifndef HELPER_H_INCLUDED
#define HELPER_H_INCLUDED

#include <cstdlib>
#include <cmath>
#include <sstream>
#include <string>
using std::string;
#include <iostream>
#include <vector>
using std::vector;
#include <ctime>


// returns a random float between min and max
inline float randfloat(const float min, const float max)
{ return rand()/(float(RAND_MAX)+1)*(max-min)+min; }

//returns a random integer between x and y
inline int	  randint(int x,int y) 
{ return rand()%(y-x+1)+x; }

#endif