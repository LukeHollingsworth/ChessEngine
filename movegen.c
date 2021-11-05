#include "stdio.h"
#include "defs.h"

#define MOVE(f,t,ca,pro,fl) ( (f) | ( (t) << 7) | ( (ca) << 14 ) | ( (pro) << 20 ) | (fl) ) // Create move integer (fl=flag)
#define SQOFFBOARD(sq) (FilesBrd[(sq)]==OFFBOARD)

const int LoopSlidePce[8] = { wB, wR, wQ, 0, bB, bR, bQ, 0 }; // Sliding pieces
const int LoopNonSlidePce[6] = { wN, wK, 0, bN, bK, 0 };
const int LoopSlideIndex[2] = { 0, 4 };   // Tells LoopSlidePce where to start depending on colour (LoopSlideIndex[BLACK]=4)
const int LoopNonSlideIndex[2] = { 0, 3 };

const int PceDir[13][8] = { // Directions each piece can make (not including pawns)
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { -8, -19, -21, -12, 8, 19, 21, 12 },   // Knight
    { -9, -11, 11, 9, 0, 0, 0, 0 }, // Bishop
    { -1, -10, 1, 10, 0, 0, 0, 0 }, // Rook
    { -1, -10, 1, 10, -9, -11, 11, 9 }, // Queen
    { -1, -10, 1, 10, -9, -11, 11, 9 }, // King
    { 0, 0, 0, 0, 0, 0, 0 },
    { -8, -19, -21, -12, 8, 19, 21, 12 },
    { -9, -11, 11, 9, 0, 0, 0, 0 },
    { -1, -10, 1, 10, 0, 0, 0, 0 },
    { -1, -10, 1, 10, -9, -11, 11, 9 },
    { -1, -10, 1, 10, -9, -11, 11, 9 }
};

const int NumDir[13] = { 0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8 }; // Number of directions wach piece (excluding pawns) can move

const int VictimScore[13] = { 0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600 };
static int MvvLvaScores[13][13];

int InitMvvLva()    {

    int Attacker;
    int Victim;

    for (Attacker = wP; Attacker <= bK; ++Attacker) {
        for (Victim = wP; Victim <= bK; ++Victim)   {
            MvvLvaScores[Victim][Attacker] = VictimScore[Victim] + 6 - (VictimScore[Attacker] / 100);   // Gives indexing of captures related to values of involved pieces
        }
    }
}

int MoveExists(S_BOARD *pos, const int move)    {   // To check that posKey entered into our PVtable isn't a duplicate (same index might be produced from diff keys)

    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);

    int MoveNum = 0;

    for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {

        if (!MakeMove(pos, list->moves[MoveNum].move))  {
            continue;
        }
        TakeMove(pos);
        if (list->moves[MoveNum].move == move)  {
            return TRUE;
        }
    }
    return FALSE;

}

static void AddQuietMove(const S_BOARD *pos, int move, S_MOVELIST *list)   {   // Adding moves to the array of move list - will be called from GenerateAllMoves

    ASSERT(SqOnBoard(FROMSQ(move)));
    ASSERT(SqOnBoard(TOSQ(move)));

    list->moves[list->count].move = move;

    if (pos->searchKillers[0][pos->ply] == move)    {   // First (best) killer move
        list->moves[list->count].score = 900000;
    } else if (pos->searchKillers[1][pos->ply] == move) {
        list->moves[list->count].score = 800000;
    } else {
        list->moves[list->count].score = pos->searchHistory[pos->pieces[FROMSQ(move)]][TOSQ(move)];
    }

    list->count++;
}

static void AddCaptureMove(const S_BOARD *pos, int move, S_MOVELIST *list)   {

    ASSERT(SqOnBoard(FROMSQ(move)));
    ASSERT(SqOnBoard(TOSQ(move)));
    ASSERT(PieceValid(CAPTURED(move)));

    list->moves[list->count].move = move;
    list->moves[list->count].score = MvvLvaScores[CAPTURED(move)][pos->pieces[FROMSQ(move)]] + 1000000;
    list->count++;
}

static void AddEnPassantMove(const S_BOARD *pos, int move, S_MOVELIST *list)   {

    ASSERT(SqOnBoard(FROMSQ(move)));
    ASSERT(SqOnBoard(TOSQ(move)));

    list->moves[list->count].move = move;
    list->moves[list->count].score = 105 + 1000000;
    list->count++;
}

