#include "stdio.h"
#include "defs.h"

#define INFINITE 30000
#define MATE 29000

static void CheckUp()   {   // Check if time is up or interrupt from GUI

}

static void PickNextMove(int moveNum, S_MOVELIST *list) {

    S_MOVE temp;
    int index = 0;
    int bestScore = 0;
    int bestNum = moveNum;

    for (index = moveNum; index < list->count; ++index) {
        if (list->moves[index].score > bestScore)   {
            bestScore = list->moves[index].score;
            bestNum = index;
        }
    }
    temp = list->moves[moveNum];
    list->moves[moveNum] = list->moves[bestNum];
    list->moves[bestNum] = temp;  // Swaps indexed movenum for the bestnum (i.e. puts best move first)
}

static int IsRepetition(const S_BOARD *pos)    {
    
    int index = 0;

    for (index = pos->hisPly - pos->fiftyMove; index < pos->hisPly-1; ++index)  {

        ASSERT(index >= 0 && index < MAXGAMEMOVES);

        if (pos->posKey == pos->history[index].posKey)  {
            return TRUE;
        }
    }
    return FALSE;
}

static void ClearForSearch(S_BOARD *pos, S_SEARCHINFO *info)    {   // Clear everything for new search

    int index = 0;
    int index2 = 0;

    for (index = 0; index < 13; ++index)    {
        for (index2 = 0; index2 < BRD_SQ_NUM; ++index2) {
            pos->searchHistory[index][index2] = 0;
        }
    }

    for (index = 0; index < 2; ++index)    {
        for (index2 = 0; index2 < MAXDEPTH; ++index2) {
            pos->searchKillers[index][index2] = 0;
        }
    }

    ClearPvTable(pos->PvTable);
    pos->ply = 0;

    info->starttime = GetTimeMs();  // Stop time set in function that talks to GUI
    info->stopped = 0;
    info->nodes = 0;
    info->fh = 0;
    info->fhf = 0;
}

static int Quiescence(int alpha, int beta, S_BOARD *pos, S_SEARCHINFO *info)    {   // Finds quiet move (removes horizon effect)

    ASSERT(CheckBoard(pos));
    info->nodes++;

    if (IsRepetition(pos) || pos->fiftyMove >= 100) {
        return 0;
    }

    if (pos->ply > MAXDEPTH - 1)    {
        return EvalPosition(pos);
    }

    int Score = EvalPosition(pos);

    if (Score >= beta)  {
        return beta;
    }

    if (Score > alpha)  {   // "Standing pat"
        alpha = Score;
    }

    S_MOVELIST list[1];
    GenerateAllCaps(pos, list);

    int MoveNum = 0;
    int Legal = 0;
    int OldAlpha = alpha;
    int BestMove = NOMOVE;
    Score = -INFINITE;
    int PvMove = ProbePvTable(pos);

    for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        
        PickNextMove(MoveNum, list);    // Move-Ordering

        if (!MakeMove(pos, list->moves[MoveNum].move))  {
            continue;
        }

        Legal++;
        Score = -Quiescence(-beta, -alpha, pos, info);  // Negamax the score and decrease the depth
        TakeMove(pos);

        if (Score > alpha)  {
            if (Score >= beta)  {
                if (Legal == 1) {
                    info->fhf++;    // Searched best move first
                }
                info->fh++;
                return beta;    // Beta cutoff
            }
            alpha = Score;
            BestMove = list->moves[MoveNum].move;
        }
        if (alpha != OldAlpha)  {
            StorePvMove(pos, BestMove);
        }
    }
    return alpha;
}

static int AlphaBeta(int alpha, int beta, int depth, S_BOARD *pos, S_SEARCHINFO *info, int DoNull)  {

    ASSERT(CheckBoard(pos));

    if (depth == 0) {
        return Quiescence(alpha, beta, pos, info);
        // return EvalPosition(pos);
    }

    info->nodes++;

    if (IsRepetition(pos) || pos->fiftyMove >= 100) {
        return 0;
    }

    if (pos->ply > MAXDEPTH - 1)    {
        return EvalPosition(pos);
    }

    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);

    int MoveNum = 0;
    int Legal = 0;  // To determine if checkmate / stalemate reached
    int OldAlpha = alpha;
    int BestMove = NOMOVE;
    int Score = -INFINITE;
    int PvMove = ProbePvTable(pos);

    if (PvMove != NOMOVE)   {   // Playing the principal variation move first
        for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
            if (list->moves[MoveNum].move == PvMove)    {
                list->moves[MoveNum].score = 2000000;
                break;
            }
        }
    }

    for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        
        PickNextMove(MoveNum, list);    // Move-Ordering

        if (!MakeMove(pos, list->moves[MoveNum].move))  {
            continue;
        }

        Legal++;
        Score = -AlphaBeta(-beta, -alpha, depth-1, pos, info, TRUE);  // Negamax the score and decrease the depth
        TakeMove(pos);

        if (Score > alpha)  {
            if (Score >= beta)  {
                if (Legal == 1) {
                    info->fhf++;    // Searched best move first
                }
                info->fh++;

                if (!(list->moves[MoveNum].move & MFLAGCAP))    {   // Non-capture moves that casue beta cut-off
                    pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
                    pos->searchKillers[0][pos->ply] = list->moves[MoveNum].move;
                }

                return beta;    // Beta cutoff
            }
            alpha = Score;
            BestMove = list->moves[MoveNum].move;
            if (!(list->moves[MoveNum].move & MFLAGCAP))  {
                pos->searchHistory[pos->pieces[FROMSQ(BestMove)]][TOSQ(BestMove)] += depth; // Error?!
            }
        }
    }

    if (Legal == 0) {
        if (SqAttacked(pos->KingSq[pos->side], pos->side^1, pos))   {   // If our King is attacked by the other side
            return -MATE + pos->ply;    // Used to calculate the number of moves to mate
        } else {
            return 0;
        }
    }

    if (alpha != OldAlpha)  {
        StorePvMove(pos, BestMove);
    }

    return alpha;
}

int SearchPosition(S_BOARD *pos, S_SEARCHINFO *info, int Side)   {   // Iterative Deepening & Search Init

    int bestMove = NOMOVE;
    int bestScore = -INFINITE;
    int currentDepth = 0;
    int pvMoves = 0;
    int pvNum = 0;
    info->depth = 5;

    ClearForSearch(pos, info);

    if (Side == pos->side)  {
        bestScore = AlphaBeta(-INFINITE, INFINITE, info->depth, pos, info, TRUE); 
        pvMoves = GetPvLine(info->depth, pos); // Get the best series of moves for that depth & position
        bestMove = pos->PvArray[0];

        printf("Depth: %d Score:%d Move:%s",info->depth, bestScore, PrMove(bestMove));

        printf("\n");

        return bestMove;

    } else {
        return NOMOVE;
    }
}