// This is a the GUI for Project Checkmate's Checkerboard
// It uses OpenGL and GLUT in order to draw the board and checkers
// Reads the board information from the input file and positions the checker pieces accordingly
// Move occurs by blinking the square that the piece is moving to (animation may be implemented later)

#include <cmath>
using std::sin;
using std::cos;
#include <vector>
using std::vector;
#include "board.h"
#include <string>
using std::string;
#include <cstdlib>
using std::pair;


#include <GL/glut.h> /* OpenGL Utilities Toolkit, for GUI tools */

// Global Variables


// Window Information
const int windowsizeW = 480;
const int windowsizeH = 480;

int pWinw, pWinh;             // Current window width, height (pixels)
const double wWinleft = -1, wWinright = 1, wWinbottom =-1, wWintop =1;

// Mouse Window Coord's
double wMousex, wMousey;

// Board and player info
Board b;
bool red_player;
bool gameover, won;
vector <Board> previous;
vector < int > hum_move; // contains the move for player 1
vector < vector < int > > possible; // contains the possible moves for each position
bool forced_capture;
bool chosen;

// look up database for positions
vector < pair <int, int> > LookUpPos(32);

// Menu Toggles etc..
bool menu;
bool loadnet;

// printBitmap
// Prints the given string at the given raster position
//  using GLUT bitmap fonts.
// You probably don't want any rotations in the model/view
//  transformation when calling this function.
void printBitmap(const string msg, double x, double y)
{
    glRasterPos2d(x, y);
    for (string::const_iterator ii = msg.begin();
         ii != msg.end();
         ++ii)
    {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *ii);
    }
}

// drawMenu
// Draws the menu on the screen with names for selecting items
void drawMenu()
{
	// Draw Menu
	glPushMatrix();
	glLoadIdentity();
	glLoadName(1);
    glColor3d(.0,.0,.0);
	glRectd(-.1, wWintop - .45, .1,wWintop - .5);
	glColor3d(9., 9., 9.);  // Grey Text
	printBitmap("RESUME", -.1, wWintop - .5);
	glLoadName(2);
	glColor3d(.0,.0,.0);
	glRectd(-.5, wWintop - .95, -.3, wWintop - 1.);
	glColor3d(1.,1.,1.);
	printBitmap("LOAD NN", -.5,wWintop - 1.);
	glLoadName(3);
	glColor3d(.0,.0,.0);
	glRectd(.3, wWintop - .95, .5, wWintop - 1.);
	glColor3d(1.,1.,1.);
	printBitmap("RETRY",.3,wWintop -1.);
	printBitmap("Esc   Quit", wWinleft, wWintop - .05);
	printBitmap("b - undo, r - Restart", -.6, wWinbottom+.2);
	glPopMatrix();

}

// Draw Move
// takes the position of the move and draws it on the screen
void drawMove (vector <int> move)
{
	glColor3f(.2,.7,.7);
	for (int i = 1; i < (int)move.size(); ++i)
	{
	glBegin(GL_POLYGON);
		glVertex2d(LookUpPos[move[i]].first, LookUpPos[move[i]].second);
		glVertex2d(LookUpPos[move[i]].first, LookUpPos[move[i]].second+1);
		glVertex2d(LookUpPos[move[i]].first+1, LookUpPos[move[i]].second+1);
		glVertex2d(LookUpPos[move[i]].first+1, LookUpPos[move[i]].second);
	glEnd();
	}
}

