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
// tr_map.c

#include "tr_local.h"

/*


Loads and prepares a map file for scene rendering.

A single entry point:

void RE_LoadWorldMap( const char *name );

*/

static	world_t		s_worldData;
static	byte		*fileBase;

int			c_subdivisions;
int			c_gridVerts;


srfSurfaceDecal_t* TempDecalStore;
int			numTempDecalStore;

static void R_CalculateTangents( void );
static void R_OptimizeSurfaces_bySurface( void );


//===============================================================================

/*
==============
R_InitVBO
==============
*/
void	R_InitVBO( void ) 
{
	//27:  This should probably be relocated

	//tr.staticIndexData;
	//tr.staticVertexData;
	if (qglGenBuffersARB==NULL) return;

	qglGenBuffersARB(1, &tr.VBO_indexBuffer);
	qglGenBuffersARB(1, &tr.VBO_vertexBuffer);
	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, tr.VBO_vertexBuffer);
	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, tr.VBO_indexBuffer);
	qglBufferDataARB(GL_ARRAY_BUFFER_ARB, (GLsizeiptrARB)(tr.numStaticVertex * sizeof(staticVert_t)),tr.staticVertexData, GL_STATIC_DRAW);
	
	qglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, (GLsizeiptrARB)(tr.numStaticIndex * sizeof(unsigned int)), tr.staticIndexData, GL_STATIC_DRAW);
	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}

static void HSVtoRGB( float h, float s, float v, float rgb[3] )
{
	int i;
	float f;
	float p, q, t;

	h *= 5;

	i = floor( h );
	f = h - i;

	p = v * ( 1 - s );
	q = v * ( 1 - s * f );
	t = v * ( 1 - s * ( 1 - f ) );

	switch ( i )
	{
	case 0:
		rgb[0] = v;
		rgb[1] = t;
		rgb[2] = p;
		break;
	case 1:
		rgb[0] = q;
		rgb[1] = v;
		rgb[2] = p;
		break;
	case 2:
		rgb[0] = p;
		rgb[1] = v;
		rgb[2] = t;
		break;
	case 3:
		rgb[0] = p;
		rgb[1] = q;
		rgb[2] = v;
		break;
	case 4:
		rgb[0] = t;
		rgb[1] = p;
		rgb[2] = v;
		break;
	case 5:
		rgb[0] = v;
		rgb[1] = p;
		rgb[2] = q;
		break;
	}
}

/*
===============
R_ColorShiftLightingBytes

===============
*/
static	void R_ColorShiftLightingBytes( byte in[4], byte out[4] ) {
	int		shift, r, g, b;

	// shift the color data based on overbright range
	shift = r_mapOverBrightBits->integer - tr.overbrightBits;

	// shift the data based on overbright range
	r = in[0] << shift;
	g = in[1] << shift;
	b = in[2] << shift;
	
	// normalize by color instead of saturating to white
	if ( ( r | g | b ) > 255 ) {
		int		max;

		max = r > g ? r : g;
		max = max > b ? max : b;
		r = r * 255 / max;
		g = g * 255 / max;
		b = b * 255 / max;
	}

	out[0] = r;
	out[1] = g;
	out[2] = b;
	out[3] = in[3];
}

/*
===============
R_LoadLightmaps

===============
*/
#define	LIGHTMAP_SIZE	128

static	void R_LoadLightmaps( lump_t *l ) {
	byte		*buf, *buf_p;
	int			len;
	byte		image[LIGHTMAP_SIZE*LIGHTMAP_SIZE*4];
	int			i, j;
	float maxIntensity = 0;
	double sumIntensity = 0;

	len = l->filelen;
	if ( !len ) {
		return;
	}
	buf = fileBase + l->fileofs;

	// we are about to upload textures
	R_SyncRenderThread();

	// create all the lightmaps
	tr.numLightmaps = len / (LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3);
	if ( tr.numLightmaps == 1 ) {
		//FIXME: HACK: maps with only one lightmap turn up fullbright for some reason.
		//this avoids this, but isn't the correct solution.
		tr.numLightmaps++;
	} else if ( tr.numLightmaps >= MAX_LIGHTMAPS ) { // 20051020 misantropia
		ri.Printf( PRINT_WARNING, "WARNING: number of lightmaps > MAX_LIGHTMAPS\n" );
		tr.numLightmaps = MAX_LIGHTMAPS;
	}

	// if we are in r_vertexLight mode, we don't need the lightmaps at all
	if ( r_vertexLight->integer || glConfig.hardwareType == GLHW_PERMEDIA2 ) {
		return;
	}

	for ( i = 0 ; i < tr.numLightmaps ; i++ ) {
		// expand the 24 bit on-disk to 32 bit
		buf_p = buf + i * LIGHTMAP_SIZE*LIGHTMAP_SIZE * 3;

		if ( r_lightmap->integer == 2 )
		{	// color code by intensity as development tool	(FIXME: check range)
			for ( j = 0; j < LIGHTMAP_SIZE * LIGHTMAP_SIZE; j++ )
			{
				float r = buf_p[j*3+0];
				float g = buf_p[j*3+1];
				float b = buf_p[j*3+2];
				float intensity;
				float out[3] = {0.0, 0.0, 0.0};

				intensity = 0.33f * r + 0.685f * g + 0.063f * b;

				if ( intensity > 255 )
					intensity = 1.0f;
				else
					intensity /= 255.0f;

				if ( intensity > maxIntensity )
					maxIntensity = intensity;

				HSVtoRGB( intensity, 1.00, 0.50, out );

				image[j*4+0] = out[0] * 255;
				image[j*4+1] = out[1] * 255;
				image[j*4+2] = out[2] * 255;
				image[j*4+3] = 255;

				sumIntensity += intensity;
			}
		} else {
			for ( j = 0 ; j < LIGHTMAP_SIZE * LIGHTMAP_SIZE; j++ ) {
				R_ColorShiftLightingBytes( &buf_p[j*3], &image[j*4] );
				image[j*4+3] = 255;
			}
		}
		tr.lightmaps[i] = R_CreateImage( va("*lightmap%d",i), image, 
			LIGHTMAP_SIZE, LIGHTMAP_SIZE, qfalse, qfalse, GL_CLAMP_TO_EDGE,qfalse,qfalse );
	}

	if ( r_lightmap->integer == 2 )	{
		ri.Printf( PRINT_DEVELOPER, "Brightest lightmap value: %d\n", ( int ) ( maxIntensity * 255 ) );
	}
}

//27 update: We create a single 1024x1024 monstrosity and pack everything in
static	void R_LoadFatLightmaps( lump_t *l )
{
	byte		*buf, *buf_p;
	int			len;
	int			i;

	//int		BIGSIZE=2048;
	//int		BIGNUM=16;

	byte		*fatbuffer;
	int   xoff,yoff,x,y;

	tr.fatLightmapSize=2048;
	tr.fatLightmapStep=16;

	len = l->filelen;
	if ( !len ) {
		return;
	}
	buf = fileBase + l->fileofs;

	// we are about to upload textures
	R_SyncRenderThread();

	// create all the lightmaps
	tr.numLightmaps = len / (LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3);
	if ( tr.numLightmaps >= MAX_LIGHTMAPS ) { // 20051020 misantropia
		ri.Printf( PRINT_WARNING, "WARNING: number of lightmaps > MAX_LIGHTMAPS\n" );
		tr.numLightmaps = MAX_LIGHTMAPS;
	}

	// if we are in r_vertexLight mode, we don't need the lightmaps at all
	if ( r_vertexLight->integer || glConfig.hardwareType == GLHW_PERMEDIA2 ) {
		return;
	}
	
	if (tr.numLightmaps<65) 
	{
		//Optimize: use a 1024 if we can get away with it
		tr.fatLightmapSize=1024;
		tr.fatLightmapStep=8;
	}

	fatbuffer = malloc( sizeof(byte)* tr.fatLightmapSize*tr.fatLightmapSize*4 );

	memset(fatbuffer,128,tr.fatLightmapSize*tr.fatLightmapSize*4);
	for ( i = 0 ; i < tr.numLightmaps ; i++ ) 
	{
		// expand the 24 bit on-disk to 32 bit
		buf_p = buf + i * LIGHTMAP_SIZE*LIGHTMAP_SIZE * 3;
		
		xoff = i % tr.fatLightmapStep;
		yoff = i / tr.fatLightmapStep;

		//if (tr.radbumping==qfalse)
		if (1)
		{
			for ( y = 0 ; y < LIGHTMAP_SIZE ; y++ ) 
			{
				for ( x = 0 ; x < LIGHTMAP_SIZE ; x++ ) 
				{
					int index = (x+(y*tr.fatLightmapSize))+((xoff*LIGHTMAP_SIZE)+(yoff*tr.fatLightmapSize*LIGHTMAP_SIZE));
					fatbuffer[(index*4)+0 ]=buf_p[((x+(y*LIGHTMAP_SIZE))*3)+0];
					fatbuffer[(index*4)+1 ]=buf_p[((x+(y*LIGHTMAP_SIZE))*3)+1];
					fatbuffer[(index*4)+2 ]=buf_p[((x+(y*LIGHTMAP_SIZE))*3)+2];
					fatbuffer[(index*4)+3 ]=255;
				}
			}
		}
	}
	//memset(fatbuffer,128,tr.fatLightmapSize*tr.fatLightmapSize*4);
	
	tr.fatlightmap = R_CreateImage("*fatmap", fatbuffer, tr.fatLightmapSize, tr.fatLightmapSize, qfalse, qfalse, GL_CLAMP,qfalse,qfalse );

	free(fatbuffer);
}

void R_CheckForRadbumping( const char* name )
{
	char radfilename[1024];
	char filename[1024];
	byte		*buffer;
	int result;

	if ( !r_ext_vertex_shader->integer ) 
	{
		tr.radbumping = qfalse;
		return;
	}

	COM_StripExtension(name, &filename[0], sizeof(filename));
	Com_sprintf( radfilename, sizeof(radfilename), "%s.rad", filename );

	ri.Printf( PRINT_DEVELOPER, "Checking for availability of radbumping in %s...", radfilename );

	result = ri.FS_ReadFile( radfilename, (void **)&buffer );

	if ( ( tr.radbumping = (result > 0 ) ) )
	{
		ri.Printf( PRINT_DEVELOPER, "yes %i\n", result );
	}
	else
	{
		ri.Printf( PRINT_DEVELOPER, "no %i\n", result );
	}
}

