/*
 *	DATA.H
 *	Tom Kerrigan's Simple Chess Program (TSCP)
 *
 *	Copyright 1997 Tom Kerrigan
 */


/* this is basically a copy of data.c that's included by most
   of the source files so they can use the data.c variables */
#include "defs.h"

char canAttack[6][64][64]; // canAttack[TypeDePiece][Source][Destination]
extern int pospiece[33];
extern int board[64];
extern int color[64];
extern int piece[64];
extern int side;
extern int xside;
extern int castle;
extern int ep;
extern int fifty;
extern int ply;
extern int hply;
extern gen_t gen_dat[GEN_STACK];
extern int first_move[MAX_PLY];
extern int history[64][64];
extern hist_t hist_dat[HIST_STACK];
extern long long max_time;
extern int max_depth;
extern long long start_time;
extern long long stop_time;
extern unsigned long long nodes;
extern move pv[MAX_PLY][MAX_PLY];
extern int pv_length[MAX_PLY];
extern BOOL follow_pv;
extern HashType  hash;
extern HashType hash_piece[2][6][64];
extern HashType hash_side;
extern HashType hash_ep[64];
extern int mailbox[120];
extern int mailbox64[64];
extern BOOL slide[6];
extern int offsets[6];
extern int offset[6][8];
extern int castle_mask[64];
extern char piece_char[6];
extern int init_color[64];
extern int init_piece[64];
extern HtTyp HT[HT_SIZE];
extern HtLearning HT_Learning[HT_LEARNING_SIZE];

#ifdef _DEBUG
	#define ASSERT(a) if (!(a)) {int n=0; n/=n;};
#else
	#define ASSERT(a) ;
#endif