// showPositions
// This function is where all the magic happens
// It takes the selection and stores the available positions for that selection in possible
// also makes a move if it is a legal second click, else it starts over
void showPositions (int position)
{
	bool forced_capture;
	int num_pieces;

	vector < vector <int> > moves = b.gen_moves(red_player, forced_capture, num_pieces);
	if (hum_move.size() > 0)
	{
		for (int i = 0; i < (int)possible.size(); ++i)
			if (position == possible[i].back())
				for (int j = 1; j < (int)possible[i].size(); ++j)
					hum_move.push_back(possible[i][j]);
		if(b.verify_move(hum_move, red_player))
		{
			b.move(hum_move);
			red_player = !red_player;
			previous.push_back(b);

		}

		hum_move.clear();
		possible.clear();
	}


	else // start over
	{
		possible.clear();
		hum_move.clear();
		for (int i = 0; i < (int)moves.size(); ++i)
			if (moves[i][0] == position)
			{
				if (hum_move.empty())
						hum_move.push_back(position);
				possible.push_back(moves[i]);
			}
	}
}


// DrawCheckerBoard
// This function draw's the checker board;
void DrawCheckerBoard(float x, float y)
{
	glColor3d(.4,.4,.4);
	// each black square is a potential position
	glBegin(GL_POLYGON);
	glVertex2d(x, y);
	glVertex2d(x, y+1);
	glVertex2d(x+1, y+1);
	glVertex2d(x+1, y);
	glEnd();
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
// uses glLoadName() in order to allow for picking in GUI
void DrawCheckerPieces()
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
		LookUpPos[i].first=start+x;
		LookUpPos[i].second=y;
		glLoadName(i);
		DrawCheckerBoard(start+x, y);
		if(red_flag)
		{
			if(king_flag){
				glColor3f(.9,0.0,0.0);
				DrawKing(start+x,y);
				glColor3f(1.0,1.0,1.0);
				printBitmap("K",start+x+.4, y+.4);}
			else{
				glColor3f(.9,0.0,0.0);
				DrawChecker(start+x,y);}
		}
		if(black_flag)
		{
			if(king_flag){
				glColor3f(0.0,0.0,0.0);
				DrawKing(start+x,y);
				glColor3f(1.0,1.0,1.0);
				printBitmap("K",start+x+.4, y+.4);}
			else{
				glColor3f(0.0,0.0,0.0);
				DrawChecker(start+x,y);}
		}



		x+=2.0;


	}

}

// set_up_coords
// Does our gluOrtho2D to set up a coordinate system.
// Assumes pWinw, pWinh have been set up by reshape.
// Does NOT set the matrix mode or load an identity.
// Called by reshape (for reshape) and doselect (in selection mode).
void set_up_coords()
{
    // We set up coordinate system so that aspect ratios are always correct,
    //  and the region from -1..1 in x & y always just fits in the viewport.
    if (glutGet(GLUT_WINDOW_HEIGHT) > glutGet(GLUT_WINDOW_HEIGHT))
    {
        gluOrtho2D(-double(glutGet(GLUT_WINDOW_WIDTH))/glutGet(GLUT_WINDOW_HEIGHT), double(glutGet(GLUT_WINDOW_WIDTH))/glutGet(GLUT_WINDOW_HEIGHT), -1., 1.);
    }
    else
    {
        gluOrtho2D( -1., 1., -double(glutGet(GLUT_WINDOW_HEIGHT))/glutGet(GLUT_WINDOW_WIDTH), double(glutGet(GLUT_WINDOW_HEIGHT))/glutGet(GLUT_WINDOW_WIDTH));
    }
}

