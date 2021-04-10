/*
 *	BOARD.C
 *	Tom Kerrigan's Simple Chess Program (TSCP)
 *
 *	Copyright 1997 Tom Kerrigan
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "defs.h"
#include "data.h"
#include "protos.h"

// ----------------------------------------------------------------------------

typedef unsigned long long uint64;

/*
A 32 bit random number generator. An implementation in C of the algorithm
given by Knuth, the art of computer programming, vol. 2, pp. 26-27. We use e=32, so
we have to evaluate y(n) = y(n - 24) + y(n - 55) mod 2^32, which is
implicitly
done by unsigned arithmetic.
*/
/*
 random numbers from Mathematica 2.0.
 SeedRandom = 1;
 Table[Random[Integer, {0, 2^32 - 1}]
 */
unsigned long x[55] = {
	1410651636UL, 3012776752UL, 3497475623UL, 2892145026UL, 1571949714UL,
	3253082284UL, 3489895018UL, 387949491UL, 2597396737UL, 1981903553UL,
	3160251843UL, 129444464UL, 1851443344UL, 4156445905UL, 224604922UL,
	1455067070UL, 3953493484UL, 1460937157UL, 2528362617UL, 317430674UL,
	3229354360UL, 117491133UL, 832845075UL, 1961600170UL, 1321557429UL,
	747750121UL, 545747446UL, 810476036UL, 503334515UL, 4088144633UL,
	2824216555UL, 3738252341UL, 3493754131UL, 3672533954UL, 29494241UL,
	1180928407UL, 4213624418UL, 33062851UL, 3221315737UL, 1145213552UL,
	2957984897UL, 4078668503UL, 2262661702UL, 65478801UL, 2527208841UL,
	1960622036UL, 315685891UL, 1196037864UL, 804614524UL, 1421733266UL,
	2017105031UL, 3882325900UL, 810735053UL, 384606609UL, 2393861397UL };

static unsigned long y[55];
static int j, k;
static int initAl = 1;
unsigned long ul;

static unsigned int Aleatoire(void)
{
	unsigned long ul;
	if (initAl)
	{
		int i;
		initAl = 0;
		for (i = 0; i < 55; i++) y[i] = x[i];
		j = 24 - 1;
		k = 55 - 1;
	}
	ul = (y[k] += y[j]);
	if (--j < 0) j = 55 - 1;
	if (--k < 0) k = 55 - 1;
	return((unsigned int)ul);
}

