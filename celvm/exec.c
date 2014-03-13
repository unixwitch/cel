/*
 * CEL: The Compact Embeddable Language.
 * Copyright (c) 2014 Felicity Tarnell.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely. This software is provided 'as-is', without any express or implied
 * warranty.
 */

#include	"celcore/expr.h"

#include	"celvm/vm.h"
#include	"celvm/instr.h"

#define	STACKSZ 16

#define	GET_UINT16(p)	((uint16_t) (p)[0] <<  8 |	\
			 (uint16_t) (p)[1])

#define	GET_UINT32(p)	((uint32_t) (p)[0] << 24 |	\
			 (uint32_t) (p)[1] << 16 |	\
			 (uint32_t) (p)[2] <<  8 |	\
			 (uint32_t) (p)[3])

#define	GET_UINT64(p)	((uint64_t) (p)[0] << 56 |	\
			 (uint64_t) (p)[1] << 48 |	\
			 (uint64_t) (p)[2] << 40 |	\
			 (uint64_t) (p)[3] << 32 |	\
			 (uint64_t) (p)[4] << 24 |	\
			 (uint64_t) (p)[5] << 16 |	\
			 (uint64_t) (p)[6] <<  8 |	\
			 (uint64_t) (p)[7])

#define	GET_II16(v)	do { (v) = GET_UINT16(ip); ip += 2; } while (0)
#define	GET_IU16(v)	do { (v) = GET_UINT16(ip); ip += 2; } while (0)
#define	GET_IU32(v)	do { (v) = GET_UINT32(ip); ip += 4; } while (0)
#define	GET_IU64(v)	do { (v) = GET_UINT64(ip); ip += 8; } while (0)
#define	GET_SU32(v)	((v) = stack[--sp].u32)
#define	GET_SI32(v)	GET_SU32(v)
#define	GET_SU64(v)	do { sp -= 2; (v) = (((uint64_t) stack[sp].u32) << 32) | stack[sp + 1].u32; } while (0)
#define PUT_SU32(v)	(stack[sp++].u32 = (v))
#define	PUT_SI32(v)	PUT_SU32(v)
#define PUT_SU64(v)	do { PUT_SU32((v) >> 32); PUT_SU32((v) & 0xFFFFFFFF); } while (0)
#define	GET_SI64(v)	GET_SU64(v)
#define	PUT_SI64(v)	PUT_SU64((uint64_t) (v))

typedef union any {
	uint32_t	u32;
	int32_t		i32;
} any_t;

