/*
 *	DEFS.H
 *	Tom Kerrigan's Simple Chess Program (TSCP)
 *
 *	Copyright 1997 Tom Kerrigan
 */
#ifndef _DEFS__H_
#define _DEFS__H_

#define BOOL			int
#define TRUE			1
#define FALSE			0

#define GEN_STACK		1120
#define MAX_PLY			32
#define HIST_STACK		400

#define LIGHT			0
#define DARK			1

#define PAWN			0
#define KNIGHT			1
#define BISHOP			2
#define ROOK			3
#define QUEEN			4
#define KING			5

#define EMPTY			6

/* useful squares */
#define A1				56
#define B1				57
#define C1				58
#define D1				59
#define E1				60
#define F1				61
#define G1				62
#define H1				63
#define A8				0
#define B8				1
#define C8				2
#define D8				3
#define E8				4
#define F8				5
#define G8				6
#define H8				7

#define ROW(x)			(x >> 3)
#define COL(x)			(x & 7)


/* This is the basic description of a move. promote is what
   piece to promote the pawn to, if the move is a pawn
   promotion. bits is a bitfield that describes the move,
   with the following bits:

   1	capture
   2	castle
   4	en passant capture
   8	pushing a pawn 2 squares
   16	pawn move
   32	promote

   It's union'ed with an integer so two moves can easily
   be compared with each other. */

typedef struct {
	char from;
	char to;
	char promote;
	char bits;
} move_bytes;

typedef union {
	move_bytes b;
	int u;
} move;

/* an element of the move stack. it's just a move with a
   score, so it can be sorted by the search functions. */
typedef struct {
	move m;
	int score;
} gen_t;

typedef unsigned long long HashType; // Type pour les hash
/* an element of the history stack, with the information
   necessary to take a move back. */
typedef struct {
	move m;
	int capture;
	int castle;
	int ep;
	int fifty;
	HashType hash;
} hist_t;

// Assertions
#ifdef _DEBUG
#define ASSERT(a) if (!(a)) {int n=0; n/=n;};
#else
#define ASSERT(a) ;
#endif

#define USE_HASH
#define HT_SIZE 10000000

#define USE_LEARNING
#define HT_LEARNING_SIZE 5000

// DM2 - optim hash de Zobrist
#define USE_FAST_HASH

#define PLUS_INFINI (10000)
#define MOINS_INFINI (-10000)

#define SCALE_MATE_VALUE(value) \
{\
	if (value > PLUS_INFINI-MAX_PLY) \
	{value += (ply);};\
	if (value < MOINS_INFINI+MAX_PLY) \
	{value -= (ply);};\
}

#define UNSCALE_MATE_VALUE(value) \
{\
	if (value > PLUS_INFINI-MAX_PLY) \
		value -= (ply);\
	if (value < MOINS_INFINI+MAX_PLY) \
		value += (ply);\
}

#define FLAG_VALID				1
#define FLAG_L_BOUND			2 // Eval is the lower bound, real eval should be higher
#define FLAG_U_BOUND			4


typedef struct
{
	HashType hash;
	move	move;     // est utilise pour le tri des coups (pas indispensable dans 1 premier temps)
	short   score;
	unsigned char flag;     // indique si le score est vrai, inferieur ou superieur
	unsigned char depth;    // le nombre de coup avant la quiescence.
} HtTyp;

typedef struct
{
	HashType hash;
	short   score;
	unsigned char depth;    // le nombre de coup avant la quiescence.
} HtLearning;

#endif