uint64 Aleatoire64()
{
	uint64 n1, n2, n3, n4;

	n1 = (uint64)(Aleatoire()) & 0xFFFF;
	n2 = (uint64)(Aleatoire()) & 0xFFFF;
	n3 = (uint64)(Aleatoire()) & 0xFFFF;
	n4 = (uint64)(Aleatoire()) & 0xFFFF;

	return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

// ----------------------------------------------------------------------------

void initHtLearning()
{
	FILE* f;
	size_t nb_entre;

	f = fopen("learned_mouv.dat", "r");
	int file_size = HT_LEARNING_SIZE * sizeof(HtLearning);

	if (f == NULL)
	{
		memset(HT_Learning, 0, sizeof(HT_LEARNING_SIZE));
		f = fopen("learned_mouv.dat", "w");
		fseek(f, file_size, SEEK_SET);
		fputc('\0', f);
		fclose(f);
	}
	else
	{
		printf("READ FILE\n");
		nb_entre = fread(&HT_Learning, sizeof(HtLearning), HT_LEARNING_SIZE, f);

		printf("%d entrees trouvees\n", (int)nb_entre);

		fclose(f);
	}
}

/* init_board() sets the board to the initial game state. */

void init_board()
{
	int i;

	for (i = 0; i < 64; ++i) {
		color[i] = init_color[i];
		piece[i] = init_piece[i];
	}
	side = LIGHT;
	xside = DARK;
	castle = 15;
	ep = -1;
	fifty = 0;
	ply = 0;
	hply = 0;
	set_hash();  /* init_hash() must be called before this function */
	first_move[0] = 0;

	//initHtLearning();
}


/* init_hash() initializes the random numbers used by set_hash(). */

void init_hash()
{
	int i, j, k;

	srand(0);
	for (i = 0; i < 2; ++i)
		for (j = 0; j < 6; ++j)
			for (k = 0; k < 64; ++k)
				hash_piece[i][j][k] = Aleatoire64();
	hash_side = Aleatoire64();
	for (i = 0; i < 64; ++i)
		hash_ep[i] = Aleatoire64();
}


/* hash_rand() XORs some shifted random numbers together to make sure
   we have good coverage of all 32 bits. (rand() returns 16-bit numbers
   on some systems.) */

HashType hash_rand()
{
	int i;
	HashType r = 0;

	for (i = 0; i < 32; ++i)
		r ^= Aleatoire64() << i;
	return r;
}


/* set_hash() uses the Zobrist method of generating a unique number (hash)
   for the current chess position. Of course, there are many more chess
   positions than there are 32 bit numbers, so the numbers generated are
   not really unique, but they're unique enough for our purposes (to detect
   repetitions of the position). 
   The way it works is to XOR random numbers that correspond to features of
   the position, e.g., if there's a black knight on B8, hash is XORed with
   hash_piece[BLACK][KNIGHT][B8]. All of the pieces are XORed together,
   hash_side is XORed if it's black's move, and the en passant square is
   XORed if there is one. (A chess technicality is that one position can't
   be a repetition of another if the en passant state is different.) */

void set_hash()
{
	int i;

	hash = 0;	
	for (i = 0; i < 64; ++i)
		if (color[i] != EMPTY)
			hash ^= hash_piece[color[i]][piece[i]][i];
	if (side == DARK)
		hash ^= hash_side;
	if (ep != -1)
		hash ^= hash_ep[ep];
}

HashType get_hash()
{
	int i;

	HashType hashTmp = 0; // local
	for (i = 0; i < 64; ++i)
		if (color[i] != EMPTY)
			hashTmp ^= hash_piece[color[i]][piece[i]][i];
	if (side == DARK)
		hashTmp ^= hash_side;
	if (ep != -1)
		hashTmp ^= hash_ep[ep];
	return hashTmp;
}


/* in_check() returns TRUE if side s is in check and FALSE
   otherwise. It just scans the board to find side s's king
   and calls attack() to see if it's being attacked. */

BOOL in_check(int s)
{
	int i;

	for (i = 0; i < 64; ++i)
		if (piece[i] == KING && color[i] == s)
			return attack(i, s ^ 1);
	return TRUE;  /* shouldn't get here */
}


/* attack() returns TRUE if square sq is being attacked by side
   s and FALSE otherwise. */

BOOL attack(int sq, int s)
{
	int i, j, n;

	for (i = 0; i < 64; ++i)
		if (color[i] == s) {
			if (piece[i] == PAWN) {
				if (s == LIGHT) {
					if (COL(i) != 0 && i - 9 == sq)
						return TRUE;
					if (COL(i) != 7 && i - 7 == sq)
						return TRUE;
				}
				else {
					if (COL(i) != 0 && i + 7 == sq)
						return TRUE;
					if (COL(i) != 7 && i + 9 == sq)
						return TRUE;
				}
			}
			else
				for (j = 0; j < offsets[piece[i]]; ++j)
					for (n = i;;) {
						n = mailbox[mailbox64[n] + offset[piece[i]][j]];
						if (n == -1)
							break;
						if (n == sq)
							return TRUE;
						if (color[n] != EMPTY)
							break;
						if (!slide[piece[i]])
							break;
					}
		}
	return FALSE;
}


/* gen() generates pseudo-legal moves for the current position.
   It scans the board to find friendly pieces and then determines
   what squares they attack. When it finds a piece/square
   combination, it calls gen_push to put the move on the "move
   stack." */

void gen()
{
	int i, j, n;

	/* so far, we have no moves for the current ply */
	first_move[ply + 1] = first_move[ply];

	for (i = 0; i < 64; ++i)
		if (color[i] == side) {
			if (piece[i] == PAWN) {
				if (side == LIGHT) {
					if (COL(i) != 0 && color[i - 9] == DARK)
						gen_push(i, i - 9, 17);
					if (COL(i) != 7 && color[i - 7] == DARK)
						gen_push(i, i - 7, 17);
					if (color[i - 8] == EMPTY) {
						gen_push(i, i - 8, 16);
						if (i >= 48 && color[i - 16] == EMPTY)
							gen_push(i, i - 16, 24);
					}
				}
				else {
					if (COL(i) != 0 && color[i + 7] == LIGHT)
						gen_push(i, i + 7, 17);
					if (COL(i) != 7 && color[i + 9] == LIGHT)
						gen_push(i, i + 9, 17);
					if (color[i + 8] == EMPTY) {
						gen_push(i, i + 8, 16);
						if (i <= 15 && color[i + 16] == EMPTY)
							gen_push(i, i + 16, 24);
					}
				}
			}
			else
				for (j = 0; j < offsets[piece[i]]; ++j)
					for (n = i;;) {
						n = mailbox[mailbox64[n] + offset[piece[i]][j]];
						if (n == -1)
							break;
						if (color[n] != EMPTY) {
							if (color[n] == xside)
								gen_push(i, n, 1);
							break;
						}
						gen_push(i, n, 0);
						if (!slide[piece[i]])
							break;
					}
		}

	/* generate castle moves */
	if (side == LIGHT) {
		if (castle & 1)
			gen_push(E1, G1, 2);
		if (castle & 2)
			gen_push(E1, C1, 2);
	}
	else {
		if (castle & 4)
			gen_push(E8, G8, 2);
		if (castle & 8)
			gen_push(E8, C8, 2);
	}
	
	/* generate en passant moves */
	if (ep != -1) {
		if (side == LIGHT) {
			if (COL(ep) != 0 && color[ep + 7] == LIGHT && piece[ep + 7] == PAWN)
				gen_push(ep + 7, ep, 21);
			if (COL(ep) != 7 && color[ep + 9] == LIGHT && piece[ep + 9] == PAWN)
				gen_push(ep + 9, ep, 21);
		}
		else {
			if (COL(ep) != 0 && color[ep - 9] == DARK && piece[ep - 9] == PAWN)
				gen_push(ep - 9, ep, 21);
			if (COL(ep) != 7 && color[ep - 7] == DARK && piece[ep - 7] == PAWN)
				gen_push(ep - 7, ep, 21);
		}
	}
}


/* gen_caps() is basically a copy of gen() that's modified to
   only generate capture and promote moves. It's used by the
   quiescence search. */

void gen_caps()
{
	int i, j, n;

	first_move[ply + 1] = first_move[ply];
	for (i = 0; i < 64; ++i)
		if (color[i] == side) {
			if (piece[i]==PAWN) {
				if (side == LIGHT) {
					if (COL(i) != 0 && color[i - 9] == DARK)
						gen_push(i, i - 9, 17);
					if (COL(i) != 7 && color[i - 7] == DARK)
						gen_push(i, i - 7, 17);
					if (i <= 15 && color[i - 8] == EMPTY)
						gen_push(i, i - 8, 16);
				}
				if (side == DARK) {
					if (COL(i) != 0 && color[i + 7] == LIGHT)
						gen_push(i, i + 7, 17);
					if (COL(i) != 7 && color[i + 9] == LIGHT)
						gen_push(i, i + 9, 17);
					if (i >= 48 && color[i + 8] == EMPTY)
						gen_push(i, i + 8, 16);
				}
			}
			else
				for (j = 0; j < offsets[piece[i]]; ++j)
					for (n = i;;) {
						n = mailbox[mailbox64[n] + offset[piece[i]][j]];
						if (n == -1)
							break;
						if (color[n] != EMPTY) {
							if (color[n] == xside)
								gen_push(i, n, 1);
							break;
						}
						if (!slide[piece[i]])
							break;
					}
		}
	if (ep != -1) {
		if (side == LIGHT) {
			if (COL(ep) != 0 && color[ep + 7] == LIGHT && piece[ep + 7] == PAWN)
				gen_push(ep + 7, ep, 21);
			if (COL(ep) != 7 && color[ep + 9] == LIGHT && piece[ep + 9] == PAWN)
				gen_push(ep + 9, ep, 21);
		}
		else {
			if (COL(ep) != 0 && color[ep - 9] == DARK && piece[ep - 9] == PAWN)
				gen_push(ep - 9, ep, 21);
			if (COL(ep) != 7 && color[ep - 7] == DARK && piece[ep - 7] == PAWN)
				gen_push(ep - 7, ep, 21);
		}
	}
}


/* gen_push() puts a move on the move stack, unless it's a
   pawn promotion that needs to be handled by gen_promote().
   It also assigns a score to the move for alpha-beta move
   ordering. If the move is a capture, it uses MVV/LVA
   (Most Valuable Victim/Least Valuable Attacker). Otherwise,
   it uses the move's history heuristic value. Note that
   1,000,000 is added to a capture move's score, so it
   always gets ordered above a "normal" move. */

void gen_push(int from, int to, int bits)
{
	gen_t *g;
	
	if (bits & 16) {
		if (side == LIGHT) {
			if (to <= H8) {
				gen_promote(from, to, bits);
				return;
			}
		}
		else {
			if (to >= A1) {
				gen_promote(from, to, bits);
				return;
			}
		}
	}
	g = &gen_dat[first_move[ply + 1]++];
	g->m.b.from = (char)from;
	g->m.b.to = (char)to;
	g->m.b.promote = 0;
	g->m.b.bits = (char)bits;
	if (color[to] != EMPTY)
		g->score = 1000000 + (piece[to] * 10) - piece[from];
	else
		g->score = history[from][to];
}


/* gen_promote() is just like gen_push(), only it puts 4 moves
   on the move stack, one for each possible promotion piece */

void gen_promote(int from, int to, int bits)
{
	int i;
	gen_t *g;
	
	for (i = KNIGHT; i <= QUEEN; ++i) {
		g = &gen_dat[first_move[ply + 1]++];
		g->m.b.from = (char)from;
		g->m.b.to = (char)to;
		g->m.b.promote = (char)i;
		g->m.b.bits = (char)(bits | 32);
		g->score = 1000000 + (i * 10);
	}
}


/* makemove() makes a move. If the move is illegal, it
   undoes whatever it did and returns FALSE. Otherwise, it
   returns TRUE. */

BOOL makemove(move_bytes m)
{
#ifdef USE_FAST_HASH
	hist_dat[hply].hash = hash; //  Sauvegarde du hash de la position. A faire avant les modifs de roque...
#endif
	ASSERT(piece[m.from]!=EMPTY);

	/* test to see if a castle move is legal and move the rook
	   (the king is moved with the usual move code later) */
	if (m.bits & 2) {
		int from, to;

		if (in_check(side))
			return FALSE;
		switch (m.to) {
			case 62:
				if (color[F1] != EMPTY || color[G1] != EMPTY ||
						attack(F1, xside) || attack(G1, xside))
					return FALSE;
				from = H1;
				to = F1;
				break;
			case 58:
				if (color[B1] != EMPTY || color[C1] != EMPTY || color[D1] != EMPTY ||
						attack(C1, xside) || attack(D1, xside))
					return FALSE;
				from = A1;
				to = D1;
				break;
			case 6:
				if (color[F8] != EMPTY || color[G8] != EMPTY ||
						attack(F8, xside) || attack(G8, xside))
					return FALSE;
				from = H8;
				to = F8;
				break;
			case 2:
				if (color[B8] != EMPTY || color[C8] != EMPTY || color[D8] != EMPTY ||
						attack(C8, xside) || attack(D8, xside))
					return FALSE;
				from = A8;
				to = D8;
				break;
			default:  /* shouldn't get here */
				from = -1;
				to = -1;
				break;
		}
		// MAJ HASH
#ifdef USE_FAST_HASH
		hash ^=hash_piece[side][ROOK][from]^// On enlève la tour de from
		       hash_piece[side][ROOK][to];  // On pose la tour dans to
#endif
		color[to] = color[from];
		piece[to] = piece[from];
		color[from] = EMPTY;
		piece[from] = EMPTY;
	}

	/* back up information so we can take the move back later. */
	hist_dat[hply].m.b = m;
	hist_dat[hply].capture = piece[(int)m.to];
	hist_dat[hply].castle = castle;
	hist_dat[hply].ep = ep;
	hist_dat[hply].fifty = fifty;
#ifndef USE_FAST_HASH
	hist_dat[hply].hash = hash; // Ligne déplacée en début de fonction à cause du roque
#endif
	++ply;
	++hply;

	/* update the castle, en passant, and
	   fifty-move-draw variables */
	castle &= castle_mask[(int)m.from] & castle_mask[(int)m.to];
#ifdef USE_FAST_HASH
	if (ep != -1) hash ^= hash_ep[ep]; // On supprime l'ancien flag ep s'il y en avait 1
#endif
	if (m.bits & 8) {
		if (side == LIGHT)
			ep = m.to + 8;
		else
			ep = m.to - 8;
#ifdef USE_FAST_HASH
		ASSERT(ep != -1);
		hash ^= hash_ep[ep]; // Nouveau flag ep ?
#endif
	}
	else
		ep = -1;
	if (m.bits & 17)
		fifty = 0;
	else
		++fifty;

#ifdef USE_FAST_HASH
	if (piece[m.to] != EMPTY)                                  // Capture régulière de pièce ?
		hash ^= hash_piece[xside][piece[(int)m.to]][(int)m.to];// On supprime la pièce capturée 
#endif

    /* move the piece */
	color[(int)m.to] = side;
	if (m.bits & 32)
		piece[(int)m.to] = m.promote;
	else
		piece[(int)m.to] = piece[(int)m.from];

#ifdef USE_FAST_HASH
    // A cet endroit du code, piece[(int)m.to] est soit la pièce venant de from ou une pièce qui a été promue (Cf. code juste au-dessus)
	// et piece[(int)m.from] n'a pas encore été mise à "EMPTY".
	hash ^= hash_piece[side][piece[(int)m.to]][(int)m.to]^// On pose la pièce qui doit se trouver dans la case m.to (capturée ou promue)
	        hash_piece[side][piece[(int)m.from]][(int)m.from];// On supprime la pièce de de la case from 
#endif
	color[(int)m.from] = EMPTY;
	piece[(int)m.from] = EMPTY;

	/* erase the pawn if this is an en passant move */
	if (m.bits & 4) {
		if (side == LIGHT) {
#ifdef USE_FAST_HASH
			hash ^= hash_piece[DARK][PAWN][m.to + 8];// On supprime le pion de la prise en passant
#endif
			color[m.to + 8] = EMPTY;
			piece[m.to + 8] = EMPTY;
		}
		else {
#ifdef USE_FAST_HASH
			hash ^= hash_piece[LIGHT][PAWN][m.to - 8];// On supprime le pion de la prise en passant
#endif
			color[m.to - 8] = EMPTY;
			piece[m.to - 8] = EMPTY;
		}
	}

	/* switch sides and test for legality (if we can capture
	   the other guy's king, it's an illegal position and
	   we need to take the move back) */
	side ^= 1;
	xside ^= 1;
#ifdef USE_FAST_HASH
	hash ^= hash_side; // Changement de trait: il faut le faire à chaque fois et pas une fois sur 2.
#endif
	if (in_check(xside)) {
		takeback();
		return FALSE;
	}
#ifdef USE_FAST_HASH
	ASSERT(hash == get_hash());
#else
	set_hash();// ancienne méthode
#endif
	return TRUE;
}


/* takeback() is very similar to makemove(), only backwards :)  */

void takeback()
{
	move_bytes m;

	side ^= 1;
	xside ^= 1;
	--ply;
	--hply;
	m = hist_dat[hply].m.b;
	castle = hist_dat[hply].castle;
	ep = hist_dat[hply].ep;
	fifty = hist_dat[hply].fifty;
	hash = hist_dat[hply].hash;
	color[(int)m.from] = side;
	if (m.bits & 32)
		piece[(int)m.from] = PAWN;
	else
		piece[(int)m.from] = piece[(int)m.to];
	if (hist_dat[hply].capture == EMPTY) {
		color[(int)m.to] = EMPTY;
		piece[(int)m.to] = EMPTY;
	}
	else {
		color[(int)m.to] = xside;
		piece[(int)m.to] = hist_dat[hply].capture;
	}
	if (m.bits & 2) {
		int from, to;

		switch(m.to) {
			case 62:
				from = F1;
				to = H1;
				break;
			case 58:
				from = D1;
				to = A1;
				break;
			case 6:
				from = F8;
				to = H8;
				break;
			case 2:
				from = D8;
				to = A8;
				break;
			default:  /* shouldn't get here */
				from = -1;
				to = -1;
				break;
		}
		color[to] = side;
		piece[to] = ROOK;
		color[from] = EMPTY;
		piece[from] = EMPTY;
	}
	if (m.bits & 4) {
		if (side == LIGHT) {
			color[m.to + 8] = xside;
			piece[m.to + 8] = PAWN;
		}
		else {
			color[m.to - 8] = xside;
			piece[m.to - 8] = PAWN;
		}
	}
}




HtTyp *getTT()
{
	HtTyp *pTransp;
	pTransp = &HT[hash % (HT_SIZE - 1)];

	if (pTransp->hash == hash)
		return pTransp;
	return NULL;
}

void putTT(int depth, int eval, move m, int alpha, int beta)
{
	HtTyp *pTransp;
	pTransp = &HT[hash % (HT_SIZE - 1)];

	if (pTransp->depth<=depth)
	{
		pTransp->hash = hash;
		pTransp->depth = depth;
		pTransp->move = m;
		
		if (eval <= alpha)
		{
			pTransp->flag = FLAG_U_BOUND;
			SCALE_MATE_VALUE(eval);
			pTransp->score = eval;
		}
		else
		{
			if (eval >= beta)
			{
				pTransp->flag = FLAG_L_BOUND;
				SCALE_MATE_VALUE(eval);
				pTransp->score = eval;
			}
			else
			{
				SCALE_MATE_VALUE(eval);
				pTransp->score = eval;
				pTransp->flag = FLAG_VALID;
			}
		}
	}
}

void initHT()
{
	memset(HT, 0, sizeof(HT));
}