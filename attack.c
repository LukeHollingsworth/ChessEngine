#include "stdio.h"
#include "defs.h"

const int KnDir[8] = { -8, -19, -21, -12, 8, 19, 21, 12 };  // Direction a knight etc. can move (imagine the 120-base board for clarity)
const int RkDir[4] = { -1, -10, 1, 10 };
const int BiDir[4] = { -9, -11, 11, 9 };
const int KiDir[8] = { -1, -10, 1, 10, -9, -11, 11, 9 };

int SqAttacked (const int sq, const int side, const S_BOARD *pos)    {

    int pce, index, t_sq, dir;

    ASSERT(SqOnBoard(sq));
    ASSERT(SideValid(side));
    ASSERT(CheckBoard(pos));

    // Pawns (determines whether the adjacent diagonals for each side contain pawns)
    if (side == WHITE)  {
        if (pos->pieces[sq-11] == wP || pos->pieces[sq-9] == wP) {
            return TRUE;
        }
    }
    else    {
        if (pos->pieces[sq+11] == bP || pos->pieces[sq+9] == bP)    {
            return TRUE;
        }
    }

    // Knights
    for (index = 0; index < 8; ++index) {
        pce = pos->pieces[sq + KnDir[index]];
        if (IsKn(pce) && PieceCol[pce] == side) {
            return TRUE;
        }
    }

    // Rooks & Queens
    for (index = 0; index < 4; ++index) {
        dir = RkDir[index];
        t_sq = sq + dir;
        pce = pos->pieces[t_sq];

        while (pce != OFFBOARD) {
            if (pce != EMPTY)   {   // First encountered piece 
                if (IsRQ(pce) && PieceCol[pce] == side) {
                    return TRUE;
                }
                break;
            }
            t_sq += dir;
            pce = pos->pieces[t_sq];
        }
    }

    // Bishops & Queens
    for (index = 0; index < 4; ++index) {
        dir = BiDir[index];
        t_sq = sq + dir;
        pce = pos->pieces[t_sq];

        while (pce != OFFBOARD) {
            if (pce != EMPTY)   {
                if (IsBQ(pce) && PieceCol[pce] == side) {
                    return TRUE;
                }
                break;
            }
            t_sq += dir;
            pce = pos->pieces[t_sq];
        }
    }

    // Kings
    for (index = 0; index < 8; ++index) {
        pce = pos->pieces[sq + KiDir[index]];
        if (pce != OFFBOARD && IsKi(pce) && PieceCol[pce] == side) {
            return TRUE;
        }
    }

    return FALSE;

}