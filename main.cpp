#include <iostream>
#include <stdio.h>

#define U64 unsigned long long

//bitboard macros
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define get_bit(bitboard, square) (bitboard & (1ULL << square))
#define pop_bit(bitboard, square) (get_bit(bitboard, square) ? bitboard ^= (1ULL << square) : 0)

//count bits in bitboard
static inline int count_bits(U64 bitboard)
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
static inline int get_least_sig_idx(U64 bitboard)
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

//enumerate board
enum{
    a7, b7, c7, d7, e7, f7, g7,
    a6, b6, c6, d6, e6, f6, g6,
    a5, b5, c5, d5, e5, f5, g5,
    a4, b4, c4, d4, e4, f4, g4,
    a3, b3, c3, d3, e3, f3, g3,
    a2, b2, c2, d2, e2, f2, g2,
    a1, b1, c1, d1, e1, f1, g1
};

//this constant allows the conversion from the square bit index to a readable coordinate
const char *bit_idx_to_coord[] = {
    "a7", "b7", "c7", "d7", "e7", "f7", "g7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1"
};

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
U64 get_rand_U64_num()
{
    U64 n1, n2, n3, n4;

    n1 = (U64)(get_rand_num_u32() & 0xFFFF);
    n2 = (U64)(get_rand_num_u32() & 0xFFFF);
    n3 = (U64)(get_rand_num_u32() & 0xFFFF);
    n4 = (U64)(get_rand_num_u32() & 0xFFFF);

    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

//generate magic number candidate
U64 gen_magic_numb()
{
    return get_rand_U64_num() & get_rand_U64_num() & get_rand_U64_num();
}

//colours
enum {white, black};

//print bitboard
void printBitBoard(U64 bitboard)
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
const U64 not_a_file = 558517276622718ULL;

// not H file const
const U64 not_g_file = 279258638311359ULL;

// not HG file const
const U64 not_fg_file = 137412980756383ULL;

// not AB file const
const U64 not_ab_file = 549651923025532ULL;

//not black lion castle
const U64 not_black_castle = 462364ULL;

//not white lion castle
const U64 not_white_castle = 124114891177984ULL;

//black half of the board split by the river(incls river)
const U64 black_territory = 268435455ULL;

//white half of the board split by the river(incls river)
const U64 white_territory = 562949951324160ULL;

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

//pawn attacks
U64 pawn_att[2][49];

//zebra attacks
U64 zebra_att[49];

//lion attacks
U64 lion_att[2][49];

//elephant attacks
U64 eleph_att[49];

//generate pawn attacks
//TODO: ADD COLLISION DETECTION
U64 msk_pawn_att(int side, int square)
{
    // result attacks bitboard
    U64 attacks = 0ULL;

    // piece bitboard
    U64 bitboard = 0ULL;

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
            if(bitboard << 7) attacks |= (bitboard << 7);
            if(bitboard << 14) attacks |= (bitboard << 14);
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
            if(bitboard >> 7) attacks |= (bitboard >> 7);
            if(bitboard >> 14) attacks |= (bitboard >> 14);
        }

    }



    //return attack bitboard
    return attacks;
}

//generate zebra attacks
U64 msk_zebra_att(int square)
{
    // result attacks bitboard
    U64 attacks = 0ULL;

    // piece bitboard
    U64 bitboard = 0ULL;

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
U64 msk_lion_att(int side, int square)
{
    // result attacks bitboard
    U64 attacks = 0ULL;

    // piece bitboard
    U64 bitboard = 0ULL;

    // set pieces
    set_bit(bitboard, square);

    // white lion
    // lion offset shifts: 6,7,8, 1 & 1
    if(!side)
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
    else // black lion
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
U64 msk_eleph_att(int square)
{
    // result attacks bitboard
    U64 attacks = 0ULL;

    // piece bitboard
    U64 bitboard = 0ULL;

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
U64 msk_diag_att(int square)
{
    // result attacks bitboard
    U64 attacks = 0ULL;

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
U64 msk_str_att(int square)
{
    // result attacks bitboard
    U64 attacks = 0ULL;

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
U64 diag_att_otf(int square, U64 block)
{
    // result attacks bitboard
    U64 attacks = 0ULL;

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
U64 str_att_otf(int square, U64 block)
{
    // result attacks bitboard
    U64 attacks = 0ULL;

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
U64 set_occupancy(int index, int bits_msk, U64 att_msk)
{
    U64 occupancy = 0ULL;

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

int main()
{
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

    init_leaper_attacks();

/*    U64 bitboard = 0ULL;

    for(int i = 0; i < 7; i++)
    {
        if(i < 3)
        {

        }
        else{
            for(int j = 0; j < 7; j++)
            {
                    set_bit(bitboard,i * 7 + j);
            }
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

    printBitBoard((U64)get_rand_num_u32());
    printBitBoard((U64)get_rand_num_u32() & 0xFFFF);

    printBitBoard(gen_magic_numb());
    return 0;
}
