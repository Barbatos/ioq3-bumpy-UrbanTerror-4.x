#include "tr_local.h"

void R_LoadFrag( fragShader_t *fragprog, char* vertex, char* fragment )
{
	qboolean status;
	char buffer[8096];
	const char *bufp = &buffer[0];
	int i;
	int size;

	ri.Printf (PRINT_ALL, "R_LoadFragmentPrograms: Loading %s...\n", vertex);
	memset(fragprog, 0, sizeof(fragShader_t));

	strcpy(fragprog->name,fragment);

	fragprog->vertexShader = qglCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	fragprog->fragShader = qglCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	fragprog->vertexType=1;

	memset(buffer, 0, sizeof(buffer));
	i = FS_ReadFile(vertex, NULL);
	if ( i < 0 )
	{
		ri.Error (ERR_DROP, "R_LoadFragmentPrograms: Vertex shader program not found.");
		return;
	}
		
	if ( i >= sizeof(buffer)) 
	{
		ri.Error (ERR_DROP, "R_LoadFragmentPrograms: Vertex shader program too large.");
		return;
	}
	i = FS_ReadFile(vertex, (void **)&bufp);

	qglShaderSourceARB(fragprog->vertexShader, 1, &bufp, NULL);
	qglCompileShaderARB(fragprog->vertexShader);

	ri.Printf (PRINT_ALL, "R_LoadFragmentPrograms: Loading %s...\n", fragment);
	memset(buffer, 0, sizeof(buffer));
	i = FS_ReadFile(fragment, NULL);
	if ( i < 0 )
	{
		ri.Error (ERR_DROP, "R_LoadFragmentPrograms: Fragment shader program not found");
		return;
	}
		
	if ( i >= sizeof(buffer)) 
	{
		ri.Error (ERR_DROP, "R_LoadFragmentPrograms: Fragment shader program too large");
		return;
	}

	i = FS_ReadFile(fragment, (void **)&bufp);
	
	qglShaderSourceARB(fragprog->fragShader, 1, &bufp, NULL);
	qglCompileShaderARB(fragprog->fragShader);

	qglGetObjectParameterivARB(fragprog->vertexShader, GL_OBJECT_COMPILE_STATUS_ARB, (GLint*)&status);
	if (!status)
	{
		qglGetInfoLogARB(fragprog->vertexShader,sizeof(buffer),&size, &buffer[0]);
		ri.Error (ERR_DROP, "R_LoadFragmentPrograms: Vertex shader program %s %s", vertex, buffer);
		return;
	}

	qglGetObjectParameterivARB(fragprog->fragShader, GL_OBJECT_COMPILE_STATUS_ARB, (GLint*)&status);
	if (!status)
	{
		qglGetInfoLogARB(fragprog->fragShader, sizeof(buffer), &size, &buffer[0]);
		ri.Error (ERR_DROP, "R_LoadFragmentPrograms: Fragment shader program %s %s", fragment, buffer);
		return;
	}

}

void R_LoadSimpleFragmentPrograms()
{

	if (!r_ext_vertex_shader->integer) return;

	R_LoadWorldFragShader(&tr.fragshader[FS_CHEAP],"glsl/world_vertex.cfg", "glsl/fs_fragment_cheap.cfg");
	
	//Wireframe thingy
	R_LoadWorldFragShader(&tr.fragshader[FS_WIREFRAME],"glsl/world_vertex_wireframe.cfg","glsl/fs_wireframe.cfg");

	//super Simple vertex lit static geometry
	R_LoadWorldFragShader(&tr.fragshader[FS_VERTEX_LIGHT_CHEAP],"glsl/world_vertex.cfg","glsl/fs_vertex_light_cheap.cfg");

	//super simple terrain optimization 
	R_LoadWorldFragShader(&tr.fragshader[FS_TERRAIN_ALPHA_CHEAP],"glsl/world_vertex.cfg","glsl/fs_terrain_alpha_cheap.cfg");

}


void R_LoadSM3FragmentPrograms()
{
	qboolean status;
	char buffer[8096];
	int size;


	if (r_dof->integer)
	{
		R_LoadFrag(&tr.bloom_fog_dof_fragshader,"glsl/pass_vertex.cfg","glsl/bloom_dof_fog_fragment.cfg");
		
		//Program creation + bindings 
		tr.bloom_fog_dof_fragshader.program = qglCreateProgramObjectARB();
		
		qglAttachObjectARB( tr.bloom_fog_dof_fragshader.program, tr.bloom_fog_dof_fragshader.vertexShader );
		qglAttachObjectARB( tr.bloom_fog_dof_fragshader.program, tr.bloom_fog_dof_fragshader.fragShader );

		qglLinkProgramARB( tr.bloom_fog_dof_fragshader.program);    
		
		qglGetObjectParameterivARB( tr.bloom_fog_dof_fragshader.program, GL_OBJECT_LINK_STATUS_ARB, (GLint*)&status );

		if ( !status )
		{
			 qglGetInfoLogARB(tr.bloom_fog_dof_fragshader.program,sizeof(buffer),&size, &buffer[0]);
			 ri.Printf( PRINT_WARNING, "R_LoadSM3FragmentPrograms:bloom_dof_fog_fragment LinkStatus %s",buffer);
			 Cvar_SetValue( r_dof->name, 0 );
		}
	}
}

