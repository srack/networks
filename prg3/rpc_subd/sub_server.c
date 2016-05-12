/* Samantha Rack
 * CSE 30264
 * Program 3 - RPC
 * server.c
 */

#include "../shared/sub.h"
#include "../shared/shared.h"

int *movesub_proc_1_svc(moveStruct *argp, struct svc_req *rqstp)
{
	// will be the returned value
	static int  result;

	// extract the arguments from the structure
	int turn = argp->turn;
	int x1 = argp->x1;
	int y1 = argp->y1;

	// (x0, y0) define the current location of the submarine
	//  static to allow it to be consistent over function calls
	static int x0, y0;

	// (xBomb, yBomb) defines the location of the one stationary 
	//  bomb in the game
	// the user wins if the submarine hits the bomb, and the user
	//  loses if her destroyer hits the bomb
	static int xBomb, yBomb;

	// local variables for calculations
	int deltax1, deltax2, deltay1, deltay2;
	int sqdistance, deltaxsq, deltaysq;
	int move_dir;

	// if it is the first turn, the submarine and the bomb has to be generate on the grid
	if (turn == 0) { 
		do {
			x0 = rand()%MAX_GRID;
			y0 = rand()%MAX_GRID;
		} while ((x1 == x0) && (y1 == y0));
		do {
			xBomb = rand()%MAX_GRID;
			yBomb = rand()%MAX_GRID;
		} while ((xBomb == x0 && yBomb == y0) || (xBomb == x1 && yBomb == y1));
	}

	// if the position of the submarine matches the user, there is a hit
	if ((x0 == x1) && (y0 == y1)) {
		result = 0;
		return &result;
	}	

	// if the user is at the same position as the bomb, then the user loses (-1)
	if ( x1 == xBomb && y1 == yBomb ) {
		result = -1;
		return &result;
	}


	// the submarine moves every DURATION turns
	if (turn % DURATION == 0){
		move_dir = rand()%9+1;
		switch (move_dir){
			case 7: 
				x0 = (x0 + MAX_GRID - 1) % MAX_GRID;
			case 8: 
				y0 = (y0 + MAX_GRID - 1) % MAX_GRID; 
				break;
			case 9: 
				y0 = (y0 + MAX_GRID - 1) % MAX_GRID;
			case 6: 
				x0 = (x0 + MAX_GRID + 1) % MAX_GRID; 
				break;
			case 3: 
				x0 = (x0 + MAX_GRID + 1) % MAX_GRID;
			case 2: 
				y0 = (y0 + MAX_GRID + 1) % MAX_GRID; 
				break;
			case 1: 
				y0 = (y0 + MAX_GRID + 1) % MAX_GRID;
			case 4: 
				x0 = (x0 + MAX_GRID - 1) % MAX_GRID; 
				break;
			case 5: 
				break;
			default: 
				// on illegal movement, just do nothing
				break;
		}
	} 

	// if the submarine is at the same position as the bomb, then the user wins!
	if ( x0 == xBomb && y0 == yBomb ) {
		result = 4;
		return &result;
	}

	// calculate the x distances between the user and the submarine
	//  (note: the user and submarine can "wrap around" the grid)
	if (x0 > x1) { 
		deltax1 = x0 - x1; 
		deltax2 = x1 + MAX_GRID - x0; 
	} else { 
		deltax1 = x1 - x0; 
		deltax2 = x0 + MAX_GRID - x1; 
	}
	// use minimum x distance to get deltaxsq
	if (deltax1 > deltax2) deltaxsq = deltax2 * deltax2;
	else deltaxsq = deltax1 * deltax1;

	// calculate the y distances between the user and the submarine
	if (y0 > y1) { 
		deltay1 = y0 - y1; 
		deltay2 = y1 + MAX_GRID - y0; 
	} else { 
		deltay1 = y1 - y0; 
		deltay2 = y0 + MAX_GRID - y1; 
	}
	// use minimum y distance to get deltaysq
	if (deltay1 > deltay2) deltaysq = deltay2 * deltay2;
	else deltaysq = deltay1 * deltay1;

	sqdistance = deltaxsq + deltaysq;

	if (sqdistance <= 2) result = 1;	// red alert
	if (sqdistance <= 8) result = 2;	// yellow alert
	else result = 3; 	// green alert

	return &result;
}
