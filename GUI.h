// This is a the GUI for Project Checkmate's Checkerboard
// It uses OpenGL and GLUT in order to draw the board and checkers
// Reads the board information from the input file and positions the checker pieces accordingly
// Move occurs by blinking the square that the piece is moving to (animation may be implemented later)
#include "board.h"
#include <cmath>
using std::sin;
using std::cos;
#include <vector>
using std::vector;

#include <GL/glut.h> /* OpenGL Utilities Toolkit, for GUI tools */

// Global Variables


// Window Information
const int windowsizeW = 480;
const int windowsizeH = 480;


// showPositions
// This function shows the positions available to a certain piece
void showPositions (Board & game, int position, bool red_player)
{
	bool forced_capture;
	int num_pieces;
	
	vector < vector <int> > moves = game.gen_moves(red_player, forced_capture, num_pieces);
	for (int i = 0; i < moves.size(); ++i)
	{
		if (moves[i][0] == position)
			for (int j= 1; j < moves[i].size(); ++j)
			{
			}
	}
}


// DrawCheckerBoard
// This function draw's the checker board;
void DrawCheckerBoard()
{
	for(int x = 0; x <8; ++x)
		for (int y = 0; y<8; ++y)
			{
				if (y% 2 != 0 && x%2==0)
					glColor3d(.6,.0,.0);
				else if (y% 2 == 0 && x%2 != 0)
					glColor3d(.6,0.0,0.0);
				else
					glColor3d(.2,.2,.2);
				glBegin(GL_POLYGON);
				glVertex2d(x, y);
				glVertex2d(x, y+1);
				glVertex2d(x+1, y+1);
				glVertex2d(x+1, y);
				glEnd();
			}
}

// Draw Checker
void DrawChecker(float x, float y)
{
	glBegin(GL_TRIANGLE_FAN);
 	glVertex2f(x+.5, y+.5);
 	for (double angle = 0; angle < 360.0; angle+=7.5)
 	glVertex2f(x+.5 + sin(angle) * .4, y+.5 + cos(angle) * .4);
 	glEnd();
}

// Draw Checker
void DrawKing (float x, float y)
{
	glBegin(GL_TRIANGLE_FAN);
 	glVertex2f(x+.5, y+.5);
 	for (double angle = 0; angle < 360.0; angle+=17.5)
 	glVertex2f(x+.5 + sin(angle) * .4, y+.5 + cos(angle) * .4);
 	glEnd();
}

// DrawCheckerPieces
// This function pulls out the board layout from the function Board and draws the corresponding piece layout
void DrawCheckerPieces(Board &b)
{
	
	int row = 0;
	float start = 0;
	float x =0, y = 7;
	for(int i=0; i< 32; i++)
	{		
		bool red_flag=false, black_flag=false, king_flag=false;
		red_flag = ((b.red>>i) & 1);
		black_flag = ((b.black>>i) & 1);
		king_flag = ((b.king>>i) & 1);

		if (x >= 8){
			x = 0;
			--y;
			row++;
		}
		
		if (row%2 == 0)		
			start = 1.0;
		else 
			start = 0.0;
		if(red_flag)
		{
			if(king_flag){
				glColor3f(.9,0.0,0.0);
				DrawKing(start+x,y);}
			else{
				glColor3f(.9,0.0,0.0);
				DrawChecker(start+x,y);}
		}
		if(black_flag)
		{
			if(king_flag){
				glColor3f(0.0,0.0,0.0);
				DrawKing(start+x,y);}
			else{
				glColor3f(0.0,0.0,0.0);
				DrawChecker(start+x,y);}
		}
		
		x+=2.0;
		
		
	}
	
}


void GUIinit(int argc, char ** argv)
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGBA + GLUT_DEPTH + GLUT_DOUBLE);
	glutInitWindowSize(windowsizeW,windowsizeH);
	glutCreateWindow("Checkmate GUI");
	
}
