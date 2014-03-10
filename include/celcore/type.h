/*
 * CEL: The Compact Embeddable Language.
 * Copyright (c) 2014 Felicity Tarnell.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely. This software is provided 'as-is', without any express or implied
 * warranty.
 */

#ifndef	CEL_TYPE_H
#define	CEL_TYPE_H

#include	<wchar.h>

typedef enum cel_type_tag {
	cel_type_int,
	cel_type_bool,
	cel_type_string,
	cel_type_array,
	cel_type_function,
} cel_type_tag_t;

typedef struct cel_type {
	cel_type_tag_t	ct_tag;

	union {
		struct cel_type	*ct_array_type;
	} ct_type;
} cel_type_t;

void		 cel_type_free(cel_type_t *);

cel_type_t	*cel_make_type(cel_type_tag_t);
cel_type_t	*cel_make_array(cel_type_t *);

typedef struct cel_typedef {
	cel_type_t	*ct_type;
	wchar_t const	*ct_name;
} cel_typedef_t;

cel_typedef_t	*cel_make_typedef(wchar_t const *name, cel_type_t *type);
#endif	/* !CEL_TYPE_H */