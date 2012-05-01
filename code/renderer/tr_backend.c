/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#include "tr_local.h"

backEndData_t	*backEndData[SMP_FRAMES];
backEndState_t	backEnd;

int	EndSurfaceCount=0;

static float	s_flipMatrix[16] = {
	// convert from our coordinate system (looking down X)
	// to OpenGL's coordinate system (looking down -Z)
	0, 0, -1, 0,
	-1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 0, 1
};


/*
** GL_Bind
*/
void GL_Bind( image_t *image ) {
	int texnum;
	image_t *img;

	if ( !image ) {
		ri.Printf( PRINT_WARNING, "GL_Bind: NULL image\n" );
		img = tr.defaultImage;
	} else {
		img = image;
	}
	texnum = img->texnum;

	if ( r_nobind->integer && tr.dlightImage ) {		// performance evaluation option
		texnum = tr.dlightImage->texnum;
	}

	if ( glState.currenttextures[glState.currenttmu] != texnum ) {
		img->frameUsed = tr.frameCount;
		glState.currenttextures[glState.currenttmu] = texnum;
		qglBindTexture (GL_TEXTURE_2D, texnum);
	}
}

void GL_BindCube( image_t *image ) 
{
	int texnum;

	if ( !image ) {
		ri.Printf( PRINT_WARNING, "GL_Bind: NULL image\n" );
		texnum = tr.defaultImage->texnum;
	} else {
		texnum = image->texnum;
	}

	if ( r_nobind->integer && tr.dlightImage ) {		// performance evaluation option
		texnum = tr.dlightImage->texnum;
	}

	if ( glState.currenttextures[glState.currenttmu] != texnum ) {

		image->frameUsed = tr.frameCount;
		glState.currenttextures[glState.currenttmu] = texnum;
		qglBindTexture (GL_TEXTURE_CUBE_MAP, texnum);
	}
}


void GL_BindCubeTexnum( int texnum ) 
{

	if ( glState.currenttextures[glState.currenttmu] != texnum ) 
	{
		glState.currenttextures[glState.currenttmu] = texnum;
		qglBindTexture (GL_TEXTURE_CUBE_MAP, texnum);
	}
}


void GL_BindTexnum( int texnum ) 
{

	if ( glState.currenttextures[glState.currenttmu] != texnum ) 
	{
		glState.currenttextures[glState.currenttmu] = texnum;
		qglBindTexture (GL_TEXTURE_2D, texnum);
	}
}


/*
** GL_SelectTexture
*/
void GL_SelectTexture( int unit )
{
	if ( glState.currenttmu == unit )
	{
		return;
	}

	if ( unit == 0 )
	{
		qglActiveTextureARB( GL_TEXTURE0_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE0_ARB )\n" );
		qglClientActiveTextureARB( GL_TEXTURE0_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE0_ARB )\n" );
	}
	else if ( unit == 1 )
	{
		qglActiveTextureARB( GL_TEXTURE1_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE1_ARB )\n" );
		qglClientActiveTextureARB( GL_TEXTURE1_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE1_ARB )\n" );
	}
	else if ( unit == 2 )
	{
		qglActiveTextureARB( GL_TEXTURE2_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE2_ARB )\n" );
		qglClientActiveTextureARB( GL_TEXTURE2_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE2_ARB )\n" );
	} 
	else if ( unit == 3 )
	{
		qglActiveTextureARB( GL_TEXTURE3_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE3_ARB )\n" );
		qglClientActiveTextureARB( GL_TEXTURE3_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE3_ARB )\n" );
	} 
	else if ( unit == 4 )
	{
		qglActiveTextureARB( GL_TEXTURE4_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE4_ARB )\n" );
		qglClientActiveTextureARB( GL_TEXTURE4_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE4_ARB )\n" );
	} 
	else if ( unit == 5 )
	{
		qglActiveTextureARB( GL_TEXTURE5_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE5_ARB )\n" );
		qglClientActiveTextureARB( GL_TEXTURE5_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE5_ARB )\n" );
	} 
	else if ( unit == 6 )
	{
		qglActiveTextureARB( GL_TEXTURE6_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE6_ARB )\n" );
		qglClientActiveTextureARB( GL_TEXTURE6_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE6_ARB )\n" );
	} 
	else if ( unit == 7 )
	{
		qglActiveTextureARB( GL_TEXTURE7_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE7_ARB )\n" );
		qglClientActiveTextureARB( GL_TEXTURE7_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE7_ARB )\n" );
	} 
	else if ( unit == 8 )
	{
		qglActiveTextureARB( GL_TEXTURE8_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE8_ARB )\n" );
		qglClientActiveTextureARB( GL_TEXTURE8_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE8_ARB )\n" );
	} 
	else if ( unit == 9 )
	{
		qglActiveTextureARB( GL_TEXTURE9_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE9_ARB )\n" );
		qglClientActiveTextureARB( GL_TEXTURE9_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE9_ARB )\n" );
	} 
	else if ( unit == 10)
	{
		qglActiveTextureARB( GL_TEXTURE10_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE10_ARB )\n" );
		qglClientActiveTextureARB( GL_TEXTURE10_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE10_ARB )\n" );
	} 
	else if ( unit == 11 )
	{
		qglActiveTextureARB( GL_TEXTURE11_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE11_ARB )\n" );
		qglClientActiveTextureARB( GL_TEXTURE11_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE11_ARB )\n" );
	} 
	else if ( unit == 12 )
	{
		qglActiveTextureARB( GL_TEXTURE12_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE12_ARB )\n" );
		qglClientActiveTextureARB( GL_TEXTURE12_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE12_ARB )\n" );
	} 
	else {
		ri.Error( ERR_DROP, "GL_SelectTexture: unit = %i", unit );
	}

	glState.currenttmu = unit;
}


