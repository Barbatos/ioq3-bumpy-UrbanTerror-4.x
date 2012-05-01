#ifndef TR_SHADER_H
#define TR_SHADER_H

#include "tr_local.h"

typedef struct {
	fileHandle_t   *file;
	char           *bot, *tok, *ptr, *cur, *pos, *lim, *top, *eof;
	int             lasttok;
	unsigned int    line;
	int             cond;
	int             lastcond;
} shaderScanner_t;

typedef struct shaderToken_s {
	union {
		int         ival;
		double      dval;
		struct {
			char *s;
			int   l;
		} sval;
		int         alias[2];
	} lex;
	struct shaderToken_s *next;
} shaderToken_t;

extern int R_ShaderScan( shaderScanner_t *s, shaderToken_t *t );

#endif
