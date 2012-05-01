--------------------
4.2 Alpha Client For Public Testing
--------------------

This concerns iobumpy merged with upcoming client.

Features
--------

1.) Time in console
2.) Persistant console history
3.) New commands
4.) New config location - more vista friendly too.
5.) Bumpy stuff, see below
6.) VoIP support (requires VoIP server) - no admin yet. (play with cl_voip, +voiprecord stuff)

Known Issues
------------

1.) Firing a gun has sounds missing every 10 bullets.
For some reason the default dsound backend of SDL doesn't work properly.
Use OpenAL backend using:
/set s_useopenal 1
/snd_restart

2.) Playing CTF, flag sounds are too quiet. Glass shattering is way too loud.
This is a bug in the game code that only appears in the new client (due to the use of openal).
It is fixed in 4.2.

----------------------
IOBUMPY FEATURES
----------------------
-Update 12:

Added r_SSAO, off by default.  I consider this a bit of a personal failure but its subtle and there, if someone else wants to er.. tune?.. around with it be my guest.
Retuned r_bloom to be more scifi looking on r_bloom 1.
Added r_simpleshaders to turn off all the shader processing and just let you get the performance of using r_vbo etc.

-Known Bug: skybox/object outline using AA and heavy global fog.


-Update 11: Optimized the post processing shaders a bit by doing a pre-pass to build the linear z into scene alpha.  Removed a few redundant texture reads and alot of redundant math.  About 30% faster dof this way, and it'll be better for SSAO when I get aroudn to it next.
Requested framebuffer to have an 8 bit alpha channel + updated code.

-update 10: cubemap_probe entitys now supported, for manual probe placement.


-update 9:  Seperated r_dof out into its own set of shaders.. so turn it off if it chugs you.


-update 8: Changed depth of field to be a proper circle of confusion
r_dofscale controls how blurry things get (r_dofscale 0 to turn off)
r_dofblend controls how quickly (percent wise)  to transition to a new autofocus dof.. slower tends to be nicer, but its going to be user subjective.

-Update 7: added fog, added depth of field

r_fogdensity  0.00002 is nice
r_fog_r/r_fog_g/r_fog_b controls color
you can set this per map via the globalfog worldspawn key eg
globalfog 0.2 0.2 0.2  0.0001

r_dof 0/1 controls depth of field

you need either dof or bloom on to see fog.

-Update 6:  Fixed the too bright specular in window mode
            Fixed parallax mapping tangent stuff
            Fixed/Rederived radiosity bumpmapping so that it doesn't produce darker results then regular lighting

-Update 5: Added Cheap Water, Added Offsetmapping


Cheap Water - add q3map_cheapwater to your water surface shader

q3map_cheapwater scale r g b - this is not really designed for swimming in - use it for small puddles
however you can use as much of this as you like.

Offset mapping works the same as _N texture usage for normal maps
but you supply a _B texture.. like stonewall_b and stonewall_n

It requires both a _b and and _n texture to work.



-Update 4: Added FancyWater

useage:  add q3map_fancywater to your water surface shader
	 
q3map_fancywater scale r g b  -- makes the surface a reflective/refractive water surf
q3map_nofancywater -- turns a surface off if r_fancywater is turned on. 
see: scripts/foamyseawater.shader for an example

Currently the paramters for q3map_fancywater dont do anything and you can't specify the normalmap to use
You do NOT need to recompile your map in q3map2 to make this work/test it.


-Update 3: added -radbumpdebug, fixed a heap of issues with the compiler.


-Update 2:  Huge changes - Added support for "radiosity bumpmapping".
See the valve/hl2 shader paper for whats going on.

1) Compile your map with the included q3map2_fs_radbump.exe using the -radbump flag in your lighting stage
2) Wait 4x longer then normal =)
3) include the mapname.rad with your mapname.bsp, much like an .aas file

Maps compiled for radbump will look perfectly normal in standard q3. 
But if you have _N texture files, the .rad file and the new engine, you'll get high quality per pixel
radiosity bumpmapping.



-Update: Added fresnel, removed "pow" specular. This looks alot better.

Bumpmap and Specular Goodness in q3 without a recompile!

* This includes all of the r_vbo, bloom, lightmap merging and decal code from the previous engine modifications.
* run with r_vbo 2 or 3 for now.  r_vbo 1 might have trouble getting suitable specular for indoor areas.
* All surfaces that can be optimized by r_vbo (diffuse*lightmap, no fancy alpha, blending or tcmods)
   will try and load an automatic normal map stage. Anything on the fallback path will render normally.


It will try and search for filename_n.tga for existing textures
eg:  textures\sweetnutz\sn_dirt1.jpg 
     will get bumpy if you add
     textures\sweetnutz\sn_dirt1_n.tga

It expects normals in the standard rgb/xyz format (the light blue ones)
The alpha channel of the normalmap is used to control specularity


About the Technique:

The "bump" shading is done by simply treating all surfaces as though they have a light pointed directly at them.
It will darken down as far as 10% of the current lightmap lighting. 

Specular is done by cubemap "probes" automatically placed around the level and assigned to each surface. (hl2 specular style)

Later, I'll probably add the ability for mappers to add an entity to the map to specify where to sample specularity from..
but atm it seems to work fairly well for the small set of levels I've tested for.

TwentySeven
Peter McNeill
n27@tpg.com.au