/*
** GL_BindMultitexture
*/
void GL_BindMultitexture( image_t *image0, GLuint env0, image_t *image1, GLuint env1 ) {
	int		texnum0, texnum1;

	texnum0 = image0->texnum;
	texnum1 = image1->texnum;

	if ( r_nobind->integer && tr.dlightImage ) {		// performance evaluation option
		texnum0 = texnum1 = tr.dlightImage->texnum;
	}

	if ( glState.currenttextures[1] != texnum1 ) {
		GL_SelectTexture( 1 );
		image1->frameUsed = tr.frameCount;
		glState.currenttextures[1] = texnum1;
		qglBindTexture( GL_TEXTURE_2D, texnum1 );
	}
	if ( glState.currenttextures[0] != texnum0 ) {
		GL_SelectTexture( 0 );
		image0->frameUsed = tr.frameCount;
		glState.currenttextures[0] = texnum0;
		qglBindTexture( GL_TEXTURE_2D, texnum0 );
	}
}



void RB_SetupFragment( fragShader_t* frag, shader_t* shader )
{
	int uniform_location;
	vec3_t forward;
	int j = 0;

	if (glState.currentFragShader==frag)
		return;
	
	// We're going back to standard rendering now
	qglUseProgramObjectARB( 0 );
	GL_UnBindVBO( );
	
	//turn off all texture units
	for (j=1;j<13;j++)
	{
		GL_SelectTexture(j);
		qglDisable( GL_TEXTURE_2D );
		qglDisable( GL_TEXTURE_CUBE_MAP );
		qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
	}
	GL_SelectTexture( 0 );	

	glState.currentFragShader=frag;

	if (frag==NULL ) return;

	glState.currentFragShader=frag;
	
	// ENABLE A FRAGSHADER
	GL_BindVBO(frag->vertexType);
	qglUseProgramObjectARB(frag->program);

	if (backEnd.viewParms.isMirror==qfalse)
	{
		uniform_location = qglGetUniformLocationARB(frag->program, "IN_eye_pos");
		qglUniform3fARB(uniform_location, backEnd.refdef.vieworg[0],backEnd.refdef.vieworg[1],backEnd.refdef.vieworg[2]);
		uniform_location = qglGetUniformLocationARB(frag->program, "IN_eye_vec");
		VectorCopy( backEnd.refdef.viewaxis[0], forward );
		qglUniform3fARB(uniform_location, forward[0],forward[1],forward[2]);
	}
	else
	{
		uniform_location = qglGetUniformLocationARB(frag->program, "IN_eye_pos");
		qglUniform3fARB(uniform_location, backEnd.refdef.vieworg[0],backEnd.refdef.vieworg[1],backEnd.refdef.vieworg[2]);
		uniform_location = qglGetUniformLocationARB(frag->program, "IN_eye_vec");
		VectorCopy( backEnd.or.axis[0], forward );
		qglUniform3fARB(uniform_location, forward[0],forward[1],forward[2]);
	}
	

	//set the time for the animation
	uniform_location = qglGetUniformLocationARB(frag->program, "time");
	if (uniform_location>-1) qglUniform1fARB(uniform_location, backEnd.refdef.floatTime);
	
	//turn all the deforms off
	uniform_location = qglGetUniformLocationARB(frag->program, "IN_deformtype");
	if (uniform_location>-1) qglUniform1fARB(uniform_location, 0);



	if (shader!=NULL)
	{
		//set the tiling
		uniform_location = qglGetUniformLocationARB(frag->program, "scale");
		if (uniform_location>-1) qglUniform1fARB(uniform_location,shader->scale );
	
		//set the refraction tint
		uniform_location = qglGetUniformLocationARB(frag->program, "undertint");
		if (uniform_location>-1) qglUniform3fARB(uniform_location,1,1,1);
			
		//overtint
		uniform_location = qglGetUniformLocationARB(frag->program, "tint");
		if (uniform_location>-1) qglUniform3fARB(uniform_location,shader->tint[0],shader->tint[1],shader->tint[2]);
	}


	if (backEnd.viewParms.isUnderwater)
	{
		//set the flip 		
		uniform_location = qglGetUniformLocationARB(frag->program, "flip");		
		if (uniform_location>-1) qglUniform1fARB(uniform_location,-1 );
	}
	else
	{
		uniform_location = qglGetUniformLocationARB(frag->program, "flip");		
		if (uniform_location>-1) qglUniform1fARB(uniform_location,1 );

	}

	
}

void GL_BindVBO(int VBOMode)
{
	int oldMode=glState.VBOBound;

	fragShader_t* frag=glState.currentFragShader;

	//Unbind the vbo
	if (glState.VBOBound!=VBOMode)
	{
		qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
		
		if (glState.VBOBound==1)
		{
			//disable static vertexes		
			qglDisableVertexAttribArrayARB(glState.currentFragShader->position_i);
			qglDisableVertexAttribArrayARB(glState.currentFragShader->uv_i);
			qglDisableVertexAttribArrayARB(glState.currentFragShader->light_uv_i);
			qglDisableVertexAttribArrayARB(glState.currentFragShader->normal_i);
			qglDisableVertexAttribArrayARB(glState.currentFragShader->stv_i);
			qglDisableVertexAttribArrayARB(glState.currentFragShader->ttv_i);
			qglDisableVertexAttribArrayARB(glState.currentFragShader->rgba_i);
		}

		glState.VBOBound=0;
		if (VBOMode==0) return;
	}

	//bind the new vbo
	glState.VBOBound=VBOMode;

	if (glState.VBOBound==1) //Static vbo mode
	{
		qglBindBufferARB(GL_ARRAY_BUFFER_ARB, tr.VBO_vertexBuffer);
		qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, tr.VBO_indexBuffer);

		qglVertexPointer(3,GL_FLOAT,sizeof(staticVert_t),GEOMETRY_VERTEX_OFFSET(0, staticVert_t, position ));
		qglTexCoordPointer(2,GL_FLOAT,sizeof(staticVert_t),GEOMETRY_VERTEX_OFFSET(0, staticVert_t, uv ));
		qglTexCoordPointer(2,GL_FLOAT,sizeof(staticVert_t),GEOMETRY_VERTEX_OFFSET(0, staticVert_t, light_uv ));
	
		qglEnableVertexAttribArrayARB(frag->position_i);
		qglEnableVertexAttribArrayARB(frag->uv_i);
		qglEnableVertexAttribArrayARB(frag->light_uv_i);
		qglEnableVertexAttribArrayARB(frag->normal_i);
		qglEnableVertexAttribArrayARB(frag->stv_i);
		qglEnableVertexAttribArrayARB(frag->ttv_i);
		qglEnableVertexAttribArrayARB(frag->rgba_i);

		qglVertexAttribPointerARB(frag->position_i, 3, GL_FLOAT, GL_FALSE, sizeof(staticVert_t), GEOMETRY_VERTEX_OFFSET(0, staticVert_t, position ));
		qglVertexAttribPointerARB(frag->uv_i, 2, GL_FLOAT, GL_FALSE, sizeof(staticVert_t), GEOMETRY_VERTEX_OFFSET(0, staticVert_t, uv ));		
		qglVertexAttribPointerARB(frag->light_uv_i, 2, GL_FLOAT, GL_FALSE, sizeof(staticVert_t), GEOMETRY_VERTEX_OFFSET(0, staticVert_t, light_uv ));
		qglVertexAttribPointerARB(frag->normal_i, 3, GL_FLOAT, GL_FALSE, sizeof(staticVert_t), GEOMETRY_VERTEX_OFFSET(0, staticVert_t, normal ));
		qglVertexAttribPointerARB(frag->stv_i, 3, GL_FLOAT, GL_FALSE, sizeof(staticVert_t), GEOMETRY_VERTEX_OFFSET(0, staticVert_t, stv ));
		qglVertexAttribPointerARB(frag->ttv_i, 3, GL_FLOAT, GL_FALSE, sizeof(staticVert_t), GEOMETRY_VERTEX_OFFSET(0, staticVert_t, ttv ));
		qglVertexAttribPointerARB(frag->rgba_i, 4, GL_FLOAT, GL_FALSE, sizeof(staticVert_t), GEOMETRY_VERTEX_OFFSET(0, staticVert_t, rgba ));

		qglEnableClientState(GL_VERTEX_ARRAY);
		qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
		qglDisableClientState(GL_COLOR_ARRAY);
	}
	// other model formats go here, like hardware accelerated md5...
}