// doselect
// Use selection mode to check hits on a single pixel.
// Given:
//    x, y: pixel position in GLUT mouse-position format.
//    hitList: vector of int's (will be overwritten!).
// Returns:
//    hitList: contains names of hits.
// Calls set_up_coords to do projection.
// Calls draw_with_names to draw & load names.
void doselect(int x, int y, vector<int> & hitList)
{
    const int BUFFERSIZE = 512;
    GLuint selectionBuffer[BUFFERSIZE];

    // Initialize selection mode
    glSelectBuffer(BUFFERSIZE, selectionBuffer);
    glRenderMode(GL_SELECT);
    glInitNames();
    glPushName(0);                 // Can't have an empty name stack

    // Set up the pick matrix
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();                // Save the projection
    glLoadIdentity();
    // Insert the following three lines just before
    //  your standard projection setup.
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    gluPickMatrix(x, vp[3]-y, 1, 1, vp);
    set_up_coords();               // Setup same projection as in reshape
    // "Draw" the scene (in selection mode, so no actual output;
    //                   I'm not breaking THE RULES!)
    glMatrixMode(GL_MODELVIEW);    // Always go back to model/view mode
    if (menu)
		drawMenu();
	else
	{
		glTranslatef(-1, -1.,0.);
		glScalef(.25,.25,0.);
		DrawCheckerPieces();
	}
    glFlush();                     // Empty the pipeline

    // Done with pick matrix
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();                 // Restore the old projection
    glMatrixMode(GL_MODELVIEW);    // Always go back to model/view mode

    // Done with selection mode
    int hitCount = glRenderMode(GL_RENDER);

    // Put hits into the caller's array, in a simpler format.
    GLuint * p = selectionBuffer;  // Moving ptr into selectionBuffer
    hitList.clear();
    for (int i = 0; i < hitCount; ++i)
    {
        if (p >= selectionBuffer + BUFFERSIZE)
            break;                 // Beyond end of buffer!

        GLuint stackDepth = *p;    // Get name-stack depth for this hit
        p += (3+stackDepth);       // Skip current cell,
                                   //  min & max depth values,
                                   //  and saved stack

        if (stackDepth > 0)
        {
            if (p-1 >= selectionBuffer + BUFFERSIZE)
                break;             // Beyond end of buffer!

            GLuint hitName = *(p-1);
                                   // Name of object that was hit
            hitList.push_back(int(hitName));
                                   // Save name in caller's vector
        }
    }
}


// mouseHelper
// Checks for object hits and prints them
void mouseHelper(int x, int y)
{
  	vector<int> hitList;
	doselect(x, y, hitList);

	for (int i = 0; i < (int)hitList.size(); ++i)
		std::cout << hitList[i] << std::endl;
	if (!menu && !hitList.empty())
		showPositions(hitList[0]);

	else if (menu)
	if(!hitList.empty())
	{
		switch(hitList[0])
		{
		 case 1:
			 menu = !menu;
			break;
		 case 2:
			 loadnet = true;
			 break;
		 case 3:
			 b.set();
			 menu = !menu;
			 break;
		}
	}

}

// myKeyboard
// The GLUT keyboard function
void myKeyboard(unsigned char key, int x, int y)
{
	switch (key)
    {
    case 27:  // ESC: Quit
		if(menu || gameover || won)
		{
			exit(0);
			break;
		}
		else
		{
			menu = true;
			break;
		}
	case 'l':
	case 'L':
		loadnet = true;
		break;
	case 'r':
	case 'R':
		previous.clear();
		b.set();
		previous.push_back(b);
		break;
	case 'b':
	case 'B':
		b = previous.back();
		previous.pop_back();
		red_player = !red_player;
		break;
	case ' ':
		hum_move.pop_back();
		break;
    }

}
void computeMouse(int x, int y)
{
    wMousex = wWinleft + double(x)/glutGet(GLUT_WINDOW_WIDTH)*(wWinright-wWinleft);
    wMousey = wWintop + double(y)/glutGet(GLUT_WINDOW_HEIGHT)*(wWinbottom-wWintop);
}


// myMouse
// The GLUT mouse function
void myMouse(int button, int state, int x, int y)
{
	if (VS_RED || VS_BLACK)
		if (state == GLUT_LEFT_BUTTON)
			mouseHelper(x, y);
}



void GUIinit(int argc, char ** argv)
{

	// Set Red to go first
	red_player = true;
	// set the starting board for the undo list
	previous.push_back(b);
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGBA + GLUT_DEPTH + GLUT_DOUBLE);
	glutInitWindowSize(windowsizeW,windowsizeH);
	glutCreateWindow("Checkmate GUI");

	glutMouseFunc(myMouse);
	glutKeyboardFunc(myKeyboard);

}
