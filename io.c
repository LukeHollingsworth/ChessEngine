#include "stdio.h"
#include "defs.h"

char *PrSq(const int sq)   {

    static char SqStr[3];   // Using info stored here used elsewhere so needs to remain same after being returned

    int file = FilesBrd[sq];
    int rank = RanksBrd[sq];

    sprintf(SqStr, "%c%c", ('a'+file), ('1'+rank));

    return SqStr;
}

char *PrMove(const int move)    {

    static char MvStr[6];

    int ff = FilesBrd[FROMSQ(move)];
    int rf = RanksBrd[FROMSQ(move)];
    int ft = FilesBrd[TOSQ(move)];
    int rt = RanksBrd[TOSQ(move)];

    int promoted = PROMOTED(move);

    if (promoted)   {
        char pchar = 'q';
        if (IsKn(promoted)) {
            pchar = 'n';
        } else if (IsRQ(promoted) && !IsBQ(promoted))   {
            pchar = 'r';
        } else if (!IsRQ(promoted) && IsBQ(promoted))    {
            pchar = 'b';
        }
        sprintf(MvStr, "%c%c%c%c%c", ('a'+ff), ('1'+rf), ('a'+ft), ('1'+rt), pchar);
    } else  {
        sprintf(MvStr, "%c%c%c%c", ('a'+ff), ('1'+rf), ('a'+ft), ('1'+rt));
    }

    return MvStr;
}

int ParseMove(char *ptrChar, S_BOARD *pos)  {

    if (ptrChar[1] > '8' || ptrChar[1] < '1') return FALSE; // Checking move entered is in valid format
    if (ptrChar[3] > '8' || ptrChar[3] < '1') return FALSE;
    if (ptrChar[0] > 'H' || ptrChar[0] < 'A') return FALSE;
    if (ptrChar[2] > 'H' || ptrChar[2] < 'A') return FALSE;

    int from = FR2SQ(ptrChar[0] - 'A', ptrChar[1] - '1');
    int to = FR2SQ(ptrChar[2] - 'A', ptrChar[3] - '1');

    ASSERT(SqOnBoard(from) && SqOnBoard(to));

    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);
    int MoveNum = 0;
    int Move = 0;
    int PromPce = EMPTY;

    for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        Move = list->moves[MoveNum].move;
        if (FROMSQ(Move) == from && TOSQ(Move) == to)   {
            PromPce = PROMOTED(Move);
            if (PromPce != EMPTY)   {   // Setting promotion permissions
                if (IsRQ(PromPce) && !IsBQ(PromPce) && ptrChar[4] == 'R')   {
                    return Move;
                }
                if (!IsRQ(PromPce) && IsBQ(PromPce) && ptrChar[4] == 'B')   {
                    return Move;
                }
                if (IsRQ(PromPce) && IsBQ(PromPce) && ptrChar[4] == 'Q')   {
                    return Move;
                }
                if (IsKn(PromPce) && ptrChar[4] == 'N')   {
                    return Move;
                }
                continue;
            }
            return Move;
        }
    }
    return NOMOVE;
}

void PrintMoveList(const S_MOVELIST *list)  {

    int index = 0;
    int score = 0;
    int move = 0;
    printf("MoveList:\n", list->count);

    for (index = 0; index < list->count; ++index)   {

        move = list->moves[index].move;
        score = list->moves[index].score;

        printf("Move:%d > %s (score:%d)\n", index+1, PrMove(move), score);

    }
    printf("MoveList Total: %d moves\n\n", list->count);
}