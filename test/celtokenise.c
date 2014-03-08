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
#include	<stdio.h>
#include	<wchar.h>

#include	"celcore/tokens.h"

int
main(argc, argv)
	char	**argv;
{
FILE	*inf;
char	 line[1024];
size_t	 buflen = 0;
wchar_t	*buf = NULL, *bufp;

cel_token_t	tok;
int		skip;

	if (!argv[1]) {
		fprintf(stderr, "usage: %s <file>\n", argv[0]);
		return 1;
	}

	if ((inf = fopen(argv[1], "r")) == NULL) {
		perror(argv[1]);
		return 1;
	}

	while (fgets(line, sizeof(line), inf)) {
	wchar_t	wline[1024];
		mbstowcs(wline, line, sizeof(wline) / sizeof(wchar_t) - 1);

		bufp = wline;
		while ((skip = cel_next_token(bufp, &tok)) != T_ERR) {
			bufp += skip;

			if (tok.ct_token == T_EOT)
				break;

			fwprintf(stderr, L"<%ls> ", tok.ct_literal);
		}

		if (skip == T_ERR) {
			fprintf(stderr, "\n%s: parse error at ...%ls\n", argv[1], bufp);
			return 1;
		}

		printf("\n");
	}

	fclose(inf);
	return 1;
}