void R_LoadRadFile( const char* name, lump_t *l )
{
	if (r_simpleshaders->integer>0) return;
	
	//Attempt to load the radbump stuff if it exists
	if (r_ext_vertex_shader->integer )
	{
		char radfilename[1024];
		char filename[1024];
		byte		*buffer;
		byte		*buf_p;
		byte		*buf;
		int j;
		int result;
		float darken=0.5f;

		byte		*fatbuffer;
		int   xoff,yoff,x,y,i;
		int   components;
		int max=0;
		
	
		COM_StripExtension( name, &filename[0], sizeof( filename ) );
		Com_sprintf( radfilename, sizeof( radfilename ), "%s.rad", filename );

		result = ri.FS_ReadFile( radfilename, (void **)&buffer );

		//Figure out if we have a specular part
		components=result / (sizeof(byte)* LIGHTMAP_SIZE*LIGHTMAP_SIZE*3) ;
		if ( components / 4 == tr.numLightmaps )
		{
			max=4;  //yes we have specular
		}
		else
		{
			max=3;   //No we dont, so create a dummy specular map
			ri.Printf( PRINT_DEVELOPER, "No specular available in .RAD file... update your map.");
		}

		if (result>0)
		{
			// TODO: Calculate the right size needed
			fatbuffer = malloc( sizeof(byte)* tr.fatLightmapSize * tr.fatLightmapSize * 8 );
			memset(fatbuffer,128,tr.fatLightmapSize*tr.fatLightmapSize*max);
			
			for ( j=0;j<max;j++)
			{
				for ( i = 0 ; i < tr.numLightmaps ; i++ ) 
				{
					// expand the 24 bit on-disk to 32 bit
					buf_p = (buffer + i * LIGHTMAP_SIZE*LIGHTMAP_SIZE * 3)+ (j*  (LIGHTMAP_SIZE*LIGHTMAP_SIZE * 3)*tr.numLightmaps ) ;
			
					xoff = i % tr.fatLightmapStep;
					yoff = i / tr.fatLightmapStep;

					for ( y = 0 ; y < LIGHTMAP_SIZE ; y++ ) 
					{
						for ( x = 0 ; x < LIGHTMAP_SIZE ; x++ ) 
						{
							int index = (x+(y*tr.fatLightmapSize))+((xoff*LIGHTMAP_SIZE)+(yoff*tr.fatLightmapSize*LIGHTMAP_SIZE));
							fatbuffer[(index*4)+0 ]=buf_p[((x+(y*LIGHTMAP_SIZE))*3)+0];
							fatbuffer[(index*4)+1 ]=buf_p[((x+(y*LIGHTMAP_SIZE))*3)+1];
							fatbuffer[(index*4)+2 ]=buf_p[((x+(y*LIGHTMAP_SIZE))*3)+2];
							fatbuffer[(index*4)+3 ]=255;
						}
					}
				}
				if (j==0) tr.radbump[j] = R_CreateImage("*radmap0", fatbuffer, tr.fatLightmapSize, tr.fatLightmapSize, qfalse, qfalse, GL_CLAMP,qfalse,qfalse );
				if (j==1) tr.radbump[j] = R_CreateImage("*radmap1", fatbuffer, tr.fatLightmapSize, tr.fatLightmapSize, qfalse, qfalse, GL_CLAMP,qfalse,qfalse );
				if (j==2) tr.radbump[j] = R_CreateImage("*radmap2", fatbuffer, tr.fatLightmapSize, tr.fatLightmapSize, qfalse, qfalse, GL_CLAMP,qfalse,qfalse );
				if (j==3) tr.radbump[j] = R_CreateImage("*radmap3", fatbuffer, tr.fatLightmapSize, tr.fatLightmapSize, qfalse, qfalse, GL_CLAMP,qfalse,qfalse );
			}

			if (max==3)
			{
				
				//Create a dummy spec channel full of 0's			
				memset(fatbuffer,0,4);
				tr.radbump[3] = R_CreateImage("*radmap3", fatbuffer, 1, 1, qfalse, qfalse, GL_CLAMP,qfalse,qfalse );
			}
			free(fatbuffer);
			ri.FS_FreeFile( buffer );
		}
		else
		{
			//autogen one from the lightmaps
			fatbuffer = malloc( sizeof(byte)* tr.fatLightmapSize*tr.fatLightmapSize*4 );
			memset(fatbuffer,128,tr.fatLightmapSize*tr.fatLightmapSize*4);

			buf = fileBase + l->fileofs;
			
			for ( i = 0 ; i < tr.numLightmaps ; i++ ) 
			{
		
				buf_p = buf + i * LIGHTMAP_SIZE*LIGHTMAP_SIZE * 3;
				
				xoff = i % tr.fatLightmapStep;
				yoff = i / tr.fatLightmapStep;
				for ( y = 0 ; y < LIGHTMAP_SIZE ; y++ ) 
				{
					for ( x = 0 ; x < LIGHTMAP_SIZE ; x++ ) 
					{
						int index = (x+(y*tr.fatLightmapSize))+((xoff*LIGHTMAP_SIZE)+(yoff*tr.fatLightmapSize*LIGHTMAP_SIZE));
						fatbuffer[(index*4)+0 ]=buf_p[((x+(y*LIGHTMAP_SIZE))*3)+0] * darken;
						fatbuffer[(index*4)+1 ]=buf_p[((x+(y*LIGHTMAP_SIZE))*3)+1] * darken;
						fatbuffer[(index*4)+2 ]=buf_p[((x+(y*LIGHTMAP_SIZE))*3)+2] * darken;
						fatbuffer[(index*4)+3 ]=255;
					}
				}
			}
			tr.radbump[2] = tr.radbump[1] = tr.radbump[0] = R_CreateImage("*radmap0", fatbuffer, tr.fatLightmapSize, tr.fatLightmapSize, qfalse, qfalse, GL_CLAMP ,qfalse,qfalse);
			memset(fatbuffer,0,4);
			tr.radbump[3] = R_CreateImage("*radmap3", fatbuffer, 1, 1, qfalse, qfalse, GL_CLAMP,qfalse,qfalse );
			
			free(fatbuffer);
		}
	}
}

/*
=================
RE_SetWorldVisData

This is called by the clipmodel subsystem so we can share the 1.8 megs of
space in big maps...
=================
*/
void		RE_SetWorldVisData( const byte *vis ) {
	tr.externalVisData = vis;
}


/*
=================
R_LoadVisibility
=================
*/
static	void R_LoadVisibility( lump_t *l ) {
	int		len;
	byte	*buf;

	len = ( s_worldData.numClusters + 63 ) & ~63;
	s_worldData.novis = ri.Hunk_Alloc( len, h_low );
	Com_Memset( s_worldData.novis, 0xff, len );

    len = l->filelen;
	if ( !len ) {
		return;
	}
	buf = fileBase + l->fileofs;

	s_worldData.numClusters = LittleLong( ((int *)buf)[0] );
	s_worldData.clusterBytes = LittleLong( ((int *)buf)[1] );

	// CM_Load should have given us the vis data to share, so
	// we don't need to allocate another copy
	if ( tr.externalVisData ) {
		s_worldData.vis = tr.externalVisData;
	} else {
		byte	*dest;

		dest = ri.Hunk_Alloc( len - 8, h_low );
		Com_Memcpy( dest, buf + 8, len - 8 );
		s_worldData.vis = dest;
	}
}

//===============================================================================


/*
===============
ShaderForShaderNum
===============
*/
static shader_t *ShaderForShaderNum( int shaderNum, int lightmapNum ) {
	shader_t	*shader;
	dshader_t	*dsh;

	shaderNum = LittleLong( shaderNum );
	if ( shaderNum < 0 || shaderNum >= s_worldData.numShaders ) {
		ri.Error( ERR_DROP, "ShaderForShaderNum: bad num %i", shaderNum );
	}
	dsh = &s_worldData.shaders[ shaderNum ];

	if ( r_vertexLight->integer || glConfig.hardwareType == GLHW_PERMEDIA2 ) {
		lightmapNum = LIGHTMAP_BY_VERTEX;
	}

	if ( r_fullbright->integer ) {
		lightmapNum = LIGHTMAP_WHITEIMAGE;
	}

	shader = R_FindShader( dsh->shader, lightmapNum, qtrue );

	// if the shader had errors, just use default shader
	if ( shader->defaultShader ) {
		return tr.defaultShader;
	}

	return shader;
}


void R_BufferAllocTris(int* StartIndex,int* StartVertex,int numIndicies,int numVerts)
{
	if (tr.numStaticIndex+numIndicies>r_maxstaticverts->integer-1 ||tr.numStaticVertex+numVerts>r_maxstaticverts->integer-1)
	{
		ri.Error (ERR_DROP, "LoadMap: Ran out of static Verts.  Increase r_maxstaticverts.");

	}
	*StartIndex=tr.numStaticIndex;
	*StartVertex=tr.numStaticVertex;
	tr.numStaticIndex+=numIndicies;
	tr.numStaticVertex+=numVerts;
}


float FatPackU(float input,int lightmapnum)
{
	int x= lightmapnum % tr.fatLightmapStep;
	
	return (input/((float)tr.fatLightmapStep))+ ( (1.0/((float)tr.fatLightmapStep)) * (float)x );

}

float FatPackV(float input,int lightmapnum)
{
	int y= lightmapnum / ((float)tr.fatLightmapStep);
	return (input/((float)tr.fatLightmapStep))+ ( (1.0/((float)tr.fatLightmapStep)) * (float)y );
}

/*
===============
ParseFace
===============
*/
static void ParseFace( dsurface_t *ds, drawVert_t *verts, msurface_t *surf, int *indexes  ) {
	int			i, j;
	srfSurfaceFace_t	*cv;
	srfSurfaceStatic_t	*sv;
	int			numPoints, numIndexes;
	int			lightmapNum;
	int			sfaceSize, ofsIndexes;
	staticVert_t		*data;
	unsigned int		*indexs;
	int			realLightmapNum;

	realLightmapNum = LittleLong( ds->lightmapNum );
	lightmapNum = 0;

	// get fog volume
	surf->fogIndex = LittleLong( ds->fogNum ) + 1;

	// get shader value
	if (realLightmapNum<0) lightmapNum=realLightmapNum;
	surf->shader = ShaderForShaderNum( ds->shaderNum, lightmapNum );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	if ( surf->fogIndex==0)
	{
		surf->shader = GLSLOptimizeShader(surf->shader);
	}

	//27: Static mesh optimization 
	if (r_ext_vertex_buffer_object->integer && surf->shader->isGLSLStatic)
	{
		//we're a lightmapped texture, so we don't build a regular surface
		numPoints = LittleLong( ds->numVerts );
		numIndexes = LittleLong( ds->numIndexes );
		sfaceSize = sizeof(srfSurfaceStatic_t);
		sv = ri.Hunk_Alloc( sfaceSize, h_low );
		sv->surfaceType=SF_STATIC;
		sv->numIndices = numIndexes;
		sv->numVerts = numPoints;

		R_BufferAllocTris(&sv->startIndex,&sv->startVertex,sv->numIndices,sv->numVerts);
		
		data = &tr.staticVertexData[sv->startVertex];
		indexs = &tr.staticIndexData[sv->startIndex];
		
		verts += LittleLong( ds->firstVert );
		for ( i = 0 ; i < sv->numVerts ; i++ ) 
		{
			data[i].position[0] = LittleFloat( verts[i].xyz[0] );
			data[i].position[1] = LittleFloat( verts[i].xyz[1] );
			data[i].position[2] = LittleFloat( verts[i].xyz[2] );
			
			data[i].uv[0] = LittleFloat( verts[i].st[0] );
			data[i].uv[1] = LittleFloat( verts[i].st[1] );
			

			data[i].light_uv[0] =  FatPackU(LittleFloat( verts[i].lightmap[0] ),realLightmapNum);
			data[i].light_uv[1] =  FatPackV(LittleFloat( verts[i].lightmap[1] ),realLightmapNum);
			

			data[i].normal[0] = LittleFloat( verts[i].normal[0] );
			data[i].normal[1] = LittleFloat( verts[i].normal[1] );
			data[i].normal[2] = LittleFloat( verts[i].normal[2] ); 

			data[i].rgba[0] = verts[i].color[0] / 255.0f;
			data[i].rgba[1] = verts[i].color[1] / 255.0f;
			data[i].rgba[2] = verts[i].color[2] / 255.0f;
			data[i].rgba[3] = verts[i].color[3]  / 255.0f;
		}
		
		indexes += LittleLong( ds->firstIndex );
		for ( i= 0 ; i < sv->numIndices; i++ )
		{
			indexs[i]= LittleLong( indexes[ i ] )+sv->startVertex; // indexes are all 0 based..
		}

		sv->shader=surf->shader;
		sv->surfnum=rand()% 1000;
		surf->data = (surfaceType_t *)sv;
		
		
		if (r_decals->integer) 
		{
			//For each tri, add it to the decal list
			for ( i= 0 ; i < sv->numIndices; i+=3 )
			{
				R_StoreDecalSurf(data[indexes[i] ].position,data[indexes[i+1]].position,data[indexes[i+2]].position,surf);
			}
		}


		return;
	}

	numPoints = LittleLong( ds->numVerts );
	if (numPoints > MAX_FACE_POINTS) {
		ri.Printf( PRINT_WARNING, "WARNING: MAX_FACE_POINTS exceeded: %i\n", numPoints);
		numPoints = MAX_FACE_POINTS;
		surf->shader = tr.defaultShader;
	}

	numIndexes = LittleLong( ds->numIndexes );

	// create the srfSurfaceFace_t
	sfaceSize = ( size_t ) &((srfSurfaceFace_t *)0)->points[numPoints];
	ofsIndexes = sfaceSize;
	sfaceSize += sizeof( int ) * numIndexes;

	cv = ri.Hunk_Alloc( sfaceSize, h_low );
	cv->surfaceType = SF_FACE;
	cv->numPoints = numPoints;
	cv->numIndices = numIndexes;
	cv->ofsIndices = ofsIndexes;

	verts += LittleLong( ds->firstVert );
	for ( i = 0 ; i < numPoints ; i++ ) {
		for ( j = 0 ; j < 3 ; j++ ) {
			cv->points[i][j] = LittleFloat( verts[i].xyz[j] );
		}
		
			cv->points[i][3] = LittleFloat( verts[i].st[0] );
			cv->points[i][4] = LittleFloat( verts[i].st[1] );
			cv->points[i][5] =FatPackU( LittleFloat( verts[i].lightmap[0] ), realLightmapNum);
			cv->points[i][6] =FatPackV( LittleFloat( verts[i].lightmap[1] ), realLightmapNum);
		
		R_ColorShiftLightingBytes( verts[i].color, (byte *)&cv->points[i][7] );
	}

	indexes += LittleLong( ds->firstIndex );
	for ( i = 0 ; i < numIndexes ; i++ ) {
		((int *)((byte *)cv + cv->ofsIndices ))[i] = LittleLong( indexes[ i ] );
	}

	// take the plane information from the lightmap vector
	for ( i = 0 ; i < 3 ; i++ ) {
		cv->plane.normal[i] = LittleFloat( ds->lightmapVecs[2][i] );
	}
	cv->plane.dist = DotProduct( cv->points[0], cv->plane.normal );
	SetPlaneSignbits( &cv->plane );
	cv->plane.type = PlaneTypeForNormal( cv->plane.normal );

	surf->data = (surfaceType_t *)cv;

	if (r_decals->integer)
	{
		//check to see if its insane to decal this surface
		if (!(surf->shader->surfaceFlags & SURF_NOMARKS) &&
			!(surf->shader->surfaceFlags & SURF_NONSOLID) &&
			surf->shader->numDeforms==0 &&
			cv->plane.type ==PLANE_NON_AXIAL)
		{
			for ( i = 0 ; i < numIndexes ; i+=3 )
			{
				int Index0,Index1,Index2;
				Index0 = LittleLong( indexes[i+0] );
				Index1 = LittleLong( indexes[i+1] );
				Index2 = LittleLong( indexes[i+2] );
				R_StoreDecalSurf(verts[Index0].xyz,verts[Index1].xyz,verts[Index2].xyz, surf);
			}
		}
	}
}