void R_LoadFragmentPrograms()
{
	qboolean status;
	char buffer[8096];
	int size;

	if (!r_ext_vertex_shader->integer) return;
	if (r_simpleshaders->integer) return;

	{
		R_LoadFrag(&tr.row3_fragshader,"glsl/pass_vertex.cfg","glsl/row3_fragment.cfg");
		
		//Program creation + bindings 
		tr.row3_fragshader.program = qglCreateProgramObjectARB();

		
		qglAttachObjectARB(tr.row3_fragshader.program, tr.row3_fragshader.vertexShader);
		qglAttachObjectARB(tr.row3_fragshader.program, tr.row3_fragshader.fragShader);

		qglLinkProgramARB( tr.row3_fragshader.program);    
		
		qglGetObjectParameterivARB(tr.row3_fragshader.program, GL_OBJECT_LINK_STATUS_ARB, (GLint *)&status);

		if (!status)
		{
			 qglGetInfoLogARB(tr.row3_fragshader.program,sizeof(buffer),&size, &buffer[0]);
			 ri.Error (ERR_DROP, "R_LoadFragmentPrograms:row3_fragment LinkStatus %s",buffer);
			 return;
		}
		
	}



	{
	
		R_LoadFrag(&tr.combine2_fragshader,"glsl/pass_vertex.cfg","glsl/combine2_fragment.cfg");
		
		//Program creation + bindings 
		tr.combine2_fragshader.program = qglCreateProgramObjectARB();
		
		qglAttachObjectARB(tr.combine2_fragshader.program, tr.combine2_fragshader.vertexShader);
		qglAttachObjectARB(tr.combine2_fragshader.program, tr.combine2_fragshader.fragShader);

		qglLinkProgramARB( tr.combine2_fragshader.program);    
		
		qglGetObjectParameterivARB(tr.combine2_fragshader.program, GL_OBJECT_LINK_STATUS_ARB, (GLint*)&status);

		if (!status)
		{
			 qglGetInfoLogARB(tr.combine2_fragshader.program,sizeof(buffer),&size, &buffer[0]);
			 ri.Error (ERR_DROP, "R_LoadFragmentPrograms:combine2_fragment LinkStatus %s",buffer);
			 return;
		}
	}

	{
		R_LoadFrag(&tr.bloom_fragshader,"glsl/pass_vertex.cfg","glsl/bloom_fragment.cfg");
		
		//Program creation + bindings 
		tr.bloom_fragshader.program = qglCreateProgramObjectARB();
		
		qglAttachObjectARB(tr.bloom_fragshader.program, tr.bloom_fragshader.vertexShader);
		qglAttachObjectARB(tr.bloom_fragshader.program, tr.bloom_fragshader.fragShader);

		qglLinkProgramARB( tr.bloom_fragshader.program);    
		
		qglGetObjectParameterivARB(tr.bloom_fragshader.program, GL_OBJECT_LINK_STATUS_ARB, (GLint*)&status);

		if (!status)
		{
			 qglGetInfoLogARB(tr.bloom_fragshader.program,sizeof(buffer),&size, &buffer[0]);
			 ri.Error (ERR_DROP, "R_LoadFragmentPrograms:bloom_fragment LinkStatus %s",buffer);
			 return;
		}
	}
	{
		R_LoadFrag(&tr.bloom_fog_fragshader,"glsl/pass_vertex.cfg","glsl/bloom_fog_fragment.cfg");
		
		//Program creation + bindings 
		tr.bloom_fog_fragshader.program = qglCreateProgramObjectARB();
		
		qglAttachObjectARB(tr.bloom_fog_fragshader.program, tr.bloom_fog_fragshader.vertexShader);
		qglAttachObjectARB(tr.bloom_fog_fragshader.program, tr.bloom_fog_fragshader.fragShader);

		qglLinkProgramARB( tr.bloom_fog_fragshader.program);    
		
		qglGetObjectParameterivARB(tr.bloom_fog_fragshader.program, GL_OBJECT_LINK_STATUS_ARB, (GLint*)&status);

		if (!status)
		{
			 qglGetInfoLogARB(tr.bloom_fog_fragshader.program,sizeof(buffer),&size, &buffer[0]);
			 ri.Error (ERR_DROP, "R_LoadFragmentPrograms: bloom_fog_fragment LinkStatus %s",buffer);
			 return;
		}
	}


	{
		R_LoadFrag(&tr.hdrcull_fragshader,"glsl/pass_vertex.cfg","glsl/hdrcull_fragment.cfg");
		
		//Program creation + bindings 
		tr.hdrcull_fragshader.program = qglCreateProgramObjectARB();
		
		qglAttachObjectARB(tr.hdrcull_fragshader.program, tr.hdrcull_fragshader.vertexShader);
		qglAttachObjectARB(tr.hdrcull_fragshader.program, tr.hdrcull_fragshader.fragShader);

		qglLinkProgramARB( tr.hdrcull_fragshader.program);    
		
		qglGetObjectParameterivARB(tr.hdrcull_fragshader.program, GL_OBJECT_LINK_STATUS_ARB, (GLint*)&status);

		if (!status)
		{
			 qglGetInfoLogARB(tr.hdrcull_fragshader.program,sizeof(buffer),&size, &buffer[0]);
			 ri.Error (ERR_DROP, "R_LoadFragmentPrograms: hdrcull_fragment LinkStatus %s",buffer);
			 return;
		}
	}



	//Complex Version, radbump
	R_LoadWorldFragShader(&tr.fragshader[FS_RAD_BUMP],"glsl/world_vertex.cfg","glsl/fs_fragment_radbump.cfg");
	
	//Complex Version, radbump, offset
	R_LoadWorldFragShader(&tr.fragshader[FS_RAD_BUMP_OFFSET],"glsl/world_vertex_offset.cfg","glsl/fs_fragment_radbump_offset.cfg");

	//Complex Version, Water
	R_LoadWorldFragShader(&tr.fragshader[FS_WATER],"glsl/water_vertex.cfg","glsl/water_fragment.cfg");
	
	//Cheapy water
	R_LoadWorldFragShader(&tr.fragshader[FS_WATER_CHEAP],"glsl/water_vertex.cfg","glsl/water_fragment_cheap.cfg");
	
	
	//Terrain optimizations
	R_LoadWorldFragShader(&tr.fragshader[FS_RAD_TERRAIN_ALPHA],"glsl/world_vertex_offset.cfg","glsl/fs_rad_terrain_alpha.cfg");
	

	//Simple vertex lit static geometry
	R_LoadWorldFragShader(&tr.fragshader[FS_VERTEX_LIGHT],"glsl/world_vertex.cfg","glsl/fs_vertex_light.cfg");
	
	//Generate textures for holding the bloom effect
	//autogen cubemap stuff
	{
		int xx=REF_CUBEMAP_SIZE;
		int yy=REF_CUBEMAP_SIZE;
		int i;
		for (i=0;i<6;i++)
		{
			tr.cubeTemp[i]=ri.Malloc(xx*yy*4);
		}
	}
	

	//bloomstuff
	{
		int bloomx=256;
		int bloomy=256;
		R_CreateRenderTextures(&tr.bloomTexture,bloomx,bloomy,qfalse,qfalse);
		R_CreateRenderTextures(&tr.bloomTempX,bloomx,bloomy,qfalse,qfalse);

	}

	qglGenTextures(1, &tr.sceneTexnum);
	GL_BindTexnum(tr.sceneTexnum);
	qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, glConfig.vidWidth,glConfig.vidHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//Depth buffer tex
	qglGenTextures(1, &tr.depthTexnum);
	GL_BindTexnum(tr.depthTexnum);
	qglTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, glConfig.vidWidth, glConfig.vidHeight, 0, GL_DEPTH_COMPONENT , GL_FLOAT, NULL);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,  GL_NEAREST);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,  GL_NEAREST);
	
	//Underwater tex 
	qglGenTextures(1, &tr.underwaterTexnum);
	GL_BindTexnum(tr.underwaterTexnum);
	qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WATER_RES_X,WATER_RES_Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//Reflection tex 
	qglGenTextures(1, &tr.reflectionTexnum);
	GL_BindTexnum(tr.reflectionTexnum);
	qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WATER_RES_X,WATER_RES_Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//
	//tr.testcubemap = R_FindImageFile( "textures/twoseven/cubemap/test", qfalse, qfalse, GL_CLAMP_TO_EDGE ,qtrue);	
}