cel_expr_t *
cel_vm_func_execute(s, f)
	cel_scope_t	*s;
	cel_vm_func_t	*f;
{
any_t		 stack[STACKSZ];
int		 sp;
uint8_t	const	*ip, *oip;

	ip = f->vf_bytecode;
	sp = 0;

	for (;;) {
	int64_t		s16a;
	uint32_t	u32a, u32b;
	uint64_t	u64a, u64b, i64a;
	int16_t		i16;
	uint8_t		inst;

		if (ip > (f->vf_bytecode + f->vf_bytecodesz)) {
			printf("ip overflow\n");
			return NULL;
		}

		oip = ip;
		inst = *ip++;
		switch (inst) {
		case CEL_I_RET4:
			if (sp == 0)
				return NULL;
			GET_SU32(u32a);
			return cel_make_int32(u32a);

		case CEL_I_RET8:
			if (sp <= 1)
				return NULL;
			GET_SU64(u64a);
			return cel_make_int64(u64a);

		case CEL_I_LOADI4:
			GET_IU32(u32a);
			PUT_SU32(u32a);
			break;

		case CEL_I_LOADI8:
			GET_IU64(u64a);
			PUT_SU64(u64a);
			break;

		case CEL_I_ADD4:
			GET_SU32(u32a);
			stack[sp - 1].u32 += u32a;
			break;

		case CEL_I_ADD8:
			GET_SU64(u64b);
			GET_SU64(u64a);
			u64a += u64b;
			PUT_SU64(u64a);
			break;

		case CEL_I_SUB4:
			GET_SU32(u32a);
			stack[sp - 1].u32 -= u32a;
			break;

		case CEL_I_SUB8:
			GET_SU64(u64b);
			GET_SU64(u64a);
			u64a -= u64b;
			PUT_SU64(u64a);
			break;

		case CEL_I_MUL4:
			GET_SU32(u32a);
			stack[sp - 1].u32 *= u32a;
			break;

		case CEL_I_MUL8:
			GET_SU64(u64b);
			GET_SU64(u64a);
			u64a *= u64b;
			PUT_SU64(u64a);
			break;

		case CEL_I_DIV4:
			GET_SU32(u32a);
			stack[sp - 1].u32 /= u32a;
			break;

		case CEL_I_DIV8:
			GET_SU64(u64b);
			GET_SU64(u64a);
			u64a /= u64b;
			PUT_SU64(u64a);
			break;

		case CEL_I_NEG4:
			stack[sp - 1].i32 = -stack[sp - 1].i32;
			break;

		case CEL_I_NEG8:
			GET_SI64(u64a);
			u64a = -u64a;
			PUT_SI64(u64a);
			break;

		case CEL_I_NOT4:
			stack[sp - 1].u32 = !stack[sp - 1].u32;
			break;

		case CEL_I_NOT8:
			GET_SU64(i64a);
			i64a = !i64a;
			PUT_SU64(i64a);
			break;
	
#define	CMP(in, sz, op)						\
		case in:					\
			GET_SI##sz(u##sz##b);			\
			GET_SI##sz(u##sz##a);			\
			PUT_SI##sz(u##sz##a op u##sz##b);	\
			break;

		CMP(CEL_I_TEQ4, 32, ==)
		CMP(CEL_I_TNE4, 32, !=)
		CMP(CEL_I_TLE4, 32, <=)
		CMP(CEL_I_TLT4, 32, <)
		CMP(CEL_I_TGT4, 32, >)
		CMP(CEL_I_TGE4, 32, >=)

		CMP(CEL_I_TEQ8, 64, ==)
		CMP(CEL_I_TNE8, 64, !=)
		CMP(CEL_I_TLT8, 64, <=)
		CMP(CEL_I_TLE8, 64, <)
		CMP(CEL_I_TGT8, 64, >)
		CMP(CEL_I_TGE8, 64, >=)

		case CEL_I_BR:
			GET_II16(i16);
			ip = oip + i16;
			break;

		case CEL_I_BRT:
			GET_II16(i16);
			GET_SU32(u32a);
			if (u32a)
				ip = oip + i16;
			break;

		case CEL_I_BRF:
			GET_II16(i16);
			GET_SU32(u32a);
			if (!u32a)
				ip = oip + i16;
			break;

		case CEL_I_INCV4:
			GET_II16(i16);
			GET_SU32(u32a);
			*f->vf_vars[i16]->ce_op.ce_uint32 += u32a;
			break;

		case CEL_I_DECV4:
			GET_II16(i16);
			GET_SU32(u32a);
			*f->vf_vars[i16]->ce_op.ce_uint32 -= u32a;
			break;

		case CEL_I_MULV4:
			GET_II16(i16);
			GET_SU32(u32a);
			*f->vf_vars[i16]->ce_op.ce_uint32 *= u32a;
			break;

		case CEL_I_DIVV4:
			GET_II16(i16);
			GET_SU32(u32a);
			*f->vf_vars[i16]->ce_op.ce_uint32 /= u32a;
			break;

		case CEL_I_LOADV4:
			GET_II16(i16);
			PUT_SU32(*f->vf_vars[i16]->ce_op.ce_uint32);
			break;

		case CEL_I_STOV4:
			GET_II16(i16);
			GET_SU32(u32a);
			*f->vf_vars[i16]->ce_op.ce_uint32 = u32a;
			break;

		default:
			printf("can't decode %d\n", inst);
			return NULL;
		}
	}
}