//Converts a patch mesh to a highest resolution SF_STATIC
srfSurfaceStatic_t *SurfaceGrid_To_Static( srfGridMesh_t *cv ,shader_t *shader) 
{
	int		i, j;
	float	*xyz;
	float	*texCoords;
	float	*texCoordsLight;
	float	*normal;
	float   *color;
	drawVert_t	*dv;
	int		rows;
	int		used;
	int		widthTable[MAX_GRID_SIZE];
	int		heightTable[MAX_GRID_SIZE];
	int		lodWidth, lodHeight;
	staticVert_t	*data;
	unsigned int			*indexs;
	int h,w;

	int vertexcount=0;
	int indexcount=0;

	int totalverts=0;
	int totalindexes=0;

	int pos=0;

	srfSurfaceStatic_t	*sv;	
	int sfaceSize = sizeof(srfSurfaceStatic_t);

	
	// determine which rows and columns of the subdivision
	// we are actually going to use
	widthTable[0] = 0;
	lodWidth = 1;
	for ( i = 1 ; i < cv->width-1 ; i++ ) 
	{
		widthTable[lodWidth] = i;
		lodWidth++;
	}
	widthTable[lodWidth] = cv->width-1;
	lodWidth++;

	heightTable[0] = 0;
	lodHeight = 1;
	for ( i = 1 ; i < cv->height-1 ; i++ ) 
	{
		heightTable[lodHeight] = i;
		lodHeight++;
	}
	heightTable[lodHeight] = cv->height-1;
	lodHeight++;

	//Precount the verts/indexes used in tesselation by giving it a run through
	used = 0;
	rows = 0;
	totalverts=0;
	totalindexes=0;
	
	while ( used < lodHeight - 1 ) 
	{
		rows = lodHeight - used;
		
		
		for ( i = 0 ; i < rows ; i++ ) 
		{
			for ( j = 0 ; j < lodWidth ; j++ ) 
			{
				totalverts++;	
			}
		}

		h = rows - 1;
		w = lodWidth - 1;
			
		for (i = 0 ; i < h ; i++) 
		{
			for (j = 0 ; j < w ; j++) 
			{
				totalindexes+=6;
			}
		}
		used += rows - 1;
	}

	//Allocate and do it for real, now
	used = 0;
	rows = 0;
	
	sv = ri.Hunk_Alloc( sfaceSize, h_low );
	sv->surfaceType=SF_STATIC;
	sv->numVerts = totalverts;
	sv->numIndices = totalindexes;
		
	R_BufferAllocTris(&sv->startIndex,&sv->startVertex,sv->numIndices,sv->numVerts);
		
	data=&tr.staticVertexData[sv->startVertex];
	indexs=&tr.staticIndexData[sv->startIndex];


	while ( used < lodHeight - 1 ) 
	{
		rows = lodHeight - used;
		
		pos = vertexcount;
		for ( i = 0 ; i < rows ; i++ ) 
		{
			for ( j = 0 ; j < lodWidth ; j++ ) 
			{
				dv = cv->verts + heightTable[ used + i ] * cv->width + widthTable[ j ];

				xyz = data[pos].position;
				normal = data[pos].normal;
				texCoordsLight =data[pos].light_uv;
				texCoords =data[pos].uv;
				color = data[pos].rgba;


				xyz[0] = dv->xyz[0];
				xyz[1] = dv->xyz[1];
				xyz[2] = dv->xyz[2];
				texCoords[0] = dv->st[0];
				texCoords[1] = dv->st[1];
				texCoordsLight[0] = dv->lightmap[0];
				texCoordsLight[1] = dv->lightmap[1];
				normal[0] = dv->normal[0];
				normal[1] = dv->normal[1];
				normal[2] = dv->normal[2];

				color[0]=(float)dv->color[0] / 255.0f;
				color[1]=(float)dv->color[1] / 255.0f;
				color[2]=(float)dv->color[2] / 255.0f;
				color[3]=(float)dv->color[3] / 255.0f;
				
				pos++;
			}
		}


		// add the indexes
		h = rows - 1;
		w = lodWidth - 1;
		
		for (i = 0 ; i < h ; i++)
		{
			for (j = 0 ; j < w ; j++) 
			{
				int		v1, v2, v3, v4;
		
				// vertex order to be reckognized as tristrips
				v1 = vertexcount + i*lodWidth + j + 1;
				v2 = v1 - 1;
				v3 = v2 + lodWidth;
				v4 = v3 + 1;

				indexs[indexcount] = v2+sv->startVertex;
				indexs[indexcount+1] = v3+sv->startVertex;
				indexs[indexcount+2] = v1+sv->startVertex;
				
				indexs[indexcount+3] = v1+sv->startVertex;
				indexs[indexcount+4] = v3+sv->startVertex;
				indexs[indexcount+5] = v4+sv->startVertex;
				indexcount += 6;
			}
		}


		vertexcount += rows * lodWidth;

		used += rows - 1;
	}

	sv->shader=shader;

	return sv;
}


/*
===============
ParseMesh
===============
*/
static void ParseMesh ( dsurface_t *ds, drawVert_t *verts, msurface_t *surf ) {
	srfGridMesh_t	*grid;
	int				i, j;
	int				width, height, numPoints;
	drawVert_t points[MAX_PATCH_SIZE*MAX_PATCH_SIZE];
	int				lightmapNum, realLightmapNum;
	vec3_t			bounds[2];
	vec3_t			tmpVec;
	static surfaceType_t	skipData = SF_SKIP;

	
	realLightmapNum = LittleLong( ds->lightmapNum );
	lightmapNum = 0;

	if (realLightmapNum<0) lightmapNum=realLightmapNum;

	// get fog volume
	surf->fogIndex = LittleLong( ds->fogNum ) + 1;

	// get shader value
	surf->shader = ShaderForShaderNum( ds->shaderNum, lightmapNum );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	// we may have a nodraw surface, because they might still need to
	// be around for movement clipping
	if ( s_worldData.shaders[ LittleLong( ds->shaderNum ) ].surfaceFlags & SURF_NODRAW ) {
		surf->data = &skipData;
		return;
	}

	width = LittleLong( ds->patchWidth );
	height = LittleLong( ds->patchHeight );

	verts += LittleLong( ds->firstVert );
	numPoints = width * height;
	for ( i = 0 ; i < numPoints ; i++ ) {
		for ( j = 0 ; j < 3 ; j++ ) {
			points[i].xyz[j] = LittleFloat( verts[i].xyz[j] );
			points[i].normal[j] = LittleFloat( verts[i].normal[j] );
		}
		
			points[i].st[0] = LittleFloat( verts[i].st[0] );
			points[i].st[1] = LittleFloat( verts[i].st[1] );
			
			points[i].lightmap[0] = FatPackU( LittleFloat( verts[i].lightmap[0] ),realLightmapNum);
			points[i].lightmap[1] = FatPackV( LittleFloat( verts[i].lightmap[1] ),realLightmapNum);
		
		R_ColorShiftLightingBytes( verts[i].color, points[i].color );
	}

	// pre-tesseleate
	grid = R_SubdividePatchToGrid( width, height, points );
	surf->data = (surfaceType_t *)grid;

	// copy the level of detail origin, which is the center
	// of the group of all curves that must subdivide the same
	// to avoid cracking
	for ( i = 0 ; i < 3 ; i++ ) {
		bounds[0][i] = LittleFloat( ds->lightmapVecs[0][i] );
		bounds[1][i] = LittleFloat( ds->lightmapVecs[1][i] );
	}
	VectorAdd( bounds[0], bounds[1], bounds[1] );
	VectorScale( bounds[1], 0.5f, grid->lodOrigin );
	VectorSubtract( bounds[0], grid->lodOrigin, tmpVec );
	grid->lodRadius = VectorLength( tmpVec );

	if (surf->fogIndex==0)
	{
		surf->shader = GLSLOptimizeShader(surf->shader);
	}

	if (surf->shader->isGLSLStatic && r_ext_vertex_buffer_object->integer )
	{
		//turn us into a SF_STATIC if we need to be one
		surf->data = (surfaceType_t *)SurfaceGrid_To_Static(grid,surf->shader);
	}


}




