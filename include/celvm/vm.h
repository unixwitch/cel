/*
 * CEL: The Compact Embeddable Language.
 * Copyright (c) 2014 Felicity Tarnell.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely. This software is provided 'as-is', without any express or implied
 * warranty.
 */

#ifndef	CEL_VM_H
#define	CEL_VM_H

#include	<inttypes.h>

#include	"celcore/expr.h"
#include	"celcore/scope.h"

typedef struct cel_vm_func {
	uint8_t		*vf_bytecode;
	size_t		 vf_bytecodesz;

	cel_expr_t	**vf_vars;
	size_t		  vf_nvars;
} cel_vm_func_t;

cel_vm_func_t	*cel_vm_func_compile(cel_scope_t *, cel_expr_list_t *);
cel_expr_t	*cel_vm_func_execute(cel_scope_t *, cel_vm_func_t*);
void		 cel_vm_func_free(cel_scope_t *, cel_expr_t *);

#endif	/* CEL_VM_H */
