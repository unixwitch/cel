/*
 * CEL: The Compact Embeddable Language.
 * Copyright (c) 2014 Felicity Tarnell.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely. This software is provided 'as-is', without any express or implied
 * warranty.
 */

#ifndef	CEL_TOKENS_H
#define	CEL_TOKENS_H

#define	T_ERR		(-1)
#define	T_FUNC		1
#define	T_VAR		2
#define	T_BEGIN		3
#define	T_INT		4
#define	T_STRING	5
#define	T_END		6
#define	T_LPAR		7	/* (  */
#define	T_RPAR		8	/* )  */
#define	T_LSQ		9	/* [  */
#define	T_RSQ		10	/* ]  */
#define	T_LCUR		11	/* {  */
#define	T_RCUR		12	/* }  */
#define	T_LIT_INT	13
#define	T_LIT_STR	14
#define	T_LIT_BOOL	15
#define	T_COMMA		16
#define	T_COLON		17
#define	T_SEMI		18	/* ;  */
#define	T_ASSIGN	19	/* := */
#define	T_EQ		20	/* =  */
#define	T_LT		21	/* <  */
#define	T_GT		22	/* >  */
#define	T_LE		23	/* <= */
#define	T_GE		24	/* >= */
#define	T_NE		25	/* /= */
#define	T_EOT		26	/* end of file */
#define	T_ID		27	/* identifier */
#define	T_PLUS		28	/* +  */
#define	T_MINUS		29	/* -  */
#define	T_STAR		30	/* *  */
#define	T_SLASH		31	/* /  */
#define	T_ARROW		32	/* -> */

typedef struct cel_token {
	int	 ct_token;
	wchar_t	*ct_literal;
} cel_token_t;

int cel_next_token(wchar_t const *, cel_token_t *ret);

#endif	/* !CEL_TOKENS_H */