void GL_UnBindVBO()
{
	GL_BindVBO(0);
}




/*
** GL_Cull
*/
void GL_Cull( int cullType ) {
	if ( glState.faceCulling == cullType ) {
		return;
	}

	glState.faceCulling = cullType;

	if ( cullType == CT_TWO_SIDED ) 
	{
		qglDisable( GL_CULL_FACE );
	} 
	else 
	{
		qglEnable( GL_CULL_FACE );

		if ( cullType == CT_BACK_SIDED )
		{
			if ( backEnd.viewParms.isMirror )
			{
				qglCullFace( GL_FRONT );
			}
			else
			{
				qglCullFace( GL_BACK );
			}
		}
		else
		{
			if ( backEnd.viewParms.isMirror )
			{
				qglCullFace( GL_BACK );
			}
			else
			{
				qglCullFace( GL_FRONT );
			}
		}
	}
}

/*
** GL_TexEnv
*/
void GL_TexEnv( int env )
{
	if ( env == glState.texEnv[glState.currenttmu] )
	{
		return;
	}

	glState.texEnv[glState.currenttmu] = env;


	switch ( env )
	{
	case GL_MODULATE:
		qglTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		break;
	case GL_REPLACE:
		qglTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
		break;
	case GL_DECAL:
		qglTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
		break;
	case GL_ADD:
		qglTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD );
		break;
	default:
		ri.Error( ERR_DROP, "GL_TexEnv: invalid env '%d' passed\n", env );
		break;
	}
}

