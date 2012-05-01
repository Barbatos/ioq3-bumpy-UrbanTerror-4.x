
#include "tr_local.h"
#include "tr_shader.h"
#include "tr_parse.h"

#define	BSIZE	8192

#define	YYCTYPE		  unsigned char
#define	YYCURSOR	  cursor
#define	YYLIMIT		  s->lim
#define	YYMARKER	  s->ptr
#define	YYFILL(n)	  {cursor = R_ScanFill(s, cursor);}
#define YYCONDTYPE        scanCondition_t
#define YYGETCONDITION()  s->cond
#define YYSETCONDITION(c) {s->lastcond = s->cond; s->cond = c;}

#define	RET(i)          {                             \
				s->lasttok = i; \
				s->cur = cursor;      \
				return i;       \
                        }

/*!types:re2c*/

unsigned char *R_ScanFill(shaderScanner_t *s, char *cursor)
{
	if(!s->eof) {
		uint cnt = s->tok - s->bot;
		if(cnt){
			memcpy(s->bot, s->tok, s->lim - s->tok);
			s->tok = s->bot;
			s->ptr -= cnt;
			cursor -= cnt;
			s->pos -= cnt;
			s->lim -= cnt;
		}
		if((s->top - s->lim) < BSIZE){
			char *buf = (char*) malloc(((s->lim - s->bot) + BSIZE)*sizeof(char));
			memcpy(buf, s->tok, s->lim - s->tok);
			s->tok = buf;
			s->ptr = &buf[s->ptr - s->bot];
			cursor = &buf[cursor - s->bot];
			s->pos = &buf[s->pos - s->bot];
			s->lim = &buf[s->lim - s->bot];
			s->top = &s->lim[BSIZE];
			free(s->bot);
			s->bot = buf;
		}
		if((cnt = FS_Read((char*) s->lim, BSIZE, s->file)) != BSIZE){
			s->eof = &s->lim[cnt]; *(s->eof)++ = '\n';
		}
		s->lim += cnt;
	}
	return cursor;
}

