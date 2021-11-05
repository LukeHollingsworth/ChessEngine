#include "stdio.h"
#include "defs.h"
#include "stdlib.h"

#define PAWNMOVESW "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define PAWNMOVESB "rnbqkbnr/p1p1p3/3p3p/1p1p4/2P1Pp2/8/PP1P1PpP/RNBQKB1R b KQkq e3 0 1"
#define KNIGHTSKINGS "5k2/1n6/4n3/6N1/8/3N4/8/5K2 w - - 0 1"
#define ROOKS "6k1/8/5r2/8/1nR5/5N2/8/6K1 w - - 0 1"
#define QUEENS "6k1/8/4nq2/8/1nQ5/5N2/1N6/6K1 w - - 0 1"
#define BISHOPS "6k1/1b6/4n3/8/1n4B1/1B3N2/1N6/2b3K1 b - - 0 1"
#define CASTLE "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"
#define PERFTFEN "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1"  // Notoriously tricky position
#define WAC1 "r1b1k2r/ppppnppp/2n2q2/2b5/3NP3/2P1B3/PP3PPP/RN1QKB1R w KQkq - 0 1"

int main() {	

	AllInit();	

    S_BOARD board[1];
    InitPvTable(board->PvTable);

    S_MOVELIST list[1];
    S_SEARCHINFO info[1];

    ParseFen(START_FEN, board);
    
    char input[6];
    int Move = NOMOVE;
    int PvNum = 0;
    int Max = 0;

    while (TRUE)    {
        PrintBoard(board);
        printf("Please enter a move > ");
        fgets(input, 6, stdin);

        if (input[0] == 'q')    {
            break;
        } else if (input[0] == 't') {
            TakeMove(board);
        } else if (input[0] == 's') {
            info->depth = 6;
            SearchPosition(board, info);
        } else {
            Move = ParseMove(input, board);
            if (Move != NOMOVE) {
                StorePvMove(board, Move);
                MakeMove(board, Move);
            } else {
                printf("NO NO NO! %s is not a valid move\n", input);
            }
        }

        fflush(stdin);
    }

    free(board->PvTable->pTable);   // Free the memory that points to the PV Table
	
	return 0;
}