static void CheckFBO( void )
{
    char enums[][20] =
    {
        "attachment",         // GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT........... All framebuffer attachment points are 'framebuffer attachment complete'.
        "missing attachment", // GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT....There is at least one image attached to the framebuffer.
        "",                   //
        "dimensions",         // GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT............All attached images have the same width and height.
        "formats",            // GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT...............All images attached to the attachment points COLOR_ATTACHMENT0_EXT through COLOR_ATTACHMENTn_EXT must have the same internal format.
        "draw buffer",        // GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT...........The value of FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT must not be NONE for any color attachment point(s) named by DRAW_BUFFERi.
        "read buffer",        // GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT...........If READ_BUFFER is not NONE, then the value of FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT must not be NONE for the color attachment point named by READ_BUFFER.
        "unsupported format"  // GL_FRAMEBUFFER_UNSUPPORTED_EXT......................The combination of internal formats of the attached images does not violate an implementation-dependent set of restrictions.
    };

    GLenum status = qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status == GL_FRAMEBUFFER_COMPLETE_EXT)
        return;
	if (status==0)
		return;
		//ri.Error(ERR_DROP,"incomplete framebuffer object due to 0");
	
    status -= GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT;
    ri.Error(ERR_DROP,"incomplete framebuffer object due to %s", enums[status]);
}

void R_CreateRenderTextures(renderbuffer_t *buffer,int xres,int yres, qboolean depth,qboolean depthtexture)
{

	memset(buffer,0,sizeof(renderbuffer_t));
	
	buffer->x=xres;
	buffer->y=yres;
	buffer->depthtexture=depthtexture;
	
	//colour buffer texture
	qglGenTextures(1, &buffer->texnum);
	GL_BindTexnum( buffer->texnum);
	//qglBindTexture(GL_TEXTURE_2D, buffer->texnum);
	if (!depthtexture)
	{
		qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, xres, yres, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	}
	else
	{
		qglTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, xres, yres, 0, GL_DEPTH_COMPONENT , GL_FLOAT, 0);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,  GL_NEAREST);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,  GL_NEAREST);
	}
	

	//depth buffer
	if (depth)
	{
		//use a depth buffer
		qglGenRenderbuffersEXT(1, &buffer->depth_rb);
		qglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, buffer->depth_rb);
		qglRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT , xres, yres);
		qglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

		/*if	(depthtexture)
		{
			//use a texture buffer
			qglGenTextures(1, &buffer->depthtexnum);
			qglBindTexture(GL_TEXTURE_2D, buffer->depthtexnum);
			
			//qglTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, xres, yres, 0, GL_LUMINACE, GL_FLOAT, 0);
			//qglTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,xres,yres   ,0 ,GL_DEPTH_COMPONENT,GL_INT,0);
			//qglTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT , xres, yres, 0, GL_LUMINANCE , GL_FLOAT, 0);
			qglTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, xres, yres, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, 0);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			GL_CheckErrors();			
		
		} */


	}
	else
	{
		buffer->depth_rb=0;
	}
	
	//Create Framebuffer
	qglGenFramebuffersEXT(1, &buffer->frameBuffer);
	qglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, buffer->frameBuffer); 

	//Bind Colour buffer
	qglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,buffer->texnum, 0);

	//bind depth texture
/*	if (buffer->depthtexture)
	{
		qglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,buffer->depthtexnum, 0);
	   //glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, depth2D, 0);//line from ati
	} */

	//Bind Depth Buffer?
	if (buffer->depth_rb>0 )
	{
		qglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, buffer->depth_rb);
	}

	CheckFBO();
	GL_CheckErrors();
	qglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

}



void RB_SetFrameBuffer(renderbuffer_t *buffer)
{
	if (buffer)
	{
		//initialize frame buffers 
		qglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, buffer->frameBuffer); 
		//qglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, buffer->texnum, 0);


		qglViewport(0, 0, buffer->x, buffer->y);
		qglScissor(0, 0, buffer->x, buffer->y);
		
	}
	else
	{
		qglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		qglViewport( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY, 
			backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );
		qglScissor( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY, 
			backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );

	}
}




//temp and dest should be the same size
//neither need a depth buffer.
void RB_Blur(unsigned int source, renderbuffer_t *tempx, renderbuffer_t *dest,qboolean hdrclip,float HdrCut)
{
	GLint loc;
	float offset;
	float pixelx;
	float pixely;
	float halfpixelx;
	float halfpixely;
	qboolean fastmode=qfalse;

	int fragshader=tr.row3_fragshader.program;
	
	if (source==dest->texnum) 
	{
		fastmode=qtrue; //source and dest are the same size, so no downsample required
	}

	pixelx=1.0f / (float)dest->x;
	pixely=1.0f / (float)dest->y;
	halfpixelx=pixelx * 0.5f;
 	halfpixely=pixely * 0.5f;
	//TMU 0
	GL_SelectTexture( 0 );
	qglEnable(GL_TEXTURE_2D);
	GL_State(GLS_DEFAULT);
	
	qglViewport(0, 0, dest->x, dest->y);
	qglMatrixMode(GL_PROJECTION);
	qglLoadIdentity();
	qglMatrixMode(GL_MODELVIEW);
	qglLoadIdentity();
	GL_BindTexnum(source);
	//	qglBindTexture(GL_TEXTURE_2D, source);

	if (!fastmode)
	{
		if (hdrclip)
		{
			qglUseProgramObjectARB(tr.hdrcull_fragshader.program);
			loc = qglGetUniformLocationARB(tr.hdrcull_fragshader.program, "hipass");
			qglUniform1fARB(loc, HdrCut);
		}

		// Perform the Downsample into dest 
		// downsample is offset by a halfpixel to get a free blin blur
		
		RB_SetFrameBuffer(dest);
		
		qglBegin(GL_TRIANGLE_STRIP);
			qglTexCoord2f(0+halfpixelx, 0+halfpixely);
			qglVertex3f(-1, -1, 0);
		
			qglTexCoord2f(0+halfpixelx, 1+halfpixely);
			qglVertex3f(-1, 1, 0);
	 	
			qglTexCoord2f(1+halfpixelx, 0+halfpixely);
			qglVertex3f(1, -1, 0);
		
			qglTexCoord2f(1+halfpixelx, 1+halfpixely);
			qglVertex3f(1, 1, 0);
		qglEnd();

		RB_SetFrameBuffer(0);
		if (hdrclip)
		{
			qglUseProgramObjectARB(0);
		}
	}

	// Set up the filter.
 	qglUseProgramObjectARB(fragshader);
	loc = qglGetUniformLocationARB(fragshader, "source");
	qglUniform1iARB(loc, 0);

	//draw the source image on the X axis
	RB_SetFrameBuffer(tempx);
	loc = qglGetUniformLocationARB(fragshader, "offsetx");
	offset = pixelx;
	qglUniform1fARB(loc, offset);

	loc = qglGetUniformLocationARB(fragshader, "offsety");
	qglUniform1fARB(loc, 0);

	//qglBindTexture(GL_TEXTURE_2D, dest->texnum);
	GL_BindTexnum(dest->texnum);

	qglBegin(GL_TRIANGLE_STRIP);
		qglTexCoord2f(0, 0);
		qglVertex3f(-1, -1, 0);
		
		qglTexCoord2f(0, 1);
		qglVertex3f(-1, 1, 0);
	 	
		qglTexCoord2f(1, 0);
		qglVertex3f(1, -1, 0);
		
		qglTexCoord2f(1, 1);
		qglVertex3f(1, 1, 0);
		
	qglEnd();
	RB_SetFrameBuffer(0);


	//Now take what we just blurred on the x, and blur it on the Y
	RB_SetFrameBuffer(dest);  
	loc = qglGetUniformLocationARB(fragshader, "offsety");
	offset = pixely;
	qglUniform1fARB(loc, offset);

	loc = qglGetUniformLocationARB(fragshader, "offsetx");
	qglUniform1fARB(loc, 0);

	//qglBindTexture(GL_TEXTURE_2D, tempx->texnum);
	GL_BindTexnum( tempx->texnum);

	qglBegin(GL_TRIANGLE_STRIP);
		qglTexCoord2f(0, 0);
		qglVertex3f(-1, -1, 0);
		
		qglTexCoord2f(0, 1);
		qglVertex3f(-1, 1, 0);
	 	
		qglTexCoord2f(1, 0);
		qglVertex3f(1, -1, 0);
		
		qglTexCoord2f(1, 1);
		qglVertex3f(1, 1, 0);
	qglEnd();
		
	RB_SetFrameBuffer(0);

	qglUseProgramObjectARB(0);

	GL_SelectTexture( 1 ); 
	qglDisable( GL_TEXTURE_2D );

	GL_SelectTexture( 0 ); //leave it on 0

}