int R_ShaderScan( shaderScanner_t *s, shaderToken_t *t )
{
	char *cursor = s->cur;
std:
	s->tok = cursor;
/*!re2c
O	= [0-7];
D	= [0-9];
L	= [a-zA-Z_$];
PL	= (L|D|[-.]);
PS	= [/\\];
P	= PL (PL|PS)+;
H	= [a-fA-F0-9];
E	= [Ee] [+-]? D+;
W	= [ \t\v\f\r];
NL	= "\r"? "\n";
ID	= L (L|D)*;
FS	= [fFlL];
IS	= [uUlL]*;
ESC	= [\\] ([abfnrtv?'"\\] | "x" H+ | O+);
*/
/*!re2c

<NORMAL,SHADER,STAGE>	"/" "*" :=> COMMENT
<NORMAL,SHADER,STAGE>	"//" :=> SKIPTOEOL
<NORMAL,SHADER,STAGE>	W			{ goto std; }
<NORMAL,SHADER,STAGE>	"\n"
			{
				if( cursor == s->eof ) RET(EOI);
				s->line++;
				if( s->lasttok == EOL ||
				    s->lasttok == RBRACE ||
				    s->lasttok == LBRACE )
				    goto std;
				RET(EOL);
			}
<SHADER,STAGE>		";"			{ RET(SEMICOLON); }
<SHADER,STAGE>		"."			{ RET(PERIOD); }
<SHADER,STAGE>		","			{ RET(COMMA); }
<SHADER,STAGE>		":"			{ RET(COLON); }
<SHADER,STAGE>		"("			{ RET(LPAREN); }
<SHADER,STAGE>		")"			{ RET(RPAREN); }
<SHADER,STAGE>		"["			{ RET(LBRACKET); }
<SHADER,STAGE>		"]"			{ RET(RBRACKET); }
<SHADER,STAGE>		"&"			{ RET(AND); }
<SHADER,STAGE>		"&"			{ RET(OR); }
<SHADER,STAGE>		"^"			{ RET(XOR); }
<SHADER,STAGE>		"!"			{ RET(NOT); }
<SHADER,STAGE>		"~"			{ RET(COMPLEMENT); }
<SHADER,STAGE>		"?"			{ RET(QUESTION); }
<SHADER,STAGE>		"+"			{ RET(PLUS); }
<SHADER,STAGE>		"-"			{ RET(MINUS); }
<SHADER,STAGE>		"*"			{ RET(TIMES); }
<SHADER,STAGE>		"/"			{ RET(DIVIDE); }
<SHADER,STAGE>		"%"			{ RET(PERCENT); }
<SHADER,STAGE>		"<<"			{ RET(LSHIFT); }
<SHADER,STAGE>		">>"			{ RET(RSHIFT); }
<SHADER,STAGE>		"&&"			{ RET(ANDAND); }
<SHADER,STAGE>		"||"			{ RET(OROR); }
<SHADER,STAGE>		"<"			{ RET(LT); }
<SHADER,STAGE>		">"			{ RET(GT); }
<SHADER,STAGE>		"<="			{ RET(LEQ); }
<SHADER,STAGE>		">="			{ RET(GEQ); }
<SHADER,STAGE>		"=="			{ RET(EQL); }
<SHADER,STAGE>		"!="			{ RET(NEQ); }

<NORMAL>	"{" => SHADER			{ RET(LBRACE); }

<COMMENT>	"*" "/"
			{
				s->cond = s->lastcond;
				goto std;
			}
<COMMENT>	NL				{ s->line++; goto yyc_COMMENT; }
<COMMENT>	[^]				{ goto yyc_COMMENT; }

<SKIPTOEOL>	"??/" NL			{ s->line++; goto yyc_SKIPTOEOL; }
<SKIPTOEOL>	"\\" NL				{ s->line++; goto yyc_SKIPTOEOL; }
<SKIPTOEOL>	NL				
			{
				if( cursor == s->eof ) RET(EOI);
				s->line++;
				s->cond = s->lastcond;
				goto std;
			}
<SKIPTOEOL>	[^]				{ goto yyc_SKIPTOEOL; }

<SHADER>	"}" => NORMAL			{ RET(RBRACE); }
<SHADER>	"{" => STAGE			{ RET(LBRACE); }

<SHADER>	'q3map_sun'			{ RET(Q3MAP_SUN); }
<SHADER>	'q3map_fancywater'		{ RET(Q3MAP_FANCYWATER); }
<SHADER>	'q3map_cheapwater'		{ RET(Q3MAP_CHEAPWATER); }
<SHADER>	'q3map_nofancywater'		{ RET(Q3MAP_NOFANCYWATER); }
<SHADER>	'q3map' L* :=> SKIPTOEOL

<SHADER>	'qer' L* :=> SKIPTOEOL

<SHADER>	'deformvertexes'		{ RET(DEFORM); }
<SHADER>	'wave'
			{
				t->lex.alias[0] = DEFORM_WAVE;
				RET(WAVE);
			}
<SHADER>	'normal'
			{
				t->lex.alias[0] = DEFORM_NORMALS;
				RET(NORMAL);
			}
<SHADER>	'move'
			{
				t->lex.alias[0] = DEFORM_BULGE;
				RET(MOVE);
			}
<SHADER>	'bulge'
			{
				t->lex.alias[0] = DEFORM_MOVE;
				RET(BULGE);
			}
<SHADER>	'projectionshadow'
			{
				t->lex.alias[0] = DEFORM_PROJECTION_SHADOW;
				RET(PROJSHADOW);
			}
<SHADER>	'autosprite'
			{
				t->lex.alias[0] = DEFORM_AUTOSPRITE;
				RET(AUTOSPRITE);
			}
<SHADER>	'autosprite2'
			{
				t->lex.alias[0] = DEFORM_AUTOSPRITE2;
				RET(AUTOSPRITE2);
			}
<SHADER>	'text'[0-7]
			{
				t->lex.alias[0] = DEFORM_TEXT0 + s->tok[4] - '0';
				RET(TEXT);
			}

<SHADER>	'tesssize'			{ RET(TESSSIZE); }
<SHADER>	'clamptime'			{ RET(CLAMPTIME); }
<SHADER>	'surfaceparm'			{ RET(SURFACEPARM); }
<SHADER>	'nomipmaps'			{ RET(NOMIPMAPS); }
<SHADER>	'nopicmip'			{ RET(NOPICMIP); }
<SHADER>	'polygonoffset'			{ RET(POLYGONOFFSET); }
<SHADER>	'entitymergable'		{ RET(ENTITYMERGABLE); }
<SHADER>	'fogparms'			{ RET(FOGPARMS); }
<SHADER>	'skyparms'			{ RET(SKYPARMS); }
<SHADER>	'light'				{ RET(LIGHT); }

<SHADER>	'cull'				{ RET(CULL); }
<SHADER>	'none'|'twosided'|'disable'
			{
				t->lex.alias[0] = CT_TWO_SIDED;
				RET(CULLTYPE);
			}
<SHADER>	'back'|'backside'|'backsided'
			{
				t->lex.alias[0] = CT_BACK_SIDED;
				RET(CULLTYPE);
			}

<SHADER>	'sort'				{ RET(SORT); }

<SHADER>	'portal'
			{
				t->lex.alias[0] = SS_PORTAL;
				RET(SORTTYPE);
			}
<SHADER>	'sky'
			{
				t->lex.alias[0] = SS_ENVIRONMENT;
				RET(SORTTYPE);
			}
<SHADER>	'opaque'
			{
				t->lex.alias[0] = SS_OPAQUE;
				RET(SORTTYPE);
			}
<SHADER>	'decal'
			{
				t->lex.alias[0] = SS_DECAL;
				RET(SORTTYPE);
			}
<SHADER>	'seethrough'
			{
				t->lex.alias[0] = SS_SEE_THROUGH;
				RET(SORTTYPE);
			}
<SHADER>	'banner'
			{
				t->lex.alias[0] = SS_BANNER;
				RET(SORTTYPE);
			}
<SHADER>	'additive'
			{
				t->lex.alias[0] = SS_BLEND1;
				RET(SORTTYPE);
			}
<SHADER>	'nearest'
			{
				t->lex.alias[0] = SS_NEAREST;
				RET(SORTTYPE);
			}
<SHADER>	'underwater'
			{
				t->lex.alias[0] = SS_UNDERWATER;
				RET(SORTTYPE);
			}

<STAGE>		"}" => SHADER			{ RET(RBRACE); }

<STAGE>		'map'				{ RET(MAP); }
<STAGE>		'clampmap'			{ RET(CLAMPMAP); }
<STAGE>		'animmap'			{ RET(ANIMMAP); }
<STAGE>		'videomap'			{ RET(VIDEOMAP); }
<STAGE>		'$whiteimage'			{ RET(WHITEIMAGE); }
<STAGE>		'$lightmap'			{ RET(LIGHTMAP); }

<STAGE>		'alphafunc'			{ RET(ALPHAFUNC); }
<STAGE>		'gt0'
			{
				t->lex.alias[0] = GLS_ATEST_GT_0;
				RET(ALPHAMODE);
			}
<STAGE>		'lt128'
			{
				t->lex.alias[0] = GLS_ATEST_LT_80;
				RET(ALPHAMODE);
			}
<STAGE>		'ge128'
			{
				t->lex.alias[0] = GLS_ATEST_GE_80;
				RET(ALPHAMODE);
			}
<STAGE>		'blendfunc'			{ RET(BLENDFUNC); }
<STAGE>		'add'
			{
				t->lex.alias[0] = GLS_SRCBLEND_ONE;
				t->lex.alias[1] = GLS_DSTBLEND_ONE;
				RET(DBLENDMODE);
			}
<STAGE>		'filter'
			{
				t->lex.alias[0] = GLS_SRCBLEND_DST_COLOR;
				t->lex.alias[1] = GLS_DSTBLEND_ZERO;
				RET(DBLENDMODE);
			}
<STAGE>		'blend'
			{
				t->lex.alias[0] = GLS_SRCBLEND_SRC_ALPHA;
				t->lex.alias[1] = GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA;
				RET(DBLENDMODE);
			}
<STAGE>		'gl_one'			
			{
				t->lex.alias[0] = GL_ONE;
				RET(SBLENDMODE);
			}
<STAGE>		'gl_zero'			
			{
				t->lex.alias[0] = GL_ZERO;
				RET(SBLENDMODE);
			}
<STAGE>		'gl_dst_color'			
			{
				t->lex.alias[0] = GL_DST_COLOR;
				RET(SBLENDMODE);
			}
<STAGE>		'gl_one_minus_dst_color'	
			{
				t->lex.alias[0] = GL_ONE_MINUS_DST_COLOR;
				RET(SBLENDMODE);
			}
<STAGE>		'gl_src_alpha'			
			{
				t->lex.alias[0] = GL_SRC_ALPHA;
				RET(SBLENDMODE);
			}
<STAGE>		'gl_one_minus_src_alpha'	
			{
				t->lex.alias[0] = GL_ONE_MINUS_SRC_ALPHA;
				RET(SBLENDMODE);
			}
<STAGE>		'gl_dst_alpha'			
			{
				t->lex.alias[0] = GL_DST_ALPHA;
				RET(SBLENDMODE);
			}
<STAGE>		'gl_one_minus_dst_alpha'	
			{
				t->lex.alias[0] = GL_ONE_MINUS_DST_ALPHA;
				RET(SBLENDMODE);
			}
<STAGE>		'gl_src_alpha_saturate'		
			{
				t->lex.alias[0] = GL_SRC_ALPHA_SATURATE;
				RET(SBLENDMODE);
			}
<STAGE>		'gl_src_color'			
			{
				t->lex.alias[0] = GL_SRC_COLOR;
				RET(SBLENDMODE);
			}
<STAGE>		'gl_one_minus_src_color'	
			{
				t->lex.alias[0] = GL_ONE_MINUS_SRC_COLOR;
				RET(SBLENDMODE);
			}

<STAGE>		'depthfunc'			{ RET(DEPTHFUNC); }
<STAGE>		'lequal'			
			{
				t->lex.alias[0] = 0;
				RET(DEPTHMODE);
			}

<STAGE>		'equal'			
			{
				t->lex.alias[0] = GLS_DEPTHFUNC_EQUAL;
				RET(DEPTHMODE);
			}

<STAGE>		'rgbgen'			{ RET(CGEN); }
<STAGE>		'alphagen'			{ RET(AGEN); }
<STAGE>		'wave'
		{
			if( s->lasttok == AGEN )
			{
				t->lex.alias[0] = AGEN_WAVEFORM;
			}
			else
			{
				t->lex.alias[0] = CGEN_WAVEFORM;
			}
			RET(WAVE);
		}
<STAGE>		'const'			
		{
			if( s->lasttok == AGEN )
			{
				t->lex.alias[0] = AGEN_CONST;
				RET(AGENTYPE);
			}
			else
			{
				t->lex.alias[0] = CGEN_CONST;
				RET(CGENTYPE);
			}
		}
<STAGE>		'identity'
		{
			if( s->lasttok == AGEN )
			{
				t->lex.alias[0] = AGEN_IDENTITY;
				RET(AGENTYPE);
			}
			else
			{
				t->lex.alias[0] = CGEN_IDENTITY;
				RET(CGENTYPE);
			}
		}
<STAGE>		'identitylighting'
		{
			//if ( !r_overBrightBits->integer )
			//	t->lex.alias[0] = CGEN_IDENTITY;
			//else
				t->lex.alias[0] = CGEN_IDENTITY_LIGHTING;
			RET(CGENTYPE);
		}
<STAGE>		'entity'
		{
			t->lex.alias[0] = CGEN_ENTITY;
			RET(CGENTYPE);
		}
<STAGE>		'entity'
		{

			if( s->lasttok == AGEN )
			{
				t->lex.alias[0] = AGEN_ENTITY;
				RET(AGENTYPE);
			}
			else
			{
				t->lex.alias[0] = CGEN_ENTITY;
				RET(CGENTYPE);
			}
		}
<STAGE>		'oneminusentity'
		{
			if( s->lasttok == AGEN )
			{

				t->lex.alias[0] = AGEN_ONE_MINUS_ENTITY;
				RET(AGENTYPE);
			}
			else
			{
				t->lex.alias[0] = CGEN_ONE_MINUS_ENTITY;
				RET(CGENTYPE);
			}
		}
<STAGE>		'vertex'
		{
			if( s->lasttok == AGEN )
			{

				t->lex.alias[0] = AGEN_VERTEX;
				RET(AGENTYPE);
			}
			else
			{
				t->lex.alias[0] = CGEN_VERTEX;
				RET(CGENTYPE);
			}
		}
<STAGE>		'exactvertex'
		{
			t->lex.alias[0] = CGEN_EXACT_VERTEX;
			RET(CGENTYPE);
		}
<STAGE>		'lightingdiffuse'
		{
			t->lex.alias[0] = CGEN_LIGHTING_DIFFUSE;
			RET(CGENTYPE);
		}
<STAGE>		'oneminusvertex'
		{

			if( s->lasttok == AGEN )
			{
				t->lex.alias[0] = AGEN_ONE_MINUS_VERTEX;
				RET(AGENTYPE);
			}
			else
			{
				t->lex.alias[0] = CGEN_ONE_MINUS_VERTEX;
				RET(CGENTYPE);
			}
		}
<STAGE>		'lightingspecular'
		{
			t->lex.alias[0] = AGEN_LIGHTING_SPECULAR;
			RET(AGENTYPE);
		}
<STAGE>		'portal'
		{
			t->lex.alias[0] = AGEN_PORTAL;
			RET(PORTAL);
		}

<SHADER,STAGE>	'sin'
			{
				t->lex.alias[0] = GF_SIN;
				RET(GENFUNC);
			}
<SHADER,STAGE>	'square'
			{
				t->lex.alias[0] = GF_SQUARE;
				RET(GENFUNC);
			}
<SHADER,STAGE>	'triangle'
			{
				t->lex.alias[0] = GF_TRIANGLE;
				RET(GENFUNC);
			}
<SHADER,STAGE>	'sawtooth'
			{
				t->lex.alias[0] = GF_SAWTOOTH;
				RET(GENFUNC);
			}
<SHADER,STAGE>	'inversesawtooth'
			{
				t->lex.alias[0] = GF_INVERSE_SAWTOOTH;
				RET(GENFUNC);
			}
<SHADER,STAGE>	'noise'
			{
				t->lex.alias[0] = GF_NOISE;
				RET(GENFUNC);
			}

<STAGE>		'detail'			{ RET(DETAIL); }

<STAGE>		'tcgen'				{ RET(TCGEN); }
<STAGE>		'texgen'			{ RET(TCGEN); }
<STAGE>		'environment'
			{
				t->lex.alias[0] = TCGEN_ENVIRONMENT_MAPPED;
				RET(TCGENTYPE);
			}
<STAGE>		'lightmap'
			{
				t->lex.alias[0] = TCGEN_LIGHTMAP;
				RET(TCGENTYPE);
			}
<STAGE>		'texture'|'base'
			{
				t->lex.alias[0] = TCGEN_TEXTURE;
				RET(TCGENTYPE);
			}
<STAGE>		'vector'
			{
				t->lex.alias[0] = TCGEN_VECTOR;
				RET(VECTOR);
			}

<STAGE>		'tcmod'				{ RET(TCMOD); }
<STAGE>		'turb'
			{
				t->lex.alias[0] = TMOD_TURBULENT;
				RET(TURB);
			}
<STAGE>		'scale'
			{
				t->lex.alias[0] = TMOD_SCALE;
				RET(SCALE);
			}
<STAGE>		'scroll'
			{
				t->lex.alias[0] = TMOD_SCROLL;
				RET(SCROLL);
			}
<STAGE>		'stretch'
			{
				t->lex.alias[0] = TMOD_STRETCH;
				RET(STRETCH);
			}
<STAGE>		'transform'
			{
				t->lex.alias[0] = TMOD_TRANSFORM;
				RET(TRANSFORM);
			}
<STAGE>		'rotate'
			{
				t->lex.alias[0] = TMOD_ROTATE;
				RET(ROTATE);
			}
<STAGE>		'entitytranslate'
			{
				t->lex.alias[0] = TMOD_ENTITY_TRANSLATE;
				RET(ENTITYTRANSLATE);
			}

<STAGE>		'depthwrite'
			{
				t->lex.alias[0] = GLS_DEPTHMASK_TRUE;
				RET(DEPTHWRITE);
			}

<STAGE,SHADER>
		("0" [xX] H+ IS?) | 
		("0" D+ IS?) | (D+ IS?) |
		(['] (ESC|[^]\[\n\\'])* ['])	
			{
				t->lex.ival = strtol(s->tok, 0, 0);
				RET(INTEGER);
			}
<STAGE,SHADER>
		(D+ E FS?) | 
		(D* "." D+ E? FS?) | 
		(D+ "." D* E? FS?)
			{
				t->lex.dval = strtod(s->tok, 0);
				RET(FLOAT);
			}
<NORMAL,SHADER,STAGE>
		(["] (ESC|[^]\[\n\\"])* ["])
			{
				t->lex.sval.s = s->tok + 1;
				t->lex.sval.l = cursor - s->tok - 1;
				RET(STRING);
			}
<NORMAL,SHADER,STAGE>
		ID
			{
				t->lex.sval.s = s->tok;
				t->lex.sval.l = cursor - s->tok;
				RET(IDENTIFIER);
			}
<NORMAL,SHADER,STAGE>
		P
			{
				t->lex.sval.s = s->tok;
				t->lex.sval.l = cursor - s->tok;
				RET(PATH);
			}
<*>
		[^] => NORMAL			{ RET(ERROR); }
*/
}
