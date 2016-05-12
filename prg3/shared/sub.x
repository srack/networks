/* Samantha Rack
 * CSE 30264
 * Program 3 - RPC
 * sub.x
 */

struct moveStruct {
	int turn;
	int x1;
	int y1;
};

program MOVESUB_PROG {

	version MOVESUB_VERSION {
		int MOVESUB_PROC(moveStruct) = 1;
	} = 1;

} = 0x31660284; 
