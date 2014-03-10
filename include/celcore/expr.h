/*
 * CEL: The Compact Embeddable Language.
 * Copyright (c) 2014 Felicity Tarnell.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely. This software is provided 'as-is', without any express or implied
 * warranty.
 */

#ifndef	CEL_EXPR_H
#define	CEL_EXPR_H

#include	<wchar.h>
#include	<stdlib.h>

#include	"celcore/tailq.h"

struct cel_function;

typedef enum cel_expr_tag {
	cel_exp_int,
	cel_exp_string,
	cel_exp_identifier,
	cel_exp_bool,
	cel_exp_unary,
	cel_exp_binary,
	cel_exp_function,
	cel_exp_vardecl,
	cel_exp_if
} cel_expr_tag_t;

typedef enum cel_bi_oper {
	cel_op_mult,
	cel_op_div,
	cel_op_plus,
	cel_op_minus,
	cel_op_lt,
	cel_op_le,
	cel_op_eq,
	cel_op_neq,
	cel_op_gt,
	cel_op_ge,
	cel_op_assign,
	cel_op_or,
	cel_op_and,
	cel_op_xor,
	cel_op_modulus
} cel_bi_oper_t;

typedef enum cel_uni_oper {
	cel_op_negate,
	cel_op_uni_minus
} cel_uni_oper_t;

struct cel_expr;

typedef struct cel_if_branch {
	struct cel_expr	*ib_condition;
	struct cel_expr	*ib_expr;
} cel_if_branch_t;

typedef struct cel_expr {
	cel_expr_tag_t	 ce_tag;

	union {
		int	 ce_int;
		int	 ce_bool;
		wchar_t	*ce_string;
		wchar_t	*ce_identifier;

		struct {
			cel_bi_oper_t	 oper;
			struct cel_expr	*left, *right;
		} ce_binary;

		struct {
			cel_uni_oper_t	 oper;
			struct cel_expr	*operand;
		} ce_unary;

		struct {
			cel_if_branch_t	*branches;
			int		 nbranches;
		} ce_if;

		struct cel_function *ce_function;
		struct cel_vardecl  *ce_vardecl;
	} ce_op;

	CEL_TAILQ_ENTRY(cel_expr) ce_entry;
} cel_expr_t;

typedef CEL_TAILQ_HEAD(cel_expr_list, cel_expr) cel_expr_list_t;

typedef struct cel_arglist {
	int		 ca_nargs;
	cel_expr_t	**ca_args;
} cel_arglist_t;

void		 cel_expr_free(cel_expr_t *);

cel_expr_t	*cel_make_int(int);
cel_expr_t	*cel_make_bool(int);
cel_expr_t	*cel_make_string(wchar_t const *);
cel_expr_t	*cel_make_identifier(wchar_t const *);

cel_expr_t	*cel_make_unary(cel_uni_oper_t, cel_expr_t *);
#define	cel_make_negate(e)	cel_make_unary(cel_op_negate, (e))
#define	cel_make_uni_minus(e)	cel_make_unary(cel_op_uni_minus, (e))

cel_expr_t	*cel_make_binary(cel_bi_oper_t, cel_expr_t *, cel_expr_t *);
#define	cel_make_plus(e,f)	cel_make_binary(cel_op_plus, (e), (f))
#define	cel_make_minus(e,f)	cel_make_binary(cel_op_minus, (e), (f))
#define	cel_make_mult(e,f)	cel_make_binary(cel_op_mult, (e), (f))
#define	cel_make_div(e,f)	cel_make_binary(cel_op_div, (e), (f))
#define	cel_make_modulus(e,f)	cel_make_binary(cel_op_modulus, (e), (f))
#define	cel_make_le(e,f)	cel_make_binary(cel_op_le, (e), (f))
#define	cel_make_lt(e,f)	cel_make_binary(cel_op_lt, (e), (f))
#define	cel_make_eq(e,f)	cel_make_binary(cel_op_eq, (e), (f))
#define	cel_make_neq(e,f)	cel_make_binary(cel_op_neq, (e), (f))
#define	cel_make_ge(e,f)	cel_make_binary(cel_op_ge, (e), (f))
#define	cel_make_gt(e,f)	cel_make_binary(cel_op_gt, (e), (f))
#define	cel_make_and(e,f)	cel_make_binary(cel_op_and, (e), (f))
#define	cel_make_or(e,f)	cel_make_binary(cel_op_or, (e), (f))
#define	cel_make_assign(e,f)	cel_make_binary(cel_op_assign, (e), (f))
#define	cel_make_xor(e,f)	cel_make_binary(cel_op_xor, (e), (f))

cel_expr_t	*cel_make_function(struct cel_function *);

#endif	/* !CEL_EXPR_H */