/*
** GL_State
**
** This routine is responsible for setting the most commonly changed state
** in Q3.
*/
void GL_State( unsigned long stateBits )
{
	unsigned long diff = stateBits ^ glState.glStateBits;

	if ( !diff )
	{
		return;
	}

	//
	// check depthFunc bits
	//
	if ( diff & GLS_DEPTHFUNC_EQUAL )
	{
		if ( stateBits & GLS_DEPTHFUNC_EQUAL )
		{
			qglDepthFunc( GL_EQUAL );
		}
		else
		{
			qglDepthFunc( GL_LEQUAL );
		}
	}

	//
	// check blend bits
	//
	if ( diff & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) )
	{
		GLenum srcFactor, dstFactor;

		if ( stateBits & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) )
		{
			switch ( stateBits & GLS_SRCBLEND_BITS )
			{
			case GLS_SRCBLEND_ZERO:
				srcFactor = GL_ZERO;
				break;
			case GLS_SRCBLEND_ONE:
				srcFactor = GL_ONE;
				break;
			case GLS_SRCBLEND_DST_COLOR:
				srcFactor = GL_DST_COLOR;
				break;
			case GLS_SRCBLEND_ONE_MINUS_DST_COLOR:
				srcFactor = GL_ONE_MINUS_DST_COLOR;
				break;
			case GLS_SRCBLEND_SRC_ALPHA:
				srcFactor = GL_SRC_ALPHA;
				break;
			case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:
				srcFactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case GLS_SRCBLEND_DST_ALPHA:
				srcFactor = GL_DST_ALPHA;
				break;
			case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA:
				srcFactor = GL_ONE_MINUS_DST_ALPHA;
				break;
			case GLS_SRCBLEND_ALPHA_SATURATE:
				srcFactor = GL_SRC_ALPHA_SATURATE;
				break;
			default:
				srcFactor = GL_ONE;		// to get warning to shut up
				ri.Error( ERR_DROP, "GL_State: invalid src blend state bits\n" );
				break;
			}

			switch ( stateBits & GLS_DSTBLEND_BITS )
			{
			case GLS_DSTBLEND_ZERO:
				dstFactor = GL_ZERO;
				break;
			case GLS_DSTBLEND_ONE:
				dstFactor = GL_ONE;
				break;
			case GLS_DSTBLEND_SRC_COLOR:
				dstFactor = GL_SRC_COLOR;
				break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR:
				dstFactor = GL_ONE_MINUS_SRC_COLOR;
				break;
			case GLS_DSTBLEND_SRC_ALPHA:
				dstFactor = GL_SRC_ALPHA;
				break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:
				dstFactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case GLS_DSTBLEND_DST_ALPHA:
				dstFactor = GL_DST_ALPHA;
				break;
			case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA:
				dstFactor = GL_ONE_MINUS_DST_ALPHA;
				break;
			default:
				dstFactor = GL_ONE;		// to get warning to shut up
				ri.Error( ERR_DROP, "GL_State: invalid dst blend state bits\n" );
				break;
			}

			qglEnable( GL_BLEND );
			qglBlendFunc( srcFactor, dstFactor );
		}
		else
		{
			qglDisable( GL_BLEND );
		}
	}

	//
	// check depthmask
	//
	if ( diff & GLS_DEPTHMASK_TRUE )
	{
		if ( stateBits & GLS_DEPTHMASK_TRUE )
		{
			qglDepthMask( GL_TRUE );
		}
		else
		{
			qglDepthMask( GL_FALSE );
		}
	}

	//
	// fill/line mode
	//
	if ( diff & GLS_POLYMODE_LINE )
	{
		if ( stateBits & GLS_POLYMODE_LINE )
		{
			qglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		else
		{
			qglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
	}

	//
	// depthtest
	//
	if ( diff & GLS_DEPTHTEST_DISABLE )
	{
		if ( stateBits & GLS_DEPTHTEST_DISABLE )
		{
			qglDisable( GL_DEPTH_TEST );
		}
		else
		{
			qglEnable( GL_DEPTH_TEST );
		}
	}

	//
	// alpha test
	//
	if ( diff & GLS_ATEST_BITS )
	{
		switch ( stateBits & GLS_ATEST_BITS )
		{
		case 0:
			qglDisable( GL_ALPHA_TEST );
			break;
		case GLS_ATEST_GT_0:
			qglEnable( GL_ALPHA_TEST );
			qglAlphaFunc( GL_GREATER, 0.0f );
			break;
		case GLS_ATEST_LT_80:
			qglEnable( GL_ALPHA_TEST );
			qglAlphaFunc( GL_LESS, 0.5f );
			break;
		case GLS_ATEST_GE_80:
			qglEnable( GL_ALPHA_TEST );
			qglAlphaFunc( GL_GEQUAL, 0.5f );
			break;
		default:
			assert( 0 );
			break;
		}
	}

	glState.glStateBits = stateBits;
}



/*
================
RB_Hyperspace

A player has predicted a teleport, but hasn't arrived yet
================
*/
static void RB_Hyperspace( void ) {
	float		c;

	if ( !backEnd.isHyperspace ) {
		// do initialization shit
	}

	c = ( backEnd.refdef.time & 255 ) / 255.0f;
	qglClearColor( c, c, c, 1 );
	qglClear( GL_COLOR_BUFFER_BIT );

	backEnd.isHyperspace = qtrue;
}


void SetViewportAndScissor( void ) {
	qglMatrixMode(GL_PROJECTION);
	qglLoadMatrixf( backEnd.viewParms.projectionMatrix );
	qglMatrixMode(GL_MODELVIEW);

	// set the window clipping
	qglViewport( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY, 
		backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );
	qglScissor( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY, 
		backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );
}

/*
=================
RB_BeginDrawingView

Any mirrored or portaled views have already been drawn, so prepare
to actually render the visible surfaces for this view
=================
*/
void RB_BeginDrawingView (void) {
	int clearBits = 0;

	// sync with gl if needed
	if ( r_finish->integer == 1 && !glState.finishCalled ) {
		qglFinish ();
		glState.finishCalled = qtrue;
	}
	if ( r_finish->integer == 0 ) {
		glState.finishCalled = qtrue;
	}

	// we will need to change the projection matrix before drawing
	// 2D images again
	backEnd.projection2D = qfalse;

	//
	// set the modelview matrix for the viewer
	//
	SetViewportAndScissor();

	// ensures that depth writes are enabled for the depth clear
	GL_State( GLS_DEFAULT );
	// clear relevant buffers
	clearBits = GL_DEPTH_BUFFER_BIT;

	if ( r_measureOverdraw->integer || r_shadows->integer == 2 )
	{
		clearBits |= GL_STENCIL_BUFFER_BIT;
	}
	if ( r_fastsky->integer && !( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) )
	{
		clearBits |= GL_COLOR_BUFFER_BIT;	// FIXME: only if sky shaders have been used
#ifdef _DEBUG
		qglClearColor( 0.8f, 0.7f, 0.4f, 1.0f );	// FIXME: get color of sky
#else
		qglClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// FIXME: get color of sky
#endif
	}
		
	
	if (r_ext_vertex_buffer_object->integer && backEnd.viewParms.doWater )
	{
		clearBits |= GL_COLOR_BUFFER_BIT;
		qglClearColor( 0, 0, 0, 1.0f );
	}

	//clear the backbuffer each target render
	if (backEnd.refdef.pixelTarget!=NULL)
	{
		clearBits |= GL_COLOR_BUFFER_BIT;	
		qglClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// FIXME: get color of sky
	}


	if (backEnd.refdef.rdflags & RDF_NOCLEAR)
	{
		clearBits=GL_COLOR_BUFFER_BIT;
	}

	qglClear( clearBits );

	if ( ( backEnd.refdef.rdflags & RDF_HYPERSPACE ) )
	{
		RB_Hyperspace();
		return;
	}
	else
	{
		backEnd.isHyperspace = qfalse;
	}

	glState.faceCulling = -1;		// force face culling to set next time

	// we will only draw a sun if there was sky rendered in this view
	backEnd.skyRenderedThisView = qfalse;

	// clip to the plane of the portal
	if ( backEnd.viewParms.isPortal ) {
		float	plane[4];
		double	plane2[4];

		plane[0] = backEnd.viewParms.portalPlane.normal[0];
		plane[1] = backEnd.viewParms.portalPlane.normal[1];
		plane[2] = backEnd.viewParms.portalPlane.normal[2];
		plane[3] = backEnd.viewParms.portalPlane.dist;

		plane2[0] = DotProduct (backEnd.viewParms.or.axis[0], plane);
		plane2[1] = DotProduct (backEnd.viewParms.or.axis[1], plane);
		plane2[2] = DotProduct (backEnd.viewParms.or.axis[2], plane);
		plane2[3] = DotProduct (plane, backEnd.viewParms.or.origin) - plane[3];

		qglLoadMatrixf( s_flipMatrix );
		qglClipPlane (GL_CLIP_PLANE0, plane2);
		qglEnable (GL_CLIP_PLANE0);
	} else {
		qglDisable (GL_CLIP_PLANE0);
	}


	// clip to the plane of the water, only show whats underneath
	if ( backEnd.viewParms.isWater==1 || backEnd.viewParms.isWater==2 ) 
	{
		vec4_t plane;
		float bias=8.0f;
	
		plane[0]=backEnd.viewParms.waterPlane.normal[0];
		plane[1]=backEnd.viewParms.waterPlane.normal[1];
		plane[2]=backEnd.viewParms.waterPlane.normal[2];
		plane[3]=backEnd.viewParms.waterPlane.dist; //fudge it 1 unit to clean up the edge cuts

		//we're underwater  so flip
		if (backEnd.viewParms.isUnderwater) 
		{
			plane[0]=-plane[0];
			plane[1]=-plane[1];
			plane[2]=-plane[2];
			plane[3]=-plane[3];
		}

		if (backEnd.viewParms.isWater==1)
		{
			double f[4];
			f[0]=plane[0];
			f[1]=plane[1];
			f[2]=plane[2];
			f[3]=plane[3]+bias;

			qglLoadMatrixf(backEnd.viewParms.world.modelMatrix );
			qglClipPlane (GL_CLIP_PLANE1, f);
			qglEnable (GL_CLIP_PLANE1);
		}

		if (backEnd.viewParms.isWater==2)
		{
			double f[4];
			f[0]=-plane[0];
			f[1]=-plane[1];
			f[2]=-plane[2];
			f[3]=-plane[3]+bias;

			qglLoadMatrixf(backEnd.viewParms.world.modelMatrix );
			qglClipPlane (GL_CLIP_PLANE1, f);
			qglEnable (GL_CLIP_PLANE1);
		}
	}
	else
	{
		qglDisable(GL_CLIP_PLANE1);
		
	}
	
	GL_Cull( CT_FRONT_SIDED );
	
	
}


#define	MAC_EVENT_PUMP_MSEC		5

/*
==================
RB_RenderDrawSurfList
==================
*/
void RB_RenderDrawSurfList( drawSurf_t *drawSurfs, int numDrawSurfs ) {
	shader_t		*shader, *oldShader;
	int				fogNum, oldFogNum;
	int				entityNum, oldEntityNum;
	int				dlighted, oldDlighted;
	qboolean		depthRange, oldDepthRange, isCrosshair, wasCrosshair;
	int				i;
	drawSurf_t		*drawSurf;
	int			oldSurf;
	int				oldSort;
	float			originalTime;

	// save original time for entity shader offsets
	originalTime = backEnd.refdef.floatTime;

	// clear the z buffer, set the modelview, etc
	RB_BeginDrawingView ();

	// draw everything
	oldEntityNum = -1;
	backEnd.currentEntity = &tr.worldEntity;
	oldShader = NULL;
	oldFogNum = -1;
	oldDepthRange = qfalse;
	wasCrosshair = qfalse;
	oldDlighted = qfalse;
	oldSort = -1;
	oldSurf = -1;
	
	depthRange = qfalse;

	backEnd.pc.c_surfaces += numDrawSurfs;

	for (i = 0, drawSurf = drawSurfs ; i < numDrawSurfs ; i++, drawSurf++)
	{

		if (drawSurf->sort == oldSort )  
		{
			// fast path, same as previous sort
			rb_surfaceTable[ *drawSurf->surface ]( drawSurf->surface );
			continue;
		}
		oldSort = drawSurf->sort;
		R_DecomposeSort( drawSurf->sort, &entityNum, &shader, &fogNum, &dlighted );

		//
		// change the tess parameters if needed
		// a "entityMergable" shader is a shader that can have surfaces from seperate
		// entities merged into a single batch, like smoke and blood puff sprites
		if (shader != oldShader || fogNum != oldFogNum || dlighted != oldDlighted 
			|| ( entityNum != oldEntityNum && !shader->entityMergable ) ) 
		{
			if (oldShader != NULL) {
				RB_EndSurface();
			}
			RB_BeginSurface( shader, fogNum );
			oldShader = shader;
			oldFogNum = fogNum;
			oldDlighted = dlighted;
			oldSurf   = *drawSurf->surface;
		}

		//
		// change the modelview matrix if needed
		//
		if ( entityNum != oldEntityNum ) {
			depthRange = isCrosshair = qfalse;

			if ( entityNum != ENTITYNUM_WORLD ) {
				backEnd.currentEntity = &backEnd.refdef.entities[entityNum];
				backEnd.refdef.floatTime = originalTime - backEnd.currentEntity->e.shaderTime;
				// we have to reset the shaderTime as well otherwise image animations start
				// from the wrong frame
				tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;

				// set up the transformation matrix
				R_RotateForEntity( backEnd.currentEntity, &backEnd.viewParms, &backEnd.or );

				// set up the dynamic lighting if needed
				if ( backEnd.currentEntity->needDlights ) {
					R_TransformDlights( backEnd.refdef.num_dlights, backEnd.refdef.dlights, &backEnd.or );
				}

				if ( backEnd.currentEntity->e.renderfx & RF_DEPTHHACK )
				{
					// hack the depth range to prevent view model from poking into walls
					depthRange = qtrue;
					
					if(backEnd.currentEntity->e.renderfx & RF_CROSSHAIR)
						isCrosshair = qtrue;
				}
			} else {
				backEnd.currentEntity = &tr.worldEntity;
				backEnd.refdef.floatTime = originalTime;
				backEnd.or = backEnd.viewParms.world;
				// we have to reset the shaderTime as well otherwise image animations on
				// the world (like water) continue with the wrong frame
				tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;
				R_TransformDlights( backEnd.refdef.num_dlights, backEnd.refdef.dlights, &backEnd.or );
			}

			qglLoadMatrixf( backEnd.or.modelMatrix );

			//
			// change depthrange. Also change projection matrix so first person weapon does not look like coming
			// out of the screen.
			//
			if ( oldDepthRange != depthRange || wasCrosshair != isCrosshair )
			{
				if ( depthRange )
				{
					if ( backEnd.viewParms.stereoFrame != STEREO_CENTER )
					{
						if ( isCrosshair )
						{
							if ( oldDepthRange )
							{
								// was not a crosshair but now is, change back proj matrix
								qglMatrixMode(GL_PROJECTION);
								qglLoadMatrixf(backEnd.viewParms.projectionMatrix);
								qglMatrixMode(GL_MODELVIEW);
							}
						}
						else
						{
							viewParms_t temp = backEnd.viewParms;

							R_SetupProjection(&temp, r_znear->value, qfalse);

							qglMatrixMode(GL_PROJECTION);
							qglLoadMatrixf(temp.projectionMatrix);
							qglMatrixMode(GL_MODELVIEW);
						}
					}

					if(!oldDepthRange)
						qglDepthRange (0, 0.3);
				}
				else
				{
					if ( !wasCrosshair && backEnd.viewParms.stereoFrame != STEREO_CENTER )
					{
						qglMatrixMode(GL_PROJECTION);
						qglLoadMatrixf(backEnd.viewParms.projectionMatrix);
						qglMatrixMode(GL_MODELVIEW);
					}

					qglDepthRange (0, 1);
				}

				oldDepthRange = depthRange;
				wasCrosshair = isCrosshair;
			}

			oldEntityNum = entityNum;
		}

		// add the triangles for this surface
		rb_surfaceTable[ *drawSurf->surface ]( drawSurf->surface );
	}

	backEnd.refdef.floatTime = originalTime;

	// draw the contents of the last shader batch
	if (oldShader != NULL) 
	{
		EndSurfaceCount++;	
		if (EndSurfaceCount==2)
		{
			EndSurfaceCount = 2;
		}
		RB_EndSurface();
	}

	// go back to the world modelview matrix
	qglLoadMatrixf( backEnd.viewParms.world.modelMatrix );
	if ( depthRange ) {
		qglDepthRange (0, 1);
	}

#if 0
	RB_DrawSun();
#endif
	// darken down any stencil shadows
	RB_ShadowFinish();		

	// add light flares on lights that aren't obscured
	RB_RenderFlares();

	// post process for dof, bloom 
	RB_PostProcess();

	// copy the underwater view if we have one
	if (backEnd.viewParms.isWater==1)
	{
		//Copy scene
		GL_SelectTexture(0);
		GL_BindTexnum(tr.underwaterTexnum);
		qglCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0,0,0, WATER_RES_X, WATER_RES_Y ); 
	}
	// copy the reflected view if we have one
	if (backEnd.viewParms.isWater==2)
	{
		//Copy scene
		GL_SelectTexture(0);
		GL_BindTexnum(tr.reflectionTexnum);
		qglCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0,0,0, WATER_RES_X,WATER_RES_Y ); 
	}

	// make a copy if we want one
	if (tr.refdef.pixelTarget)
	{
		int i;
		
		// Bugfix: drivers absolutely hate running in high res and using qglReadPixels near the top or bottom edge.
		// soo.. lets do it in the middle.
		qglReadPixels( glConfig.vidWidth / 2,glConfig.vidHeight / 2,tr.refdef.pixelTargetX, tr.refdef.pixelTargetY, GL_RGBA, GL_UNSIGNED_BYTE, tr.refdef.pixelTarget);
		
		for (i=0;i<tr.refdef.pixelTargetX*tr.refdef.pixelTargetY;i++)
		{
			tr.refdef.pixelTarget[(i*4)+3]=255; //set the alpha pure white
		}
	}

}


