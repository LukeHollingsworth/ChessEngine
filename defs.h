#ifndef DEFS_H
#define DEFS_H

#include "stdlib.h"

// #define DEBUG	// Can be commented out to quicken process

#ifndef DEBUG
#define ASSERT(n)
#else
#define ASSERT(n) \
if(!(n)) { \
printf("%s - Failed\n",#n); \
printf("In file %s ",__FILE__); \
printf("at line %d\n",__LINE__); \
exit(1);}
#endif

typedef unsigned long long U64;

#define NAME "OmegaOne 1.0"
#define BRD_SQ_NUM 120

#define MAXGAMEMOVES 2048
#define MAXPOSITIONMOVES 256
#define MAXDEPTH 64

#define START_FEN  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK };
enum { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE };
enum { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE };

enum { WHITE, BLACK, BOTH };

enum {
  A1 = 21, B1, C1, D1, E1, F1, G1, H1,
  A2 = 31, B2, C2, D2, E2, F2, G2, H2,
  A3 = 41, B3, C3, D3, E3, F3, G3, H3,
  A4 = 51, B4, C4, D4, E4, F4, G4, H4,
  A5 = 61, B5, C5, D5, E5, F5, G5, H5,
  A6 = 71, B6, C6, D6, E6, F6, G6, H6,
  A7 = 81, B7, C7, D7, E7, F7, G7, H7,
  A8 = 91, B8, C8, D8, E8, F8, G8, H8, NO_SQ, OFFBOARD
};	// Board array

enum { FALSE, TRUE };

enum { WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8 };	// Castling permissions


typedef struct	{

	int move;
	int score;

} S_MOVE;

typedef struct {	// Contains array of moves
	
	S_MOVE moves[MAXPOSITIONMOVES];
	int count;

} S_MOVELIST;

typedef struct {	// When a move that beats alpha is found, store key that lead to that move

	U64 posKey;
	int move;

} S_PVENTRY;

typedef struct	{	// PV Table structure

	S_PVENTRY *pTable;	// Will allocate memory for number of entries needed
	int numEntries;

} S_PVTABLE;

typedef struct {
	
	int move;
	int castlePerm;
	int enPas;
	int fiftyMove;
	U64 posKey;

} S_UNDO;	// Structure for undoing move

typedef struct {

	int pieces[BRD_SQ_NUM];	// 120 squares from board
	U64 pawns[3];
		
	int KingSq[2];
	
	int side;
	int enPas;
	int fiftyMove;	// Draw condition
	
	int ply;	// Number of (half) moves into current search
	int hisPly;	// Number of (half) moves in game so far
	
	int castlePerm;
	
	U64 posKey;	// Unique key for each position
	
	int pceNum[13];
	int bigPce[2];	// Anything not a pawn
	int majPce[2];	// Rook, Queen
	int minPce[2];	// Bishop & Knight
	int material[2];	// Material score
	
	S_UNDO history[MAXGAMEMOVES];	// Array of move history (used for draw by repition)
	
	int pList[13][10];	// Piece list

	S_PVTABLE PvTable[1];
	int PvArray[MAXDEPTH];

	int searchHistory[13][BRD_SQ_NUM];	// Each time a move improves on alpha, increment this array
	int searchKillers[2][MAXDEPTH];	// Used in  move ordering, beta cutoff
	
} S_BOARD;

typedef struct	{	// Determines how long the search goes on for etc.

	int starttime;
	int stoptime;
	int depth;
	int depthset;
	int timeset;	// Time limit?
	int movestogo;	// Move limit?
	int infinite;

	long nodes;	// Count of all positons visited in search tree

	int quit;
	int stopped;	// Manually stopped

	float fh;	// fail high (move ordering)
	float fhf;	// fail high first 

} S_SEARCHINFO;


/* MACROS */

/* GAME MOVE

0000 0000 0000 0000 0000 0111 1111 -> From Square (0x7F)
0000 0000 0000 0011 1111 1000 0000 -> To Square (>> 7, 0x7f)
0000 0000 0011 1100 0000 0000 0000 -> Captured Piece (>> 14, 0xF)
0000 0000 0100 0000 0000 0000 0000 -> En Passant (0x40000)
0000 0000 1000 0000 0000 0000 0000 -> Pawn Start (0x80000)
0000 1111 0000 0000 0000 0000 0000 -> Promoted Piece (>> 20, 0xF)
0001 0000 0000 0000 0000 0000 0000 -> Castle (0x1000000)

*/

#define MFLAGCAP 0x7C000
#define MFLAGPROM 0xF00000

#define FROMSQ(m) ((m) & 0x7F)
#define TOSQ(m) (((m)>>7) & 0x7F)
#define CAPTURED(m) (((m)>>14) & 0xF)
#define PROMOTED(m) (((m)>>20) & 0xF)

#define MFLAGEP 0x40000
#define MFLAGPS 0x80000
#define MFLAGCA 0x1000000

#define NOMOVE 0

/* MACROS */

#define FR2SQ(f,r) ( (21 + (f) ) + ( (r) * 10 ) )	// Transforms a file, rank coord to 120-base square
#define SQ64(sq120) (Sq120ToSq64[(sq120)])	// Transforms 120-base square to 64-base
#define SQ120(sq64) (Sq64ToSq120[(sq64)])	// Transforms 64-base square to 120-base
#define POP(b) PopBit(b)	// Removes bit and returns 64-based square index
#define CNT(b) CountBits(b)
#define CLRBIT(bb,sq) ((bb) &= ClearMask[(sq)])
#define SETBIT(bb,sq) ((bb) |= SetMask[(sq)])

#define IsBQ(p) (PieceBishopQueen[(p)])
#define IsRQ(p) (PieceRookQueen[(p)])
#define IsKn(p) (PieceKnight[(p)])
#define IsKi(p) (PieceKing[(p)])

/* GLOBALS */

extern int Sq120ToSq64[BRD_SQ_NUM];
extern int Sq64ToSq120[64];
extern U64 SetMask[64];
extern U64 ClearMask[64];
extern U64 PieceKeys[13][120];
extern U64 SideKey;
extern U64 CastleKeys[16];
extern char PceChar[];
extern char SideChar[];
extern char RankChar[];
extern char FileChar[];

extern int PieceBig[13];	// Used to ask the characteristics of a piece
extern int PieceMaj[13];
extern int PieceMin[13];
extern int PieceVal[13];
extern int PieceCol[13];
extern int PiecePawn[13];

extern int FilesBrd[BRD_SQ_NUM];	// Array for squares in a file
extern int RanksBrd[BRD_SQ_NUM];	// Array for squares in a rank

extern int PieceKnight[13];
extern int PieceKing[13];
extern int PieceRookQueen[13];
extern int PieceBishopQueen[13];
extern int PieceSlides[13];

/* FUNCTIONS */

// init.c
extern void AllInit();

// bitboards.c
extern void PrintBitBoard(U64 bb);
extern int PopBit(U64 *bb);
extern int CountBits(U64 b);

// hashkeys.c
extern U64 GeneratePosKey(const S_BOARD *pos);

// board.c
extern void ResetBoard(S_BOARD *pos);
extern int ParseFen(char *fen, S_BOARD *pos);
extern void PrintBoard(const S_BOARD *pos);
extern void UpdateListMaterial(S_BOARD *pos);
extern int CheckBoard(const S_BOARD *pos);

// attack.c
extern int SqAttacked (const int sq, const int side, const S_BOARD *pos);

// io.c
extern char *PrMove(const int move);
extern char *PrSq(const int sq);
extern void PrintMoveList(const S_MOVELIST *list);
extern int ParseMove(char *ptrChar, S_BOARD *pos);

// validate.c
extern int SqOnBoard(const int sq);
extern int SideValid(const int side);
extern int FileRankValid(const int fr);
extern int PieceValidEmpty(const int pce);
extern int PieceValid(const int pce);

// movegen.c
extern void GenerateAllMoves(const S_BOARD *pos, S_MOVELIST *list);
extern void GenerateAllCaps(const S_BOARD *pos, S_MOVELIST *list);
extern int MoveExists(S_BOARD *pos, const int move);
extern int InitMvvLva();

// makemove.c
extern int MakeMove(S_BOARD *pos, int move);
extern void TakeMove(S_BOARD *pos);

// perft.c
extern void PerftTest(int depth, S_BOARD *pos);

// search.c
extern int SearchPosition(S_BOARD *pos, S_SEARCHINFO *info, int Side);

// misc.c
extern int GetTimeMs();

// pvtable.c
extern void InitPvTable(S_PVTABLE *table);
extern void StorePvMove(const S_BOARD *pos, const int move);
extern int ProbePvTable(const S_BOARD *pos);
extern int GetPvLine(const int depth, S_BOARD *pos);
extern void ClearPvTable(S_PVTABLE *table);

//evaluate.c 
extern int EvalPosition(const S_BOARD *pos);

#endif