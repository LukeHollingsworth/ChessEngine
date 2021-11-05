#include "stdio.h"
#include "defs.h"
#include "stdlib.h"

int main() {	

	AllInit();	

    S_BOARD board[1];
    InitPvTable(board->PvTable);

    S_MOVELIST list[1];
    S_SEARCHINFO info[1];

    ParseFen(START_FEN, board);
    
    char input[6];
    char Side;
    int Move = NOMOVE;
    int bestMove = NOMOVE;
    int PvNum = 0;
    int Max = 0;

    printf("Welcome to OmegaOne 1.0 - prepare to meet your DOOM!\n\n");
    printf("Which side is OmegaOne playing?");
    fgets(input, 3, stdin);
    if (input[0] == 'b')  {
        Side = BLACK;
    } else if (input[0] == 'w') {
        Side = WHITE;
    }
    // printf("What depth would you like OmegaOne to search to?\n");
    // fgets(input, 1, stdin);
    // info->depth = input;

    while (TRUE)    {
        PrintBoard(board);
        Move = SearchPosition(board, info, Side);
        if (Move != NOMOVE) {
            printf("Luke has a big brain");
            StorePvMove(board, Move);
            MakeMove(board, Move);
        } else {
            printf("Please enter a move > "); 
            fgets(input, 6, stdin);

            if (input[0] == 'q')    {
                break;
            } else if (input[0] == 't') {
                TakeMove(board);
            } else {
                Move = ParseMove(input, board);
                if (Move != NOMOVE) {
                    StorePvMove(board, Move);
                    MakeMove(board, Move);
                } else {
                    printf("NO NO NO! %s is not a valid move\n", input);
                }
            }  
        }
        // printf("Please enter a move > "); 
        // fgets(input, 6, stdin);

        // if (input[0] == 'q')    {
        //     break;
        // } else if (input[0] == 't') {
        //     TakeMove(board);
        // } else if (input[0] == ' ') {
        //     info->depth = 5;
        //     bestMove = SearchPosition(board, info);
        //     if (bestMove != NOMOVE) {
        //         StorePvMove(board, bestMove);
        //         MakeMove(board, bestMove);
        //     } else {
        //         printf("NO NO NO! %s is not a valid move\n", input);
        //     }
        // } else {
        //     Move = ParseMove(input, board);
        //     if (Move != NOMOVE) {
        //         StorePvMove(board, Move);
        //         MakeMove(board, Move);
        //     } else {
        //         printf("NO NO NO! %s is not a valid move\n", input);
        //     }
        // }

        fflush(stdin);
    }

    free(board->PvTable->pTable);   // Free the memory that points to the PV Table
	
	return 0;
}