/*
============================================================================

RENDER BACK END THREAD FUNCTIONS

============================================================================
*/

/*
================
RB_SetGL2D

================
*/
void	RB_SetGL2D (void) {
	backEnd.projection2D = qtrue;

	// set 2D virtual screen size
	qglViewport( 0, 0, glConfig.vidWidth, glConfig.vidHeight );
	qglScissor( 0, 0, glConfig.vidWidth, glConfig.vidHeight );
	qglMatrixMode(GL_PROJECTION);
	qglLoadIdentity ();
	qglOrtho (0, glConfig.vidWidth, glConfig.vidHeight, 0, 0, 1);
	qglMatrixMode(GL_MODELVIEW);
	qglLoadIdentity ();

	GL_State( GLS_DEPTHTEST_DISABLE |
			  GLS_SRCBLEND_SRC_ALPHA |
			  GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );

	qglDisable( GL_CULL_FACE );
	qglDisable( GL_CLIP_PLANE0 );
	qglDisable( GL_CLIP_PLANE1 );

	// set time for 2D shaders
	backEnd.refdef.time = ri.Milliseconds();
	backEnd.refdef.floatTime = backEnd.refdef.time * 0.001f;
}


/*
=============
RE_StretchRaw

FIXME: not exactly backend
Stretches a raw 32 bit power of 2 bitmap image over the given screen rectangle.
Used for cinematics.
=============
*/
void RE_StretchRaw (int x, int y, int w, int h, int cols, int rows, const byte *data, int client, qboolean dirty) {
	int			i, j;
	int			start, end;

	if ( !tr.registered ) {
		return;
	}
	R_SyncRenderThread();

	// we definately want to sync every frame for the cinematics
	qglFinish();

	start = end = 0;
	if ( r_speeds->integer ) {
		start = ri.Milliseconds();
	}

	// make sure rows and cols are powers of 2
	for ( i = 0 ; ( 1 << i ) < cols ; i++ ) {
	}
	for ( j = 0 ; ( 1 << j ) < rows ; j++ ) {
	}
	if ( ( 1 << i ) != cols || ( 1 << j ) != rows) {
		ri.Error (ERR_DROP, "Draw_StretchRaw: size not a power of 2: %i by %i", cols, rows);
	}

	GL_Bind( tr.scratchImage[client] );

	// if the scratchImage isn't in the format we want, specify it as a new texture
	if ( cols != tr.scratchImage[client]->width || rows != tr.scratchImage[client]->height ) {
		tr.scratchImage[client]->width = tr.scratchImage[client]->uploadWidth = cols;
		tr.scratchImage[client]->height = tr.scratchImage[client]->uploadHeight = rows;
		qglTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, cols, rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
		qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );	
	} else {
		if (dirty) {
			// otherwise, just subimage upload it so that drivers can tell we are going to be changing
			// it and don't try and do a texture compression
			qglTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, cols, rows, GL_RGBA, GL_UNSIGNED_BYTE, data );
		}
	}

	if ( r_speeds->integer ) {
		end = ri.Milliseconds();
		ri.Printf( PRINT_ALL, "qglTexSubImage2D %i, %i: %i msec\n", cols, rows, end - start );
	}

	RB_SetGL2D();

	qglColor3f( tr.identityLight, tr.identityLight, tr.identityLight );

	qglBegin (GL_QUADS);
	qglTexCoord2f ( 0.5f / cols,  0.5f / rows );
	qglVertex2f (x, y);
	qglTexCoord2f ( ( cols - 0.5f ) / cols,  0.5f / rows );
	qglVertex2f (x+w, y);
	qglTexCoord2f ( ( cols - 0.5f ) / cols, ( rows - 0.5f ) / rows );
	qglVertex2f (x+w, y+h);
	qglTexCoord2f ( 0.5f / cols, ( rows - 0.5f ) / rows );
	qglVertex2f (x, y+h);
	qglEnd ();
}

