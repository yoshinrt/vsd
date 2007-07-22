// $Id$

#ifndef LED_PATN
	#define LED_PATN LED_PAT
#endif

#ifndef LED_PATS
	#define LED_PATS( pat )
#endif

LED_PAT( 0xFF, SP )		// SP
LED_PATS( 0x29 )		// !
LED_PATS( 0xBB )		// "
LED_PATS( 0x39 )		// #
LED_PATS( 0x49 )		// $
LED_PATS( 0x4B )		// %
LED_PATS( 0xD5 )		// &
LED_PATS( 0xFB )		// '
LED_PATS( 0x63 )		// (
LED_PATS( 0x0F )		// )
LED_PATS( 0x93 )		// *
LED_PATS( 0x9D )		// +
LED_PATS( 0xF7 )		// ,
LED_PATS( 0xFD )		// -
LED_PATS( 0xF7 )		// .
LED_PATS( 0xF5 )		// /
LED_PATN( 0x03, 0 )		// 0
LED_PATN( 0x9F, 1 )		// 1
LED_PATN( 0x25, 2 )		// 2
LED_PATN( 0x0D, 3 )		// 3
LED_PATN( 0x99, 4 )		// 4
LED_PATN( 0x49, 5 )		// 5
LED_PATN( 0x41, 6 )		// 6
LED_PATN( 0x1F, 7 )		// 7
LED_PATN( 0x01, 8 )		// 8
LED_PATN( 0x09, 9 )		// 9
LED_PATS( 0xF3 )		// :
LED_PATS( 0xB7 )		// ;
LED_PATS( 0xE7 )		// <
LED_PATS( 0xED )		// =
LED_PATS( 0xCF )		// >
LED_PATS( 0x0B )		// ?
LED_PATS( 0x05 )		// @
LED_PAT( 0x11, A )		// A
LED_PAT( 0xC1, B )		// B
LED_PAT( 0x63, C )		// C
LED_PAT( 0x85, D )		// D
LED_PAT( 0x61, E )		// E
LED_PAT( 0x71, F )		// F
LED_PAT( 0x43, G )		// G
LED_PAT( 0x91, H )		// H
LED_PAT( 0x9F, I )		// I
LED_PAT( 0x87, J )		// J
LED_PAT( 0x51, K )		// K
LED_PAT( 0xE3, L )		// L
LED_PAT( 0x13, M )		// M
LED_PAT( 0xB5, N )		// N
LED_PAT( FONT_0, O )	// O
LED_PAT( 0x31, P )		// P
LED_PAT( 0x19, Q )		// Q
LED_PAT( 0x33, R )		// R
LED_PAT( 0x49, S )		// S
LED_PAT( 0x73, T )		// T
LED_PAT( 0x83, U )		// U
LED_PAT( 0xB1, V )		// V
LED_PAT( 0x81, W )		// W
LED_PAT( 0x93, X )		// X
LED_PAT( 0x89, Y )		// Y
LED_PAT( 0x6D, Z )		// Z
LED_PATS( 0x63 )		// [
LED_PATS( 0xDD )		// бя
LED_PATS( 0x0F )		// ]
LED_PATS( 0x7F )		// ^
LED_PATS( 0xEF )		// _
LED_PATS( 0xBF )		// `
LED_PAT( 0x05, a )		// a
LED_PAT( FONT_B, b )	// b
LED_PAT( 0xE5, c )		// c
LED_PAT( FONT_D, d )	// d
LED_PAT( FONT_E, e )	// e
LED_PAT( FONT_F, f )	// f
LED_PAT( FONT_G, g )	// g
LED_PAT( 0xD1, h )		// h
LED_PAT( 0xDF, i )		// i
LED_PAT( FONT_J, j )	// j
LED_PAT( FONT_K, k )	// k
LED_PAT( 0x9F, l )		// l
LED_PAT( FONT_M, m )	// m
LED_PAT( 0xD5, n )		// n
LED_PAT( 0xC5, o )		// o
LED_PAT( FONT_P, p )	// p
LED_PAT( FONT_Q, q )	// q
LED_PAT( 0xF5, r )		// r
LED_PAT( 0xCD, s )		// s
LED_PAT( 0xE1, t )		// t
LED_PAT( 0xC7, u )		// u
LED_PAT( 0xE7, v )		// v
LED_PAT( FONT_W, w )	// w
LED_PAT( 0xD7, x )		// x
LED_PAT( 0x89, y )		// y
LED_PAT( 0xED, z )		// z
LED_PATS( 0x63 )		// {
LED_PATS( 0xF3 )		// |
LED_PATS( 0x0F )		// }
LED_PATS( 0x7F )		// ~
LED_PATS( 0x00 )		// 0x7F

/*
LED_PAT( 0xEF, _1 )
LED_PAT( 0xC7, _2 )
LED_PAT( 0xC5, _3 )
LED_PAT( 0x81, _4 )
LED_PAT( 0x01, _5 )
*/
LED_PAT( 0xEF, _1 )
LED_PAT( 0xED, _2 )
LED_PAT( 0x6D, _3 )
LED_PAT( 0x6D, _4 )
LED_PAT( 0x6D, _5 )

#define LED_REV( d ) ( \
	((( d >> 7 ) & 1 ) << 4 ) | \
	((( d >> 6 ) & 1 ) << 5 ) | \
	((( d >> 5 ) & 1 ) << 6 ) | \
	((( d >> 4 ) & 1 ) << 7 ) | \
	((( d >> 3 ) & 1 ) << 2 ) | \
	((( d >> 2 ) & 1 ) << 3 ) | \
	((( d >> 1 ) & 1 ) << 1 ) | \
	((( d >> 0 ) & 1 ) << 0 )   \
)

#undef LED_PAT
#undef LED_PATN
#undef LED_PATS
