/*
 * CEL: The Compact Embeddable Language.
 * Copyright (c) 2014 Felicity Tarnell.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely. This software is provided 'as-is', without any express or implied
 * warranty.
 */

#include	<stdlib.h>
#include	<string.h>

#include	"celcore/expr.h"
#include	"celcore/type.h"

#include	"celvm/vm.h"
#include	"celvm/instr.h"

static void	cel_vm_add_bytecode(cel_vm_func_t *, uint8_t *, size_t);
static void	cel_vm_emit_expr(cel_vm_func_t *, cel_expr_t *);
static void	cel_vm_emit_instr(cel_vm_func_t *, int);
static void	cel_vm_emit_binary(cel_vm_func_t *, cel_expr_t *);
static void	cel_vm_emit_unary(cel_vm_func_t *, cel_expr_t *);
static void	cel_vm_emit_literal(cel_vm_func_t *, cel_expr_t *);
static void	cel_vm_emit_return(cel_vm_func_t *, cel_expr_t *);
static uint32_t	cel_vm_expr_to_u32(cel_expr_t *e);
static uint64_t cel_vm_expr_to_u64(cel_expr_t *e);

cel_vm_func_t *
cel_vm_func_compile(el)
	cel_expr_list_t	*el;
{
cel_expr_t	*e;
cel_vm_func_t	*ret;

	if ((ret = calloc(1, sizeof(*ret))) == NULL)
		return NULL;

	CEL_TAILQ_FOREACH(e, el, ce_entry) {
		cel_vm_emit_expr(ret, e);
	}

	return ret;
}

static void
cel_vm_emit_expr(f, e)
	cel_vm_func_t	*f;
	cel_expr_t	*e;
{
	switch (e->ce_tag) {
	case cel_exp_return:
		cel_vm_emit_return(f, e);
		break;

	case cel_exp_binary:
		cel_vm_emit_binary(f, e);
		break;

	case cel_exp_unary:
		cel_vm_emit_unary(f, e);
		break;

	case cel_exp_literal:
		cel_vm_emit_literal(f, e);
		break;

	default:
		printf("can't compile expr type %d\n", e->ce_tag);
		return;
	}
}

static void
cel_vm_emit_unary(f, e)
	cel_vm_func_t	*f;
	cel_expr_t	*e;
{
int	is64 = 0;

	if (e->ce_op.ce_unary.operand->ce_type->ct_tag == cel_type_int64)
		is64 = 1;

	cel_vm_emit_expr(f, e->ce_op.ce_unary.operand);

	switch (e->ce_op.ce_unary.oper) {
	case cel_op_uni_minus:
		cel_vm_emit_instr(f, is64 ? CEL_I_NEG8 : CEL_I_NEG4);
		break;

	case cel_op_negate:
		cel_vm_emit_instr(f, is64 ? CEL_I_NOT8 : CEL_I_NOT4);
		break;

	default:
		printf("can't emit unary for oper %d\n", e->ce_op.ce_unary.oper);
		abort();
	}
}

static void
cel_vm_emit_binary(f, e)
	cel_vm_func_t	*f;
	cel_expr_t	*e;
{
int	is64 = 0;

	if (e->ce_op.ce_binary.left->ce_type->ct_tag == cel_type_int64 ||
	    e->ce_op.ce_binary.left->ce_type->ct_tag == cel_type_uint64)
		is64 = 1;

	cel_vm_emit_expr(f, e->ce_op.ce_binary.left);
	cel_vm_emit_expr(f, e->ce_op.ce_binary.right);

	switch (e->ce_op.ce_binary.oper) {
	case cel_op_plus:
		cel_vm_emit_instr(f, is64 ? CEL_I_ADD8 : CEL_I_ADD4);
		break;

	case cel_op_minus:
		cel_vm_emit_instr(f, is64 ? CEL_I_SUB8 : CEL_I_SUB4);
		break;

	case cel_op_mult:
		cel_vm_emit_instr(f, is64 ? CEL_I_MUL8 : CEL_I_MUL4);
		break;

	case cel_op_div:
		cel_vm_emit_instr(f, is64 ? CEL_I_DIV8 : CEL_I_DIV4);
		break;

	default:
		printf("can't emit binary type %d\n", e->ce_op.ce_binary.oper);
		abort();
	}
}

static void
cel_vm_add_bytecode(f, b, sz)
	cel_vm_func_t	*f;
	uint8_t		*b;
	size_t		 sz;
{
size_t	osz = f->vf_bytecodesz;
	f->vf_bytecode = realloc(f->vf_bytecode, f->vf_bytecodesz + sz);
	memcpy(f->vf_bytecode + osz, b, sz);
	f->vf_bytecodesz += sz;
}