static float RB_DepthToLinear( float input )
{
	float fognearfar = tr.viewParms.zFar/r_znear->value;
	return  input / (fognearfar - (fognearfar - 1) * input);
}


static float RB_GetAutoFocus( void )
{
	int x,y;
	float dc = 0.0f;

	x = backEnd.refdef.width/2;
	y = backEnd.refdef.height/2;

/*
	float depth[15];
	int counter = 0;
	int j;

	//do crosshair
	qglReadPixels( x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth[counter++] );

	//weight crosshair more
	int xx,yy;
	depth[counter++]=depth[0];
	depth[counter++]=depth[0];
	depth[counter++]=depth[0];
	depth[counter++]=depth[0];


	xx=backEnd.refdef.width/60;
	yy=backEnd.refdef.height/60;

	xx2=backEnd.refdef.width/120;
	yy2=backEnd.refdef.height/120;

	//topleft
	qglReadPixels( x-xx2, y-yy2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth[counter++] );
	//topright
	qglReadPixels( x+xx2, y-yy2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth[counter++] );
	//botleft
	qglReadPixels( x-xx2, y+yy2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth[counter++] );
	//botright
	qglReadPixels( x+xx2, y+yy2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth[counter++] );

	//top
	qglReadPixels( x, y-yy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth[counter++] );
	//right
	qglReadPixels( x+xx, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth[counter++] );
	//left
	qglReadPixels( x-xx, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth[counter++] );
	//bot
	qglReadPixels( x, y+yy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth[counter++] );

	for ( j=0 ; j<counter ; j++ );
		dc+=depth[j];

	dc/=counter;
*/

	qglReadPixels( x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &dc );
	
	tr.DOF_focus = Com_Clamp( 0, 0.99999f, dc*r_dofblend->value + tr.DOF_focus*(1.0f-r_dofblend->value) );

	return tr.DOF_focus;

}


void RB_PostProcess()
{
	int loc;
	float offset;
	float HdrCut=0.0f;
	float AddBlend = 0.0f;
	float pixelx=1.0f/backEnd.refdef.width;
	float pixely=1.0f/backEnd.refdef.height;
	float nearfocus =0.0f;
	float farfocus  =1.1f;
	float focaldepth=0.5f;
	unsigned int postshader=0;
	float dofscale=  r_dofscale->integer;
	

	qboolean depth = qfalse;
	qboolean color = qfalse;

	if (dofscale>5.0f) dofscale=5.0f;

	if (backEnd.refdef.rdflags & RDF_NOEFFECTS ) return;
	if (!r_ext_vertex_shader->integer) return;
	if (r_simpleshaders->integer>0) return;

	depth = ( ( r_fogdensity->value > 0 ) || r_dof->integer );
	color = ( depth || r_bloom->integer );
	
	if( !depth && !color)
		return;

	if (r_dof->integer) 
	{
		// calculate focus
		focaldepth = RB_DepthToLinear(RB_GetAutoFocus());


		// convert focal depth into world range
		nearfocus = focaldepth - (2500/  tr.viewParms.zFar);
			
		// farfocus is easier..
		farfocus = focaldepth + (2500/  tr.viewParms.zFar);

		// use the big kahuna	
		postshader= tr.bloom_fog_dof_fragshader.program;
	}
	else
	{
		if ( r_fogdensity->value > 0 )
		{
			// just bloom and fog
			postshader= tr.bloom_fog_fragshader.program;
		}
		else
		{
			// just bloom
			postshader= tr.bloom_fragshader.program;
		}
	}

	//Set states
	GL_State( GLS_DEPTHTEST_DISABLE );

	//Turn off extra tmus
	GL_SelectTexture(1);
	qglDisable( GL_TEXTURE_2D );

	GL_SelectTexture(2);
	qglDisable( GL_TEXTURE_2D );

	GL_SelectTexture(3);
	qglDisable( GL_TEXTURE_2D );


	//Copy the depth texture
	if ( depth )
	{
		GL_SelectTexture(0);
		qglEnable(GL_TEXTURE_2D);
		GL_BindTexnum(tr.depthTexnum);
		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, glConfig.vidWidth, glConfig.vidHeight);
		
		qglViewport(0, 0, glConfig.vidWidth,glConfig.vidHeight);
		qglMatrixMode(GL_PROJECTION);
		qglLoadIdentity();
		qglMatrixMode(GL_MODELVIEW);
		qglLoadIdentity();
	
	}

	//Copy scene
	if ( color )
	{
		GL_SelectTexture(0);
		GL_BindTexnum(tr.sceneTexnum);
		qglCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0,0,0, glConfig.vidWidth, glConfig.vidHeight ); 
	}
	
	//Turn off extra tmus
	GL_SelectTexture(1);
	qglDisable( GL_TEXTURE_2D );

	GL_SelectTexture(2);
	qglDisable( GL_TEXTURE_2D );

	GL_SelectTexture(3);
	qglDisable( GL_TEXTURE_2D );

	if ( r_bloom->integer ) 
	{
		switch ( r_bloom->integer )
		{
			case 0:
				HdrCut=0.0;
				AddBlend=0.0;
			break;
			
			case 1:
				HdrCut=0.93f;
				AddBlend = 1.5f;
			break;
			
			case 2:
				HdrCut=0.60f;
				AddBlend = 0.4f;
			break;

			case 3:
				//Shiney highlights
				HdrCut=0.5f;
				AddBlend=1.0f;
			break;

			case 4:
				//Soft Light/haze
				HdrCut=0;
				AddBlend=0.33f;
			break;

			case 5:
				//EyeSear
				HdrCut=0.25f;
				AddBlend=1.0f;
			break;
		}

	   
		//build bloom
		RB_Blur(tr.sceneTexnum,  &tr.bloomTempX,   &tr.bloomTexture ,qtrue,HdrCut);
		RB_Blur(tr.bloomTexture.texnum,  &tr.bloomTempX,  &tr.bloomTexture ,qfalse,0);
	}