/*
===============
ParseTriSurf
===============
*/
static void ParseTriSurf( dsurface_t *ds, drawVert_t *verts, msurface_t *surf, int *indexes ) {
	srfTriangles_t	*tri;
	int				i, j;
	int				numVerts, numIndexes;
	int				sfaceSize;
	staticVert_t	*data;
	unsigned int			*indexs;
	srfSurfaceStatic_t	*sv;

	// get fog volume
	surf->fogIndex = LittleLong( ds->fogNum ) + 1;

	// get shader
	surf->shader = ShaderForShaderNum( ds->shaderNum, LIGHTMAP_BY_VERTEX );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	if ( surf->fogIndex==0)
	{
		surf->shader = GLSLOptimizeShader(surf->shader);
	}

	//27: Static mesh optimization 
	if (r_ext_vertex_buffer_object->integer && surf->shader->isGLSLStatic )
	{
		//we're a lightmapped texture, so we don't build a regular surface
		numVerts = LittleLong( ds->numVerts );
		numIndexes = LittleLong( ds->numIndexes );
		sfaceSize = sizeof(srfSurfaceStatic_t);
		sv = ri.Hunk_Alloc( sfaceSize, h_low );
		sv->surfaceType=SF_STATIC;
		sv->numIndices = numIndexes;
		sv->numVerts = numVerts;
		
		R_BufferAllocTris(&sv->startIndex,&sv->startVertex,sv->numIndices,sv->numVerts);
		
		data=&tr.staticVertexData[sv->startVertex];
		indexs=&tr.staticIndexData[sv->startIndex];
		
		verts += LittleLong( ds->firstVert );
		for ( i = 0 ; i < sv->numVerts ; i++ ) 
		{
			data[i].position[0] = LittleFloat( verts[i].xyz[0] );
			data[i].position[1] = LittleFloat( verts[i].xyz[1] );
			data[i].position[2] = LittleFloat( verts[i].xyz[2] );
			
			data[i].uv[0] = LittleFloat( verts[i].st[0] );
			data[i].uv[1] = LittleFloat( verts[i].st[1] );
			
			//Todo: These surfaces dont use lightmaps at all.
			data[i].light_uv[0] = FatPackU(LittleFloat( verts[i].lightmap[0] ),0);
			data[i].light_uv[1] = FatPackV(LittleFloat( verts[i].lightmap[1] ),0);
			
			data[i].normal[0] = LittleFloat( verts[i].normal[0] );
			data[i].normal[1] = LittleFloat( verts[i].normal[1] );
			data[i].normal[2] = LittleFloat( verts[i].normal[2] );

			data[i].rgba[0] = verts[i].color[0] / 255.0f;
			data[i].rgba[1] = verts[i].color[1] / 255.0f;
			data[i].rgba[2] = verts[i].color[2] / 255.0f;
			data[i].rgba[3] = verts[i].color[3] / 255.0f;

		}
		
		indexes += LittleLong( ds->firstIndex );
		for ( i= 0 ; i < sv->numIndices; i++ )
		{
			indexs[i]= LittleLong( indexes[ i ])  +sv->startVertex ; // indexes are all 0 based..
		}

		sv->shader=surf->shader;
		sv->surfnum=rand()% 2000;
		surf->data = (surfaceType_t *)sv;
		
		return;
	}

	numVerts = LittleLong( ds->numVerts );
	numIndexes = LittleLong( ds->numIndexes );

	tri = ri.Hunk_Alloc( sizeof( *tri ) + numVerts * sizeof( tri->verts[0] ) 
		+ numIndexes * sizeof( tri->indexes[0] ), h_low );
	tri->surfaceType = SF_TRIANGLES;
	tri->numVerts = numVerts;
	tri->numIndexes = numIndexes;
	tri->verts = (drawVert_t *)(tri + 1);
	tri->indexes = (int *)(tri->verts + tri->numVerts );

	surf->data = (surfaceType_t *)tri;

	// copy vertexes
	ClearBounds( tri->bounds[0], tri->bounds[1] );
	verts += LittleLong( ds->firstVert );
	for ( i = 0 ; i < numVerts ; i++ ) {
		for ( j = 0 ; j < 3 ; j++ ) {
			tri->verts[i].xyz[j] = LittleFloat( verts[i].xyz[j] );
			tri->verts[i].normal[j] = LittleFloat( verts[i].normal[j] );
		}
		AddPointToBounds( tri->verts[i].xyz, tri->bounds[0], tri->bounds[1] );
		for ( j = 0 ; j < 2 ; j++ ) {
			tri->verts[i].st[j] = LittleFloat( verts[i].st[j] );
			tri->verts[i].lightmap[j] = LittleFloat( verts[i].lightmap[j] );
		}

		R_ColorShiftLightingBytes( verts[i].color, tri->verts[i].color );
	}

	// copy indexes
	indexes += LittleLong( ds->firstIndex );
	for ( i = 0 ; i < numIndexes ; i++ ) {
		tri->indexes[i] = LittleLong( indexes[i] );
		if ( tri->indexes[i] < 0 || tri->indexes[i] >= numVerts ) {
			ri.Error( ERR_DROP, "Bad index in triangle surface" );
		}
	}

	
	if (r_decals->integer)
	{
		//check to see if its insane to decal this surface
		if (!(surf->shader->surfaceFlags & SURF_NOMARKS) &&
			!(surf->shader->surfaceFlags & SURF_NONSOLID) &&
			surf->shader->numDeforms==0 )
		{
			for ( i = 0 ; i < numIndexes ; i+=3 )
			{
				int Index0,Index1,Index2;
				Index0 = LittleLong( indexes[i+0] );
				Index1 = LittleLong( indexes[i+1] );
				Index2 = LittleLong( indexes[i+2] );
				R_StoreDecalSurf(tri->verts[Index0].xyz,tri->verts[Index1].xyz,tri->verts[Index2].xyz, surf);
			}
		}
	}
}

/*
===============
ParseFlare
===============
*/
static void ParseFlare( dsurface_t *ds, drawVert_t *verts, msurface_t *surf, int *indexes ) {
	srfFlare_t		*flare;
	int				i;

	// get fog volume
	surf->fogIndex = LittleLong( ds->fogNum ) + 1;

	// get shader
	surf->shader = ShaderForShaderNum( ds->shaderNum, LIGHTMAP_BY_VERTEX );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	flare = ri.Hunk_Alloc( sizeof( *flare ), h_low );
	flare->surfaceType = SF_FLARE;

	surf->data = (surfaceType_t *)flare;

	for ( i = 0 ; i < 3 ; i++ ) {
		flare->origin[i] = LittleFloat( ds->lightmapOrigin[i] );
		flare->color[i] = LittleFloat( ds->lightmapVecs[0][i] );
		flare->normal[i] = LittleFloat( ds->lightmapVecs[2][i] );
	}

}


/*
=================
R_MergedWidthPoints

returns true if there are grid points merged on a width edge
=================
*/
int R_MergedWidthPoints(srfGridMesh_t *grid, int offset) {
	int i, j;

	for (i = 1; i < grid->width-1; i++) {
		for (j = i + 1; j < grid->width-1; j++) {
			if ( fabs(grid->verts[i + offset].xyz[0] - grid->verts[j + offset].xyz[0]) > .1) continue;
			if ( fabs(grid->verts[i + offset].xyz[1] - grid->verts[j + offset].xyz[1]) > .1) continue;
			if ( fabs(grid->verts[i + offset].xyz[2] - grid->verts[j + offset].xyz[2]) > .1) continue;
			return qtrue;
		}
	}
	return qfalse;
}

/*
=================
R_MergedHeightPoints

returns true if there are grid points merged on a height edge
=================
*/
int R_MergedHeightPoints(srfGridMesh_t *grid, int offset) {
	int i, j;

	for (i = 1; i < grid->height-1; i++) {
		for (j = i + 1; j < grid->height-1; j++) {
			if ( fabs(grid->verts[grid->width * i + offset].xyz[0] - grid->verts[grid->width * j + offset].xyz[0]) > .1) continue;
			if ( fabs(grid->verts[grid->width * i + offset].xyz[1] - grid->verts[grid->width * j + offset].xyz[1]) > .1) continue;
			if ( fabs(grid->verts[grid->width * i + offset].xyz[2] - grid->verts[grid->width * j + offset].xyz[2]) > .1) continue;
			return qtrue;
		}
	}
	return qfalse;
}

/*
=================
R_FixSharedVertexLodError_r

NOTE: never sync LoD through grid edges with merged points!

FIXME: write generalized version that also avoids cracks between a patch and one that meets half way?
=================
*/
void R_FixSharedVertexLodError_r( int start, srfGridMesh_t *grid1 ) {
	int j, k, l, m, n, offset1, offset2, touch;
	srfGridMesh_t *grid2;

	for ( j = start; j < s_worldData.numsurfaces; j++ ) {
		//
		grid2 = (srfGridMesh_t *) s_worldData.surfaces[j].data;
		// if this surface is not a grid
		if ( grid2->surfaceType != SF_GRID ) continue;
		// if the LOD errors are already fixed for this patch
		if ( grid2->lodFixed == 2 ) continue;
		// grids in the same LOD group should have the exact same lod radius
		if ( grid1->lodRadius != grid2->lodRadius ) continue;
		// grids in the same LOD group should have the exact same lod origin
		if ( grid1->lodOrigin[0] != grid2->lodOrigin[0] ) continue;
		if ( grid1->lodOrigin[1] != grid2->lodOrigin[1] ) continue;
		if ( grid1->lodOrigin[2] != grid2->lodOrigin[2] ) continue;
		//
		touch = qfalse;
		for (n = 0; n < 2; n++) {
			//
			if (n) offset1 = (grid1->height-1) * grid1->width;
			else offset1 = 0;
			if (R_MergedWidthPoints(grid1, offset1)) continue;
			for (k = 1; k < grid1->width-1; k++) {
				for (m = 0; m < 2; m++) {

					if (m) offset2 = (grid2->height-1) * grid2->width;
					else offset2 = 0;
					if (R_MergedWidthPoints(grid2, offset2)) continue;
					for ( l = 1; l < grid2->width-1; l++) {
					//
						if ( fabs(grid1->verts[k + offset1].xyz[0] - grid2->verts[l + offset2].xyz[0]) > .1) continue;
						if ( fabs(grid1->verts[k + offset1].xyz[1] - grid2->verts[l + offset2].xyz[1]) > .1) continue;
						if ( fabs(grid1->verts[k + offset1].xyz[2] - grid2->verts[l + offset2].xyz[2]) > .1) continue;
						// ok the points are equal and should have the same lod error
						grid2->widthLodError[l] = grid1->widthLodError[k];
						touch = qtrue;
					}
				}
				for (m = 0; m < 2; m++) {

					if (m) offset2 = grid2->width-1;
					else offset2 = 0;
					if (R_MergedHeightPoints(grid2, offset2)) continue;
					for ( l = 1; l < grid2->height-1; l++) {
					//
						if ( fabs(grid1->verts[k + offset1].xyz[0] - grid2->verts[grid2->width * l + offset2].xyz[0]) > .1) continue;
						if ( fabs(grid1->verts[k + offset1].xyz[1] - grid2->verts[grid2->width * l + offset2].xyz[1]) > .1) continue;
						if ( fabs(grid1->verts[k + offset1].xyz[2] - grid2->verts[grid2->width * l + offset2].xyz[2]) > .1) continue;
						// ok the points are equal and should have the same lod error
						grid2->heightLodError[l] = grid1->widthLodError[k];
						touch = qtrue;
					}
				}
			}
		}
		for (n = 0; n < 2; n++) {
			//
			if (n) offset1 = grid1->width-1;
			else offset1 = 0;
			if (R_MergedHeightPoints(grid1, offset1)) continue;
			for (k = 1; k < grid1->height-1; k++) {
				for (m = 0; m < 2; m++) {

					if (m) offset2 = (grid2->height-1) * grid2->width;
					else offset2 = 0;
					if (R_MergedWidthPoints(grid2, offset2)) continue;
					for ( l = 1; l < grid2->width-1; l++) {
					//
						if ( fabs(grid1->verts[grid1->width * k + offset1].xyz[0] - grid2->verts[l + offset2].xyz[0]) > .1) continue;
						if ( fabs(grid1->verts[grid1->width * k + offset1].xyz[1] - grid2->verts[l + offset2].xyz[1]) > .1) continue;
						if ( fabs(grid1->verts[grid1->width * k + offset1].xyz[2] - grid2->verts[l + offset2].xyz[2]) > .1) continue;
						// ok the points are equal and should have the same lod error
						grid2->widthLodError[l] = grid1->heightLodError[k];
						touch = qtrue;
					}
				}
				for (m = 0; m < 2; m++) {

					if (m) offset2 = grid2->width-1;
					else offset2 = 0;
					if (R_MergedHeightPoints(grid2, offset2)) continue;
					for ( l = 1; l < grid2->height-1; l++) {
					//
						if ( fabs(grid1->verts[grid1->width * k + offset1].xyz[0] - grid2->verts[grid2->width * l + offset2].xyz[0]) > .1) continue;
						if ( fabs(grid1->verts[grid1->width * k + offset1].xyz[1] - grid2->verts[grid2->width * l + offset2].xyz[1]) > .1) continue;
						if ( fabs(grid1->verts[grid1->width * k + offset1].xyz[2] - grid2->verts[grid2->width * l + offset2].xyz[2]) > .1) continue;
						// ok the points are equal and should have the same lod error
						grid2->heightLodError[l] = grid1->heightLodError[k];
						touch = qtrue;
					}
				}
			}
		}
		if (touch) {
			grid2->lodFixed = 2;
			R_FixSharedVertexLodError_r ( start, grid2 );
			//NOTE: this would be correct but makes things really slow
			//grid2->lodFixed = 1;
		}
	}
}

/*
=================
R_FixSharedVertexLodError

This function assumes that all patches in one group are nicely stitched together for the highest LoD.
If this is not the case this function will still do its job but won't fix the highest LoD cracks.
=================
*/
void R_FixSharedVertexLodError( void ) {
	int i;
	srfGridMesh_t *grid1;

	for ( i = 0; i < s_worldData.numsurfaces; i++ ) {
		//
		grid1 = (srfGridMesh_t *) s_worldData.surfaces[i].data;
		// if this surface is not a grid
		if ( grid1->surfaceType != SF_GRID )
			continue;
		//
		if ( grid1->lodFixed )
			continue;
		//
		grid1->lodFixed = 2;
		// recursively fix other patches in the same LOD group
		R_FixSharedVertexLodError_r( i + 1, grid1);
	}
}