static void AddWhitePawnCapMove(const S_BOARD *pos, const int from, const int to, const int cap, S_MOVELIST *list) {

    ASSERT(PieceValidEmpty(cap));   // Checking that argument of captured piece is valid or empty
    ASSERT(SqOnBoard(from));    // Checking that argument of from square is valid or empty
    ASSERT(SqOnBoard(to));  // Checking that argument of to square is valid or empty

    if (RanksBrd[from] == RANK_7)   {   // White move to promotion with capture
        AddCaptureMove(pos, MOVE(from, to, cap, wQ, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, wR, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, wB, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, wN, 0), list);
    } else  {   // Regular white capture with pawn
        AddCaptureMove(pos, MOVE(from, to, cap, EMPTY, 0), list);
    }

}

static void AddWhitePawnMove(const S_BOARD *pos, const int from, const int to, S_MOVELIST *list)   {

    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));

    if (RanksBrd[from] == RANK_7)   {   // White move to promotion without capture
        AddQuietMove(pos, MOVE(from, to, EMPTY, wQ, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, wR, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, wB, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, wN, 0), list);
    } else {    // Regular white pawn move
        AddQuietMove(pos, MOVE(from, to, EMPTY, EMPTY, 0), list);
    }

}

static void AddBlackPawnCapMove(const S_BOARD *pos, const int from, const int to, const int cap, S_MOVELIST *list) {

    ASSERT(PieceValidEmpty(cap));
    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));

    if (RanksBrd[from] == RANK_2)   {   // Black move to promotion with capture
        AddCaptureMove(pos, MOVE(from, to, cap, bQ, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, bR, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, bB, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, bN, 0), list);
    } else  {   // Regular black capture with pawn
        AddCaptureMove(pos, MOVE(from, to, cap, EMPTY, 0), list);
    }

}

static void AddBlackPawnMove(const S_BOARD *pos, const int from, const int to, S_MOVELIST *list)   {

    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));

    if (RanksBrd[from] == RANK_2)   {    // Black pawn promotion without capture 
        AddQuietMove(pos, MOVE(from, to, EMPTY, bQ, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, bR, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, bB, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, bN, 0), list);
    } else {    // Regular black pawn move
        AddQuietMove(pos, MOVE(from, to, EMPTY, EMPTY, 0), list);
    }

}