void RE_UploadCinematic (int w, int h, int cols, int rows, const byte *data, int client, qboolean dirty) {

	GL_Bind( tr.scratchImage[client] );

	// if the scratchImage isn't in the format we want, specify it as a new texture
	if ( cols != tr.scratchImage[client]->width || rows != tr.scratchImage[client]->height ) {
		tr.scratchImage[client]->width = tr.scratchImage[client]->uploadWidth = cols;
		tr.scratchImage[client]->height = tr.scratchImage[client]->uploadHeight = rows;
		qglTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, cols, rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
		qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );	
	} else {
		if (dirty) {
			// otherwise, just subimage upload it so that drivers can tell we are going to be changing
			// it and don't try and do a texture compression
			qglTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, cols, rows, GL_RGBA, GL_UNSIGNED_BYTE, data );
		}
	}
}


/*
=============
RB_SetColor

=============
*/
const void	*RB_SetColor( const void *data ) {
	const setColorCommand_t	*cmd;

	cmd = (const setColorCommand_t *)data;

	backEnd.color2D[0] = cmd->color[0] * 255;
	backEnd.color2D[1] = cmd->color[1] * 255;
	backEnd.color2D[2] = cmd->color[2] * 255;
	backEnd.color2D[3] = cmd->color[3] * 255;

	return (const void *)(cmd + 1);
}

