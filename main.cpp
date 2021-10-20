#include <iostream>
#include <stdio.h>
#include <string.h>
#include <array>
#include <vector>
#include <algorithm>
#include <cstring>

#define U49 unsigned long long

//bitboard macros
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))

//FEN EXAMPLES
#define empty_board "7/7/7/7/7/7/7 w 0"
#define start_pos "2ele1z/ppppppp/7/7/7/PPPPPPP/2ELE1Z w 1"

//enumerate board
enum{
    a7, b7, c7, d7, e7, f7, g7,
    a6, b6, c6, d6, e6, f6, g6,
    a5, b5, c5, d5, e5, f5, g5,
    a4, b4, c4, d4, e4, f4, g4,
    a3, b3, c3, d3, e3, f3, g3,
    a2, b2, c2, d2, e2, f2, g2,
    a1, b1, c1, d1, e1, f1, g1, no_sq
};

//colours
enum {white, black, both};

enum {straight, diagonal};

//encode pieces
enum {P, E, L, Z, p, e, l, z};

//count bits in bitboard
static inline int count_bits(U49 bitboard)
{
    //bit counter
    int count = 0;

    //iteratively reset least significant 1st bit in bitboard
    while(bitboard)
    {
        count++;
        bitboard &= bitboard - 1;
    }

    return count;
}

//get index of bits
static inline int get_least_sig_idx(U49 bitboard)
{
    //make sure bitboard isn't zero
    if(bitboard)
    {
        //count trailing bits before least significant 1st bit
        return count_bits((bitboard & -bitboard) -1);
    }
    else
    {
        //return error index
        return -1;
    }
}

//this constant allows the conversion from the square bit index to a readable coordinate
char *bit_idx_to_coord[] = {
    "a7", "b7", "c7", "d7", "e7", "f7", "g7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1"
};

//ASCII pieces
char ascii_pieces[] = {'P','E','L','Z','p','e','l','z'};

//character to encoded constants
int char_pieces[123] =
{
    char_pieces['P'] = P,
    char_pieces['E'] = E,
    char_pieces['L'] = L,
    char_pieces['Z'] = Z,
    char_pieces['p'] = p,
    char_pieces['e'] = e,
    char_pieces['l'] = l,
    char_pieces['z'] = z
};



//define pieces
U49 bitboards[8];

//define occupancies
U49 occupancies[3];

//side to move
int side = 0;



//psuedo rand num state
unsigned int state = 1804289383;

//generate 32 bit psuedo numbers
unsigned int get_rand_num_u32()
{
    unsigned int x = state;

    //XOR shift Algorithm
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;

    state = x;

    return x;
}