void GenerateAllMoves(const S_BOARD *pos, S_MOVELIST *list) {

    ASSERT(CheckBoard(pos));

    list->count = 0;

    int pce = EMPTY;
    int side = pos->side;
    int sq = 0; int t_sq = 0;
    int pceNum = 0;
    int dir = 0;
    int index = 0;
    int pceIndex = 0;

    if (side == WHITE)  {
        for (pceNum = 0; pceNum < pos->pceNum[wP]; ++pceNum)    {   // Generating white pawn moves
            sq = pos->pList[wP][pceNum];
            ASSERT(SqOnBoard(sq));

            if (pos->pieces[sq + 10] == EMPTY)  {   // Move forward
                AddWhitePawnMove(pos, sq, sq+10, list);

                if (RanksBrd[sq] == RANK_2 && pos->pieces[sq + 20] == EMPTY)    {   // Pawn start move
                    AddQuietMove(pos, MOVE(sq, sq+20, EMPTY, EMPTY, MFLAGPS), list);
                }
            }

            if (!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK)  {      // Capture diagonal
                AddWhitePawnCapMove(pos, sq, sq+9, pos->pieces[sq + 9], list);
            }
            if (!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK)    {
                AddWhitePawnCapMove(pos, sq, sq + 11, pos->pieces[sq + 11], list);
            }

            if (pos->enPas != NO_SQ)    {   // Pawn on H7 would produce an EnPass square off the board

                if (sq + 9 == pos->enPas)   {   // En Passant Move
                    AddEnPassantMove(pos, MOVE(sq, sq + 9, EMPTY, EMPTY, MFLAGEP), list);
                }
                if (sq + 11 == pos->enPas)  {
                    AddEnPassantMove(pos, MOVE(sq, sq + 11, EMPTY, EMPTY, MFLAGEP), list);
                }
            }
        }

        if (pos->castlePerm & WKCA) {   // White King-side Castling Permission
            if (pos->pieces[F1] == EMPTY && pos->pieces[G1] == EMPTY)   {
                if (!SqAttacked(E1, BLACK, pos) && !SqAttacked(F1, BLACK, pos)) {
                    AddQuietMove(pos, MOVE(E1, G1, EMPTY, EMPTY, MFLAGCA), list);
                }
            }
        }

        if (pos->castlePerm & WQCA) {      // White Queen-side Castling Permission
            if (pos->pieces[D1] == EMPTY && pos->pieces[C1] == EMPTY && pos->pieces[B1] == EMPTY)   {
                if (!SqAttacked(E1, BLACK, pos) && !SqAttacked(F1, BLACK, pos)) {
                    AddQuietMove(pos, MOVE(E1, C1, EMPTY, EMPTY, MFLAGCA), list);
                }
            }
        }

    } else  {
        for (pceNum = 0; pceNum < pos->pceNum[bP]; ++pceNum)    {   // Generating black pawn moves
            sq = pos->pList[bP][pceNum];
            ASSERT(SqOnBoard(sq));

            if (pos->pieces[sq - 10] == EMPTY)  {   // Move forward
                AddBlackPawnMove(pos, sq, sq-10, list);

                if (RanksBrd[sq] == RANK_7 && pos->pieces[sq - 20] == EMPTY)    {   // Pawn start move
                    AddQuietMove(pos, MOVE(sq, sq-20, EMPTY, EMPTY, MFLAGPS), list);
                }
            }

            if (!SQOFFBOARD(sq - 9) && PieceCol[pos->pieces[sq - 9]] == WHITE)  {      // Capture diagonal
                AddBlackPawnCapMove(pos, sq, sq-9, pos->pieces[sq - 9], list);
            }
            if (!SQOFFBOARD(sq - 11) && PieceCol[pos->pieces[sq - 11]] == WHITE)    {
                AddBlackPawnCapMove(pos, sq, sq - 11, pos->pieces[sq - 11], list);
            }

            if (sq - 9 == pos->enPas)   {   // En Passant Move
                AddEnPassantMove(pos, MOVE(sq, sq - 9, EMPTY, EMPTY, MFLAGEP), list);
            }
            if (sq - 11 == pos->enPas)  {
                AddEnPassantMove(pos, MOVE(sq, sq - 11, EMPTY, EMPTY, MFLAGEP), list);
            }
        }

        if (pos->castlePerm & BKCA) {   // Black King-side Castling Permission
            if (pos->pieces[F8] == EMPTY && pos->pieces[G8] == EMPTY)   {
                if (!SqAttacked(E8, WHITE, pos) && !SqAttacked(F8, WHITE, pos)) {
                    AddQuietMove(pos, MOVE(E8, G8, EMPTY, EMPTY, MFLAGCA), list);
                }
            }
        }

        if (pos->castlePerm & BQCA) {      // Black Queen-side Castling Permission
            if (pos->pieces[D8] == EMPTY && pos->pieces[C8] == EMPTY && pos->pieces[B8] == EMPTY)   {
                if (!SqAttacked(E8, WHITE, pos) && !SqAttacked(F8, WHITE, pos)) {
                    AddQuietMove(pos, MOVE(E8, C8, EMPTY, EMPTY, MFLAGCA), list);
                }
            }
        }
    }

    /* Sliding Pieces */
    pceIndex = LoopSlideIndex[side];
    pce = LoopSlidePce[pceIndex++]; // Takes piece at index x and increments to x+1

    while (pce != 0)    {
        ASSERT(PieceValid(pce));

        for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum)   {
            sq = pos->pList[pce][pceNum];
            ASSERT(SqOnBoard(sq));

            for (index = 0; index < NumDir[pce]; ++index)   {
                dir = PceDir[pce][index];
                t_sq = sq + dir;    // Target Square

                while (!SQOFFBOARD(t_sq))   {      // Iterates move either a piece is met or square is OFFBOARD
                    if (pos->pieces[t_sq] != EMPTY) {
                        if (PieceCol[pos->pieces[t_sq]] == side ^ 1) {  // BLACK ^ 1 == WHITE / WHITE ^ 1 == BLACK (just checking opposite colour)
                            AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
                        }
                        break;
                    }
                    AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
                    t_sq += dir;
                }
            }
        }

        pce = LoopSlidePce[pceIndex++];
    }

    /* Non-Sliding Pieces */
    pceIndex = LoopNonSlideIndex[side];
    pce = LoopNonSlidePce[pceIndex++];

    while (pce != 0)    {
        ASSERT(PieceValid(pce));

        for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum)   {
            sq = pos->pList[pce][pceNum];
            ASSERT(SqOnBoard(sq));

            for (index = 0; index < NumDir[pce]; ++index)   {
                dir = PceDir[pce][index];
                t_sq = sq + dir;    // Target Square

                if (SQOFFBOARD(t_sq))   {
                    continue;
                }

                if (pos->pieces[t_sq] != EMPTY) {
                    if (PieceCol[pos->pieces[t_sq]] == side ^ 1) {  // BLACK ^ 1 == WHITE / WHITE ^ 1 == BLACK (just checking opposite colour)
                        AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
                    }
                    continue;
                }
                AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
            }
        }
        
        pce = LoopNonSlidePce[pceIndex++];
    }
}

