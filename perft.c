#include "defs.h"
#include "stdio.h"

long leafNodes;

void Perft(int depth, S_BOARD *pos) {   // Recursive function that loops to all possible leafnodes at given depth

    ASSERT(CheckBoard(pos));

    if (depth == 0) {   // Count all leafnodes
        leafNodes++;
        return;
    }

    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);

    int MoveNum;
    for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {

        if (!MakeMove(pos, list->moves[MoveNum].move))  {
            continue;
        }

        Perft(depth - 1, pos);  // Call recursive function
        TakeMove(pos);
    }

    return;
}

void PerftTest(int depth, S_BOARD *pos) {

    ASSERT(CheckBoard(pos));

    PrintBoard(pos);
    printf("\nStarting Test to Depth %d\n", depth);
    leafNodes = 0;
    int start = GetTimeMs();

    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);

    int move;
    int MoveNum = 0;
    for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        move = list->moves[MoveNum].move;
        if (!MakeMove(pos, move))   {
            continue;
        }
        long cumNodes = leafNodes;
        Perft(depth - 1 , pos);
        TakeMove(pos);
        long oldNodes = leafNodes - cumNodes;
        printf("Move %d : %s : %ld\n", MoveNum+1, PrMove(move), oldNodes);
    }

    printf("\nTest Complete: %ld nodes visited in %dms.\n", leafNodes, GetTimeMs() - start);

    return;
}