/*
=============
RB_StretchPic
=============
*/
const void *RB_StretchPic ( const void *data ) {
	const stretchPicCommand_t	*cmd;
	shader_t *shader;
	int		numVerts, numIndexes;

	cmd = (const stretchPicCommand_t *)data;

	if ( !backEnd.projection2D ) {
		RB_SetGL2D();
	}

	shader = cmd->shader;
	if ( !shader )
		return (const void *)(cmd + 1);

	if ( shader != tess.shader ) {
		if ( tess.numIndexes ) {
			RB_EndSurface();
		}
		backEnd.currentEntity = &backEnd.entity2D;
		RB_BeginSurface( shader, 0 );
	}

	RB_CHECKOVERFLOW( 4, 6 );
	numVerts = tess.numVertexes;
	numIndexes = tess.numIndexes;

	tess.numVertexes += 4;
	tess.numIndexes += 6;

	tess.indexes[ numIndexes ] = numVerts + 3;
	tess.indexes[ numIndexes + 1 ] = numVerts + 0;
	tess.indexes[ numIndexes + 2 ] = numVerts + 2;
	tess.indexes[ numIndexes + 3 ] = numVerts + 2;
	tess.indexes[ numIndexes + 4 ] = numVerts + 0;
	tess.indexes[ numIndexes + 5 ] = numVerts + 1;

	*(int *)tess.vertexColors[ numVerts ] =
		*(int *)tess.vertexColors[ numVerts + 1 ] =
		*(int *)tess.vertexColors[ numVerts + 2 ] =
		*(int *)tess.vertexColors[ numVerts + 3 ] = *(int *)backEnd.color2D;

	tess.xyz[ numVerts ][0] = cmd->x;
	tess.xyz[ numVerts ][1] = cmd->y;
	tess.xyz[ numVerts ][2] = 0;

	tess.texCoords[ numVerts ][0][0] = cmd->s1;
	tess.texCoords[ numVerts ][0][1] = cmd->t1;

	tess.xyz[ numVerts + 1 ][0] = cmd->x + cmd->w;
	tess.xyz[ numVerts + 1 ][1] = cmd->y;
	tess.xyz[ numVerts + 1 ][2] = 0;

	tess.texCoords[ numVerts + 1 ][0][0] = cmd->s2;
	tess.texCoords[ numVerts + 1 ][0][1] = cmd->t1;

	tess.xyz[ numVerts + 2 ][0] = cmd->x + cmd->w;
	tess.xyz[ numVerts + 2 ][1] = cmd->y + cmd->h;
	tess.xyz[ numVerts + 2 ][2] = 0;

	tess.texCoords[ numVerts + 2 ][0][0] = cmd->s2;
	tess.texCoords[ numVerts + 2 ][0][1] = cmd->t2;

	tess.xyz[ numVerts + 3 ][0] = cmd->x;
	tess.xyz[ numVerts + 3 ][1] = cmd->y + cmd->h;
	tess.xyz[ numVerts + 3 ][2] = 0;

	tess.texCoords[ numVerts + 3 ][0][0] = cmd->s1;
	tess.texCoords[ numVerts + 3 ][0][1] = cmd->t2;

	return (const void *)(cmd + 1);
}


/*
=============
RB_DrawSurfs

=============
*/
const void	*RB_DrawSurfs( const void *data ) {
	const drawSurfsCommand_t	*cmd;

	// finish any 2D drawing if needed
	if ( tess.numIndexes ) {
		RB_EndSurface();
	}

	cmd = (const drawSurfsCommand_t *)data;

	backEnd.refdef = cmd->refdef;
	backEnd.viewParms = cmd->viewParms;

	//TODO Maybe check for rdf_noworld stuff but q3mme has full 3d ui
	backEnd.doneSurfaces = qtrue;
	RB_RenderDrawSurfList( cmd->drawSurfs, cmd->numDrawSurfs );

	return (const void *)(cmd + 1);
}


