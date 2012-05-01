%include {
#include "tr_shader.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
}

%token_type {shaderToken_t *}

%parse_accept {
	printf("Parsing complete!\n");
}

%parse_failure {
	fprintf(stderr, "Parse failure\n");
}

%left SEMICOLON COMMA COLON PERIOD.
%left LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET.
%left AND OR XOR COMPLEMENT NOT.
%left QUESTION.
%left PLUS MINUS TIMES DIVIDE.
%left PERCENT.

%left LSHIFT RSHIFT.
%left ANDAND OROR.
%left GT LT.
%left LEQ GEQ EQL NEQ.

%left EOL EOI.

%left Q3MAP_SUN Q3MAP_FANCYWATER Q3MAP_CHEAPWATER Q3MAP_NOFANCYWATER.
%left DEFORM NORMAL MOVE BULGE PROJSHADOW AUTOSPRITE AUTOSPRITE2 TEXT.
%left TESSSIZE.
%left CLAMPTIME.
%left SURFACEPARM.
%left NOMIPMAPS NOPICMIP.
%left POLYGONOFFSET.
%left ENTITYMERGABLE.
%left FOGPARMS SKYPARMS.
%left LIGHT.
%left WAVE PORTAL.

%left CULL CULLTYPE.
%left SORT SORTTYPE.

%left MAP CLAMPMAP ANIMMAP VIDEOMAP.
%left WHITEIMAGE LIGHTMAP.

%left ALPHAFUNC ALPHAMODE.
%left BLENDFUNC BLENDMODE.
%left DEPTHFUNC DEPTHMODE.

%left CGEN CGENTYPE.
%left AGEN AGENTYPE.
%left TCGEN TCGENTYPE.
%left TCMOD.
%left TURB SCALE SCROLL STRETCH TRANSFORM ROTATE ENTITYTRANSLATE.
%left CONST VECTOR.
%left GENFUNC.

%left DETAIL.
%left DEPTHWRITE.

%left INTEGER FLOAT STRING PATH.

%left ERROR.

%start_symbol shader_file

shader_file ::= shader_list.

shader_list ::= shader_list shader.
shader_list ::= shader.

// %type shader {shader_t *}
// %type stage {shaderStage_t *}

shader(S)        ::= PATH LBRACE shader_parts(SP) RBRACE.

shader_parts(SP) ::= shader_parts(SP1) shader_part(P) EOL.
shader_parts(SP) ::= shader_part(P) EOL.

shader_part(P)   ::= Q3MAP_SUN FLOAT(R) FLOAT(G) FLOAT(B) FLOAT(I) FLOAT(D) FLOAT(E).
shader_part(P)   ::= Q3MAP_FANCYWATER FLOAT(F1) FLOAT(F2) FLOAT(F3) FLOAT(F4).
shader_part(P)   ::= Q3MAP_CHEAPWATER FLOAT(F1) FLOAT(F2) FLOAT(F3) FLOAT(F4).
shader_part(P)   ::= Q3MAP_NOFANCYWATER.

shader_part(P)   ::= DEFORM WAVE waveform(W).
shader_part(P)   ::= DEFORM NORMAL FLOAT(D) GENFUNC(GF) FLOAT(B) FLOAT(A) FLOAT(F).
shader_part(P)   ::= DEFORM BULGE FLOAT(W) FLOAT(H) FLOAT(S).
shader_part(P)   ::= DEFORM MOVE FLOAT(X) FLOAT(Y) FLOAT(Z) GENFUNC(GF) FLOAT(B) FLOAT(A) FLOAT(P) FLOAT(F).
shader_part(P)   ::= DEFORM AUTOSPRITE.
shader_part(P)   ::= DEFORM AUTOSPRITE2.
shader_part(P)   ::= DEFORM TEXT(T).

waveform(W)      ::= FLOAT(D) GENFUNC(GF) FLOAT(B) FLOAT(A) FLOAT(P) FLOAT(F).

shader_part(P)   ::= TESSSIZE FLOAT(S).
shader_part(P)   ::= SURFACEPARM IDENTIFIER(I).
shader_part(P)   ::= NOPICMIPS.
shader_part(P)   ::= NOMIPMAPS.
shader_part(P)   ::= POLYGONOFFSET.
shader_part(P)   ::= ENTITYMERGABLE.
shader_part(P)   ::= FOGPARMS FLOAT(R) FLOAT(G) FLOAT(B) FLOAT(D).
shader_part(P)   ::= SKYPARMS box_path(FB) FLOAT(CH) box_path(NB).
shader_part(P)   ::= LIGHT FLOAT(L).
shader_part(P)   ::= CULL CULLTYPE(CT).
shader_part(P)   ::= PORTAL.
shader_part(P)   ::= SORT SORTTYPE(ST).
shader_part(P)   ::= SORT FLOAT(V).

shader_part(P)   ::= LBRACE stage_parts(T) RBRACE.

stage_parts(T)   ::= stage_parts(T1) stage_part(P) EOL.
stage_part(P)    ::= MAP map_path(MP).
stage_part(P)    ::= CLAMPMAP map_path(MP).
stage_part(P)    ::= ANIMMAP FLOAT(F) animmap_path(AMP).
stage_part(P)    ::= VIDEOMAP map_path(MP).

stage_part(P)    ::= ALPHAFUNC ALPHAMODE(AM).
stage_part(P)    ::= BLENDFUNC DBLENDMODE(BM).
stage_part(P)    ::= BLENDFUNC SBLENDMODE(BM).
stage_part(P)    ::= DEPTHFUNC DEPTHMODE(DM).

stage_part(P)    ::= CGEN waveform(W).
stage_part(P)    ::= CGEN CONST vector(v).
stage_part(P)    ::= CGEN CGENTYPE.

stage_part(P)    ::= AGEN waveform(W).
stage_part(P)    ::= AGEN CONST vector(V).
stage_part(P)    ::= AGEN PORTAL FLOAT(P).
stage_part(P)    ::= AGEN CGENTYPE.

stage_part(P)    ::= TCGEN VECTOR vector(V1) vector(V2).
stage_part(P)    ::= TCGEN TCGENTYPE.

stage_part(P)    ::= TCMOD TURB FLOAT(B) FLOAT(W) FLOAT(P) FLOAT(F).
stage_part(P)    ::= TCMOD SCALE FLOAT(S1) FLOAT(S2).
stage_part(P)    ::= TCMOD SCROLL FLOAT(S1) FLOAT(S2).
stage_part(P)    ::= TCMOD STETCH GENFUNC(GF) FLOAT(B) FLOAT(A) FLOAT(P) FLOAT(F).
stage_part(P)    ::= TCMOD TRANSFORM FLOAT(M00) FLOAT(M01) FLOAT(M10) FLOAT(M11) FLOAT(T1) FLOAT(T2).
stage_part(P)    ::= TCMOD ROTATE FLOAT(S).
stage_part(P)    ::= TCMOD ENTITYTRANSLATE.
stage_part(P)    ::= DETAIL.
stage_part(P)    ::= DEPTHWRITE.

vector(V)        ::= LPAREN FLOAT(F1) FLOAT(F2) FLOAT(F3) RPAREN.

box_path(BP)     ::= PATH(P).
box_path(BP)     ::= MINUS.

map_path(MP)     ::= PATH(P).
map_path(MP)     ::= WHITEIMAGE.
map_path(MP)     ::= LIGHTMAP.

animmap_path(AMP) ::= animmap_path(AMP1) map_path(MP).