/*
===============
R_StitchPatches
===============
*/
int R_StitchPatches( int grid1num, int grid2num ) {
	float *v1, *v2;
	srfGridMesh_t *grid1, *grid2;
	int k, l, m, n, offset1, offset2, row, column;

	grid1 = (srfGridMesh_t *) s_worldData.surfaces[grid1num].data;
	grid2 = (srfGridMesh_t *) s_worldData.surfaces[grid2num].data;
	for (n = 0; n < 2; n++) {
		//
		if (n) offset1 = (grid1->height-1) * grid1->width;
		else offset1 = 0;
		if (R_MergedWidthPoints(grid1, offset1))
			continue;
		for (k = 0; k < grid1->width-2; k += 2) {

			for (m = 0; m < 2; m++) {

				if ( grid2->width >= MAX_GRID_SIZE )
					break;
				if (m) offset2 = (grid2->height-1) * grid2->width;
				else offset2 = 0;
				for ( l = 0; l < grid2->width-1; l++) {
				//
					v1 = grid1->verts[k + offset1].xyz;
					v2 = grid2->verts[l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[k + 2 + offset1].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[l + offset2].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert column into grid2 right after after column l
					if (m) row = grid2->height-1;
					else row = 0;
					grid2 = R_GridInsertColumn( grid2, l+1, row,
									grid1->verts[k + 1 + offset1].xyz, grid1->widthLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
			for (m = 0; m < 2; m++) {

				if (grid2->height >= MAX_GRID_SIZE)
					break;
				if (m) offset2 = grid2->width-1;
				else offset2 = 0;
				for ( l = 0; l < grid2->height-1; l++) {
					//
					v1 = grid1->verts[k + offset1].xyz;
					v2 = grid2->verts[grid2->width * l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[k + 2 + offset1].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[grid2->width * l + offset2].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert row into grid2 right after after row l
					if (m) column = grid2->width-1;
					else column = 0;
					grid2 = R_GridInsertRow( grid2, l+1, column,
										grid1->verts[k + 1 + offset1].xyz, grid1->widthLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
		}
	}
	for (n = 0; n < 2; n++) {
		//
		if (n) offset1 = grid1->width-1;
		else offset1 = 0;
		if (R_MergedHeightPoints(grid1, offset1))
			continue;
		for (k = 0; k < grid1->height-2; k += 2) {
			for (m = 0; m < 2; m++) {

				if ( grid2->width >= MAX_GRID_SIZE )
					break;
				if (m) offset2 = (grid2->height-1) * grid2->width;
				else offset2 = 0;
				for ( l = 0; l < grid2->width-1; l++) {
				//
					v1 = grid1->verts[grid1->width * k + offset1].xyz;
					v2 = grid2->verts[l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[grid1->width * (k + 2) + offset1].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[l + offset2].xyz;
					v2 = grid2->verts[(l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert column into grid2 right after after column l
					if (m) row = grid2->height-1;
					else row = 0;
					grid2 = R_GridInsertColumn( grid2, l+1, row,
									grid1->verts[grid1->width * (k + 1) + offset1].xyz, grid1->heightLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
			for (m = 0; m < 2; m++) {

				if (grid2->height >= MAX_GRID_SIZE)
					break;
				if (m) offset2 = grid2->width-1;
				else offset2 = 0;
				for ( l = 0; l < grid2->height-1; l++) {
				//
					v1 = grid1->verts[grid1->width * k + offset1].xyz;
					v2 = grid2->verts[grid2->width * l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[grid1->width * (k + 2) + offset1].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[grid2->width * l + offset2].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert row into grid2 right after after row l
					if (m) column = grid2->width-1;
					else column = 0;
					grid2 = R_GridInsertRow( grid2, l+1, column,
									grid1->verts[grid1->width * (k + 1) + offset1].xyz, grid1->heightLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
		}
	}
	for (n = 0; n < 2; n++) {
		//
		if (n) offset1 = (grid1->height-1) * grid1->width;
		else offset1 = 0;
		if (R_MergedWidthPoints(grid1, offset1))
			continue;
		for (k = grid1->width-1; k > 1; k -= 2) {

			for (m = 0; m < 2; m++) {

				if ( grid2->width >= MAX_GRID_SIZE )
					break;
				if (m) offset2 = (grid2->height-1) * grid2->width;
				else offset2 = 0;
				for ( l = 0; l < grid2->width-1; l++) {
				//
					v1 = grid1->verts[k + offset1].xyz;
					v2 = grid2->verts[l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[k - 2 + offset1].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[l + offset2].xyz;
					v2 = grid2->verts[(l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert column into grid2 right after after column l
					if (m) row = grid2->height-1;
					else row = 0;
					grid2 = R_GridInsertColumn( grid2, l+1, row,
										grid1->verts[k - 1 + offset1].xyz, grid1->widthLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
			for (m = 0; m < 2; m++) {

				if (grid2->height >= MAX_GRID_SIZE)
					break;
				if (m) offset2 = grid2->width-1;
				else offset2 = 0;
				for ( l = 0; l < grid2->height-1; l++) {
				//
					v1 = grid1->verts[k + offset1].xyz;
					v2 = grid2->verts[grid2->width * l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[k - 2 + offset1].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[grid2->width * l + offset2].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert row into grid2 right after after row l
					if (m) column = grid2->width-1;
					else column = 0;
					grid2 = R_GridInsertRow( grid2, l+1, column,
										grid1->verts[k - 1 + offset1].xyz, grid1->widthLodError[k+1]);
					if (!grid2)
						break;
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
		}
	}
	for (n = 0; n < 2; n++) {
		//
		if (n) offset1 = grid1->width-1;
		else offset1 = 0;
		if (R_MergedHeightPoints(grid1, offset1))
			continue;
		for (k = grid1->height-1; k > 1; k -= 2) {
			for (m = 0; m < 2; m++) {

				if ( grid2->width >= MAX_GRID_SIZE )
					break;
				if (m) offset2 = (grid2->height-1) * grid2->width;
				else offset2 = 0;
				for ( l = 0; l < grid2->width-1; l++) {
				//
					v1 = grid1->verts[grid1->width * k + offset1].xyz;
					v2 = grid2->verts[l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[grid1->width * (k - 2) + offset1].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[l + offset2].xyz;
					v2 = grid2->verts[(l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert column into grid2 right after after column l
					if (m) row = grid2->height-1;
					else row = 0;
					grid2 = R_GridInsertColumn( grid2, l+1, row,
										grid1->verts[grid1->width * (k - 1) + offset1].xyz, grid1->heightLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
			for (m = 0; m < 2; m++) {

				if (grid2->height >= MAX_GRID_SIZE)
					break;
				if (m) offset2 = grid2->width-1;
				else offset2 = 0;
				for ( l = 0; l < grid2->height-1; l++) {
				//
					v1 = grid1->verts[grid1->width * k + offset1].xyz;
					v2 = grid2->verts[grid2->width * l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[grid1->width * (k - 2) + offset1].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[grid2->width * l + offset2].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert row into grid2 right after after row l
					if (m) column = grid2->width-1;
					else column = 0;
					grid2 = R_GridInsertRow( grid2, l+1, column,
										grid1->verts[grid1->width * (k - 1) + offset1].xyz, grid1->heightLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
		}
	}
	return qfalse;
}

/*
===============
R_TryStitchPatch

This function will try to stitch patches in the same LoD group together for the highest LoD.

Only single missing vertice cracks will be fixed.

Vertices will be joined at the patch side a crack is first found, at the other side
of the patch (on the same row or column) the vertices will not be joined and cracks
might still appear at that side.
===============
*/
int R_TryStitchingPatch( int grid1num ) {
	int j, numstitches;
	srfGridMesh_t *grid1, *grid2;

	numstitches = 0;
	grid1 = (srfGridMesh_t *) s_worldData.surfaces[grid1num].data;
	for ( j = 0; j < s_worldData.numsurfaces; j++ ) {
		//
		grid2 = (srfGridMesh_t *) s_worldData.surfaces[j].data;
		// if this surface is not a grid
		if ( grid2->surfaceType != SF_GRID ) continue;
		// grids in the same LOD group should have the exact same lod radius
		if ( grid1->lodRadius != grid2->lodRadius ) continue;
		// grids in the same LOD group should have the exact same lod origin
		if ( grid1->lodOrigin[0] != grid2->lodOrigin[0] ) continue;
		if ( grid1->lodOrigin[1] != grid2->lodOrigin[1] ) continue;
		if ( grid1->lodOrigin[2] != grid2->lodOrigin[2] ) continue;
		//
		while (R_StitchPatches(grid1num, j))
		{
			numstitches++;
		}
	}
	return numstitches;
}

/*
===============
R_StitchAllPatches
===============
*/
void R_StitchAllPatches( void ) {
	int i, stitched, numstitches;
	srfGridMesh_t *grid1;

	numstitches = 0;
	do
	{
		stitched = qfalse;
		for ( i = 0; i < s_worldData.numsurfaces; i++ ) {
			//
			grid1 = (srfGridMesh_t *) s_worldData.surfaces[i].data;
			// if this surface is not a grid
			if ( grid1->surfaceType != SF_GRID )
				continue;
			//
			if ( grid1->lodStitched )
				continue;
			//
			grid1->lodStitched = qtrue;
			stitched = qtrue;
			//
			numstitches += R_TryStitchingPatch( i );
		}
	}
	while (stitched);
	ri.Printf( PRINT_DEVELOPER, "stitched %d LoD cracks\n", numstitches );
}

/*
===============
R_MovePatchSurfacesToHunk
===============
*/
void R_MovePatchSurfacesToHunk(void) {
	int i, size;
	srfGridMesh_t *grid, *hunkgrid;

	for ( i = 0; i < s_worldData.numsurfaces; i++ ) {
		//
		grid = (srfGridMesh_t *) s_worldData.surfaces[i].data;
		// if this surface is not a grid
		if ( grid->surfaceType != SF_GRID )
			continue;
		//
		size = (grid->width * grid->height - 1) * sizeof( drawVert_t ) + sizeof( *grid );
		hunkgrid = ri.Hunk_Alloc( size, h_low );
		Com_Memcpy(hunkgrid, grid, size);

		hunkgrid->widthLodError = ri.Hunk_Alloc( grid->width * 4, h_low );
		Com_Memcpy( hunkgrid->widthLodError, grid->widthLodError, grid->width * 4 );

		hunkgrid->heightLodError = ri.Hunk_Alloc( grid->height * 4, h_low );
		Com_Memcpy( hunkgrid->heightLodError, grid->heightLodError, grid->height * 4 );

		R_FreeSurfaceGridMesh( grid );

		s_worldData.surfaces[i].data = (void *) hunkgrid;
	}
}

/*
===============
R_LoadSurfaces
===============
*/
static	void R_LoadSurfaces( lump_t *surfs, lump_t *verts, lump_t *indexLump ) {
	dsurface_t	*in;
	msurface_t	*out;
	drawVert_t	*dv;
	int			*indexes;
	int			count;
	int			numFaces, numMeshes, numTriSurfs, numFlares;
	int			i;

	numFaces = 0;
	numMeshes = 0;
	numTriSurfs = 0;
	numFlares = 0;

	in = (void *)(fileBase + surfs->fileofs);
	if (surfs->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	count = surfs->filelen / sizeof(*in);

	dv = (void *)(fileBase + verts->fileofs);
	if (verts->filelen % sizeof(*dv))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);

	indexes = (void *)(fileBase + indexLump->fileofs);
	if ( indexLump->filelen % sizeof(*indexes))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);

	out = ri.Hunk_Alloc ( count * sizeof(*out), h_low );	

	s_worldData.surfaces = out;
	s_worldData.numsurfaces = count;

	for ( i = 0 ; i < count ; i++, in++, out++ ) {
		switch ( LittleLong( in->surfaceType ) ) {
		case MST_PATCH:
			ParseMesh ( in, dv, out );
			numMeshes++;
			break;
		case MST_TRIANGLE_SOUP:
			ParseTriSurf( in, dv, out, indexes );
			numTriSurfs++;
			break;
		case MST_PLANAR:
			ParseFace( in, dv, out, indexes );
			numFaces++;
			break;
		case MST_FLARE:
			ParseFlare( in, dv, out, indexes );
			numFlares++;
			break;
		default:
			ri.Error( ERR_DROP, "Bad surfaceType" );
		}
	}

#ifdef PATCH_STITCHING
	R_StitchAllPatches();
#endif

	R_FixSharedVertexLodError();

#ifdef PATCH_STITCHING
	R_MovePatchSurfacesToHunk();
#endif

	ri.Printf( PRINT_DEVELOPER, "...loaded %d faces, %i meshes, %i trisurfs, %i flares\n", 
		numFaces, numMeshes, numTriSurfs, numFlares );
}



/*
=================
R_LoadSubmodels
=================
*/
static	void R_LoadSubmodels( lump_t *l ) {
	dmodel_t	*in;
	bmodel_t	*out;
	int			i, j, count;

	in = (void *)(fileBase + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	count = l->filelen / sizeof(*in);

	s_worldData.bmodels = out = ri.Hunk_Alloc( count * sizeof(*out), h_low );
	s_worldData.numbmodels = count;

	for ( i=0 ; i<count ; i++, in++, out++ ) {
		model_t *model;

		model = R_AllocModel();

		assert( model != NULL );			// this should never happen
		if ( model == NULL ) {
			ri.Error(ERR_DROP, "R_LoadSubmodels: R_AllocModel() failed");
		}

		model->type = MOD_BRUSH;
		model->bmodel = out;
		Com_sprintf( model->name, sizeof( model->name ), "*%d", i );

		for (j=0 ; j<3 ; j++) {
			out->bounds[0][j] = LittleFloat (in->mins[j]);
			out->bounds[1][j] = LittleFloat (in->maxs[j]);
		}

		out->firstSurface = s_worldData.surfaces + LittleLong( in->firstSurface );
		out->numSurfaces = LittleLong( in->numSurfaces );
	}
}



//==================================================================

/*
=================
R_SetParent
=================
*/
static	void R_SetParent (mnode_t *node, mnode_t *parent)
{
	node->parent = parent;
	if (node->contents != -1)
		return;
	R_SetParent (node->children[0], node);
	R_SetParent (node->children[1], node);
}

/*
=================
R_LoadNodesAndLeafs
=================
*/
static	void R_LoadNodesAndLeafs (lump_t *nodeLump, lump_t *leafLump) {
	int			i, j, p;
	dnode_t		*in;
	dleaf_t		*inLeaf;
	mnode_t 	*out;
	int			numNodes, numLeafs;

	in = (void *)(fileBase + nodeLump->fileofs);
	if (nodeLump->filelen % sizeof(dnode_t) ||
		leafLump->filelen % sizeof(dleaf_t) ) {
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	}
	numNodes = nodeLump->filelen / sizeof(dnode_t);
	numLeafs = leafLump->filelen / sizeof(dleaf_t);

	out = ri.Hunk_Alloc ( (numNodes + numLeafs) * sizeof(*out), h_low);	

	s_worldData.nodes = out;
	s_worldData.numnodes = numNodes + numLeafs;
	s_worldData.numDecisionNodes = numNodes;

	// load nodes
	for ( i=0 ; i<numNodes; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->mins[j] = LittleLong (in->mins[j]);
			out->maxs[j] = LittleLong (in->maxs[j]);
		}
	
		p = LittleLong(in->planeNum);
		out->plane = s_worldData.planes + p;

		out->contents = CONTENTS_NODE;	// differentiate from leafs

		for (j=0 ; j<2 ; j++)
		{
			p = LittleLong (in->children[j]);
			if (p >= 0)
				out->children[j] = s_worldData.nodes + p;
			else
				out->children[j] = s_worldData.nodes + numNodes + (-1 - p);
		}
	}
	
	// load leafs
	inLeaf = (void *)(fileBase + leafLump->fileofs);
	for ( i=0 ; i<numLeafs ; i++, inLeaf++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->mins[j] = LittleLong (inLeaf->mins[j]);
			out->maxs[j] = LittleLong (inLeaf->maxs[j]);
		}

		out->cluster = LittleLong(inLeaf->cluster);
		out->area = LittleLong(inLeaf->area);

		if ( out->cluster >= s_worldData.numClusters ) {
			s_worldData.numClusters = out->cluster + 1;
		}

		out->firstmarksurface = s_worldData.marksurfaces +
			LittleLong(inLeaf->firstLeafSurface);
		out->nummarksurfaces = LittleLong(inLeaf->numLeafSurfaces);

		
		//allocate some room for the decals
		if (r_decals->integer>0)
		{
			//out->decalTris=ri.Hunk_Alloc( sizeof(msurface_t)*out->nummarksurfaces, h_low );
			//memset(out->decalsurfs,0,sizeof(msurface_t)*out->nummarksurfaces);
			out->numDecalTris=0;
			//backup
			out->o_firstmarksurface = s_worldData.marksurfaces +
				LittleLong(inLeaf->firstLeafSurface);
			out->o_nummarksurfaces = LittleLong(inLeaf->numLeafSurfaces);
		}
	
		//newvis
		out->newVisCluster=-1;
	}	

	// chain decendants
	R_SetParent (s_worldData.nodes, NULL);
}

//=============================================================================

/*
=================
R_LoadShaders
=================
*/
static	void R_LoadShaders( lump_t *l ) {	
	int		i, count;
	dshader_t	*in, *out;
	
	in = (void *)(fileBase + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	count = l->filelen / sizeof(*in);
	out = ri.Hunk_Alloc ( count*sizeof(*out), h_low );

	s_worldData.shaders = out;
	s_worldData.numShaders = count;

	Com_Memcpy( out, in, count*sizeof(*out) );

	for ( i=0 ; i<count ; i++ ) {
		out[i].surfaceFlags = LittleLong( out[i].surfaceFlags );
		out[i].contentFlags = LittleLong( out[i].contentFlags );
	}
}


/*
=================
R_LoadMarksurfaces
=================
*/
static	void R_LoadMarksurfaces (lump_t *l)
{	
	int		i, j, count;
	int		*in;
	msurface_t **out;
	
	in = (void *)(fileBase + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	count = l->filelen / sizeof(*in);
	out = ri.Hunk_Alloc ( count*sizeof(*out), h_low);	

	s_worldData.marksurfaces = out;
	s_worldData.nummarksurfaces = count;

	for ( i=0 ; i<count ; i++)
	{
		j = LittleLong(in[i]);
		out[i] = s_worldData.surfaces + j;
	}
}


/*
=================
R_LoadPlanes
=================
*/
static	void R_LoadPlanes( lump_t *l ) {
	int			i, j;
	cplane_t	*out;
	dplane_t 	*in;
	int			count;
	int			bits;
	
	in = (void *)(fileBase + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	count = l->filelen / sizeof(*in);
	out = ri.Hunk_Alloc ( count*2*sizeof(*out), h_low);	
	
	s_worldData.planes = out;
	s_worldData.numplanes = count;

	for ( i=0 ; i<count ; i++, in++, out++) {
		bits = 0;
		for (j=0 ; j<3 ; j++) {
			out->normal[j] = LittleFloat (in->normal[j]);
			if (out->normal[j] < 0) {
				bits |= 1<<j;
			}
		}

		out->dist = LittleFloat (in->dist);
		out->type = PlaneTypeForNormal( out->normal );
		out->signbits = bits;
	}
}

/*
=================
R_LoadFogs

=================
*/
static	void R_LoadFogs( lump_t *l, lump_t *brushesLump, lump_t *sidesLump ) {
	int			i;
	fog_t		*out;
	dfog_t		*fogs;
	dbrush_t 	*brushes, *brush;
	dbrushside_t	*sides;
	int			count, brushesCount, sidesCount;
	int			sideNum;
	int			planeNum;
	shader_t	*shader;
	float		d;
	int			firstSide;

	fogs = (void *)(fileBase + l->fileofs);
	if (l->filelen % sizeof(*fogs)) {
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	}
	count = l->filelen / sizeof(*fogs);

	// create fog strucutres for them
	s_worldData.numfogs = count + 1;
	s_worldData.fogs = ri.Hunk_Alloc ( s_worldData.numfogs*sizeof(*out), h_low);
	out = s_worldData.fogs + 1;

	if ( !count ) {
		return;
	}

	brushes = (void *)(fileBase + brushesLump->fileofs);
	if (brushesLump->filelen % sizeof(*brushes)) {
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	}
	brushesCount = brushesLump->filelen / sizeof(*brushes);

	sides = (void *)(fileBase + sidesLump->fileofs);
	if (sidesLump->filelen % sizeof(*sides)) {
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	}
	sidesCount = sidesLump->filelen / sizeof(*sides);

	for ( i=0 ; i<count ; i++, fogs++) {
		out->originalBrushNumber = LittleLong( fogs->brushNum );

		if ( (unsigned)out->originalBrushNumber >= brushesCount ) {
			ri.Error( ERR_DROP, "fog brushNumber out of range" );
		}
		brush = brushes + out->originalBrushNumber;

		firstSide = LittleLong( brush->firstSide );

			if ( (unsigned)firstSide > sidesCount - 6 ) {
			ri.Error( ERR_DROP, "fog brush sideNumber out of range" );
		}

		// brushes are always sorted with the axial sides first
		sideNum = firstSide + 0;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[0][0] = -s_worldData.planes[ planeNum ].dist;

		sideNum = firstSide + 1;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[1][0] = s_worldData.planes[ planeNum ].dist;

		sideNum = firstSide + 2;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[0][1] = -s_worldData.planes[ planeNum ].dist;

		sideNum = firstSide + 3;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[1][1] = s_worldData.planes[ planeNum ].dist;

		sideNum = firstSide + 4;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[0][2] = -s_worldData.planes[ planeNum ].dist;

		sideNum = firstSide + 5;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[1][2] = s_worldData.planes[ planeNum ].dist;

		// get information from the shader for fog parameters
		shader = R_FindShader( fogs->shader, LIGHTMAP_NONE, qtrue );

		out->parms = shader->fogParms;

		out->colorInt = ColorBytes4 ( shader->fogParms.color[0] * tr.identityLight, 
			                          shader->fogParms.color[1] * tr.identityLight, 
			                          shader->fogParms.color[2] * tr.identityLight, 1.0 );

		d = shader->fogParms.depthForOpaque < 1 ? 1 : shader->fogParms.depthForOpaque;
		out->tcScale = 1.0f / ( d * 8 );

		// set the gradient vector
		sideNum = LittleLong( fogs->visibleSide );

		if ( sideNum == -1 ) {
			out->hasSurface = qfalse;
		} else {
			out->hasSurface = qtrue;
			planeNum = LittleLong( sides[ firstSide + sideNum ].planeNum );
			VectorSubtract( vec3_origin, s_worldData.planes[ planeNum ].normal, out->surface );
			out->surface[3] = -s_worldData.planes[ planeNum ].dist;
		}

		out++;
	}

}


/*
================
R_LoadLightGrid

================
*/
void R_LoadLightGrid( lump_t *l ) {
	int		i;
	vec3_t	maxs;
	int		numGridPoints;
	world_t	*w;
	float	*wMins, *wMaxs;

	w = &s_worldData;

	w->lightGridInverseSize[0] = 1.0f / w->lightGridSize[0];
	w->lightGridInverseSize[1] = 1.0f / w->lightGridSize[1];
	w->lightGridInverseSize[2] = 1.0f / w->lightGridSize[2];

	wMins = w->bmodels[0].bounds[0];
	wMaxs = w->bmodels[0].bounds[1];

	for ( i = 0 ; i < 3 ; i++ ) {
		w->lightGridOrigin[i] = w->lightGridSize[i] * ceil( wMins[i] / w->lightGridSize[i] );
		maxs[i] = w->lightGridSize[i] * floor( wMaxs[i] / w->lightGridSize[i] );
		w->lightGridBounds[i] = (maxs[i] - w->lightGridOrigin[i])/w->lightGridSize[i] + 1;
	}

	numGridPoints = w->lightGridBounds[0] * w->lightGridBounds[1] * w->lightGridBounds[2];

	if ( l->filelen != numGridPoints * 8 ) {
		ri.Printf( PRINT_WARNING, "WARNING: light grid mismatch\n" );
		w->lightGridData = NULL;
		return;
	}

	w->lightGridData = ri.Hunk_Alloc( l->filelen, h_low );
	Com_Memcpy( w->lightGridData, (void *)(fileBase + l->fileofs), l->filelen );

	// deal with overbright bits
	for ( i = 0 ; i < numGridPoints ; i++ ) {
		R_ColorShiftLightingBytes( &w->lightGridData[i*8], &w->lightGridData[i*8] );
		R_ColorShiftLightingBytes( &w->lightGridData[i*8+3], &w->lightGridData[i*8+3] );
	}
}

void R_LoadEntities( lump_t *l ) {
	char *p, *token, *s;
	char keyname[MAX_TOKEN_CHARS];
	char value[MAX_TOKEN_CHARS];
	world_t	*w;
	float red,green,blue,density;

	density=0.0;
	red=0.25;
	green=0.25;
	blue=0.25;


	w = &s_worldData;
	w->lightGridSize[0] = 64;
	w->lightGridSize[1] = 64;
	w->lightGridSize[2] = 128;

	p = (char *)(fileBase + l->fileofs);

	// store for reference by the cgame
	w->entityString = ri.Hunk_Alloc( l->filelen + 1, h_low );
	strcpy( w->entityString, p );
	w->entityParsePoint = w->entityString;

	token = COM_ParseExt( &p, qtrue );
	if (!*token || *token != '{') {
		return;
	}

	// only parse the world spawn
	while ( 1 ) {	
		// parse key
		token = COM_ParseExt( &p, qtrue );

		if ( !*token || *token == '}' ) {
			break;
		}
		Q_strncpyz(keyname, token, sizeof(keyname));

		// parse value
		token = COM_ParseExt( &p, qtrue );

		if ( !*token || *token == '}' ) {
			break;
		}
		Q_strncpyz(value, token, sizeof(value));

		// check for remapping of shaders for vertex lighting
		s = "vertexremapshader";
		if (!Q_strncmp(keyname, s, strlen(s)) ) {
			s = strchr(value, ';');
			if (!s) {
				ri.Printf( PRINT_WARNING, "WARNING: no semi colon in vertexshaderremap '%s'\n", value );
				break;
			}
			*s++ = 0;
			if (r_vertexLight->integer) {
				R_RemapShader(value, s, "0");
			}
			continue;
		}
		// check for remapping of shaders
		s = "remapshader";
		if (!Q_strncmp(keyname, s, strlen(s)) ) {
			s = strchr(value, ';');
			if (!s) {
				ri.Printf( PRINT_WARNING, "WARNING: no semi colon in shaderremap '%s'\n", value );
				break;
			}
			*s++ = 0;
			R_RemapShader(value, s, "0");
			continue;
		}
		// check for a different grid size
		if (!Q_stricmp(keyname, "gridsize")) {
			sscanf(value, "%f %f %f", &w->lightGridSize[0], &w->lightGridSize[1], &w->lightGridSize[2] );
			continue;
		}

		//check for fog
		if (!Q_stricmp(keyname, "globalfog")) {
			sscanf(&value[0], " %f %f %f %f",&red,&green,&blue ,&density);
			
			
			continue;
		}

	}

	//set the fog
	Cvar_SetValue(r_fog_r->name,red);
	Cvar_SetValue(r_fog_g->name,green);
	Cvar_SetValue(r_fog_b->name,blue);
	Cvar_SetValue(r_fogdensity->name,density);

	r_fog_r->resetString=CopyString(r_fog_r->string);
	r_fog_g->resetString=CopyString(r_fog_g->string);
	r_fog_b->resetString=CopyString(r_fog_b->string);
	r_fogdensity->resetString=CopyString(r_fogdensity->string);


}

/*
=================
R_GetEntityToken
=================
*/
qboolean R_GetEntityToken( char *buffer, int size ) {
	const char	*s;

	s = COM_Parse( &s_worldData.entityParsePoint );
	Q_strncpyz( buffer, s, size );
	if ( !s_worldData.entityParsePoint || !s[0] ) {
		s_worldData.entityParsePoint = s_worldData.entityString;
		return qfalse;
	} else {
		return qtrue;
	}
}

/*
=================
RE_LoadWorldMap

Called directly from cgame
=================
*/
void RE_LoadWorldMap( const char *name ) {
	int			i;
	dheader_t	*header;
	union {
		byte *b;
		void *v;
	} buffer;
	byte		*startMarker;

	if ( tr.worldMapLoaded ) {
		ri.Error( ERR_DROP, "ERROR: attempted to redundantly load world map\n" );
	}

	// set default sun direction to be used if it isn't
	// overridden by a shader
	tr.sunDirection[0] = 0.45f;
	tr.sunDirection[1] = 0.3f;
	tr.sunDirection[2] = 0.9f;

	VectorNormalize( tr.sunDirection );

	tr.worldMapLoaded = qtrue;

	// load it
	ri.FS_ReadFile( name, &buffer.v );

	if ( !buffer.b ) {
		ri.Error (ERR_DROP, "RE_LoadWorldMap: %s not found", name);
	}

	// clear tr.world so if the level fails to load, the next
	// try will not look at the partially loaded version
	tr.world = NULL;

	Com_Memset( &s_worldData, 0, sizeof( s_worldData ) );
	Q_strncpyz( s_worldData.name, name, sizeof( s_worldData.name ) );

	Q_strncpyz( s_worldData.baseName, COM_SkipPath( s_worldData.name ), sizeof( s_worldData.name ) );
	COM_StripExtension(s_worldData.baseName, s_worldData.baseName, sizeof(s_worldData.baseName));

	startMarker = ri.Hunk_Alloc(0, h_low);
	c_gridVerts = 0;

	header = (dheader_t *)buffer.b;
	fileBase = (byte *)header;

	i = LittleLong (header->version);
	if ( i != BSP_VERSION ) {
		ri.Error (ERR_DROP, "RE_LoadWorldMap: %s has wrong version number (%i should be %i)", 
			name, i, BSP_VERSION);
	}

	// swap all the lumps
	for (i=0 ; i<sizeof(dheader_t)/4 ; i++) {
		((int *)header)[i] = LittleLong ( ((int *)header)[i]);
	}

	if ( r_ext_vertex_buffer_object->integer ) {
		//VBo's, temporary allocate some vertex room
		tr.staticVertexData =  malloc( sizeof(staticVert_t)* r_maxstaticverts->integer );
		tr.staticIndexData =  malloc( sizeof(int)* r_maxstaticverts->integer );
	} 

	// load into heap
	R_LoadShaders( &header->lumps[LUMP_SHADERS] );
	// R_LoadLightmaps( &header->lumps[LUMP_LIGHTMAPS] );
	R_CheckForRadbumping( name ); //do this before loading lightmaps
	R_LoadFatLightmaps( &header->lumps[LUMP_LIGHTMAPS] );
	R_LoadRadFile( name,  &header->lumps[LUMP_LIGHTMAPS]  );

	R_LoadPlanes ( &header->lumps[LUMP_PLANES] );
	R_LoadFogs( &header->lumps[LUMP_FOGS], &header->lumps[LUMP_BRUSHES], &header->lumps[LUMP_BRUSHSIDES] );

	if ( r_decals->integer ) {
		TempDecalStore=malloc( r_maxstaticverts->integer*sizeof( srfSurfaceDecal_t ) );
		numTempDecalStore=0;
	}

	R_LoadSurfaces( &header->lumps[LUMP_SURFACES], &header->lumps[LUMP_DRAWVERTS], &header->lumps[LUMP_DRAWINDEXES] );
	R_LoadMarksurfaces( &header->lumps[LUMP_LEAFSURFACES] );
	R_LoadNodesAndLeafs( &header->lumps[LUMP_NODES], &header->lumps[LUMP_LEAFS]) ;
	
	if (r_decals->integer)
	{
		R_AddDecalSurfs();
		//free the temp decal storage
		free( TempDecalStore );
	}

	R_LoadSubmodels( &header->lumps[LUMP_MODELS] );
	R_LoadVisibility( &header->lumps[LUMP_VISIBILITY] );
	R_LoadEntities( &header->lumps[LUMP_ENTITIES] );
	R_LoadLightGrid( &header->lumps[LUMP_LIGHTGRID] );

	s_worldData.dataSize = (byte *)ri.Hunk_Alloc(0, h_low) - startMarker;

	// only set tr.world now that we know the entire level has loaded properly
	tr.world = &s_worldData;

	///27: R_InitVBO's here
	if ( r_ext_vertex_buffer_object->integer )
	{
		ri.Printf( PRINT_ALL, "R_VBO Initializing...\n" );
		R_OptimizeSurfaces_bySurface();
		
		R_CalculateTangents();
		R_InitVBO();

		//Free VBo's temporary memory.  Note that you have to free in this order
		free(tr.staticIndexData);
		free(tr.staticVertexData);
	}

	ri.FS_FreeFile( buffer.v );

	//Clear the texture binding on all TMU's now
	//This stops the issue with only 1 lightmap being loaded - long standing q3 bug
	//(white lightmaps issue on abbey)
	Com_Memset( glState.currenttextures, 0, sizeof( glState.currenttextures ) );

	// Build up the cubemaps now
	R_BuildCubeMaps();

}


static void R_OptimizeSurfaces_bySurface()
{
	//Flip through the surfaces and try and collapse them
	int j,r,q,i;

	srfSurfaceStatic_t *sv;
	srfSurfaceStatic_t *sc;

	int			c;
	msurface_t	*surf, **mark;
	qboolean CanSee;


	int leafCount=0;
	mnode_t *Leafs[1024];


	int vertcount=0;
	int indexcount=0;

	int surfcount=0;

	unsigned int *IndexData;
	staticVert_t* VertexData;
	vec3_t mins;
	vec3_t maxs;

	VertexData =  malloc( sizeof(staticVert_t)*tr.numStaticVertex );
	IndexData =  malloc( sizeof(int)*tr.numStaticIndex );


	for (j=0;j<s_worldData.numsurfaces;j++)
	{
		sv =(void*)s_worldData.surfaces[j].data;
		
		if (sv->surfaceType!=SF_STATIC) continue; //otherwise makes no sense
		sv->merged=-1;
		sv->modelNum=-1;
	}


	//flip through all the bmodels and mark which surfaces they own
	for (j=0;j<s_worldData.numbmodels;j++)
	{
		msurface_t *surf;
		bmodel_t *model;
		model=&s_worldData.bmodels[j];

		for (r=0;r<model->numSurfaces;r++)
		{
			surf = model->firstSurface+r;
			//sv=surf->data;
			
			if (*surf->data==SF_STATIC)
			{
				sv =(void*)surf->data;
				sv->modelNum=j;
			}
		}
	}


 	for (j=0;j<s_worldData.numsurfaces;j++)
	{
		sv =(void*)s_worldData.surfaces[j].data;
		
		if (sv->surfaceType!=SF_STATIC) continue; //otherwise makes no sense
		if (sv->merged>-1) continue;

		//start by copying this surfaces data 
		for (q=0;q<sv->numIndices;q++)
		{
			IndexData[indexcount+q]= ( tr.staticIndexData[sv->startIndex+q]-sv->startVertex)+vertcount;
		}
		for (q=0;q<sv->numVerts;q++)
		{
			memcpy(&VertexData[vertcount+q], &tr.staticVertexData[sv->startVertex+q],sizeof(staticVert_t));
		}
		sv->startIndex=indexcount;
		sv->startVertex=vertcount;
		
		indexcount+=sv->numIndices;
		vertcount+=sv->numVerts;

		leafCount=0;
		
		//Medium Settings - Leaf based merge- obeys vis a bit better, questionable benefit.
		if ( r_ext_vertex_buffer_object->integer >= 2 )
		{
			//record all the leafs that contain this surface		
			for (i=0 ; i<tr.world->numnodes ; i++) 
			{
				if (tr.world->nodes[i].contents != CONTENTS_SOLID) 
				{
					mark = tr.world->nodes[i].firstmarksurface;
					c =  tr.world->nodes[i].nummarksurfaces;
					while (c--) 
					{
						surf = *mark;
						if ((void*)surf->data==(void*)sv) 
						{
							Leafs[leafCount++]=	&tr.world->nodes[i];	
							if (leafCount==1023) break;
						}
						mark++;
					}
				}
			}
		}
		

		//Add any compatible surfaces in as well
		for (r=0;r<s_worldData.numsurfaces;r++)
		{
			
			if (r==j) continue;
			
			if ( !r_ext_vertex_buffer_object->integer || 
				r_ext_vertex_buffer_object->integer == 3 ) 
				continue; //R_vbo 3 == no merging
			
			sc =(void*)s_worldData.surfaces[r].data;
			if (sc->surfaceType!=SF_STATIC) continue; 
			if (sc->merged>-1) continue;
			
			// model check so we don't gather doors etc
			if (sc->modelNum!=sv->modelNum) continue;  
			
			if (  sc->shader->nameHash!= sv->shader->nameHash) continue;
			if ( sc->shader->lightmapIndex!=sv->shader->lightmapIndex ) continue;
			
			if (sc->shader->isFancyWater && sc->shader->isFancyWater) continue; //No water merge
			{

				//Do not merge surfaces unless they're shared by a leaf
				//ie: close to each other
				if ( r_ext_vertex_buffer_object->integer == 2 )
				{
					CanSee=qfalse;
					for (q=0;q<leafCount;q++)
					{
						mark = Leafs[q]->firstmarksurface;
						c =  Leafs[q]->nummarksurfaces;
						while (c--) 
						{
							surf = *mark;
							
							if ((void*)surf->data==(void*)sc ) 
							{
								CanSee=qtrue;
							}
							mark++;
							if (CanSee) break;
						}
						if (CanSee) break;
					}
					
					if (CanSee==qfalse) 
						continue;
				}
	
				//Merge now
				sv->merged++;
				sc->merged++;
				
				//Add these two surfaces together
				for (q=0;q<sc->numIndices;q++)
				{
					IndexData[indexcount+q]= ( tr.staticIndexData[sc->startIndex+q]-sc->startVertex)+vertcount;
				}
				for (q=0;q<sc->numVerts;q++)
				{
					memcpy(&VertexData[vertcount+q], &tr.staticVertexData[sc->startVertex+q],sizeof(staticVert_t));
				}
				sv->numIndices+=sc->numIndices;
				sv->numVerts+=sc->numVerts;
				
				indexcount+=sc->numIndices;
				vertcount+=sc->numVerts;

				sc->numIndices=0;
 				sc->numVerts=0;
				//Modify all leafs that pointed at SC to point to SV
				
			 	
				for (i=0 ; i<tr.world->numnodes ; i++) 
				{
					if (tr.world->nodes[i].contents != CONTENTS_SOLID) 
					{
						mark = tr.world->nodes[i].firstmarksurface;
						c =  tr.world->nodes[i].nummarksurfaces;
						while (c--) 
						{
							surf = *mark;
							if ((void*)surf->data==(void*)sc) 
							{
								//surf->data=sv;
								*mark=&s_worldData.surfaces[j];
							}

							mark++;
						}
					}
				}
			}
		}
	}

	memcpy(tr.staticVertexData,VertexData,sizeof(staticVert_t)*tr.numStaticVertex);
	memcpy(tr.staticIndexData,IndexData,sizeof(unsigned int)*tr.numStaticIndex);


	free(IndexData);
	free(VertexData);	


	//Assign origins now
 	for (j=0;j<s_worldData.numsurfaces;j++)
	{
		sv =(void*)s_worldData.surfaces[j].data;
		
		if (sv->surfaceType!=SF_STATIC) continue; //

		if (sv->numIndices==0 || sv->numVerts==0) continue;

		mins[0]=mins[1]=mins[2]=999999;
		maxs[0]=maxs[1]=maxs[2]=-999999;

		for (q=0;q<sv->numVerts;q++)
		{
			for (i=0;i<3;i++)
			{
				if (tr.staticVertexData[sv->startVertex+q].position[i]<mins[i])
					mins[i]=tr.staticVertexData[sv->startVertex+q].position[i];
				if (tr.staticVertexData[sv->startVertex+q].position[i]>maxs[i])
					maxs[i]=tr.staticVertexData[sv->startVertex+q].position[i];
			}
		}
		sv->origin[0]=(mins[0]+maxs[0])/2;
		sv->origin[1]=(mins[1]+maxs[1])/2;
		sv->origin[2]=(mins[2]+maxs[2])/2;

		surfcount++;
	}
	ri.Printf( PRINT_ALL, "R_VBO Surf Count: %d\n", surfcount );	
	ri.Printf( PRINT_ALL, "R_VBO Vertex Count: %d\n", vertcount );
	ri.Printf( PRINT_ALL, "R_VBO Index Count: %d\n", indexcount );
}

qboolean CalcTangentVectors(  staticVert_t *dv[3] )
{
	int		i;
	vec3_t		norm;

	vec3_t		face_bi_tangent;
	vec3_t		face_tangent;
	vec3_t		facenorm;
	
	//calculate face tangents
	{
		vec3_t d1;
		vec3_t d2;
		
		float deltaU_0;
		float deltaU_1;
		float deltaV_0;
		float deltaV_1;

		int a,b,c;
		a=0;
		b=1;
		c=2;

		VectorSubtract( dv[b]->position,dv[a]->position,d1);
		VectorSubtract( dv[c]->position,dv[a]->position,d2);

		//calculate the face normal
		CrossProduct(d1,d2,facenorm);
		VectorNormalize(facenorm);
		facenorm[0]=-facenorm[0];
		facenorm[1]=-facenorm[1];
		facenorm[2]=-facenorm[2];


		deltaU_0 = dv[b]->uv[0] - dv[a]->uv[0];
		deltaU_1 = dv[c]->uv[0] - dv[a]->uv[0];

		deltaV_0 = dv[b]->uv[1] - dv[a]->uv[1];
		deltaV_1 = dv[c]->uv[1] - dv[a]->uv[1];

		//face_tangent =  normalize( delta_V_1 * side_0 - delta_V_0 * side_1 );
		//face_bi_tangent =  normalize( delta_U_1 * side_0 - delta_U_0 * side_1 );
		
		face_tangent[0] =( deltaV_1 * d1[0] - deltaV_0 * d2[0]);
        face_tangent[1] =( deltaV_1 * d1[1] - deltaV_0 * d2[1]);
        face_tangent[2] =( deltaV_1 * d1[2] - deltaV_0 * d2[2]);

        face_bi_tangent[0] =( deltaU_1 * d1[0] - deltaU_0 * d2[0]);
        face_bi_tangent[1] =( deltaU_1 * d1[1] - deltaU_0 * d2[1]);
        face_bi_tangent[2] =( deltaU_1 * d1[2] - deltaU_0 * d2[2]);


		VectorNormalize(face_tangent);
		VectorNormalize(face_bi_tangent);


		//check for opposing tangents
		CrossProduct(face_tangent,face_bi_tangent,norm);
		if (DotProduct(norm,facenorm)<0 ) 
		{
			//flip
			face_bi_tangent[0]=-face_bi_tangent[0];
			face_bi_tangent[1]=-face_bi_tangent[1];
			face_bi_tangent[2]=-face_bi_tangent[2];
			face_tangent[0]=-face_tangent[0];
			face_tangent[1]=-face_tangent[1];
			face_tangent[2]=-face_tangent[2];

		}


	}
	

	/* do each vertex */
	for( i = 0; i < 3; i++ )
	{
		VectorCopy(face_tangent,dv[i]->stv);
		VectorCopy(face_bi_tangent,dv[i]->ttv);
	//	VectorCopy(facenorm,dv[i]->normal);
	}
	
	/* return to caller */
	return qtrue;
}


static void R_CalculateTangents()
{
	staticVert_t* verts[3];


	int j;
	for (j=0;j<tr.numStaticIndex;j+=3)
	{
		verts[0]=&tr.staticVertexData[tr.staticIndexData[j+0]];
		verts[1]=&tr.staticVertexData[tr.staticIndexData[j+1]];
		verts[2]=&tr.staticVertexData[tr.staticIndexData[j+2]];
		
		CalcTangentVectors(verts);
	} 
}


qboolean R_ParseEntitysForKey(char* classname, char* key, char *storage,int sizeofstore, int num)
{
	int stage=-1;
	
	char *p;
	char *s;
	char *token;
	char keyname[MAX_TOKEN_CHARS];
	char value[MAX_TOKEN_CHARS];
	
	int brackets;
	int classtype;

	p=s_worldData.entityString;
	brackets=0;
	classtype=0;
	while ( 1 ) 
	{	
		if (brackets==0)
		{
			//not inside a class
			classtype=0;
		}
			
		// parse key
		token = COM_ParseExt( &p, qtrue );
		if ( !*token ) break;
		if (*token == '}') { brackets--; continue; }
		if (*token == '{') { brackets++; continue; }
		Q_strncpyz(keyname, token, sizeof(keyname));
		
		token = COM_ParseExt( &p, qtrue );
		if ( !*token ) break;
		if (*token == '}') { brackets--; continue; }
		if (*token == '{') { brackets++; continue; }
		Q_strncpyz(value, token, sizeof(value));

		// check for remapping of shaders for vertex lighting
		
		s="classname";
		if (!Q_strncmp(keyname,s, strlen(s)) ) 
		{
			if (!Q_strncmp(value,classname, strlen(classname)) ) 
			{
				classtype++;	
			}
		}
		if (!Q_strncmp(keyname, key, strlen(key)) ) 
		{
			classtype++;
			Q_strncpyz(storage, value, sizeofstore);
		}

		if (classtype==2)
		{
			stage++;
			if (stage<num)
			{
				continue;
			}
			else
			{
				break;
			}

		}
	}
	
	if (stage<num) 
	{
		return qfalse;
	}
	return qtrue;
}

void R_StoreDecalSurf(vec3_t v0,vec3_t v1,vec3_t v2, msurface_t* data)
{
	//Store the surface data of a decal surface and the surf it was from for later
	//We can't just add this directly because the worldnode struct isn't loaded yet
	vec4_t plane;
	TempDecalStore[numTempDecalStore].surf=data;
	TempDecalStore[numTempDecalStore].surfType=SF_DECALSURF;
	TempDecalStore[numTempDecalStore].viewCount=0;
	
	VectorCopy(v0,TempDecalStore[numTempDecalStore].v[0]);
	VectorCopy(v1,TempDecalStore[numTempDecalStore].v[1]);
	VectorCopy(v2,TempDecalStore[numTempDecalStore].v[2]);

	PlaneFromPoints(plane,v0,v1,v2);
	VectorCopy(plane,TempDecalStore[numTempDecalStore].plane.normal);
	TempDecalStore[numTempDecalStore].plane.dist=plane[3];
	TempDecalStore[numTempDecalStore].plane.type=3;
	SetPlaneSignbits(&TempDecalStore[numTempDecalStore].plane);
	
	numTempDecalStore++;
}


//Adds a decal surface to a leaf, if the leaf had a reference to the original surface
//this is incredibly slow.. Ive sped it up as much as I can without prestoring anything.
//The malloc juggling is designed to keep working scratchpad ram low. Final allocations waste nothing.
//Todo: This only adds the r_ext_vertex_buffer_object generated surfaces, but really it could accept any tris that need decalling
//      like tri soups with complex shaders.
void R_AddDecalSurfs()
{
	int i,c;
	int j;
	msurface_t **mark;
	msurface_t *comp;

	mnode_t *leafs[1024];
	int leafCount = 0;
	
	//Copy the temporary list to a final home
	s_worldData.decalTris = ri.Hunk_Alloc( numTempDecalStore * sizeof(srfSurfaceDecal_t), h_low );
	s_worldData.numdecalTris = numTempDecalStore;
	memcpy( s_worldData.decalTris, &TempDecalStore[0], numTempDecalStore * sizeof(srfSurfaceDecal_t) );

	for( i = 0; i < s_worldData.numnodes; i++) 
	{
		if( s_worldData.nodes[i].contents != CONTENTS_SOLID ) 
			s_worldData.nodes[i].numDecalTris = 0;
	}

	for( j = 0, comp = 0; j < numTempDecalStore; j++ )
	{
		// see if we need to calculate a new list of leafs
		if( comp != TempDecalStore[j].surf )
		{
			comp = TempDecalStore[j].surf;
			for( i = 0, leafCount = 0; i < s_worldData.numnodes; i++) 
			{
				if( s_worldData.nodes[i].contents != CONTENTS_SOLID ) 
				{
					mark = s_worldData.nodes[i].o_firstmarksurface;
					c = s_worldData.nodes[i].o_nummarksurfaces;
		
					for( ; leafCount < 1024 && c ; c--, mark++ ) 
						if ( *mark == comp )
							leafs[leafCount++] = &s_worldData.nodes[i];	
				}
			}
		}

		for( i = 0; i < leafCount; i++)
			leafs[i]->numDecalTris++;
		
	}

	for( i = 0; i < s_worldData.numnodes; i++) 
	{
		if( s_worldData.nodes[i].contents != CONTENTS_SOLID ) 
		{
			s_worldData.nodes[i].decalTris = (srfSurfaceDecal_t**)
				ri.Hunk_Alloc( s_worldData.nodes[i].numDecalTris * 
						sizeof(srfSurfaceDecal_t*), h_low );
			s_worldData.nodes[i].numDecalTris = 0;
		}
	}

	for( j = 0, comp = 0; j < numTempDecalStore; j++ )
	{
		// see if we need to calculate a new list of leafs
		if( comp != TempDecalStore[j].surf )
		{
			comp = TempDecalStore[j].surf;
			for( i = 0, leafCount = 0; i < s_worldData.numnodes; i++) 
			{
				if( s_worldData.nodes[i].contents != CONTENTS_SOLID ) 
				{
					mark = s_worldData.nodes[i].o_firstmarksurface;
					c = s_worldData.nodes[i].o_nummarksurfaces;
					for( ; leafCount < 1024 && c; c--, mark++ ) 
						if ( *mark == comp )
							leafs[leafCount++] = &s_worldData.nodes[i];	
				}
			}
		}

		// add this face to every leaf in the list
		for( i = 0; i < leafCount; i++)
			leafs[i]->decalTris[leafs[i]->numDecalTris++] = 
				&s_worldData.decalTris[j];
	}
	
	//ri.Printf( PRINT_WARNING, "r_decal 1 added %dms to loading.\n",time );									
}
	