/*
=============
RB_DrawBuffer

=============
*/
const void	*RB_DrawBuffer( const void *data ) {
	const drawBufferCommand_t	*cmd;

	cmd = (const drawBufferCommand_t *)data;

	qglDrawBuffer( cmd->buffer );

	// clear screen for debugging
	if ( r_clear->integer ) {
		qglClearColor( 1, 0, 0.5, 1 );
		qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}

	return (const void *)(cmd + 1);
}

/*
===============
RB_ShowImages

Draw all the images to the screen, on top of whatever
was there.  This is used to test for texture thrashing.

Also called by RE_EndRegistration
===============
*/
void RB_ShowImages( void ) {
	int		i;
	image_t	*image;
	float	x, y, w, h;
	int		start, end;

	if ( !backEnd.projection2D ) {
		RB_SetGL2D();
	}

	qglClear( GL_COLOR_BUFFER_BIT );

	qglFinish();

	start = ri.Milliseconds();

	for ( i=0 ; i<tr.numImages ; i++ ) {
		image = tr.images[i];

		w = glConfig.vidWidth / 20;
		h = glConfig.vidHeight / 15;
		x = i % 20 * w;
		y = i / 20 * h;

		// show in proportional size in mode 2
		if ( r_showImages->integer == 2 ) {
			w *= image->uploadWidth / 512.0f;
			h *= image->uploadHeight / 512.0f;
		}

		GL_Bind( image );
		qglBegin (GL_QUADS);
		qglTexCoord2f( 0, 0 );
		qglVertex2f( x, y );
		qglTexCoord2f( 1, 0 );
		qglVertex2f( x + w, y );
		qglTexCoord2f( 1, 1 );
		qglVertex2f( x + w, y + h );
		qglTexCoord2f( 0, 1 );
		qglVertex2f( x, y + h );
		qglEnd();
	}

	qglFinish();

	end = ri.Milliseconds();
	ri.Printf( PRINT_DEVELOPER, "%i msec to draw all images\n", end - start );

}

/*
=============
RB_ColorMask

=============
*/
const void *RB_ColorMask(const void *data)
{
	const colorMaskCommand_t *cmd = data;
	
	qglColorMask(cmd->rgba[0], cmd->rgba[1], cmd->rgba[2], cmd->rgba[3]);
	
	return (const void *)(cmd + 1);
}

/*
=============
RB_ClearDepth

=============
*/
const void *RB_ClearDepth(const void *data)
{
	const clearDepthCommand_t *cmd = data;
	
	if(tess.numIndexes)
		RB_EndSurface();

	// texture swapping test
	if (r_showImages->integer)
		RB_ShowImages();

	qglClear(GL_DEPTH_BUFFER_BIT);
	
	return (const void *)(cmd + 1);
}

/*
=============
RB_SwapBuffers

=============
*/
const void	*RB_SwapBuffers( const void *data ) {
	const swapBuffersCommand_t	*cmd;

	// finish any 2D drawing if needed
	if ( tess.numIndexes ) {
		RB_EndSurface();
	}

	// texture swapping test
	if ( r_showImages->integer ) {
		RB_ShowImages();
	}

	cmd = (const swapBuffersCommand_t *)data;

	// we measure overdraw by reading back the stencil buffer and
	// counting up the number of increments that have happened
	if ( r_measureOverdraw->integer ) {
		int i;
		long sum = 0;
		unsigned char *stencilReadback;

		stencilReadback = ri.Hunk_AllocateTempMemory( glConfig.vidWidth * glConfig.vidHeight );
		qglReadPixels( 0, 0, glConfig.vidWidth, glConfig.vidHeight, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, stencilReadback );

		for ( i = 0; i < glConfig.vidWidth * glConfig.vidHeight; i++ ) {
			sum += stencilReadback[i];
		}

		backEnd.pc.c_overDraw += sum;
		ri.Hunk_FreeTempMemory( stencilReadback );
	}


	if ( !glState.finishCalled ) {
		qglFinish();
	}

	GLimp_LogComment( "***************** RB_SwapBuffers *****************\n\n\n" );

	GLimp_EndFrame();

	backEnd.projection2D = qfalse;

	backEnd.doneBloom = qfalse;
	backEnd.doneSurfaces = qfalse;
	
	return (const void *)(cmd + 1);
}

/*
====================
RB_ExecuteRenderCommands

This function will be called synchronously if running without
smp extensions, or asynchronously by another thread.
====================
*/
void RB_ExecuteRenderCommands( const void *data ) {
	int		t1, t2;

	t1 = ri.Milliseconds ();

	if ( !r_smp->integer || data == backEndData[0]->commands.cmds ) {
		backEnd.smpFrame = 0;
	} else {
		backEnd.smpFrame = 1;
	}

	while ( 1 ) {
		switch ( *(const int *)data ) {
		case RC_SET_COLOR:
			data = RB_SetColor( data );
			break;
		case RC_STRETCH_PIC:
			data = RB_StretchPic( data );
			break;
		case RC_DRAW_SURFS:
			data = RB_DrawSurfs( data );
			break;
		case RC_DRAW_BUFFER:
			data = RB_DrawBuffer( data );
			break;
		case RC_SWAP_BUFFERS:
			data = RB_SwapBuffers( data );
			break;
		case RC_SCREENSHOT:
			data = RB_TakeScreenshotCmd( data );
			break;
		case RC_VIDEOFRAME:
			data = RB_TakeVideoFrameCmd( data );
			break;
		case RC_COLORMASK:
			data = RB_ColorMask(data);
			break;
		case RC_CLEARDEPTH:
			data = RB_ClearDepth(data);
			break;
		case RC_END_OF_LIST:
		default:
			// stop rendering on this thread
			t2 = ri.Milliseconds ();
			backEnd.pc.msec = t2 - t1;
			return;
		}
	}

}


/*
================
RB_RenderThread
================
*/
void RB_RenderThread( void ) {
	const void	*data;

	// wait for either a rendering command or a quit command
	while ( 1 ) {
		// sleep until we have work to do
		data = GLimp_RendererSleep();

		if ( !data ) {
			return;	// all done, renderer is shutting down
		}

		renderThreadActive = qtrue;

		RB_ExecuteRenderCommands( data );

		renderThreadActive = qfalse;
	}
}


