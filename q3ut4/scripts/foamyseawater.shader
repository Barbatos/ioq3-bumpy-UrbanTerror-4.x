

textures/bbq/FoamySeaWater
{
	qer_editorimage textures/bbq/FoamySeaWater.tga
	qer_trans 0.75
	q3map_fancywater 10 0.85 0.8 0.75
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm water
	surfaceparm nolightmap
	surfaceparm nomarks
	surfaceparm fog
	fogparms ( .6 .48 .4 ) 512
	q3map_cloneshader textures/bbq/FoamySeaWater2

	{
		map textures/bbq/FoamySeaWater.tga
		blendfunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		tcMod turb 0.1 0.1 0.1 .1
	}
	{
		map textures/bbq/FoamySeaWater.tga
		blendfunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		tcMod turb 0 0.1 1 .1
	}
	{
		map $lightmap
		blendfunc gl_dst_color gl_src_color
	}
}

textures/bbq/FoamySeaWater2
{
	qer_editorimage textures/bbq/FoamySeaWater.tga
	q3map_nofancywater
	qer_trans 0.75
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm water
	surfaceparm nolightmap
	surfaceparm nomarks

	{
		map textures/bbq/FoamySeaWater.tga
		blendfunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		tcMod turb 0.1 0.1 0.1 .1
	}
}