//	if (r_dof->integer>0)
//	{
//		RB_DofBlur(tr.sceneTexnum,  &tr.dofTempX,   &tr.dofTexture ,qfalse,0);	
//		RB_DofBlur(tr.dofTexture.texnum,  &tr.dofTempX,   &tr.dofTexture ,qfalse,0);	
//	}
	
	
	//RB_Blur(tr.dofTexture.texnum,  &tr.bloomTempX,  &tr.dofTexture ,qfalse,0);//slightly higher quality (less pixely)
			
	// we're in 2d mode but need to set the viewport up

	qglMatrixMode(GL_PROJECTION);
	qglLoadIdentity();
	qglMatrixMode(GL_MODELVIEW);
	qglLoadIdentity();

	GL_SelectTexture(1);
	qglEnable(GL_TEXTURE_2D);

	GL_SelectTexture(2);
	qglEnable(GL_TEXTURE_2D);

	GL_SelectTexture(3);
	qglEnable(GL_TEXTURE_2D);

	qglUseProgramObjectARB(postshader);

	loc = qglGetUniformLocationARB(postshader, "scene");
	qglUniform1iARB(loc, 0);
	loc = qglGetUniformLocationARB(postshader, "bloom");
	qglUniform1iARB(loc, 1);
	loc = qglGetUniformLocationARB(postshader, "depth");
	qglUniform1iARB(loc, 3);
	
	loc = qglGetUniformLocationARB(postshader, "blend");
	offset =AddBlend;
	qglUniform1fARB(loc, offset);
	
	loc = qglGetUniformLocationARB(postshader, "offsetx");
	qglUniform1fARB(loc, pixelx);
	loc = qglGetUniformLocationARB(postshader, "offsety");
	qglUniform1fARB(loc, pixely);

	loc = qglGetUniformLocationARB(postshader, "focalplane");
	qglUniform1fARB(loc,  focaldepth );
	loc = qglGetUniformLocationARB(postshader, "focalnearplane");
	qglUniform1fARB(loc,  nearfocus );
	loc = qglGetUniformLocationARB(postshader, "focalfarplane");
	qglUniform1fARB(loc,  farfocus );
	
	loc = qglGetUniformLocationARB(postshader, "dofscale");
	
	qglUniform1fARB(loc,  dofscale );

	loc = qglGetUniformLocationARB(postshader, "fognearfar");
	qglUniform1fARB(loc,  tr.viewParms.zFar/r_znear->value );

	loc = qglGetUniformLocationARB(postshader, "fogdensity");
	qglUniform1fARB(loc,  r_fogdensity->value);

	loc = qglGetUniformLocationARB(postshader, "fogcolor");
	qglUniform3fARB(loc, r_fog_r->value, r_fog_g->value, r_fog_b->value);

	loc = qglGetUniformLocationARB(postshader, "zfar");
	qglUniform1fARB(loc, tr.viewParms.zFar);

	loc = qglGetUniformLocationARB(postshader, "inv_fp_sub_fnp");
	qglUniform1fARB(loc, 1.0 / (focaldepth - nearfocus));

	loc = qglGetUniformLocationARB(postshader, "inv_ffp_sub_fp");
	qglUniform1fARB(loc, 1.0 / (farfocus-focaldepth ));

	qglViewport(0, 0, glConfig.vidWidth,glConfig.vidHeight);
	GL_State( GLS_DEPTHTEST_DISABLE );

	GL_SelectTexture(0);
	GL_BindTexnum(  tr.sceneTexnum);

	GL_SelectTexture(1);
	GL_BindTexnum( tr.bloomTexture.texnum);

	GL_SelectTexture(3);
	GL_BindTexnum( tr.depthTexnum);

	qglBegin(GL_TRIANGLE_STRIP);
	qglTexCoord2f(0, 0);
	qglVertex3f(-1, -1, 0);
			
	qglTexCoord2f(0, 1);
	qglVertex3f(-1, 1, 0);
 	
	qglTexCoord2f(1, 0);
	qglVertex3f(1, -1, 0);
	
	qglTexCoord2f(1, 1);
	qglVertex3f(1, 1, 0);

	qglEnd();
		
	qglUseProgramObjectARB(0); 

	//Turn off extra tmus
	GL_SelectTexture(1);
	qglDisable( GL_TEXTURE_2D );
	
	GL_SelectTexture(2);
	qglDisable( GL_TEXTURE_2D );

	GL_SelectTexture(3);
	qglDisable( GL_TEXTURE_2D );


	GL_SelectTexture(0);

	//Done with 2d stuff for now
	SetViewportAndScissor();

	//because we've done manual texture setting, let q3 know
	//Com_Memset( glState.currenttextures, 0, sizeof( glState.currenttextures ) );
}

static const byte *R_ClusterPVS (int cluster) {
	if (!tr.world || !tr.world->vis || cluster < 0 || cluster >= tr.world->numClusters ) {
		return tr.world->novis;
	}

	return tr.world->vis + cluster * tr.world->clusterBytes;
}



