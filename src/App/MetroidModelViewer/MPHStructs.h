//=========== Copyright (c) 2015, mike260 & PG, All rights reserved. ============//
//
// Purpose:		model/texture structs and typedefs
//
// $NoKeywords: $
//===============================================================================//

#ifndef MPHSTRUCTS_H
#define MPHSTRUCTS_H

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef int8_t		s8;
typedef int16_t		s16;
typedef int32_t		s32;

typedef struct
{
	char			name[64];
	u32				dunno1;		// some kind of render flag? (appears as 1f0200 or 1f0000 only)
	u16				palid;
	u16				texid;
	u32				dunno2[15];
}
MetroidMaterial;

typedef struct
{
	u32				start_ofs;
	u32				size;
	s32				bounds[3][2];
}
Dlist;

typedef struct
{
	char			name[64];
	u32				dunno[104];
}
Node;

typedef struct
{
	u16				matid;
	u16				dlistid;
}
Mesh;

typedef struct
{
	u16				format;
	u16				width;
	u16				height;
	u16				pad;
	u32				image_ofs;
	u32				imagesize;
	u32				dunno1;
	u32				dunno2;
	u32				dunno3;
	u32				dunno4;
	u32				dunno5;
	u32				dunno6;
}
Texture;

typedef struct
{
	u32				entries_ofs;
	u32				count;
	u32				dunno1;
	u32				dunno2;
}
Palette;

typedef struct
{
	u32				dunno1;
	u32				dunno2;
	u32				dunno3;
	u32				dunno4;
	MetroidMaterial *materials_p;
	Dlist *			dlists_p;
	Node *			nodes_p;
	u32				dunno5;
	u32				dunno6;
	Mesh *			meshes_p;
	u32				dunnoMeshes;		// maybe meshcount NOPE look below
	Texture *		textures_p;
	u32				num_textures;
	Palette *		palettes_p;
	u32				dunno7[4];
	u16				num_materials;
	u16				dunno8;
	u32				dunno9[5];
	u16				meshCount;			// meshcount is actually here
}
Header;

#endif