static void
cel_vm_emit_instr(f, i)
	cel_vm_func_t	*f;
{
uint8_t	bc[] = { i };
	cel_vm_add_bytecode(f, bc, 1);
}

static void
cel_vm_emit_immed32(f, i)
	cel_vm_func_t	*f;
	uint32_t	 i;
{
uint8_t	bc[] = {
		(i >> 24) & 0xff,
		(i >> 16) & 0xff,
		(i >>  8) & 0xff,
		(i      ) & 0xff
};
	cel_vm_add_bytecode(f, bc, sizeof(bc));
}

static void
cel_vm_emit_immed64(f, i)
	cel_vm_func_t	*f;
	uint64_t	 i;
{
uint8_t	bc[] = {
		(i >> 56) & 0xff,
		(i >> 48) & 0xff,
		(i >> 40) & 0xff,
		(i >> 32) & 0xff,
		(i >> 24) & 0xff,
		(i >> 16) & 0xff,
		(i >>  8) & 0xff,
		(i      ) & 0xff
};
	cel_vm_add_bytecode(f, bc, sizeof(bc));
}

static void
cel_vm_emit_loadi32(f, i)
	cel_vm_func_t	*f;
	uint32_t	 i;
{
	cel_vm_emit_instr(f, CEL_I_LOADI4);
	cel_vm_emit_immed32(f, i);
}

static void
cel_vm_emit_loadi64(f, i)
	cel_vm_func_t	*f;
	uint64_t	 i;
{
	cel_vm_emit_instr(f, CEL_I_LOADI8);
	cel_vm_emit_immed64(f, i);
}

static uint32_t
cel_vm_expr_to_u32(e)
	cel_expr_t	*e;
{
	switch (e->ce_type->ct_tag) {
	case cel_type_int8:	return e->ce_op.ce_int8; break;
	case cel_type_uint8:	return e->ce_op.ce_uint8; break;
	case cel_type_int16:	return e->ce_op.ce_int16; break;
	case cel_type_uint16:	return e->ce_op.ce_uint16; break;
	case cel_type_int32:	return e->ce_op.ce_int32; break;
	case cel_type_uint32:	return e->ce_op.ce_uint32; break;
		break;

	default:
		printf("can't convert tag %d to u32\n", e->ce_type->ct_tag);
		abort();
	}
}

static uint64_t
cel_vm_expr_to_u64(e)
	cel_expr_t	*e;
{
	switch (e->ce_type->ct_tag) {
	case cel_type_int64:	return e->ce_op.ce_int64;
	case cel_type_uint64:	return e->ce_op.ce_uint64;
	default:
		printf("can't convert tag %d to u64\n", e->ce_type->ct_tag);
		abort();
	}
}

static void
cel_vm_emit_literal(f, e)
	cel_vm_func_t	*f;
	cel_expr_t	*e;
{
	switch (e->ce_type->ct_tag) {
	case cel_type_int8:	cel_vm_emit_loadi32(f, e->ce_op.ce_int8); break;
	case cel_type_uint8:	cel_vm_emit_loadi32(f, e->ce_op.ce_uint8); break;
	case cel_type_int16:	cel_vm_emit_loadi32(f, e->ce_op.ce_int16); break;
	case cel_type_uint16:	cel_vm_emit_loadi32(f, e->ce_op.ce_uint16); break;
	case cel_type_int32:	cel_vm_emit_loadi32(f, e->ce_op.ce_int32); break;
	case cel_type_uint32:	cel_vm_emit_loadi32(f, e->ce_op.ce_uint32); break;
	case cel_type_int64:	cel_vm_emit_loadi64(f, e->ce_op.ce_int64); break;
	case cel_type_uint64:	cel_vm_emit_loadi64(f, e->ce_op.ce_uint64); break;
	case cel_type_bool:	cel_vm_emit_loadi32(f, e->ce_op.ce_bool); break;
	default:
		printf("can't emit literal for tag %d\n", e->ce_tag);
		abort();
	}
}

static void
cel_vm_emit_return(f, e)
	cel_vm_func_t	*f;
	cel_expr_t	*e;
{
	cel_vm_emit_expr(f, e->ce_op.ce_unary.operand);

	switch (e->ce_op.ce_unary.operand->ce_type->ct_tag) {
	case cel_type_int8:
	case cel_type_uint8:
	case cel_type_int16:
	case cel_type_uint16:
	case cel_type_int32:
	case cel_type_uint32:
	case cel_type_bool:
		cel_vm_emit_instr(f, CEL_I_RET4);
		break;

	case cel_type_int64:
	case cel_type_uint64:
		cel_vm_emit_instr(f, CEL_I_RET8);
		break;

	default:
		printf("can't emit return for tag %d\n", e->ce_op.ce_unary.operand->ce_type->ct_tag);
		abort();
	}
}