void R_BuildCubeMaps( void ) 
{
	int i,j;
	int ii,jj;
	int cl;
	refdef_t rf;
	qboolean flipx;
	qboolean flipy;
	int x,y,xy,xy2;
	char value[MAX_TOKEN_CHARS];

	byte temp[128*128*4];
	byte *dest;
	image_t *cubemap = 0;

	const byte	*vis;
	mnode_t	*leaf, *parent;
	int		cluster;
	vec3_t	mins,maxs;
	int counter=0;
	
	int distance=512;
	vec3_t origin;
	qboolean bad;
	srfSurfaceStatic_t *sv;

	if( !r_ext_vertex_shader->integer || r_simpleshaders->integer ) 
		return;

	memset(&rf,0,sizeof(refdef_t));
	
	//Calculate origins for our probes
	tr.cubeProbesCount=0;

	//Add all the entity ones
	counter=0;
	while  ( R_ParseEntitysForKey("cubemap_probe","origin",&value[0],MAX_TOKEN_CHARS,counter++ ) )
	{
		if (tr.cubeProbesCount<MAX_PROBES-1)
		{
			sscanf(&value[0], "%f %f %f",&origin[0],&origin[1],&origin[2]);
			tr.cubeProbes[tr.cubeProbesCount].origin[0]=origin[0];
			tr.cubeProbes[tr.cubeProbesCount].origin[1]=origin[1];
			tr.cubeProbes[tr.cubeProbesCount].origin[2]=origin[2];
			tr.cubeProbesCount++;
		}
	}


	if (tr.world->vis) 
	{
		for (cl=0;cl<tr.world->numClusters;cl++)
		{
			vis = R_ClusterPVS (cl);
			
			mins[0]=mins[1]=mins[2]=999999;
			maxs[0]=maxs[1]=maxs[2]=-999999;

			for (i=0,leaf=tr.world->nodes ; i<tr.world->numnodes ; i++, leaf++) 
			{
				cluster = leaf->cluster;
				if ( cluster < 0 || cluster >= tr.world->numClusters ) {
					continue;
				}
				// check general pvs
				if ( !(vis[cluster>>3] & (1<<(cluster&7))) ) {
					continue;
				}

				parent = leaf;
				//Mark the leaf
				//if (leafcount<1024)	Leafs[leafcount++]=parent;
				
				for (j=0;j<3;j++)
				{
					if (parent->mins[j]<mins[j]) mins[j]=parent->mins[j];
					if (parent->maxs[j]>maxs[j]) maxs[j]=parent->maxs[j];
				}
			
				
			}
			
			origin[0]=(mins[0]+maxs[0])/2 ;
			origin[1]=(mins[1]+maxs[1])/2 ;
			origin[2]=(mins[2]+maxs[2])/2 ;

			bad=qfalse;
			
			//Check to see if this is a shit location
			if (CM_PointContents(origin,0)==CONTENTS_SOLID) continue; 

			
			//check to see if we're too close to an existing probe
			for (i=0;i<tr.cubeProbesCount;i++)
			{
				vec3_t pos;
				pos[0]=tr.cubeProbes[i].origin[0]-origin[0];
				pos[1]=tr.cubeProbes[i].origin[1]-origin[1];
				pos[2]=tr.cubeProbes[i].origin[2]-origin[2];

				if (VectorLength(pos)<distance) 
				{
					bad=qtrue;			
					break;
				}
			}
			if (bad==qfalse)
			{
				tr.cubeProbes[tr.cubeProbesCount].origin[0]=origin[0];
				tr.cubeProbes[tr.cubeProbesCount].origin[1]=origin[1];
				tr.cubeProbes[tr.cubeProbesCount].origin[2]=origin[2];
				tr.cubeProbesCount++;
				if (tr.cubeProbesCount>=MAX_PROBES) break;
				
			}
		}
	}


	//If we can't find one, fake one
	if (tr.cubeProbesCount==0)
	{
		tr.cubeProbes[tr.cubeProbesCount].origin[0]=0;
		tr.cubeProbes[tr.cubeProbesCount].origin[1]=0;
		tr.cubeProbes[tr.cubeProbesCount].origin[2]=0;
		tr.cubeProbesCount++;
	}


	ri.Printf( PRINT_ALL, "R_VBO Cube ProbeCount: %d\n",tr.cubeProbesCount );	 
	for (j=0;j<tr.cubeProbesCount;j++)
	{
	
		VectorSet(rf.vieworg,tr.cubeProbes[j].origin[0],tr.cubeProbes[j].origin[1],tr.cubeProbes[j].origin[2]);
		
		AxisClear( rf.viewaxis );

		rf.fov_x = 90;
		rf.fov_y = 90;

		rf.x = 0;
		rf.y = 0;
		rf.width = REF_CUBEMAP_SIZE;
		rf.height = REF_CUBEMAP_SIZE;
		rf.time = 0;

		rf.rdflags = RDF_NOEFFECTS | RDF_NOCUBEMAP | RDF_NODISPLAY ;

		for (i=0;i<6;i++)
		{
			flipx=qfalse;
			flipy=qfalse;
			switch (i)
			{

				case 0:		
				{
					//X+
					rf.viewaxis[0][0]=1;
					rf.viewaxis[0][1]=0;
					rf.viewaxis[0][2]=0;

					rf.viewaxis[1][0]=0;
					rf.viewaxis[1][1]=0;
					rf.viewaxis[1][2]=1;

					CrossProduct(rf.viewaxis[0],rf.viewaxis[1],rf.viewaxis[2]);
					//flipx=qtrue;
					break;
				}
				case 1:		
				{
					//X-
					rf.viewaxis[0][0]=-1;
					rf.viewaxis[0][1]=0;
					rf.viewaxis[0][2]=0;

					rf.viewaxis[1][0]=0;
					rf.viewaxis[1][1]=0;
					rf.viewaxis[1][2]=-1;

					CrossProduct(rf.viewaxis[0],rf.viewaxis[1],rf.viewaxis[2]);
					//flipx=qtrue;
					break;
				}
				case 2:		
				{
					//Y+
					rf.viewaxis[0][0]=0;
					rf.viewaxis[0][1]=1;
					rf.viewaxis[0][2]=0;

					rf.viewaxis[1][0]=-1;
					rf.viewaxis[1][1]=0;
					rf.viewaxis[1][2]=0;

					CrossProduct(rf.viewaxis[0],rf.viewaxis[1],rf.viewaxis[2]);
					//flipx=qtrue;
					break;
				}
				case 3:		
				{
					//Y-
					rf.viewaxis[0][0]=0;
					rf.viewaxis[0][1]=-1;
					rf.viewaxis[0][2]=0;

					rf.viewaxis[1][0]=-1;//-1
					rf.viewaxis[1][1]=0;
					rf.viewaxis[1][2]=0;

					CrossProduct(rf.viewaxis[0],rf.viewaxis[1],rf.viewaxis[2]);
					//flipx=qtrue;
					break;
				}
				case 4:		
				{
					//Z+
					rf.viewaxis[0][0]=0;
					rf.viewaxis[0][1]=0;
					rf.viewaxis[0][2]=1;

					rf.viewaxis[1][0]=-1;
					rf.viewaxis[1][1]=0;
					rf.viewaxis[1][2]=0;

					CrossProduct(rf.viewaxis[0],rf.viewaxis[1],rf.viewaxis[2]);
					//	flipx=qtrue;
					break;
				}
				case 5:		
				{
					//Z-
					rf.viewaxis[0][0]=0;
					rf.viewaxis[0][1]=0;
					rf.viewaxis[0][2]=-1;

					rf.viewaxis[1][0]=1;
					rf.viewaxis[1][1]=0;
					rf.viewaxis[1][2]=0;

					CrossProduct(rf.viewaxis[0],rf.viewaxis[1],rf.viewaxis[2]);
					//flipx=qtrue;
					break;
				}

			}
			
			tr.refdef.pixelTarget = tr.cubeTemp[i];
			memset(tr.cubeTemp[i],255,REF_CUBEMAP_SIZE*REF_CUBEMAP_SIZE*4);
			tr.refdef.pixelTargetX=REF_CUBEMAP_SIZE;
			tr.refdef.pixelTargetY=REF_CUBEMAP_SIZE;

			RE_BeginFrame(STEREO_CENTER);
			RE_RenderScene(&rf);
			RE_EndFrame(&ii,&jj);

			if (flipx)
			{
				dest=tr.cubeTemp[i];
				memcpy(temp,dest,REF_CUBEMAP_SIZE*REF_CUBEMAP_SIZE*4);

				for (y=0;y<REF_CUBEMAP_SIZE;y++)
				{
					for (x=0;x<REF_CUBEMAP_SIZE;x++)
					{
						xy=((y*REF_CUBEMAP_SIZE)+x)*4;
						xy2=((y*REF_CUBEMAP_SIZE)+((REF_CUBEMAP_SIZE-1)-x))*4;
						dest[xy2+0]=temp[xy+0];
						dest[xy2+1]=temp[xy+1];
						dest[xy2+2]=temp[xy+2];
						dest[xy2+3]=temp[xy+3];
					
					}
				}
			}
			if (flipy)
			{
				dest=tr.cubeTemp[i];
				memcpy(temp,dest,REF_CUBEMAP_SIZE*REF_CUBEMAP_SIZE*4);

				for (y=0;y<REF_CUBEMAP_SIZE;y++)
				{
					for (x=0;x<REF_CUBEMAP_SIZE;x++)
					{
						xy=((y*REF_CUBEMAP_SIZE)+x)*4;
						xy2=((((REF_CUBEMAP_SIZE-1)-y)*REF_CUBEMAP_SIZE)+x)*4;
						dest[xy2+0]=temp[xy+0];
						dest[xy2+1]=temp[xy+1];
						dest[xy2+2]=temp[xy+2];
						dest[xy2+3]=temp[xy+3];
					
					}
				}
			}
	


			// encode the pixel intensity into the alpha channel, saves work in the shader
			if (1)
			{
				byte r,g,b;
				int lum,realmax,realmin;
				dest=tr.cubeTemp[i];
				for (y=0;y<REF_CUBEMAP_SIZE;y++)
				{
					for (x=0;x<REF_CUBEMAP_SIZE;x++)
					{
						xy=((y*REF_CUBEMAP_SIZE)+x)*4;
						
						r=dest[xy+0];
						g=dest[xy+1];
						b=dest[xy+2];
						
						//calculate luminosity as  (max+min) /2 
						lum = MAX(r, g);
						realmax = MAX(b, lum);

						lum = MIN(r, g);
						realmin = MIN( b, lum);

						lum =  ( realmax + realmin ) * 0.5f;

						dest[xy+3]=lum;
					
					}
				}
			}
		

		}

		// build the cubemap
		for (i=0;i<6;i++)
		{
			
			if (i==0)
			{
				cubemap = R_CreateCubeImage( "*testcube",(byte *)tr.cubeTemp[i], REF_CUBEMAP_SIZE,REF_CUBEMAP_SIZE, qfalse, qfalse, GL_CLAMP_TO_EDGE, 1, 0 );
			}
			else
			{
				image_t *result;
				result = R_CreateCubeImage( "*testcube",(byte *)tr.cubeTemp[i], REF_CUBEMAP_SIZE, REF_CUBEMAP_SIZE, qfalse, qfalse, GL_CLAMP_TO_EDGE, i, cubemap );
			}
		}

		tr.cubeProbes[j].cubemap = cubemap;
	
	}

	// turn pixel targets off
	tr.refdef.pixelTarget = 0;

	// assign the surfs a cubemap
	for (i=0 ; i<tr.world->numnodes ; i++) 
	{
		msurface_t	 **mark;
		msurface_t	*surf;

		if (tr.world->nodes[i].contents != CONTENTS_SOLID) 
		{
			mark = tr.world->nodes[i].firstmarksurface;
			j =  tr.world->nodes[i].nummarksurfaces;
			while (j--) 
			{
				int dist=9999999;
				int best=0;

				surf = *mark;
				mark++;
				sv=(void*)surf->data;
				if( sv->surfaceType != SF_STATIC ||
					sv->numIndices == 0 ||
					sv->numVerts == 0 ||
					sv->cubemap )
					continue;
				
				for (x=0;x<tr.cubeProbesCount;x++)
				{
					vec3_t pos;
					pos[0]=tr.cubeProbes[x].origin[0]-sv->origin[0];
					pos[1]=tr.cubeProbes[x].origin[1]-sv->origin[1];
					pos[2]=tr.cubeProbes[x].origin[2]-sv->origin[2];

					distance=VectorLength(pos);
					if (distance<dist) 
					{
						dist=distance;
						best=x;
					}
				}
				sv->cubemap=tr.cubeProbes[best].cubemap;


			}
		}
	}

	// enable display again
	tr.refdef.rdflags &= ~RDF_NODISPLAY;
}