//generate 64 bit pseudo numbers
U49 get_rand_U64_num()
{
    U49 n1, n2, n3, n4;

    n1 = (U49)((get_rand_num_u32()) & 0xFFFF);
    n2 = (U49)((get_rand_num_u32()) & 0xFFFF);
    n3 = (U49)((get_rand_num_u32()) & 0xFFFF);
    n4 = (U49)((get_rand_num_u32()) & 0xFFFF);

    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

//generate magic number candidate
U49 gen_magic_numb()
{
    return get_rand_U64_num() & get_rand_U64_num() & get_rand_U64_num();
}

//print bitboard
void printBitBoard(U49 bitboard)
{
    printf("\n");
    for(int rank = 0; rank < 7; rank++)
    {
        for(int files = 0; files < 7; files++)
        {
            //convert file and rank into square
            int square = rank * 7 + files;

            if(!files)
            {
                printf("  %d ", 7 - rank);
            }


            //print the state of the bit
            printf(" %d", get_bit(bitboard, square) ? 1 : 0);
        }
        printf("\n");
    }

    printf("\n     a b c d e f g\n\n");

    //print bitboard as unsigned DEC NUM
    printf("     Bitboard: %llud\n\n", bitboard);
}

//print the board
void printBoard()
{
    printf("\n");
    for(int rank = 0; rank < 7; rank++)
    {
        for(int file = 0; file < 7; file++)
        {
            int square = rank * 7 + file;

            if(!file)
            {
                printf("  %d ", 7 - rank);
            }

            int piece = -1;

            for(int i = P; i <= z; i++)
            {
                if(get_bit(bitboards[i], square))
                {
                    piece = i;
                }
            }

            printf(" %c", (piece == -1) ? '.' : ascii_pieces[piece]);
        }
        printf("\n");
    }

    printf("\n     a b c d e f g\n\n");

    printf("     Side:   %s\n", (!side) ? "white" : "black");
}

//DECODE FEN
void decodeFEN(char *fen)
{
    //reset board
    memset(bitboards, 0ULL, sizeof(bitboards));
    memset(occupancies, 0ULL, sizeof(occupancies));
    side = 0;

    for(int rank = 0; rank < 7; rank++)
    {
        for(int file = 0; file < 7; file++)
        {
            int square = rank * 7 + file;

            if((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z'))
            {
                int piece = char_pieces[*fen];

                set_bit(bitboards[piece], square);

                fen++;
            }

            if(*fen >= '0' && *fen <= '7')
            {
                int off = *fen - '0';

                file += off;

                int piece = -1;

                for(int i = P; i <= z; i++)
                {
                    if(get_bit(bitboards[i], square))
                    {
                        piece = i;
                    }
                }

                if(piece == -1)
                {
                    file--;
                }

                fen++;
            }

            if(*fen == '/')
            {
                fen++;
            }
        }
    }

    fen++;

    (*fen == 'w') ? (side = white) : (side = black);

    for(int i = P; i <= Z; i++)
    {
        occupancies[white] |= bitboards[i];
    }

    for(int i = p; i <= z; i++)
    {
        occupancies[black] |= bitboards[i];
    }

    occupancies[both] |= occupancies[white];
    occupancies[both] |= occupancies[black];

}

/*
       not A file

  7  0 1 1 1 1 1 1
  6  0 1 1 1 1 1 1
  5  0 1 1 1 1 1 1
  4  0 1 1 1 1 1 1  RIVER
  3  0 1 1 1 1 1 1
  2  0 1 1 1 1 1 1
  1  0 1 1 1 1 1 1

     a b c d e f g
*/

// not A file const
const U49 not_a_file = 558517276622718ULL;

// not H file const
const U49 not_g_file = 279258638311359ULL;

// not HG file const
const U49 not_fg_file = 137412980756383ULL;

// not AB file const
const U49 not_ab_file = 549651923025532ULL;

//not black lion castle
const U49 not_black_castle = 462364ULL;

//not white lion castle
const U49 not_white_castle = 124114891177984ULL;

//black half of the board split by the river(incls river)
const U49 black_territory = 268435455ULL;

//white half of the board split by the river(incls river)
const U49 white_territory = 562949951324160ULL;

//river rank
U49 river = 266338304ULL;

//diag attack masks
U49 diag_msks[49];

//str attack masks
U49 str_msks[49];

//diags attacks table
U49 diag_atts[49][256];

//str attacks table
U49 str_atts[49][1024];

//diagonal relevant occupancy bit count for every square on board
const int diag_rel_bits[49] = {
    5, 4, 4, 4, 4, 4, 5,
    4, 4, 4, 4, 4, 4, 4,
    4, 4, 6, 6, 6, 4, 4,
    4, 4, 6, 8, 6, 4, 4,
    4, 4, 6, 6, 6, 4, 4,
    4, 4, 4, 4, 4, 4, 4,
    5, 4, 4, 4, 4, 4, 5
};

//straight relevant occupancy bit count for every square on board
const int str_rel_bits[49] = {
    10, 9, 9, 9, 9, 9, 10,
    9, 8, 8, 8, 8, 8, 9,
    9, 8, 8, 8, 8, 8, 9,
    9, 8, 8, 8, 8, 8, 9,
    9, 8, 8, 8, 8, 8, 9,
    9, 8, 8, 8, 8, 8, 9,
    10, 9, 9, 9, 9, 9, 10
};

// straight magic numbers
U49 str_magic_num[49] = {
0x2300010808008100ULL,
0x248100020200020aULL,
0x1401042002084000ULL,
0x100400801003008ULL,
0x4100100080600201ULL,
0x2604201000800040ULL,
0xc2008208200104a4ULL,
0x4022000400288000ULL,
0x900800c000400ULL,
0x980442407040c8ULL,
0x2044010100410002ULL,
0x12040880800008ULL,
0x164004040400018ULL,
0x4602001000200041ULL,
0x10002000c000000ULL,
0x404080808002000ULL,
0x4080404000002ULL,
0x90040802000200ULL,
0x1804080080044000ULL,
0x40102002020000ULL,
0x8800040800400000ULL,
0x662200100a400000ULL,
0x40410408000000ULL,
0x4900801008408842ULL,
0x60502008080204ULL,
0x822100801002001ULL,
0x400480400c844840ULL,
0x8002000800400082ULL,
0x2001020106810ULL,
0x280208004020040ULL,
0x2010100204000800ULL,
0x20011020040080ULL,
0x10100810020000ULL,
0x108080808100000ULL,
0x2a0080010200026ULL,
0x880140008600000ULL,
0x61000820400408ULL,
0x80a0040a00601000ULL,
0x2020041002040020ULL,
0x3008010d400006ULL,
0x7201c50041000015ULL,
0x12001002201808ULL,
0x8002000400084000ULL,
0x1001012088000ULL,
0xb0e801082108000ULL,
0x4020042101004000ULL,
0x8004002101008000ULL,
0x100092904020004ULL,
0xa84002008208010ULL
};

// diagonal magic numbers
U49 diag_magic_num[49] = {
0x22380c210802022ULL,
0x50014a0290008082ULL,
0x1025201082400404ULL,
0x404280000412004ULL,
0xe0a14800000080eULL,
0x830284200440ULL,
0x4000280804040008ULL,
0x40090024914110ULL,
0x40004a210400000ULL,
0x100a10210300000ULL,
0x1200c0400080ULL,
0x64088141410080ULL,
0x1189006124c12040ULL,
0x2e0042814003010ULL,
0x8022306150084ULL,
0x4054400810808050ULL,
0x8204804080080010ULL,
0x1080020a030340ULL,
0x8108880100800144ULL,
0x20803014005800ULL,
0x2002608203104200ULL,
0x2140482801020ULL,
0x22108150080110ULL,
0xa020100120000ULL,
0xa0100448044400ULL,
0x42200240421a0008ULL,
0x1061001080001ULL,
0x13064008400100ULL,
0x401204a24000000ULL,
0x84100a1002081224ULL,
0x100a006491000000ULL,
0x2200044301800040ULL,
0x8280030400204002ULL,
0xc01161404420818ULL,
0x4812044601886014ULL,
0x88003a8004400000ULL,
0x80810320102000ULL,
0x1000011140400020ULL,
0x10048c1c05600008ULL,
0x80900ca8404180a0ULL,
0x20c401120001005ULL,
0x2408005304002800ULL,
0x30204c600408a000ULL,
0x4008c8140c000240ULL,
0x32805092ULL,
0x400044041221c000ULL,
0xa102010014280201ULL,
0x4005800a402012d1ULL,
0x8004a0020a4b020cULL
};

//pawn attacks
U49 pawn_att[2][49];

//zebra attacks
U49 zebra_att[49];

//lion attacks
U49 lion_att[2][49];

//elephant attacks
U49 eleph_att[49];

//generate pawn attacks
//TODO: ADD COLLISION DETECTION
U49 msk_pawn_att(int side, int square)
{
    // result attacks bitboard
    U49 attacks = 0ULL;

    // piece bitboard
    U49 bitboard = 0ULL;

    // set pieces
    set_bit(bitboard, square);

    // white pawns
    if(!side)
    {
        if(bitboard & white_territory)
        {
            if((bitboard >> 6) & not_a_file) attacks |= (bitboard >> 6);
            if((bitboard >> 7)) attacks |= (bitboard >> 7);
            if((bitboard >> 8) & not_g_file) attacks |= (bitboard >> 8);
        }
        else
        {
            if((bitboard >> 6) & not_a_file) attacks |= (bitboard >> 6);
            if((bitboard >> 7)) attacks |= (bitboard >> 7);
            if((bitboard >> 8) & not_g_file) attacks |= (bitboard >> 8);
//            if(bitboard << 7) attacks |= (bitboard << 7);
//            if(bitboard << 14) attacks |= (bitboard << 14);
        }

    }
    else // black pawns
    {
        if(bitboard & black_territory)
        {
            if((bitboard << 6) & not_g_file) attacks |= (bitboard << 6);
            if((bitboard << 7)) attacks |= (bitboard << 7);
            if((bitboard << 8) & not_a_file) attacks |= (bitboard << 8);
        }
        else
        {
            if((bitboard << 6) & not_g_file) attacks |= (bitboard << 6);
            if((bitboard << 7)) attacks |= (bitboard << 7);
            if((bitboard << 8) & not_a_file) attacks |= (bitboard << 8);
//            if(bitboard >> 7) attacks |= (bitboard >> 7);
//            if(bitboard >> 14) attacks |= (bitboard >> 14);
        }

    }



    //return attack bitboard
    return attacks;
}

//generate zebra attacks
U49 msk_zebra_att(int square)
{
    // result attacks bitboard
    U49 attacks = 0ULL;

    // piece bitboard
    U49 bitboard = 0ULL;

    // set pieces
    set_bit(bitboard, square);
    //Zebra offsets for shifts: 13,15,9,5
    if((bitboard >> 15) & not_g_file) attacks |= (bitboard >> 15);
    if((bitboard >> 13) & not_a_file) attacks |= (bitboard >> 13);
    if((bitboard >> 9) & not_fg_file) attacks |= (bitboard >> 9);
    if((bitboard >> 5) & not_ab_file) attacks |= (bitboard >> 5);
    if((bitboard << 15) & not_a_file) attacks |= (bitboard << 15);
    if((bitboard << 13) & not_g_file) attacks |= (bitboard << 13);
    if((bitboard << 9) & not_ab_file) attacks |= (bitboard << 9);
    if((bitboard << 5) & not_fg_file) attacks |= (bitboard << 5);

    return attacks;
}

//Generate Lion Attacks
//TODO: ADD ATTACK TO CAPTURE OPPOSITION LION
U49 msk_lion_att(int side, int square)
{
    // result attacks bitboard
    U49 attacks = 0ULL;

    // piece bitboard
    U49 bitboard = 0ULL;

    // set pieces
    set_bit(bitboard, square);

    // white lion
    // lion offset shifts: 6,7,8, 1 & 1
    if(side == white)
    {
        if(bitboard & not_white_castle)
        {
            if((bitboard >> 6) & not_white_castle) attacks |= (bitboard >> 6);
            if((bitboard >> 7) & not_white_castle) attacks |= (bitboard >> 7);
            if((bitboard >> 8) & not_white_castle) attacks |= (bitboard >> 8);
            if((bitboard >> 1) & not_white_castle) attacks |= (bitboard >> 1);
            if((bitboard << 6) & not_white_castle) attacks |= (bitboard << 6);
            if((bitboard << 7) & not_white_castle) attacks |= (bitboard << 7);
            if((bitboard << 8) & not_white_castle) attacks |= (bitboard << 8);
            if((bitboard << 1) & not_white_castle) attacks |= (bitboard << 1);
        }
    }
    else if(side == black) // black lion
    {
        if(bitboard & not_black_castle)
        {
            if((bitboard >> 6) & not_black_castle) attacks |= (bitboard >> 6);
            if((bitboard >> 7) & not_black_castle) attacks |= (bitboard >> 7);
            if((bitboard >> 8) & not_black_castle) attacks |= (bitboard >> 8);
            if((bitboard >> 1) & not_black_castle) attacks |= (bitboard >> 1);
            if((bitboard << 6) & not_black_castle) attacks |= (bitboard << 6);
            if((bitboard << 7) & not_black_castle) attacks |= (bitboard << 7);
            if((bitboard << 8) & not_black_castle) attacks |= (bitboard << 8);
            if((bitboard << 1) & not_black_castle) attacks |= (bitboard << 1);
        }
    }

    return attacks;
}

//Generate Elephant Moves
U49 msk_eleph_att(int square)
{
    // result attacks bitboard
    U49 attacks = 0ULL;

    // piece bitboard
    U49 bitboard = 0ULL;

    // set pieces
    set_bit(bitboard, square);
    //elephant offset moves: 7, 14, 1, 2
    if(bitboard >> 7) attacks |= (bitboard >> 7);
    if(bitboard >> 14) attacks |= (bitboard >> 14);
    if(bitboard << 7) attacks |= (bitboard << 7);
    if(bitboard << 14) attacks |= (bitboard << 14);
    if((bitboard >> 1) & not_g_file) attacks |= (bitboard >> 1);
    if((bitboard >> 2) & not_fg_file) attacks |= (bitboard >> 2);
    if((bitboard << 1) & not_a_file) attacks |= (bitboard << 1);
    if((bitboard << 2) & not_ab_file) attacks |= (bitboard << 2);

    return attacks;
}

//diagonal attacks
U49 msk_diag_att(int square)
{
    // result attacks bitboard
    U49 attacks = 0ULL;

    int r, f;
    int tr = square / 7;
    int tf = square % 7;

    //mask diagonal occupancy bits
    for(r = tr + 1, f = tf + 1; r <= 5 && f <= 5; r++, f++) attacks |= (1ULL << (r * 7 + f));
    for(r = tr - 1, f = tf + 1; r >= 1 && f <= 5; r--, f++) attacks |= (1ULL << (r * 7 + f));
    for(r = tr + 1, f = tf - 1; r <= 5 && f >= 1; r++, f--) attacks |= (1ULL << (r * 7 + f));
    for(r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) attacks |= (1ULL << (r * 7 + f));

    return attacks;
}

//straight attacks over board
U49 msk_str_att(int square)
{
    // result attacks bitboard
    U49 attacks = 0ULL;

    int r, f;
    int tr = square / 7;
    int tf = square % 7;

    //mask diagonal occupancy bits
    for(r = tr + 1; r <= 5; r++) attacks |= (1ULL << (r * 7 + tf));
    for(r = tr - 1; r >= 1; r--) attacks |= (1ULL << (r * 7 + tf));
    for(f = tf + 1; f <= 5; f++) attacks |= (1ULL << (tr * 7 + f));
    for(f = tf - 1; f >= 1; f--) attacks |= (1ULL << (tr * 7 + f));

    return attacks;
}

// generate Diag attacks on the fly
U49 diag_att_otf(int square, U49 block)
{
    // result attacks bitboard
    U49 attacks = 0ULL;

    int r, f;
    int tr = square / 7;
    int tf = square % 7;

    //generate diag attacks
    for(r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++)
    {
        attacks |= (1ULL << (r * 7 + f));
        if(1ULL << (r * 7 + f) & block) break;
    }
    for(r = tr - 1, f = tf + 1; r >= 0 && f <= 6; r--, f++)
    {
        attacks |= (1ULL << (r * 7 + f));
        if(1ULL << (r * 7 + f) & block) break;
    }
    for(r = tr + 1, f = tf - 1; r <= 6 && f >= 0; r++, f--)
    {
        attacks |= (1ULL << (r * 7 + f));
        if(1ULL << (r * 7 + f) & block) break;
    }
    for(r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--)
    {
        attacks |= (1ULL << (r * 7 + f));
        if(1ULL << (r * 7 + f) & block) break;
    }

    return attacks;
}

//Generate straight line attacks
U49 str_att_otf(int square, U49 block)
{
    // result attacks bitboard
    U49 attacks = 0ULL;

    int r, f;
    int tr = square / 7;
    int tf = square % 7;

    //mask diagonal occupancy bits
    for(r = tr + 1; r <= 6; r++)
    {
        attacks |= (1ULL << (r * 7 + tf));
        if(1ULL << (r * 7 + tf) & block) break;
    }
    for(r = tr - 1; r >= 0; r--)
    {
        attacks |= (1ULL << (r * 7 + tf));
        if(1ULL << (r * 7 + tf) & block) break;
    }
    for(f = tf + 1; f <= 6; f++)
    {
        attacks |= (1ULL << (tr * 7 + f));
        if(1ULL << (tr * 7 + f) & block) break;
    }
    for(f = tf - 1; f >= 0; f--)
    {
        attacks |= (1ULL << (tr * 7 + f));
        if(1ULL << (tr * 7 + f) & block) break;
    }

    return attacks;
}

// initialise leaper pieces attacks
void init_leaper_attacks()
{
    for(int square = 0; square < 49; square++)
    {
        pawn_att[white][square] = msk_pawn_att(white, square);
        pawn_att[black][square] = msk_pawn_att(black, square);

        zebra_att[square] = msk_zebra_att(square);

        lion_att[white][square] = msk_lion_att(white, square);
        lion_att[black][square] = msk_lion_att(black, square);

        eleph_att[square] = msk_eleph_att(square);
    }
}

//set occupied squares
U49 set_occupancy(int index, int bits_msk, U49 att_msk)
{
    U49 occupancy = 0ULL;

    for(int i = 0; i < bits_msk; i++)
    {
        //get LS1B idx of att_msk
        int square = get_least_sig_idx(att_msk);

        //pop LS1B in att_msk
        pop_bit(att_msk, square);

        if(index & (1 << i))
        {
            //populate occupancy bitboard
            occupancy |= (1ULL << square);
        }
    }

    return occupancy;
}

//find good magic number
U49 findMagicNum(int square, int rel_bits, int diag)
{
    U49 occupancies[1024];
    U49 atts[1024];
    U49 used_atts[1024];

    U49 att_msk = diag ? msk_diag_att(square) : msk_str_att(square);

    int occ_indices = 1 << rel_bits;

    for(int i = 0; i < occ_indices; i++)
    {
        //init occupancies
        occupancies[i] = set_occupancy(i, rel_bits, att_msk);

        //init attacks
        atts[i] = diag ? diag_att_otf(square, occupancies[i]) : str_att_otf(square, occupancies[i]);
    }

    //test magic numbers
    for(int rand_count = 0; rand_count < 100000000; rand_count++)
    {
        //generate magic number candidates
        U49 magicNum = gen_magic_numb();

        // skip bad candidates
        if(count_bits((att_msk * magicNum) & 0xFF00000000000000) < 6) continue;

        //init used attacks
        memset(used_atts, 0ULL, sizeof(used_atts));

        //init idx and fail flag
        int index, fail;

        // test magic index
        for(index = 0, fail = 0; !fail && index < occ_indices; index++)
        {
            //init magic index
            int magic_idx = (int)((occupancies[index] * magicNum) >> (64 - rel_bits));

            if(used_atts[magic_idx] == 0ULL)
            {
                used_atts[magic_idx] = atts[index];
            }
            else if(used_atts[magic_idx] != atts[index])
            {
                fail = 1;
            }
        }

        if(!fail)
        {
            return magicNum;
        }
    }

    printf(" Magic Number Failed");
    return 0ULL;
}

//init magic numbers
void init_magic_nums()
{
    for(int square = 0; square < 49; square++)
    {
        str_magic_num[square] = findMagicNum(square, str_rel_bits[square], straight);
    }

    for(int square = 0; square < 49; square++)
    {
        diag_magic_num[square] = findMagicNum(square, str_rel_bits[square], diagonal);
    }
}

void init_sliders_atts(int diag)
{
    for(int square = 0; square < 49; square++)
    {
        diag_msks[square] = msk_diag_att(square);
        str_msks[square] = msk_str_att(square);

        U49 att_msk = diag ? diag_msks[square] : str_msks[square];

        int rel_bits = count_bits(att_msk);

        int occ_indices = (1 << rel_bits);

        for(int i = 0; i < occ_indices; i++)
        {

            //int magic_idx = (int)((occupancies[index] * magicNum) >> (64 - rel_bits));

            if(diag)
            {
                U49 occupancy = set_occupancy(i, rel_bits, att_msk);

                int magic_idx = (occupancy * diag_magic_num[square]) >> (64 - diag_rel_bits[square]);

                diag_atts[square][magic_idx] = diag_att_otf(square, occupancy);
            }
            else
            {
                U49 occupancy = set_occupancy(i, rel_bits, att_msk);

                int magic_idx = (occupancy * str_magic_num[square]) >> (64 - str_rel_bits[square]);

                str_atts[square][magic_idx] = str_att_otf(square, occupancy);
            }
        }
    }
}

static inline U49 get_diag_attacks(int square, U49 occupancy)
{
    occupancy &= diag_msks[square];
    occupancy *= diag_magic_num[square];
    occupancy >>= 64 - diag_rel_bits[square];

    return diag_atts[square][occupancy];
}

static inline U49 get_str_attacks(int square, U49 occupancy)
{
    occupancy &= str_msks[square];
    occupancy *= str_magic_num[square];
    occupancy >>= 64 - str_rel_bits[square];

    return str_atts[square][occupancy];
}

static inline U49 get_lion_slide_attacks(int square, U49 occupancy)
{
    U49 result = 0ULL;

    U49 diag_occ = occupancy;
    U49 str_occ = occupancy;

    diag_occ &= diag_msks[square];
    diag_occ *= diag_magic_num[square];
    diag_occ >>= 64 - diag_rel_bits[square];

    result = diag_atts[square][diag_occ];

    str_occ &= str_msks[square];
    str_occ *= str_magic_num[square];
    str_occ >>= 64 - str_rel_bits[square];

    result |= str_atts[square][str_occ];

    return result;
}

//check if square is under attack
static inline int sqr_under_attack(int square, int side)
{

    if((side == white) && (pawn_att[black][square] & bitboards[P]))
    {
        return 1;
    }

    if((side == black) && (pawn_att[white][square] & bitboards[p]))
    {
        return 1;
    }

    if(zebra_att[square] & ((side == white) ? bitboards[Z] : bitboards[z]))
    {
        return 1;
    }

    if(eleph_att[square] & ((side == white) ? bitboards[E] : bitboards[e]))
    {
        return 1;
    }

    if((side == black) && (lion_att[black][square] & bitboards[l]))
    {
        return 1;
    }


    if((side == white) && (lion_att[white][square] & bitboards[L]))
    {
        return 1;
    }

    if((side == black) && (square == get_least_sig_idx(bitboards[L]))
       && (diag_att_otf(get_least_sig_idx(bitboards[l]), occupancies[both]) & bitboards[L]))
    {
        return 1;
    }

    if((side == black) && (square == get_least_sig_idx(bitboards[L]))
       && (str_att_otf(get_least_sig_idx(bitboards[l]), occupancies[both]) & bitboards[L]))
    {
        return 1;
    }

    if((side == white) && (square == get_least_sig_idx(bitboards[l]))
       && (diag_att_otf(get_least_sig_idx(bitboards[L]), occupancies[both]) & bitboards[l]))
    {
        return 1;
    }

    if((side == white) && (square == get_least_sig_idx(bitboards[l]))
       && (str_att_otf(get_least_sig_idx(bitboards[L]), occupancies[both]) & bitboards[l]))
    {
        return 1;
    }

    return 0;
}

void printAttSqrs(int side)
{
    printf("\n");
    for(int rank = 0; rank < 7; rank++)
    {
        for(int file = 0; file < 7; file++)
        {
            int sqr = rank * 7 + file;

            if(!file)
            {
                printf("  %d ", 7 - rank);
            }

            printf(" %d", sqr_under_attack(sqr, side) ? 1 : 0);
        }

        printf("\n");
    }

    printf("\n     a b c d e f g\n\n");
}

std::vector<std::string> moves;

void printVec()
{
    sort(moves.begin(), moves.end());

    for(int i = 0; i < moves.size(); i++)
    {
        if(i == moves.size() -1)
        {
            std::cout << moves[i];
        }
        else
        {
            std::cout << moves[i] << " ";
        }
    }
    printf("\n");
}

//generates movesets
static inline void gen_moves()
{
    int src_sqr;
    int trg_sqr;

    U49 bitboard;
    U49 atts;

    std::string str = "";

    for(int i = P; i < z; i++)
    {
        bitboard = bitboards[i];

        //moves for colour specific pieces, i.e pawns
        if(side == white)
        {
            if(i == P)
            {
                while(bitboard)
                {
                    src_sqr = get_least_sig_idx(bitboard);

                    trg_sqr = src_sqr - 7;


                    //move ahead
                    //add code to check edges
                    if(!(trg_sqr < a7) && !get_bit(occupancies[both], trg_sqr))
                    {
                        str += bit_idx_to_coord[src_sqr];
                        str += bit_idx_to_coord[trg_sqr];
                        moves.push_back(str);
                        str = "";
                    }

                    if(!(trg_sqr < a7) && !get_bit(occupancies[both], trg_sqr + 1) && ((src_sqr + 1) % 7 != 0))
                    {
                        str += bit_idx_to_coord[src_sqr];
                        str += bit_idx_to_coord[trg_sqr + 1];
                        moves.push_back(str);
                        str = "";
                    }
                    if(!(trg_sqr < a7) && !get_bit(occupancies[both], trg_sqr - 1) && (src_sqr % 7 != 0))
                    {
                        str += bit_idx_to_coord[src_sqr];
                        str += bit_idx_to_coord[trg_sqr - 1];
                        moves.push_back(str);
                        str = "";
                    }
                    //single pawn push backwards
                    if((src_sqr <= g5) && !get_bit(occupancies[both], trg_sqr + 14))
                    {
                        str += bit_idx_to_coord[src_sqr];
                        str += bit_idx_to_coord[trg_sqr + 14];
                        moves.push_back(str);
                        str = "";
                    }
                    //double pawn push backwards
                    if((src_sqr <= g5) && !get_bit(occupancies[both], trg_sqr + 21))
                    {
                        str += bit_idx_to_coord[src_sqr];
                        str += bit_idx_to_coord[trg_sqr + 21];
                        moves.push_back(str);
                        str = "";
                    }

                    atts = pawn_att[white][src_sqr] & occupancies[black];

                    while(atts)
                    {
                        trg_sqr = get_least_sig_idx(atts);

                        str += bit_idx_to_coord[src_sqr];
                        str += bit_idx_to_coord[trg_sqr];
                        moves.push_back(str);
                        str = "";

                        pop_bit(atts, trg_sqr);
                    }

                    pop_bit(bitboard, src_sqr);
                }
            }
        }

        if(side == black)
        {
            if(i == p)
            {
                while(bitboard)
                {
                    src_sqr = get_least_sig_idx(bitboard);

                    trg_sqr = src_sqr + 7;

                    //move ahead
                    //add code to check edges
                    if(!(trg_sqr > g1) && !get_bit(occupancies[both], trg_sqr))
                    {
                        str += bit_idx_to_coord[src_sqr];
                        str += bit_idx_to_coord[trg_sqr];
                        moves.push_back(str);
                        str = "";
                    }
                    if(!(trg_sqr > g1) && !get_bit(occupancies[both], trg_sqr - 1) && (src_sqr % 7 != 0))
                    {
                        str += bit_idx_to_coord[src_sqr];
                        str += bit_idx_to_coord[trg_sqr - 1];
                        moves.push_back(str);
                        str = "";
                    }
                    if(!(trg_sqr > g1) && !get_bit(occupancies[both], trg_sqr + 1) && ((src_sqr + 1) % 7 != 0))
                    {
                        str += bit_idx_to_coord[src_sqr];
                        str += bit_idx_to_coord[trg_sqr + 1];
                        moves.push_back(str);
                        str = "";
                    }
                    //single push back
                    if((src_sqr >= a3) && !get_bit(occupancies[both], trg_sqr - 14))
                    {
                        str += bit_idx_to_coord[src_sqr];
                        str += bit_idx_to_coord[trg_sqr - 14];
                        moves.push_back(str);
                        str = "";
                    }
                    //double pawn push backwards
                    if((src_sqr >= a3) && !get_bit(occupancies[both], trg_sqr - 21))
                    {
                        str += bit_idx_to_coord[src_sqr];
                        str += bit_idx_to_coord[trg_sqr - 21];
                        moves.push_back(str);
                        str = "";
                    }

                    atts = pawn_att[black][src_sqr] & occupancies[white];

                    while(atts)
                    {
                        trg_sqr = get_least_sig_idx(atts);

                        str += bit_idx_to_coord[src_sqr];
                        str += bit_idx_to_coord[trg_sqr];
                        moves.push_back(str);
                        str = "";

                        pop_bit(atts, trg_sqr);
                    }

                    pop_bit(bitboard, src_sqr);

                }
            }
        }
    }

        //non-colour specific pieces


}

void init_all()
{
    init_leaper_attacks();
    init_sliders_atts(diagonal);
    init_sliders_atts(straight);
    //init_magic_nums();
}

int main()
{
    init_all();

    int numFens = 0;
    std::string fen;
    std::string tmp;
    std::vector<std::string> fens;

    std::cin >> numFens;

    for(int i = 0; i < numFens; i++)
    {
        fen = "";
        std::cin >> tmp;
        fen += tmp;
        std::cin >> tmp;
        fen += " " + tmp;
        std::cin >> tmp;
        fen += " " + tmp;
        fens.push_back(fen);
    }

    for(int i = 0; i < fens.size(); i++)
    {
        char *fen_arr = new char[fens[i].length() + 1];

        strcpy(fen_arr, fens[i].c_str());
        decodeFEN(fen_arr);
        gen_moves();
        printVec();
        moves.clear();
    }

    //decodeFEN("6E/3pl1p/ezZ4/E6/2L4/3p3/7 b 45");
    //decodeFEN("3l3/p6/7/7/1Z1P1p1/3L3/1p5 b 42");

    //printBoard();

    //printAttSqrs(black);





    //printAttSqrs(black);
    //printAttSqrs(white);

    /*int numFEN = 0;
    string strFEN = "";
    string side = "";
    string moveNum = "";

    cin >> numFEN;

    for(int i = 0; i < numFEN; i++)
    {
        cin >> strFEN;
        cin >> side;
        cin >> moveNum;
        if(side == "w")
        {
            sideToPlay = "white";
        }
        else
        {
            sideToPlay = "black";
        }
    }*/



    /*U49 bitboard = 0ULL;

    for(int i = 0; i < 7; i++)
    {
        if(i == 3)
        {
            for(int j = 0; j < 7; j++)
            {
                    set_bit(bitboard,i * 7 + j);
            }
        }
        else{

        }

    }

    printBitBoard(bitboard);*/

//    for(int square = 0; square < 49; square++)
//        printBitBoard(eleph_att[square]);

    //initialise occupancy bitboard
/*    U64 block = 0ULL;

    set_bit(block, d7);
    set_bit(block, d2);
    set_bit(block, b4);
    set_bit(block, f4);

    printBitBoard(block);

    printf("index: %d   coordinate: %s\n", get_least_sig_idx(block), bit_idx_to_coord[get_least_sig_idx(block)]);*/

    //Set Occupancy Test
    /*U64 attack_mask = msk_str_att(a1);

    U64 occupancy = set_occupancy(10, count_bits(attack_mask), attack_mask);

    printBitBoard(occupancy);*/

    //generating pseudo random numbers
    /*printf("%ud\n", get_rand_num_u32());*/

    //printBitBoard((U49)get_rand_num_u32());
    //printBitBoard((U49)get_rand_num_u32() & 0xFFFF);

    //printBitBoard(gen_magic_numb());

//    for(int square = 0; square < 49; square++)
//    {
//        printf("0x%llxULL,\n", str_magic_num[square]);
//    }
//    printf("\n\n");
//    for(int square = 0; square < 49; square++)
//    {
//        printf("0x%llxULL,\n", diag_magic_num[square]);
//    }

/*    U49 occ = 0ULL;
    set_bit(occ, c5);
    set_bit(occ, f2);
    set_bit(occ, g7);
    set_bit(occ, b2);
    set_bit(occ, g5);
    set_bit(occ, e2);
    set_bit(occ, e7);

    printBitBoard(occ);

    printBitBoard(get_diag_attacks(d4, occ));

    printBitBoard(get_str_attacks(e5, occ));

    */

    /*decodeFEN(start_pos);

    printBoard();

    printBitBoard(occupancies[white]);
    printBitBoard(occupancies[black]);

    printBitBoard(occupancies[both]);

    decodeFEN("1e1El2/P1P2P1/1P5/7/1E3P1/1P5/4L2 b 79");

    printBoard();

    printBitBoard(occupancies[white]);
    printBitBoard(occupancies[black]);

    printBitBoard(occupancies[both]);

    */



    return 0;
}