void GenerateAllCaps(const S_BOARD *pos, S_MOVELIST *list) {

    ASSERT(CheckBoard(pos));

    list->count = 0;

    int pce = EMPTY;
    int side = pos->side;
    int sq = 0; int t_sq = 0;
    int pceNum = 0;
    int dir = 0;
    int index = 0;
    int pceIndex = 0;

    if (side == WHITE)  {
        for (pceNum = 0; pceNum < pos->pceNum[wP]; ++pceNum)    {   // Generating white pawn moves
            sq = pos->pList[wP][pceNum];
            ASSERT(SqOnBoard(sq));

            if (!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK)  {      // Capture diagonal
                AddWhitePawnCapMove(pos, sq, sq+9, pos->pieces[sq + 9], list);
            }
            if (!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK)    {
                AddWhitePawnCapMove(pos, sq, sq + 11, pos->pieces[sq + 11], list);
            }

            if (pos->enPas != NO_SQ)    {   // Pawn on H7 would produce an EnPass square off the board

                if (sq + 9 == pos->enPas)   {   // En Passant Move
                    AddEnPassantMove(pos, MOVE(sq, sq + 9, EMPTY, EMPTY, MFLAGEP), list);
                }
                if (sq + 11 == pos->enPas)  {
                    AddEnPassantMove(pos, MOVE(sq, sq + 11, EMPTY, EMPTY, MFLAGEP), list);
                }
            }
        }

    } else  {
        for (pceNum = 0; pceNum < pos->pceNum[bP]; ++pceNum)    {   // Generating black pawn moves
            sq = pos->pList[bP][pceNum];
            ASSERT(SqOnBoard(sq));

            if (!SQOFFBOARD(sq - 9) && PieceCol[pos->pieces[sq - 9]] == WHITE)  {      // Capture diagonal
                AddBlackPawnCapMove(pos, sq, sq-9, pos->pieces[sq - 9], list);
            }
            if (!SQOFFBOARD(sq - 11) && PieceCol[pos->pieces[sq - 11]] == WHITE)    {
                AddBlackPawnCapMove(pos, sq, sq - 11, pos->pieces[sq - 11], list);
            }

            if (sq - 9 == pos->enPas)   {   // En Passant Move
                AddEnPassantMove(pos, MOVE(sq, sq - 9, EMPTY, EMPTY, MFLAGEP), list);
            }
            if (sq - 11 == pos->enPas)  {
                AddEnPassantMove(pos, MOVE(sq, sq - 11, EMPTY, EMPTY, MFLAGEP), list);
            }
        }
    }
    
    /* Sliding Pieces */
    pceIndex = LoopSlideIndex[side];
    pce = LoopSlidePce[pceIndex++]; // Takes piece at index x and increments to x+1

    while (pce != 0)    {
        ASSERT(PieceValid(pce));

        for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum)   {
            sq = pos->pList[pce][pceNum];
            ASSERT(SqOnBoard(sq));

            for (index = 0; index < NumDir[pce]; ++index)   {
                dir = PceDir[pce][index];
                t_sq = sq + dir;    // Target Square

                while (!SQOFFBOARD(t_sq))   {      // Iterates move either a piece is met or square is OFFBOARD
                    if (pos->pieces[t_sq] != EMPTY) {
                        if (PieceCol[pos->pieces[t_sq]] == side ^ 1) {  // BLACK ^ 1 == WHITE / WHITE ^ 1 == BLACK (just checking opposite colour)
                            AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
                        }
                        break;
                    }
                    t_sq += dir;
                }
            }
        }

        pce = LoopSlidePce[pceIndex++];
    }

    /* Non-Sliding Pieces */
    pceIndex = LoopNonSlideIndex[side];
    pce = LoopNonSlidePce[pceIndex++];

    while (pce != 0)    {
        ASSERT(PieceValid(pce));

        for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum)   {
            sq = pos->pList[pce][pceNum];
            ASSERT(SqOnBoard(sq));

            for (index = 0; index < NumDir[pce]; ++index)   {
                dir = PceDir[pce][index];
                t_sq = sq + dir;    // Target Square

                if (SQOFFBOARD(t_sq))   {
                    continue;
                }

                if (pos->pieces[t_sq] != EMPTY) {
                    if (PieceCol[pos->pieces[t_sq]] == side ^ 1) {  // BLACK ^ 1 == WHITE / WHITE ^ 1 == BLACK (just checking opposite colour)
                        AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
                    }
                    continue;
                }
            }
        }
        
        pce = LoopNonSlidePce[pceIndex++];
    }
}