cubemapProbe_t *R_FindBestCubeMap(vec3_t origin)
{
	int dist=9999999;
	int best=0;
	int x;
	float distance;
	for (x=0;x<tr.cubeProbesCount;x++)
	{
		vec3_t pos;
		pos[0]=tr.cubeProbes[x].origin[0]-origin[0];
		pos[1]=tr.cubeProbes[x].origin[1]-origin[1];
		pos[2]=tr.cubeProbes[x].origin[2]-origin[2];

		distance=VectorLengthSquared(pos);
		if (distance<dist) 
		{
			dist=distance;
			best=x;
		}
	}
	return &tr.cubeProbes[best];
}

//finds the two nearest cubemaps and a fractional value between them

float R_FindBestTwoCubeMaps(vec3_t origin,cubemapProbe_t **A, cubemapProbe_t **B)
{
	int dist=9999999;
	int best=0;
	int secondBest=0;
	vec3_t pos;
	int x;
	float distance;
	float blendA,blendB;
	for (x=0;x<tr.cubeProbesCount;x++)
	{
	
		pos[0]=tr.cubeProbes[x].origin[0]-origin[0];
		pos[1]=tr.cubeProbes[x].origin[1]-origin[1];
		pos[2]=tr.cubeProbes[x].origin[2]-origin[2];

		distance=VectorLengthSquared(pos);
		if (distance<dist) 
		{
			dist=distance;
			secondBest=best;
			best=x;
		}
	}


	pos[0]=tr.cubeProbes[best].origin[0]-tr.cubeProbes[secondBest].origin[0];
	pos[1]=tr.cubeProbes[best].origin[1]-tr.cubeProbes[secondBest].origin[1];
	pos[2]=tr.cubeProbes[best].origin[2]-tr.cubeProbes[secondBest].origin[2];
	distance=VectorLength(pos);
	
	*A=&tr.cubeProbes[best];
	*B=&tr.cubeProbes[secondBest];
	
	pos[0]=tr.cubeProbes[best].origin[0]-origin[0];
	pos[1]=tr.cubeProbes[best].origin[1]-origin[1];
	pos[2]=tr.cubeProbes[best].origin[2]-origin[2];
	blendA=VectorLength(pos);
	if (blendA==0) blendA=1;

	pos[0]=tr.cubeProbes[secondBest].origin[0]-origin[0];
	pos[1]=tr.cubeProbes[secondBest].origin[1]-origin[1];
	pos[2]=tr.cubeProbes[secondBest].origin[2]-origin[2];
	blendB=VectorLength(pos);
	if (blendB==0) blendB=1;

	
	return 0;

}


qboolean R_LoadWorldFragShader(fragShader_t *frag, char* vertexprog, char* fragprog)
{

	qboolean status;
	char buffer[8096];
	int size;
	int uniform_location;
	int texnum;
	
	R_LoadFrag(frag,vertexprog,fragprog);
		
	//Program creation + bindings 
	frag->program = qglCreateProgramObjectARB();

	qglAttachObjectARB(frag->program, frag->vertexShader);
	qglAttachObjectARB(frag->program, frag->fragShader);

	qglLinkProgramARB( frag->program);    
	
	qglGetObjectParameterivARB(frag->program, GL_OBJECT_LINK_STATUS_ARB, (GLint*)&status);
	qglUseProgramObjectARB(frag->program);
			
	if (!status)
	{
		 qglGetInfoLogARB(frag->program,sizeof(buffer),&size, &buffer[0]);
		 ri.Error (ERR_DROP, "R_LoadFragmentPrograms:%s LinkStatus %s",fragprog, buffer);
		 return qfalse;
	}

	//Get vertex binds
	frag->uv_i=qglGetAttribLocationARB(frag->program, "IN_uvs");
	frag->position_i=qglGetAttribLocationARB(frag->program, "IN_position");
	frag->light_uv_i=qglGetAttribLocationARB(frag->program, "IN_light_uvs");
	frag->normal_i=qglGetAttribLocationARB(frag->program, "IN_normal");
	frag->stv_i=qglGetAttribLocationARB(frag->program, "IN_stv");
	frag->ttv_i=qglGetAttribLocationARB(frag->program, "IN_ttv");
	frag->rgba_i=qglGetAttribLocationARB(frag->program, "IN_rgba");

	frag->vertexType=1;
	//md5 specific
	//See if we're md5 or not
/*	frag->boneweights_i=qglGetAttribLocationARB(frag->program, "IN_boneweights");
	if (frag->boneweights_i>-1)
	{
		frag->boneindexs_i=qglGetAttribLocationARB(frag->program, "IN_boneindexs");
		frag->pos0_i=qglGetAttribLocationARB(frag->program, "IN_pos0");
		frag->pos1_i=qglGetAttribLocationARB(frag->program, "IN_pos1");
		frag->pos2_i=qglGetAttribLocationARB(frag->program, "IN_pos2");
		frag->norm0_i=qglGetAttribLocationARB(frag->program, "IN_norm0");
		frag->norm1_i=qglGetAttribLocationARB(frag->program, "IN_norm1");
		frag->norm2_i=qglGetAttribLocationARB(frag->program, "IN_norm2");
		frag->tangent0_i=qglGetAttribLocationARB(frag->program, "IN_tangent0");
		frag->tangent1_i=qglGetAttribLocationARB(frag->program, "IN_tangent1");
		frag->tangent2_i=qglGetAttribLocationARB(frag->program, "IN_tangent2");
		frag->bitangent0_i=qglGetAttribLocationARB(frag->program, "IN_bitangent0");
		frag->bitangent1_i=qglGetAttribLocationARB(frag->program, "IN_bitangent1");
		frag->bitangent2_i=qglGetAttribLocationARB(frag->program, "IN_bitangent2");
		
		frag->vertexType=2;
	}*/
	
	

	qglUseProgramObjectARB(frag->program);
	
	texnum=0;

	//diffuse textures
	frag->diffuse0 = uniform_location = qglGetUniformLocationARB(frag->program, "diffuseMap");
	if (uniform_location!=-1) qglUniform1iARB(uniform_location, texnum++);

	frag->normal0 = uniform_location = qglGetUniformLocationARB(frag->program, "normalMap");
	if (uniform_location!=-1) qglUniform1iARB(uniform_location, texnum++);

	frag->bump0 = uniform_location = qglGetUniformLocationARB(frag->program, "bumpmap");
	if (uniform_location!=-1) qglUniform1iARB(uniform_location, texnum++);


	//secondary diffuse textures
	frag->diffuse1 = uniform_location = qglGetUniformLocationARB(frag->program, "diffuseMap1");
	if (uniform_location!=-1) qglUniform1iARB(uniform_location, texnum++);

	frag->normal1 = uniform_location = qglGetUniformLocationARB(frag->program, "normalMap1");
	if (uniform_location!=-1) qglUniform1iARB(uniform_location, texnum++);

	frag->bump1 = uniform_location = qglGetUniformLocationARB(frag->program, "bumpmap1");
	if (uniform_location!=-1) qglUniform1iARB(uniform_location, texnum++);


	//lightmaps
	frag->lightmap0 = uniform_location = qglGetUniformLocationARB(frag->program, "lightmap");
	if (uniform_location!=-1) qglUniform1iARB(uniform_location, texnum++);

	//radbump textures
	frag->radbump0 = uniform_location = qglGetUniformLocationARB(frag->program, "lightMap_0");
	if (uniform_location!=-1) qglUniform1iARB(uniform_location, texnum++);
	frag->radbump1 = uniform_location = qglGetUniformLocationARB(frag->program, "lightMap_1");
	if (uniform_location!=-1) qglUniform1iARB(uniform_location, texnum++);
	frag->radbump2 = uniform_location = qglGetUniformLocationARB(frag->program, "lightMap_2");
	if (uniform_location!=-1) qglUniform1iARB(uniform_location, texnum++);
	frag->specbump = uniform_location = qglGetUniformLocationARB(frag->program, "specMap");
	if (uniform_location!=-1) qglUniform1iARB(uniform_location, texnum++);

	//cubemaps
	frag->cubemap = uniform_location = qglGetUniformLocationARB(frag->program, "cubeMap");
	if (uniform_location!=-1) qglUniform1iARB(uniform_location, texnum++);
	frag->cubemap2 = uniform_location = qglGetUniformLocationARB(frag->program, "cubeMap2");
	if (uniform_location!=-1) qglUniform1iARB(uniform_location, texnum++);

	//reflect and refract for water
	frag->refractmap = uniform_location = qglGetUniformLocationARB(frag->program, "refractMap");
	if (uniform_location!=-1) qglUniform1iARB(uniform_location, texnum++);

	frag->reflectmap = uniform_location = qglGetUniformLocationARB(frag->program, "reflectMap");
	if (uniform_location!=-1) qglUniform1iARB(uniform_location, texnum++);
	
	//scene texture
	frag->scenetexture = uniform_location = qglGetUniformLocationARB(frag->program, "scenetexture");
	if (uniform_location!=-1) qglUniform1iARB(uniform_location, texnum++);

	qglUseProgramObjectARB(0);

	return qtrue;
}

