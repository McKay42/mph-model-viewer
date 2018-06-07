//================ Copyright (c) 2015, PG & mike260, All rights reserved. =================//
//
// Purpose:		metroid prime hunters model viewer
//
// $NoKeywords: $mph
//=========================================================================================//

// To download mike260's model viewer, go to http://filetrip.net/nds-downloads/utilities/download-dsgraph-1-0-f29517.html
// I don't take credit for the reverse-engineering of the texture formats and model format, that was all mike's work.
// You can do everything you want with this code

#include "MetroidModelViewer.h"
#include "ResourceManager.h"
#include "Engine.h"
#include "ConVar.h"
#include "Gamepad.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "Camera.h"
#include "Console.h"

#include "CBaseUIContainer.h"
#include "CWindowManager.h"

#include "MPHTextureViewer.h"
#include "MPHSettings.h"

#include "MPHPlayer.h"

#include "OpenGLHeaders.h"



ConVar map("map");
ConVar mph_model_scale("mph_model_scale", 1.0f);



void MetroidModelViewer::buildTextures(Header *hdr_p)
{
	int numMaterials = (int)hdr_p->num_materials;
	debugLog("Going to build %i materials...\n", (int)hdr_p->num_materials);

	for (u32 m=0; m<numMaterials; m++)
	{
		MetroidMaterial *mat_p = &hdr_p->materials_p[m];

		// check for no texture or no palette (invalid)
		if (mat_p->texid == 0xFFFF || mat_p->palid == 0xFFFF)
			continue;

		// check if we have already created this texture
		if (existsTexture(mat_p->texid))
			continue;

		Texture *tex_p = &hdr_p->textures_p[mat_p->texid];
		Palette *pal_p = &hdr_p->palettes_p[mat_p->palid];

		u8 *texels_p = NULL;
		u16 *paxels_p = NULL;

		// depending on whether we have a separate texture file or not
		if (m_rawTextureFileData != NULL)
		{
			texels_p = (u8 *)((u32)m_rawTextureFileData + tex_p->image_ofs);
			paxels_p = (u16 *)((u32)m_rawTextureFileData + pal_p->entries_ofs);
		}
		else
		{
			texels_p = (u8 *)((u32)hdr_p + tex_p->image_ofs);
			paxels_p = (u16 *)(((u32)hdr_p) + pal_p->entries_ofs);
		}

		u32 width = (u32)tex_p->width;
		u32 height = (u32)tex_p->height;
		u32 num_pixels = width * height;

		if (m_bDebugTextureBuilding)
			debugLog("Building texture %i [%i, %i], palette id = %i, name = %s\n", mat_p->texid, tex_p->width, tex_p->height, mat_p->palid, mat_p->name);

		// create texture object
		MPHTEXTURE tex;
		tex.id = mat_p->texid;
		tex.alpha = tex_p->format == 4;
		tex.texture = engine->getResourceManager()->createImage(width, height, false);
		tex.texture->setWrapMode(Graphics::WRAP_MODE::WRAP_MODE_REPEAT);

		if (tex.texture == NULL)
			continue;

		// remove stupid illegal characters from the filename for exporting
		for (int i=0; i<64; i++)
		{
			if (mat_p->name[i] == '\0')
				break;
			else if (mat_p->name[i] == ':')
				mat_p->name[i] = '_';
		}
		tex.texture->setName(mat_p->name);

		if (m_bDebugTextureBuilding)
			debugLog("Texture %i: tex_p = %x, image offset = %x, entries_ofs = %x, format = %x, palette count = %i, palette_p = %x\n", mat_p->texid, (int)tex_p-(u32)hdr_p, (u16)tex_p->image_ofs, pal_p->entries_ofs, tex_p->format, pal_p->count, (int)pal_p-(u32)hdr_p);

		// fill the texture with the invidivual pixels, depending on the format

		if (tex_p->format == 0)				// 2bit palettised
		{
			for (u32 p=0; p<num_pixels; p++)
			{
				u32 index = texels_p[p/4];
				index = (index >> ((p%4)*2)) & 0x3;
				u16 col = paxels_p[index];

				u32 r = ((col>> 0) & 0x1F) << 3;
				u32 g = ((col>> 5) & 0x1F) << 3;
				u32 b = ((col>>10) & 0x1F) << 3;
				u32 a = ((col>>15) & 0x1F) << 3;
				/*u32 a = (col&0x8000) ? 0x00 : 0xFF;*/

				///image_p[p] = (r<<0) | (g<<8) | (b<<16) | (a<<24);
				tex.texture->setPixel(p % width, p / width, COLOR(a, r, g, b));
			}
		}
		else if (tex_p->format == 1)		// 4bit palettised
		{
			for (u32 p=0; p<num_pixels; p++)
			{
				u32 index = texels_p[p/2];

				index = (index >> ((p%2)*4)) & 0xF;

				u16 col = paxels_p[index];

				u32 r = ((col>> 0) & 0x1F) << 3;
				u32 g = ((col>> 5) & 0x1F) << 3;
				u32 b = ((col>>10) & 0x1F) << 3;
				u32 a = ((col>>15) & 0x1F) << 3;
				/*u32 a = (col&0x8000) ? 0x00 : 0xFF;*/

				///image_p[p] = ((r<<0)) | ((g<<8)) | ((b<<16)) | ((a<<24));
				tex.texture->setPixel(p % width, p / width, COLOR(a, r, g, b));
			}
		}
		else if (tex_p->format == 2)		// 8bit palettised
		{
			for (u32 p=0; p<num_pixels; p++)
			{
				u32 index = texels_p[p];
				u16 col = paxels_p[index];

				u32 r = ((col>> 0) & 0x1F) << 3;
				u32 g = ((col>> 5) & 0x1F) << 3;
				u32 b = ((col>>10) & 0x1F) << 3;
				u32 a = ((col>>15) & 0x1F) << 3;
				/*u32 a = (col&0x8000) ? 0x00 : 0xFF;*/

				///image_p[p] = (r<<0) | (g<<8) | (b<<16) | (a<<24);
				tex.texture->setPixel(p % width, p / width, COLOR(a, r, g, b));
			}
		}
		else if (tex_p->format == 4)		// 8bit greyscale
		{
			for (u32 p=0; p<num_pixels; p++)
			{
				u32 col = texels_p[p];

				///image_p[p] = (col<<0) | (col<<8) | (col<<16) | (mat_p->dunno2[5] > 0 || mat_p->dunno2[4] == 1 ? (col<<24) : 0xFF000000);
				tex.texture->setPixel(p % width, p / width, COLOR(col, col, col, col));
			}
		}
		else if (tex_p->format == 5)		// 16bit RGB
		{
			for (u32 p=0; p<num_pixels; p++)
			{
				u16 col = (u16)texels_p[p*2+0] | (((u16)texels_p[p*2+1])<<8);

				u32 r = ((col>> 0) & 0x1F) << 3;
				u32 g = ((col>> 5) & 0x1F) << 3;
				u32 b = ((col>>10) & 0x1F) << 3;
				/*u32 a = (col&0x8000) ? 0x00 : 0xFF;*/
				u32 a = ((col>>15) & 0x1F) << 3;

				///image_p[p] = (r<<0) | (g<<8) | (b<<16) | (a<<24);
				tex.texture->setPixel(p % width, p / width, COLOR(a, r, g, b));
			}
		}
		else
		{
			debugLog("Unknown texture format %i!\n", tex_p->format);

			for (u32 p=0; p<num_pixels; p++)
			{
				tex.texture->setPixel(p % width, p / width, COLOR(255, 255, 0, 255));
			}
		}

		// and save it
		m_textures.push_back(tex);
	}

	// now push the textures to the gpu and also add them to the textureViewer
	for (int i=0; i<m_textures.size(); i++)
	{
		m_textures[i].texture->load();
		m_textures[i].texture->setWrapMode(Graphics::WRAP_MODE::WRAP_MODE_REPEAT); // HACKHACK: unloaded textures don't store properties yet, so set it again here

		m_textureViewer->addTexture(m_textures[i].texture, m_textures[i].texture->getName());
	}

	debugLog("Built all textures.\n");
}

void MetroidModelViewer::doRenderReg(u32 reg, u32 **data_pp, float vtx_state[3])
{
	u32 *data_p = *data_pp;

	switch (reg)
	{
		// NOP
		case 0x400:
			{
			}
			break;

		// MTX_RESTORE
		case 0x450:
			{
				data_p++;
			}
			break;

		// DIF_AMB
		case 0x4C0:
			{
				data_p++;
			}
			break;

		// COLOR
		case 0x480:
		{
			u32 rgb = *(data_p++);
			u32 r = (rgb>> 0) & 0x1F;
			u32 g = (rgb>> 5) & 0x1F;
			u32 b = (rgb>>10) & 0x1F;

			if (m_bLightmapped)
				glColor3f(((float)r)/31.0f, ((float)g)/31.0f, ((float)b)/31.0f);
			else
				glColor3f(1.0f, 1.0f, 1.0f);
		}
		break;

		// NORMAL
		case 0x484:
		{
			u32 xyz = *(data_p++);

			s32 x = (xyz>> 0) & 0x3FF;
			if( x & 0x200 )
				x |= 0xFFFFFC00;

			s32 y = (xyz>>10) & 0x3FF;
			if( y & 0x200 )
				y |= 0xFFFFFC00;

			s32 z = (xyz>>20) & 0x3FF;
			if( z & 0x200 )
				z |= 0xFFFFFC00;

			glNormal3f(((float)x)/512.0f, ((float)y)/512.0f, ((float)z)/512.0f);
		}
		break;

		// TEXCOORD
		case 0x488:
		{
			u32 st = *(data_p++);

			s32 s = (st>> 0) & 0xFFFF;			if( s & 0x8000 )		s |= 0xFFFF0000;
			s32 t = (st>>16) & 0xFFFF;			if( t & 0x8000 )		t |= 0xFFFF0000;

			glTexCoord2f(((float)s)/16.0f, ((float)t)/16.0f);
		}
		break;

		// VTX_16
		case 0x48C:
		{
			u32 xy = *(data_p++);

			s32 x = (xy>> 0) & 0xFFFF;
			if( x & 0x8000 )
				x |= 0xFFFF0000;

			s32 y = (xy>>16) & 0xFFFF;
			if( y & 0x8000 )
				y |= 0xFFFF0000;

			s32 z = (*(data_p++)) & 0xFFFF;
			if( z & 0x8000 )
				z |= 0xFFFF0000;

			vtx_state[0] = (((float)x) / 4096.0f) * mph_model_scale.getFloat();
			vtx_state[1] = (((float)y) / 4096.0f) * mph_model_scale.getFloat();
			vtx_state[2] = (((float)z) / 4096.0f) * mph_model_scale.getFloat();

			glVertex3fv(vtx_state);
		}
		break;

		// VTX_10
		case 0x490:
		{
			u32 xyz = *(data_p++);

			s32 x = (xyz>> 0) & 0x3FF;
			if( x & 0x200 )
				x |= 0xFFFFFC00;

			s32 y = (xyz>>10) & 0x3FF;
			if( y & 0x200 )
				y |= 0xFFFFFC00;

			s32 z = (xyz>>20) & 0x3FF;
			if( z & 0x200 )
				z |= 0xFFFFFC00;

			vtx_state[0] = (((float)x) / 64.0f) * mph_model_scale.getFloat();
			vtx_state[1] = (((float)y) / 64.0f) * mph_model_scale.getFloat();
			vtx_state[2] = (((float)z) / 64.0f) * mph_model_scale.getFloat();

			glVertex3fv(vtx_state);
		}
		break;

		// VTX_XY
		case 0x494:
		{
			u32 xy = *(data_p++);

			s32 x = (xy>> 0) & 0xFFFF;
			if( x & 0x8000 )
				x |= 0xFFFF0000;

			s32 y = (xy>>16) & 0xFFFF;
			if( y & 0x8000 )
				y |= 0xFFFF0000;

			vtx_state[0] = (((float)x) / 4096.0f) * mph_model_scale.getFloat();
			vtx_state[1] = (((float)y) / 4096.0f) * mph_model_scale.getFloat();

			glVertex3fv(vtx_state);
		}
		break;

		// VTX_XZ
		case 0x498:
		{
			u32 xz = *(data_p++);

			s32 x = (xz>> 0) & 0xFFFF;
			if( x & 0x8000 )
				x |= 0xFFFF0000;

			s32 z = (xz>>16) & 0xFFFF;
			if( z & 0x8000 )
				z |= 0xFFFF0000;

			vtx_state[0] = (((float)x) / 4096.0f) * mph_model_scale.getFloat();
			vtx_state[2] = (((float)z) / 4096.0f) * mph_model_scale.getFloat();

			glVertex3fv(vtx_state);
		}
		break;

		// VTX_YZ
		case 0x49C:
		{
			u32 yz = *(data_p++);

			s32 y = (yz>> 0) & 0xFFFF;
			if( y & 0x8000 )
				y |= 0xFFFF0000;

			s32 z = (yz>>16) & 0xFFFF;
			if( z & 0x8000 )
				z |= 0xFFFF0000;

			vtx_state[1] = (((float)y) / 4096.0f) * mph_model_scale.getFloat();
			vtx_state[2] = (((float)z) / 4096.0f) * mph_model_scale.getFloat();

			glVertex3fv(vtx_state);
		}
		break;

		// VTX_DIFF
		case 0x4A0:
		{
			u32 xyz = *(data_p++);

			s32 x = (xyz>> 0) & 0x3FF;
			if( x & 0x200 )
				x |= 0xFFFFFC00;

			s32 y = (xyz>>10) & 0x3FF;
			if( y & 0x200 )
				y |= 0xFFFFFC00;

			s32 z = (xyz>>20) & 0x3FF;
			if( z & 0x200 )
				z |= 0xFFFFFC00;

			vtx_state[0] += (((float)x) / 4096.0f) * mph_model_scale.getFloat();
			vtx_state[1] += (((float)y) / 4096.0f) * mph_model_scale.getFloat();
			vtx_state[2] += (((float)z) / 4096.0f) * mph_model_scale.getFloat();

			glVertex3fv(vtx_state);
		}
		break;

		// BEGIN_VTXS
		case 0x500:
			{
				u32 type = *(data_p++);
				switch (type)
				{
					case 0:
						glBegin(GL_TRIANGLES);
						break;
					case 1:
						glBegin(GL_QUADS);
						break;
					case 2:
						glBegin(GL_TRIANGLE_STRIP);
						break;
					case 3:
						glBegin(GL_QUAD_STRIP);
						break;
					default:
						debugLog("Unknown geometry type %i!\n", type);
					break;
				}
			}
			break;

		// END_VTXS
		case 0x504:
			{
				glEnd();
			}
			break;

		// illegal opcode
		default:
			{
				debugLog("Unknown reg write %i!\n", reg);
			}
			break;
	}

	*data_pp = data_p;
}

void MetroidModelViewer::doRenderDlist(u32 *data_p, u32 len)
{
	u32 *end_p = data_p + len/4;

	float vtx_state[3] = {0.0f, 0.0f, 0.0f};

	while (data_p < end_p)
	{
		u32 regs = *(data_p++);

		for (u32 c=0; c<4; c++, regs>>=8)
		{
			u32 reg = ((regs & 0xFF)<<2) + 0x400;

			doRenderReg(reg, &data_p, vtx_state);
		}
	}
}

void MetroidModelViewer::renderModel(Header *hdr_p)
{
	// sort meshes by translucency (alpha blended textures get rendered last)
	std::vector<Mesh*> alphaSortedMeshes;
	int meshCounter = 0;
	for (Mesh *mesh_p = hdr_p->meshes_p; meshCounter++ < m_modelHeader->meshCount; mesh_p++)
	{
		//Dlist *dlist_p = &hdr_p->dlists_p[mesh_p->dlistid];
		MetroidMaterial *material_p = &hdr_p->materials_p[mesh_p->matid];
		MPHTEXTURE *tex = getTexture(material_p->texid);

		// Experimental Alpha: this check is not always correct
		if ((material_p->dunno2[5] > 0 || (tex != NULL && tex->alpha)) && m_bAlpha)
			alphaSortedMeshes.push_back(mesh_p);
		else
			alphaSortedMeshes.insert(alphaSortedMeshes.begin(), mesh_p);
	}

	// draw sorted meshes
	for (int i=0; i<alphaSortedMeshes.size(); i++)
	{
		Mesh *mesh_p = alphaSortedMeshes[i];

		Dlist *dlist_p = &hdr_p->dlists_p[mesh_p->dlistid];
		u32 *data_p = (u32 *)((u32)hdr_p + dlist_p->start_ofs);

		// get material and texture
		MetroidMaterial *material_p = &hdr_p->materials_p[mesh_p->matid];
		MPHTEXTURE *tex = getTexture(material_p->texid);

		// set texture and other parameters
		if (m_bTextured && m_iWireframe == 0 && tex != NULL)
		{
			// enable blending
			// Experimental Alpha: again, this is not always correct
			if ((material_p->dunno2[5] > 0 || tex->alpha))
			{
				if (m_bAlpha)
					engine->getGraphics()->setBlending(true);

				engine->getGraphics()->setCulling(false);
			}

			// bind the texture
			tex->texture->bind();

			if (!m_bTextureFiltering)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}

			// normalize uv coordinates and animated textures
			if (tex->texture != NULL && tex->texture->getWidth() > 0 && tex->texture->getHeight() > 0)
			{
				glMatrixMode(GL_TEXTURE);
				glLoadIdentity();
				glScalef(1.0f / tex->texture->getWidth(), 1.0f / tex->texture->getHeight(), 1.0f);

				// animated uv scrolling textures
				// Experimental Anim: this is also not always correct
				if (m_bAnim && material_p->dunno2[4] > 1)
					glTranslatef( -engine->getTime()*5, 0, 1.0f);
			}
		}
		else
			glBindTexture(GL_TEXTURE_2D, 0);

		if (m_iWireframe > 0 && m_iWireframe != 2)
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		// render the entire mesh
		doRenderDlist(data_p, dlist_p->size);

		// re-enable everything for the next mesh
		engine->getGraphics()->setBlending(false);
		if (m_bCulling)
			engine->getGraphics()->setCulling(true);
	}

	// restore texture matrix
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	// restore matrixmode
	glMatrixMode(GL_MODELVIEW);
}

MetroidModelViewer::MetroidModelViewer()
{
	m_modelHeader = NULL;

	// convar callbacks
	convar->getConVarByName("map")->setCallback( fastdelegate::MakeDelegate(this, &MetroidModelViewer::loadMap) );
	convar->getConVarByName("ui_window_animspeed")->setValue(0.01f);

	// file
	m_iFileSize = 0;
	m_rawModelFileData = NULL;
	m_rawTextureFileData = NULL;
	m_modelHeader = NULL;
	m_bBuildTextures = true;
	m_bDebugTextureBuilding = false;
	m_bTextureDebugOverlay = false;

	// rendering
	m_iWireframe = 0;
	m_bGLLequal = false;
	m_bTextureFiltering = true;
	m_bTextured = true;
	m_bLightmapped = true;
	m_bAlpha = true;
	m_bCulling = true;
	m_bAnim = true;

	// exporting
	m_bCurMeshActive = false;
	m_iCurMeshType = -1;

	// camera/player
	m_player = new MPHPlayer();
	m_bCaptureMouse = false;

	// windows
	m_windowManager = new CWindowManager();

	m_textureViewer = new MPHTextureViewer();
	m_textureViewer->setPos(engine->getScreenWidth() - m_textureViewer->getSize().x - 12, 12);
	m_textureViewer->setSizeY(engine->getScreenHeight() - m_textureViewer->getPos().y - 12);
	m_windowManager->addWindow(m_textureViewer);

	m_settings = new MPHSettings(this);
	m_settings->setPos(12, engine->getScreenHeight() - m_settings->getSize().y - 12);
	m_windowManager->addWindow(m_settings);

	m_textureViewer->open();
	m_settings->open();

	Console::execConfigFile("autoexec.cfg");
}

MetroidModelViewer::~MetroidModelViewer()
{
	SAFE_DELETE(m_windowManager);
}

void MetroidModelViewer::draw(Graphics *g)
{
	if (m_modelHeader != NULL)
	{
		// black background
		g->setColor(0xff000000);
		g->fillRect(0, 0, engine->getScreenWidth(), engine->getScreenHeight());

		g->pushTransform();
		{
			glPushAttrib(GL_ENABLE_BIT);

			g->setDepthBuffer(true);
			g->setAntialiasing(true);
			g->setCulling(m_bCulling);
			g->setBlending(false);

			glEnable(GL_TEXTURE_2D);

			GLint blendSrc, blendDest;
			glGetIntegerv(GL_BLEND_SRC, &blendSrc);
			glGetIntegerv(GL_BLEND_DST, &blendDest);
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			if (m_bGLLequal)
				glDepthFunc(GL_LEQUAL);
			else
				glDepthFunc(GL_LESS);

			// debug textures
			if (m_bTextureDebugOverlay)
			{
				int debugImageSize = 128;
				int maxX = 0;
				int maxY = 0;

				g->setColor(0xffffffff);
				for (int i=0; i<m_textures.size(); i++)
				{
					if (maxY + debugImageSize >= engine->getScreenHeight())
					{
						maxY = 0;
						maxX += debugImageSize;
					}

					m_textures[i].texture->bind();
					{
						g->drawQuad(maxX, maxY, debugImageSize, debugImageSize);
					}
					glBindTexture(GL_TEXTURE_2D, 0);

					maxY += debugImageSize;
				}
			}

			// set projection matrix
			Matrix4 perspectiveMatrix = Camera::buildMatrixPerspectiveFov(m_player->getCamera()->getFovRad(), ((float)engine->getScreenWidth()) / ((float)engine->getScreenHeight()), 0.1f, 1000.0f);
			g->setProjectionMatrix(perspectiveMatrix);

			// set world matrix
			Matrix4 lookAt = m_player->getCamera()->buildMatrixLookAt(m_player->getCamera()->getPos(),
																	  m_player->getCamera()->getPos() + m_player->getCamera()->getViewDirection(),
																	  m_player->getCamera()->getViewUp());

			g->setWorldMatrix(lookAt);

			g->drawPixel(-1,-1); // HACKHACK: force update matrix stack

			// draw coordinate system at (0,0,0)
			// x axis
			glDisable(GL_TEXTURE_2D);
			{
				g->setColor(0xffff0000);
				glBegin(GL_LINES);
					glVertex3f(0, 0, 0);
					glVertex3f(1, 0, 0);
				glEnd();

				// y axis
				g->setColor(0xff00ff00);
				glBegin(GL_LINES);
					glVertex3f(0, 0, 0);
					glVertex3f(0, 1, 0);
				glEnd();

				// z axis
				g->setColor(0xff0000ff);
				glBegin(GL_LINES);
					glVertex3f(0, 0, 0);
					glVertex3f(0, 0, 1);
				glEnd();
			}
			glEnable(GL_TEXTURE_2D);

			// and reset the color to white before starting to render
			g->setColor(0xffffffff);

			// do standard render pass before the usual render for wireframe overlay
			if (m_iWireframe > 1)
			{
				m_iWireframe = 0;
				{
					renderModel(m_modelHeader);
				}
				m_iWireframe = 2;
			}

			// modify line widths for wireframe overlay render
			GLfloat lineWidthBackup = 1.0f;
			if (m_iWireframe > 0)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				if (m_iWireframe > 1)
				{
					glGetFloatv(GL_LINE_WIDTH, &lineWidthBackup);
					glLineWidth(2.0f);
				}
			}

			// draw it
			renderModel(m_modelHeader);

			// and reset everything
			if (m_iWireframe > 0)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				if (m_iWireframe > 1)
					glLineWidth(lineWidthBackup);
			}

			g->setAntialiasing(false);
			g->setDepthBuffer(false);

			glPopAttrib();
			glBlendFunc(blendSrc, blendDest);
		}
		g->popTransform();
	}

	// draw windows on top of everything
	m_windowManager->draw(g);
}

void MetroidModelViewer::update()
{
	m_player->update(m_bCaptureMouse);
	m_windowManager->update();
}

void MetroidModelViewer::onKeyDown(KeyboardEvent &key)
{
	m_player->onKeyDown(key);
	m_windowManager->onKeyDown(key);

	if (key == KEY_Q)
		toggleWireframe();
	else if (key == KEY_F)
	{
		m_bTextureFiltering = !m_bTextureFiltering;
		m_settings->updateFilter(m_bTextureFiltering);
	}
	else if (key == KEY_E)
	{
		m_bTextured = !m_bTextured;
		m_settings->updateTextured(m_bTextured);
	}
	else if (key == KEY_C)
	{
		setCaptureMouse(!m_bCaptureMouse);
	}
	else if (key == KEY_ESCAPE)
	{
		if (!m_bCaptureMouse)
		{
			if (m_windowManager->isVisible())
				m_windowManager->closeAll();
			else
				m_windowManager->openAll();
		}

		setCaptureMouse(false);
	}
	else if (key == KEY_TAB)
	{
		if (m_windowManager->isVisible())
			m_windowManager->closeAll();
		else
			m_windowManager->openAll();
	}
	else if (key == KEY_T)
	{
		m_textureViewer->open();
	}
	else if (key == KEY_B)
	{
		m_bAlpha = !m_bAlpha;
		m_settings->updateAlpha(m_bAlpha);
	}
	else if (key == KEY_L)
	{
		m_bLightmapped = !m_bLightmapped;
		m_settings->updateLightmapped(m_bLightmapped);
	}
	else if (key == KEY_O)
	{
		m_windowManager->openAll();
	}
}

void MetroidModelViewer::onKeyUp(KeyboardEvent &key)
{
	m_player->onKeyUp(key);
	m_windowManager->onKeyUp(key);
}

void MetroidModelViewer::onChar(KeyboardEvent &e)
{
	m_windowManager->onChar(e);
}

void MetroidModelViewer::onResolutionChanged(Vector2 newResolution)
{
	m_windowManager->onResolutionChange(newResolution);

	m_textureViewer->setPos(engine->getScreenWidth() - m_textureViewer->getSize().x - 12, 12);
	if (m_textureViewer->getPos().y + m_textureViewer->getSize().y > engine->getScreenHeight())
		m_textureViewer->setSizeY(engine->getScreenHeight() - m_textureViewer->getPos().y - 5);

	m_settings->setPos(12, engine->getScreenHeight() - m_settings->getSize().y - 12);
}

void MetroidModelViewer::onFocusLost()
{
	setCaptureMouse(false);
}

void MetroidModelViewer::toggleNoclip()
{
	// !
}

void MetroidModelViewer::setCaptureMouse(bool captureMouse)
{
	m_bCaptureMouse = captureMouse;
	engine->getMouse()->setCursorVisible(!m_bCaptureMouse);

	m_settings->updateCaptureMouse(m_bCaptureMouse);
	m_windowManager->setEnabled(!m_bCaptureMouse);
}



//*******************************//
//  Main model loading function  //
//*******************************//

void MetroidModelViewer::loadMap(UString args)
{
	UString modelFilePath = args;
	modelFilePath.insert(0, "mph/models/");
	modelFilePath.append(".bin");

	loadMapRaw(modelFilePath);
}

void MetroidModelViewer::loadMapRaw(UString filepath)
{
	// free everything
	m_textureViewer->clearTextures();
	if (m_rawModelFileData != NULL)
	{
		delete[] m_rawModelFileData;
		m_rawModelFileData = NULL;
	}
	if (m_rawTextureFileData != NULL)
	{
		delete[] m_rawTextureFileData;
		m_rawTextureFileData = NULL;
	}
	m_modelHeader = NULL;
	for (int i=0; i<m_textures.size(); i++)
	{
		engine->getResourceManager()->destroyResource(m_textures[i].texture);
	}
	m_textures.clear();

	// try opening the model file
	const char *cpath = filepath.toUtf8();
	FILE *modelFile = fopen(cpath, "rb");
	if (!modelFile)
	{
		debugLog("File \"%s\" not found!\n", cpath);
		return;
	}

	// get filesize
	fseek(modelFile, 0, SEEK_END);
	m_iFileSize = ftell(modelFile);
	fseek(modelFile, 0, SEEK_SET);

	if (m_iFileSize < 0)
	{
		debugLog("File read error!\n");
		fclose(modelFile);
		return;
	}

	// check if a separate texture file exists for this model, use it if it does
	std::string spath = cpath;
	spath = spath.substr(spath.find_last_of("/\\") + 1);
	UString filename = spath.substr(0, spath.find_last_of(".")).c_str();
	m_sFileName = filename;
	std::vector<UString> argParts = filename.split("_");
	UString textureFilePath = "mph/textures/";
	for (int i=0; i<argParts.size(); i++)
	{
		if (argParts[i] != "Model" && argParts[i] != "model")
		{
			if (i > 0)
				textureFilePath.append("_");

			textureFilePath.append(argParts[i]);
		}
		else
			break;
	}
	textureFilePath.append("_tex.bin");

	const char *textureFilename = textureFilePath.toUtf8();
	FILE *textureFile = fopen(textureFilename, "rb");
	if (!textureFile)
		debugLog("Model does not seem to have a separate texture file, or couldn't find %s!\nThis might cause vomit textures.\n", textureFilename);
	else
	{
		debugLog("Loading %s ...\n", textureFilename);

		// get filesize for texture file
		fseek(textureFile, 0, SEEK_END);
		int textureFileSize = ftell(textureFile);
		fseek(textureFile, 0, SEEK_SET);

		if (textureFileSize < 0)
		{
			debugLog("Texture file read error! This might cause vomit textures.\n");
			fclose(textureFile);
		}
		else
		{
			debugLog("Texturefile: Filesize = %i\n", textureFileSize);

			m_rawTextureFileData = new u8[textureFileSize];
			size_t freadret = fread(m_rawTextureFileData, sizeof(u8), textureFileSize, textureFile);
			fclose(textureFile);

			if (freadret <= 0 || freadret != textureFileSize)
			{
				debugLog("Fatal texture file read error! This might cause vomit textures.\n");
				if (m_rawTextureFileData != NULL)
					delete[] m_rawTextureFileData;
			}
		}
	}

	// read entire file into m_rawFileData
	m_rawModelFileData = new u8[m_iFileSize];
	size_t freadret = fread(m_rawModelFileData, sizeof(u8), m_iFileSize, modelFile);
	fclose(modelFile);

	if (freadret <= 0 || freadret != m_iFileSize)
	{
		debugLog("Fatal file read error!\n");
		return;
	}

	// fill header structure
	m_modelHeader = (Header*)m_rawModelFileData;

	// debug info
	debugLog("Modelfile: Filesize = %i\n", m_iFileSize);
	debugLog("Modelfile: numMeshes = %i\n", m_modelHeader->meshCount);

	/*
	debugLog("Modelfile: Relative material address = %x\n", (int)m_modelHeader->materials_p);
	debugLog("Modelfile: Relative texture address = %x\n", (int)m_modelHeader->textures_p);
	debugLog("Modelfile: Relative palette address = %x\n", (int)m_modelHeader->palettes_p);
	debugLog("Modelfile: Relative dlist address = %x\n", (int)m_modelHeader->dlists_p);
	debugLog("Modelfile: Relative node address = %x\n", (int)m_modelHeader->nodes_p);
	debugLog("Modelfile: Relative mesh address = %x\n", (int)m_modelHeader->meshes_p);
	*/

	// correct addressing
	m_modelHeader->materials_p = (MetroidMaterial*)((u32)m_modelHeader + (u32)m_modelHeader->materials_p );
	m_modelHeader->dlists_p = (Dlist*)(			  	(u32)m_modelHeader + (u32)m_modelHeader->dlists_p );
	m_modelHeader->nodes_p = (Node*)(				(u32)m_modelHeader + (u32)m_modelHeader->nodes_p );
	m_modelHeader->meshes_p = (Mesh*)(			  	(u32)m_modelHeader + (u32)m_modelHeader->meshes_p );
	m_modelHeader->textures_p = (Texture*)(		 	(u32)m_modelHeader + (u32)m_modelHeader->textures_p );
	m_modelHeader->palettes_p = (Palette*)(		  	(u32)m_modelHeader + (u32)m_modelHeader->palettes_p );

	// debug info
	/*
	debugLog("dunno1 = %i\n", m_modelHeader->dunno1);
	debugLog("dunno2 = %i\n", m_modelHeader->dunno2);
	debugLog("dunno3 = %i\n", m_modelHeader->dunno3);
	debugLog("dunno4 = %i\n", m_modelHeader->dunno4);
	debugLog("dunno5 = %i\n", m_modelHeader->dunno5);
	debugLog("dunno6 = %i\n", m_modelHeader->dunno6);
	for (int d=0; d<4; d++)
	{
		debugLog("dunno7[%i] = %i, ", d, m_modelHeader->dunno7);
	}
	debugLog("\ndunno8 = %i\n", m_modelHeader->dunno8);
	for (int d=0; d<5; d++)
	{
		debugLog("dunno9[%i] = %i, ", d, m_modelHeader->dunno9[d]);
	}
	debugLog("\n");
	*/

	// build textures
	if (m_bBuildTextures)
		buildTextures(m_modelHeader);

	m_player->getCamera()->setPos(Vector3(0, 0, -5));
	m_player->getCamera()->setRotation(0, 0, 0);
}



//****************************//
//  Texture helper functions  //
//****************************//

bool MetroidModelViewer::existsTexture(int texid)
{
	for (int i=0; i<m_textures.size(); i++)
	{
		if (m_textures[i].id == texid)
			return true;
	}
	return false;
}

MetroidModelViewer::MPHTEXTURE *MetroidModelViewer::getTexture(int texid)
{
	for (int i=0; i<m_textures.size(); i++)
	{
		if (m_textures[i].id == texid)
			return &m_textures[i];
	}

	return NULL;
}



//*******************************//
//  PLY Export helper functions  //
//*******************************//

void MetroidModelViewer::exportPlyModel()
{
	if (m_sFileName.length() < 3 || m_modelHeader == NULL || m_rawModelFileData == NULL)
	{
		debugLog("No model loaded, can't export!\n");
		return;
	}

	UString exportFilePath = "exports/";
	exportFilePath.append(m_sFileName);
	exportFilePath.append(".dae");
	const char *cpath = exportFilePath.toUtf8();
	debugLog("Exporting to %s...\n", cpath);

	FILE *exportFile = fopen(cpath, "w");
	if (!exportFile)
	{
		debugLog("ERROR: Couldn't export!");
		return;
	}

	// collada header
	fprintf(exportFile, "<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	fprintf(exportFile, "\n<COLLADA xmlns=\"http://www.collada.org/2005/11/COLLADASchema\" version=\"1.4.1\">");

	// assets
	fprintf(exportFile, "\n\t<asset>");
		fprintf(exportFile, "\n\t\t<up_axis>Y_UP</up_axis>");
		fprintf(exportFile, "<unit name=\"meter\" meter=\"1\"/>");
	fprintf(exportFile, "\n\t</asset>");

	// images
	fprintf(exportFile, "\n\t<library_images>");
	for (int i=0; i<m_textures.size(); i++)
	{
		UString textureName = m_textures[i].texture->getName();
		UString imageTag = "\n\t\t<image id=\"";
		imageTag.append(textureName);
		imageTag.append("\" name=\"");
		imageTag.append(textureName);
		imageTag.append("\">");
		fprintf(exportFile, imageTag.toUtf8());
			UString initTag = "\n\t\t\t<init_from>";
			textureName.append(".png");
			initTag.append(textureName);
			initTag.append("</init_from>");
			fprintf(exportFile, initTag.toUtf8());
		fprintf(exportFile, "\n\t\t</image>");
	}
	fprintf(exportFile, "\n\t</library_images>");

	// materials
	fprintf(exportFile, "\n\t<library_materials>");
	for (int i=0; i<m_textures.size(); i++)
	{
		UString textureName = m_textures[i].texture->getName();
		UString materialTag = "\n\t\t<material id=\"";
		materialTag.append(textureName);
		materialTag.append("-material\" name=\"");
		materialTag.append(textureName);
		materialTag.append("_mat\">");
		fprintf(exportFile, materialTag.toUtf8());
			UString instanceEffectTag = "\n\t\t\t<instance_effect url=\"#";
			instanceEffectTag.append(textureName);
			instanceEffectTag.append("-effect\"/>");
			fprintf(exportFile, instanceEffectTag.toUtf8());
		fprintf(exportFile, "\n\t\t</material>");
	}
	fprintf(exportFile, "\n\t</library_materials>");

	// geometries
	fprintf(exportFile, "\n\t<library_geometries>");
	{
		// mesh vars
		m_iCurMeshType = 0;
		m_bCurMeshActive = false;
		std::vector<MPHVERTEX> mesh;
		std::vector<MPHVERTEX> tempMesh;

		// go through every mesh
		int meshCounter = 0;
		for (Mesh *mesh_p = m_modelHeader->meshes_p; meshCounter++ < m_modelHeader->meshCount; mesh_p++)
		{
			// reset
			mesh.clear();
			tempMesh.clear();

			Dlist *dlist_p = &m_modelHeader->dlists_p[mesh_p->dlistid];
			MetroidMaterial *material_p = &m_modelHeader->materials_p[mesh_p->matid];
			MPHTEXTURE *tex = getTexture(material_p->texid);

			u32 *data_p = (u32 *)((u32)m_modelHeader + dlist_p->start_ofs);

			doExportDlist(data_p, dlist_p->size, &tempMesh, &mesh);

			int numVertices = mesh.size();
			UString textureName = "null";
			if (tex != NULL)
				textureName = tex->texture->getName();

			{
				UString geometryID = UString::format("geometry%i", meshCounter);
				UString geometryTag = UString::format("\n\t\t<geometry id=\"geometry%i\" name=\"geom%i\">", meshCounter, meshCounter);
				fprintf(exportFile, geometryTag.toUtf8());
				{
					fprintf(exportFile, "\n\t\t\t<mesh>");
					{
						// positions
						UString positionsTag = "\n\t\t\t\t<source id=\"";
						positionsTag.append(geometryID);
						positionsTag.append("-positions\">");
						fprintf(exportFile, positionsTag.toUtf8());
						{
							// float array
							UString positionsArrayID = geometryID;
							positionsArrayID.append("-positions-array");

							UString floatarrayTag = "\n\t\t\t\t\t<float_array id=\"";
							floatarrayTag.append(positionsArrayID);
							floatarrayTag.append("\" count=\"");
							UString countString = UString::format("%i", numVertices*3);
							floatarrayTag.append(countString);
							floatarrayTag.append("\">");
							fprintf(exportFile, floatarrayTag.toUtf8());
							for (int i=0; i<mesh.size(); i++)
							{
								UString vertPos = UString::format("%f %f %f ", mesh[i].pos.x, mesh[i].pos.y, mesh[i].pos.z);
								fprintf(exportFile, vertPos.toUtf8());
							}
							fprintf(exportFile, "</float_array>");

							// technique common (position x,y,z)
							fprintf(exportFile, "\n\t\t\t\t\t<technique_common>");
							{
								UString accessorTag = "\n\t\t\t\t\t\t<accessor source=\"#";
								accessorTag.append(positionsArrayID);
								UString countString = UString::format("%i", numVertices);
								accessorTag.append("\" count=\"");
								accessorTag.append(countString);
								accessorTag.append("\" stride=\"3\">");
								fprintf(exportFile, accessorTag.toUtf8());
								{
									fprintf(exportFile, "\n\t\t\t\t\t\t\t<param name=\"X\" type=\"float\"/>");
									fprintf(exportFile, "\n\t\t\t\t\t\t\t<param name=\"Y\" type=\"float\"/>");
									fprintf(exportFile, "\n\t\t\t\t\t\t\t<param name=\"Z\" type=\"float\"/>");
								}
								fprintf(exportFile, "\n\t\t\t\t\t\t</accessor>");
							}
							fprintf(exportFile, "\n\t\t\t\t\t</technique_common>");
						}
						fprintf(exportFile, "\n\t\t\t\t</source>");

						// normals
						UString normalsTag = "\n\t\t\t\t<source id=\"";
						normalsTag.append(geometryID);
						normalsTag.append("-normals\">");
						fprintf(exportFile, normalsTag.toUtf8());
						{
							// float array
							UString normalsArrayID = geometryID;
							normalsArrayID.append("-normals-array");

							UString floatarrayTag = "\n\t\t\t\t\t<float_array id=\"";
							floatarrayTag.append(normalsArrayID);
							floatarrayTag.append("\" count=\"");
							UString countString = UString::format("%i", numVertices*3);
							floatarrayTag.append(countString);
							floatarrayTag.append("\">");
							fprintf(exportFile, floatarrayTag.toUtf8());
							for (int i=0; i<mesh.size(); i+=3)
							{
								Vector3 normal = (mesh[i+1].pos-mesh[i].pos).cross(mesh[i+2].pos-mesh[i].pos).normalize();
								UString vertNormal = UString::format("%f %f %f ", normal.x, normal.y, normal.z);
								const char *vertNormalC = vertNormal.toUtf8();
								fprintf(exportFile, vertNormalC);
								fprintf(exportFile, vertNormalC);
								fprintf(exportFile, vertNormalC);
							}
							fprintf(exportFile, "</float_array>");

							// technique common (normal x,y,z)
							fprintf(exportFile, "\n\t\t\t\t\t<technique_common>");
							{
								UString accessorTag = "\n\t\t\t\t\t\t<accessor source=\"#";
								accessorTag.append(normalsArrayID);
								UString countString = UString::format("%i", numVertices);
								accessorTag.append("\" count=\"");
								accessorTag.append(countString);
								accessorTag.append("\" stride=\"3\">");
								fprintf(exportFile, accessorTag.toUtf8());
								{
									fprintf(exportFile, "\n\t\t\t\t\t\t\t<param name=\"X\" type=\"float\"/>");
									fprintf(exportFile, "\n\t\t\t\t\t\t\t<param name=\"Y\" type=\"float\"/>");
									fprintf(exportFile, "\n\t\t\t\t\t\t\t<param name=\"Z\" type=\"float\"/>");
								}
								fprintf(exportFile, "\n\t\t\t\t\t\t</accessor>");
							}
							fprintf(exportFile, "\n\t\t\t\t\t</technique_common>");
						}
						fprintf(exportFile, "\n\t\t\t\t</source>");

						// vertex colors
						UString vertexcolorsTag = "\n\t\t\t\t<source id=\"";
						vertexcolorsTag.append(geometryID);
						vertexcolorsTag.append("-colors\">");
						fprintf(exportFile, vertexcolorsTag.toUtf8());
						{
							// float array
							UString vertexcolorsArrayID = geometryID;
							vertexcolorsArrayID.append("-colors-array");

							UString floatarrayTag = "\n\t\t\t\t\t<float_array id=\"";
							floatarrayTag.append(vertexcolorsArrayID);
							floatarrayTag.append("\" count=\"");
							UString countString = UString::format("%i", numVertices*3);
							floatarrayTag.append(countString);
							floatarrayTag.append("\">");
							fprintf(exportFile, floatarrayTag.toUtf8());
							for (int i=0; i<mesh.size(); i++)
							{
								UString vertColor = UString::format("%f %f %f ", mesh[i].color.x, mesh[i].color.y, mesh[i].color.z);
								fprintf(exportFile, vertColor.toUtf8());
							}
							fprintf(exportFile, "</float_array>");

							// technique common (color r,g,b)
							fprintf(exportFile, "\n\t\t\t\t\t<technique_common>");
							{
								UString accessorTag = "\n\t\t\t\t\t\t<accessor source=\"#";
								accessorTag.append(vertexcolorsArrayID);
								UString countString = UString::format("%i", numVertices);
								accessorTag.append("\" count=\"");
								accessorTag.append(countString);
								accessorTag.append("\" stride=\"3\">");
								fprintf(exportFile, accessorTag.toUtf8());
								{
									fprintf(exportFile, "\n\t\t\t\t\t\t\t<param name=\"R\" type=\"float\"/>");
									fprintf(exportFile, "\n\t\t\t\t\t\t\t<param name=\"G\" type=\"float\"/>");
									fprintf(exportFile, "\n\t\t\t\t\t\t\t<param name=\"B\" type=\"float\"/>");
								}
								fprintf(exportFile, "\n\t\t\t\t\t\t</accessor>");
							}
							fprintf(exportFile, "\n\t\t\t\t\t</technique_common>");
						}
						fprintf(exportFile, "\n\t\t\t\t</source>");

						// texcoords
						UString texcoordsTag = "\n\t\t\t\t<source id=\"";
						texcoordsTag.append(geometryID);
						texcoordsTag.append("-texcoords\">");
						fprintf(exportFile, texcoordsTag.toUtf8());
						{
							// float array
							UString texcoordsArrayID = geometryID;
							texcoordsArrayID.append("-texcoords-array");

							UString floatarrayTag = "\n\t\t\t\t\t<float_array id=\"";
							floatarrayTag.append(texcoordsArrayID);
							floatarrayTag.append("\" count=\"");
							UString countString = UString::format("%i", numVertices*2);
							floatarrayTag.append(countString);
							floatarrayTag.append("\">");
							fprintf(exportFile, floatarrayTag.toUtf8());
							for (int i=0; i<mesh.size(); i++)
							{
								// compensate for glScalef(1.0f / tex->texture->getWidth()) of texture space which is applied during rendering
								if (tex != NULL)
								{
									mesh[i].uv.x = mesh[i].uv.x * (1.0f / tex->texture->getWidth());
									mesh[i].uv.y = mesh[i].uv.y * (1.0f / tex->texture->getHeight());
								}
								UString texCoord = UString::format("%f %f ", mesh[i].uv.x, mesh[i].uv.y);
								fprintf(exportFile, texCoord.toUtf8());
							}
							fprintf(exportFile, "</float_array>");

							// technique common (uv x,y)
							fprintf(exportFile, "\n\t\t\t\t\t<technique_common>");
							{
								UString accessorTag = "\n\t\t\t\t\t\t<accessor source=\"#";
								accessorTag.append(texcoordsArrayID);
								UString countString = UString::format("%i", numVertices);
								accessorTag.append("\" count=\"");
								accessorTag.append(countString);
								accessorTag.append("\" stride=\"2\">");
								fprintf(exportFile, accessorTag.toUtf8());
								{
									fprintf(exportFile, "\n\t\t\t\t\t\t\t<param name=\"S\" type=\"float\"/>");
									fprintf(exportFile, "\n\t\t\t\t\t\t\t<param name=\"T\" type=\"float\"/>");
								}
								fprintf(exportFile, "\n\t\t\t\t\t\t</accessor>");
							}
							fprintf(exportFile, "\n\t\t\t\t\t</technique_common>");
						}
						fprintf(exportFile, "\n\t\t\t\t</source>");

						// position
						UString verticesTag = "\n\t\t\t\t<vertices id=\"";
						verticesTag.append(geometryID);
						verticesTag.append("-vertices\">");
						fprintf(exportFile, verticesTag.toUtf8());
						{
							UString inputTag = "\n\t\t\t\t\t<input semantic=\"POSITION\" source=\"#";
							inputTag.append(geometryID);
							inputTag.append("-positions\"/>");
							fprintf(exportFile, inputTag.toUtf8());
						}
						fprintf(exportFile, "\n\t\t\t\t</vertices>");

						// triangles
						UString trianglesTag = "\n\t\t\t\t<triangles material=\"#";
						trianglesTag.append(textureName);
						trianglesTag.append("-material\" count=\"");
						UString countString = UString::format("%i", numVertices/3);
						trianglesTag.append(countString);
						trianglesTag.append("\">");
						fprintf(exportFile, trianglesTag.toUtf8());
						{
							// vertex
							UString vertexTag = "\n\t\t\t\t\t<input semantic=\"VERTEX\" source=\"#";
							vertexTag.append(geometryID);
							vertexTag.append("-vertices\" offset=\"0\"/>");
							fprintf(exportFile, vertexTag.toUtf8());

							// normal
							UString normalTag = "\n\t\t\t\t\t<input semantic=\"NORMAL\" source=\"#";
							normalTag.append(geometryID);
							normalTag.append("-normals\" offset=\"1\"/>");
							fprintf(exportFile, normalTag.toUtf8());

							// texcoord
							UString texcoordTag = "\n\t\t\t\t\t<input semantic=\"TEXCOORD\" source=\"#";
							texcoordTag.append(geometryID);
							texcoordTag.append("-texcoords\" offset=\"2\" set=\"1\"/>");
							fprintf(exportFile, texcoordTag.toUtf8());

							// vertex color
							UString vertexcolorTag = "\n\t\t\t\t\t<input semantic=\"COLOR\" source=\"#";
							vertexcolorTag.append(geometryID);
							vertexcolorTag.append("-colors\" offset=\"3\" set=\"0\"/>");
							fprintf(exportFile, vertexcolorTag.toUtf8());

							fprintf(exportFile, "\n\t\t\t\t\t<p>");
							{
								for (int i=0; i<mesh.size(); i++)
								{
									UString singleVertexP = UString::format("%i %i %i %i ", i, i, i, i);
									fprintf(exportFile, singleVertexP.toUtf8());
								}
							}
							fprintf(exportFile, "</p>");
						}
						fprintf(exportFile, "\n\t\t\t\t</triangles>");
					}
					fprintf(exportFile, "\n\t\t\t</mesh>");
				}
				fprintf(exportFile, "\n\t\t</geometry>");
			}
		}
	}
	fprintf(exportFile, "\n\t</library_geometries>");

	// effects (= texturing)
	fprintf(exportFile, "\n\t<library_effects>");
	{
		for (int i=0; i<m_textures.size(); i++)
		{
			UString textureName = m_textures[i].texture->getName();
			UString effectTag = "\n\t\t<effect id=\"";
			effectTag.append(textureName);
			effectTag.append("-effect\">");
			fprintf(exportFile, effectTag.toUtf8());
			{
				fprintf(exportFile, "\n\t\t\t<profile_COMMON>");
				{
					// surface
					UString newparamSurfaceTag = "\n\t\t\t\t<newparam sid=\"";
					newparamSurfaceTag.append(textureName);
					newparamSurfaceTag.append("-surface\">");
					fprintf(exportFile, newparamSurfaceTag.toUtf8());
					{
						fprintf(exportFile, "\n\t\t\t\t\t<surface type=\"2D\">");
						{
							UString initfromTag = "\n\t\t\t\t\t\t<init_from>";
							initfromTag.append(textureName);
							initfromTag.append("</init_from>");
							fprintf(exportFile, initfromTag.toUtf8());
						}
						fprintf(exportFile, "\n\t\t\t\t\t</surface>");
					}
					fprintf(exportFile, "\n\t\t\t\t</newparam>");

					// sampler
					UString newparamSamplerTag = "\n\t\t\t\t<newparam sid=\"";
					newparamSamplerTag.append(textureName);
					newparamSamplerTag.append("-sampler\">");
					fprintf(exportFile, newparamSamplerTag.toUtf8());
					{
						fprintf(exportFile, "\n\t\t\t\t\t<sampler2D>");
						{
							UString sourceTag = "\n\t\t\t\t\t\t<source>";
							sourceTag.append(textureName);
							sourceTag.append("-surface</source>");
							fprintf(exportFile, sourceTag.toUtf8());
						}
						fprintf(exportFile, "\n\t\t\t\t\t</sampler2D>");
					}
					fprintf(exportFile, "\n\t\t\t\t</newparam>");

					// technique
					fprintf(exportFile, "\n\t\t\t\t<technique sid=\"common\">");
					{
						fprintf(exportFile, "\n\t\t\t\t\t<phong>");
						{
							// emission
							fprintf(exportFile, "\n\t\t\t\t\t\t<emission>");
							fprintf(exportFile, "\n\t\t\t\t\t\t\t<color sid=\"emission\">0 0 0 1</color>");
							fprintf(exportFile, "\n\t\t\t\t\t\t</emission>");

							// ambient
							fprintf(exportFile, "\n\t\t\t\t\t\t<ambient>");
							fprintf(exportFile, "\n\t\t\t\t\t\t\t<color sid=\"ambient\">0.5 0.5 0.5 1</color>");
							fprintf(exportFile, "\n\t\t\t\t\t\t</ambient>");

							// diffuse
							fprintf(exportFile, "\n\t\t\t\t\t\t<diffuse>");
							UString diffuseTag = "\n\t\t\t\t\t\t\t<texture texture=\"";
							diffuseTag.append(textureName);
							diffuseTag.append("-sampler\" texcoord=\"UVMap\"/>");
							fprintf(exportFile, diffuseTag.toUtf8());
							fprintf(exportFile, "\n\t\t\t\t\t\t</diffuse>");

							// specular
							fprintf(exportFile, "\n\t\t\t\t\t\t<specular>");
							fprintf(exportFile, "\n\t\t\t\t\t\t\t<color sid=\"specular\">0.0 0.0 0.0 1</color>");
							fprintf(exportFile, "\n\t\t\t\t\t\t</specular>");

							// shininess
							fprintf(exportFile, "\n\t\t\t\t\t\t<shininess>");
							fprintf(exportFile, "\n\t\t\t\t\t\t\t<float sid=\"shininess\">5</float>");
							fprintf(exportFile, "\n\t\t\t\t\t\t</shininess>");

							// index of refraction
							fprintf(exportFile, "\n\t\t\t\t\t\t<index_of_refraction>");
							fprintf(exportFile, "\n\t\t\t\t\t\t\t<float sid=\"index_of_refraction\">1</float>");
							fprintf(exportFile, "\n\t\t\t\t\t\t</index_of_refraction>");
						}
						fprintf(exportFile, "\n\t\t\t\t\t</phong>");
					}
					fprintf(exportFile, "\n\t\t\t\t</technique>");
				}
				fprintf(exportFile, "\n\t\t\t</profile_COMMON>");
			}
			fprintf(exportFile, "\n\t\t</effect>");
		}
	}
	fprintf(exportFile, "\n\t</library_effects>");

	// scene
	fprintf(exportFile, "\n\t<library_visual_scenes>");
	{
		fprintf(exportFile, "\n\t\t<visual_scene id=\"Scene\" name=\"Scene\">");
		{
			// go through every mesh
			int meshCounter = 0;
			for (Mesh *mesh_p = m_modelHeader->meshes_p; meshCounter++ < m_modelHeader->meshCount; mesh_p++)
			{
				//Dlist *dlist_p = &m_modelHeader->dlists_p[mesh_p->dlistid];
				MetroidMaterial *material_p = &m_modelHeader->materials_p[mesh_p->matid];
				MPHTEXTURE *tex = getTexture(material_p->texid);

				UString textureName = "null";
				if (tex != NULL)
					textureName = tex->texture->getName();

				// node
				UString nodeTag = UString::format("\n\t\t\t<node id=\"mesh%i\" type=\"NODE\">", meshCounter);
				fprintf(exportFile, nodeTag.toUtf8());
				{
					// instance geometry
					UString instancegeometryTag = UString::format("\n\t\t\t\t<instance_geometry url=\"#geometry%i\">", meshCounter);
					fprintf(exportFile, instancegeometryTag.toUtf8());
					{
						// bind material
						fprintf(exportFile, "\n\t\t\t\t\t<bind_material>");
						{
							// technique common
							fprintf(exportFile, "\n\t\t\t\t\t\t<technique_common>");
							{
								// instance material
								UString instancematerialTag = "\n\t\t\t\t\t\t\t<instance_material symbol=\"";
								instancematerialTag.append(textureName);
								instancematerialTag.append("-material\" target=\"#");
								instancematerialTag.append(textureName);
								instancematerialTag.append("-material\">");
								fprintf(exportFile, instancematerialTag.toUtf8());
								{
									// bind vertex input
									fprintf(exportFile, "\n\t\t\t\t\t\t\t\t<bind_vertex_input semantic=\"UVMap\" input_semantic=\"TEXCOORD\" input_set=\"0\"/>");
								}
								fprintf(exportFile, "\n\t\t\t\t\t\t\t</instance_material>");
							}
							fprintf(exportFile, "\n\t\t\t\t\t\t</technique_common>");
						}
						fprintf(exportFile, "\n\t\t\t\t\t</bind_material>");
					}
					fprintf(exportFile, "\n\t\t\t\t</instance_geometry>");
				}
				fprintf(exportFile, "\n\t\t\t</node>");
			}
		}
		fprintf(exportFile, "\n\t\t</visual_scene>");
	}
	fprintf(exportFile, "\n\t</library_visual_scenes>");

	// end
	fprintf(exportFile, "\n</COLLADA>");
	fclose(exportFile);

	debugLog("Done.\n");
}

void MetroidModelViewer::doExportDlist(u32 *data_p, u32 len, std::vector<MPHVERTEX> *mesh, std::vector<MPHVERTEX> *finalMesh)
{
	u32 *end_p = data_p + len/4;

	float vtx_state[3] = {0.0f, 0.0f, 0.0f};
	float nrm_state[3] = {0.0f, 0.0f, 0.0f};
	float uv_state[2] = {0.0f, 0.0f};
	float col_state[3] = {0.0f, 0.0f, 0.0f};

	while (data_p < end_p)
	{
		u32 regs = *(data_p++);

		for (u32 c=0; c<4; c++,regs>>=8)
		{
			u32 reg = ((regs & 0xFF)<<2) + 0x400;

			doExportReg(reg, &data_p, vtx_state, nrm_state, uv_state, col_state, mesh, finalMesh);
		}
	}
}

void MetroidModelViewer::doExportReg(u32 reg, u32 **data_pp, float vtx_state[3], float nrm_state[3], float uv_state[2], float col_state[3], std::vector<MPHVERTEX> *mesh, std::vector<MPHVERTEX> *finalMesh)
{
	u32 *data_p = *data_pp;

	switch (reg)
	{
		// NOP
		case 0x400:
			break;

		// MTX_RESTORE
		case 0x450:
			{
				data_p++;
			}
			break;

		// COLOR
		case 0x480:
			{
				u32 rgb = *(data_p++);

				u32 r = (rgb>> 0) & 0x1F;
				u32 g = (rgb>> 5) & 0x1F;
				u32 b = (rgb>>10) & 0x1F;

				col_state[0] = ((float)r)/31.0f;
				col_state[1] = ((float)g)/31.0f;
				col_state[2] = ((float)b)/31.0f;
			}
			break;

		// NORMAL
		case 0x484:
			{
				u32 xyz = *(data_p++);

				s32 x = (xyz>> 0) & 0x3FF;
				if( x & 0x200 )
					x |= 0xFFFFFC00;

				s32 y = (xyz>>10) & 0x3FF;
				if( y & 0x200 )
					y |= 0xFFFFFC00;

				s32 z = (xyz>>20) & 0x3FF;
				if( z & 0x200 )
					z |= 0xFFFFFC00;

				nrm_state[0] = ((float)x)/512.0f;
				nrm_state[1] = ((float)y)/512.0f;
				nrm_state[2] = ((float)z)/512.0f;
			}
			break;

		// TEXCOORD
		case 0x488:
			{
				u32 st = *(data_p++);

				s32 s = (st>> 0) & 0xFFFF;			if( s & 0x8000 )		s |= 0xFFFF0000;
				s32 t = (st>>16) & 0xFFFF;			if( t & 0x8000 )		t |= 0xFFFF0000;

				uv_state[0] = ((float)s)/16.0f;
				uv_state[1] = ((float)t)/16.0f;
			}
			break;

		// DIF_AMB
		case 0x4C0:
			{
				data_p++;
			}
			break;

		// VTX_16
		case 0x48C:
			{
				u32 xy = *(data_p++);

				s32 x = (xy>> 0) & 0xFFFF;
				if( x & 0x8000 )
					x |= 0xFFFF0000;

				s32 y = (xy>>16) & 0xFFFF;
				if( y & 0x8000 )
					y |= 0xFFFF0000;

				s32 z = (*(data_p++)) & 0xFFFF;
				if( z & 0x8000 )
					z |= 0xFFFF0000;

				vtx_state[0] = (((float)x) / 4096.0f) * mph_model_scale.getFloat();
				vtx_state[1] = (((float)y) / 4096.0f) * mph_model_scale.getFloat();
				vtx_state[2] = (((float)z) / 4096.0f) * mph_model_scale.getFloat();

				if (m_bCurMeshActive)
					mesh->push_back(getCurrentExportTri(vtx_state, nrm_state, uv_state, col_state));
			}
			break;

		// VTX_10
		case 0x490:
			{
				u32 xyz = *(data_p++);

				s32 x = (xyz>> 0) & 0x3FF;
				if( x & 0x200 )
					x |= 0xFFFFFC00;

				s32 y = (xyz>>10) & 0x3FF;
				if( y & 0x200 )
					y |= 0xFFFFFC00;

				s32 z = (xyz>>20) & 0x3FF;
				if( z & 0x200 )
					z |= 0xFFFFFC00;

				vtx_state[0] = (((float)x) / 64.0f) * mph_model_scale.getFloat();
				vtx_state[1] = (((float)y) / 64.0f) * mph_model_scale.getFloat();
				vtx_state[2] = (((float)z) / 64.0f) * mph_model_scale.getFloat();

				if (m_bCurMeshActive)
					mesh->push_back(getCurrentExportTri(vtx_state, nrm_state, uv_state, col_state));
			}
			break;

		// VTX_XY
		case 0x494:
			{
				u32 xy = *(data_p++);

				s32 x = (xy>> 0) & 0xFFFF;
				if( x & 0x8000 )
					x |= 0xFFFF0000;

				s32 y = (xy>>16) & 0xFFFF;
				if( y & 0x8000 )
					y |= 0xFFFF0000;

				vtx_state[0] = (((float)x) / 4096.0f) * mph_model_scale.getFloat();
				vtx_state[1] = (((float)y) / 4096.0f) * mph_model_scale.getFloat();

				if (m_bCurMeshActive)
					mesh->push_back(getCurrentExportTri(vtx_state, nrm_state, uv_state, col_state));
			}
			break;

		// VTX_XZ
		case 0x498:
			{
				u32 xz = *(data_p++);

				s32 x = (xz>> 0) & 0xFFFF;
				if( x & 0x8000 )
					x |= 0xFFFF0000;

				s32 z = (xz>>16) & 0xFFFF;
				if( z & 0x8000 )
					z |= 0xFFFF0000;

				vtx_state[0] = (((float)x) / 4096.0f) * mph_model_scale.getFloat();
				vtx_state[2] = (((float)z) / 4096.0f) * mph_model_scale.getFloat();

				if (m_bCurMeshActive)
					mesh->push_back(getCurrentExportTri(vtx_state, nrm_state, uv_state, col_state));
			}
			break;

		// VTX_YZ
		case 0x49C:
			{
				u32 yz = *(data_p++);

				s32 y = (yz>> 0) & 0xFFFF;
				if( y & 0x8000 )
					y |= 0xFFFF0000;

				s32 z = (yz>>16) & 0xFFFF;
				if( z & 0x8000 )
					z |= 0xFFFF0000;

				vtx_state[1] = (((float)y) / 4096.0f) * mph_model_scale.getFloat();
				vtx_state[2] = (((float)z) / 4096.0f) * mph_model_scale.getFloat();

				if (m_bCurMeshActive)
					mesh->push_back(getCurrentExportTri(vtx_state, nrm_state, uv_state, col_state));
			}
			break;

		// VTX_DIFF
		case 0x4A0:
			{
				u32 xyz = *(data_p++);

				s32 x = (xyz>> 0) & 0x3FF;
				if( x & 0x200 )
					x |= 0xFFFFFC00;

				s32 y = (xyz>>10) & 0x3FF;
				if( y & 0x200 )
					y |= 0xFFFFFC00;

				s32 z = (xyz>>20) & 0x3FF;
				if( z & 0x200 )
					z |= 0xFFFFFC00;

				vtx_state[0] += (((float)x) / 4096.0f) * mph_model_scale.getFloat();
				vtx_state[1] += (((float)y) / 4096.0f) * mph_model_scale.getFloat();
				vtx_state[2] += (((float)z) / 4096.0f) * mph_model_scale.getFloat();

				if (m_bCurMeshActive)
					mesh->push_back(getCurrentExportTri(vtx_state, nrm_state, uv_state, col_state));
			}
			break;

		// BEGIN_VTXS
		case 0x500:
			{
				u32 type = *(data_p++);
				switch( type )
				{
					case 0:
						m_iCurMeshType = 1;
						m_bCurMeshActive = true;
						mesh->clear();
						break;
					case 1:
						m_iCurMeshType = 2;
						m_bCurMeshActive = true;
						mesh->clear();
						break;
					case 2:
						m_iCurMeshType = 3;
						m_bCurMeshActive = true;
						mesh->clear();
						break;
					case 3:
						m_iCurMeshType = 4;
						m_bCurMeshActive = true;
						mesh->clear();
						break;
				}
			}
			break;

		// END_VTXS
		case 0x504:
			{
				m_bCurMeshActive = false;

				// triangulate everything
				// and start new mesh, since everything seems mixed up (what if a new mesh starts after END_VTXS)
				std::vector<MPHVERTEX> triangulatedMesh;

				switch (m_iCurMeshType)
				{
				case 1:
					// standard triangles (nothing to do here)
					triangulatedMesh = *mesh;
					break;
				case 2:
					// quads
					if (mesh->size() > 3)
					{
						for (int i=0; i<mesh->size(); i+=4)
						{
							MPHVERTEX A = (*mesh)[i];
							MPHVERTEX B = (*mesh)[i+1];
							MPHVERTEX C = (*mesh)[i+2];
							MPHVERTEX D = (*mesh)[i+3];

							triangulatedMesh.push_back(A); triangulatedMesh.push_back(B); triangulatedMesh.push_back(C);
							triangulatedMesh.push_back(C); triangulatedMesh.push_back(D); triangulatedMesh.push_back(A);
						}
					}
					break;
				case 3:
					// triangle strip
					if (mesh->size() > 2)
					{
						for (int i=0; i<mesh->size()-2; i++)
						{
							MPHVERTEX A = (*mesh)[i];
							MPHVERTEX B = (*mesh)[i+1];
							MPHVERTEX C = (*mesh)[i+2];

							if (i % 2)
							{
								triangulatedMesh.push_back(C); triangulatedMesh.push_back(B); triangulatedMesh.push_back(A);
							}
							else
							{
								triangulatedMesh.push_back(A); triangulatedMesh.push_back(B); triangulatedMesh.push_back(C);
							}
						}
					}
					break;
				case 4:
					// quad strip
					if (mesh->size() > 3)
					{
						for (int i=0; i<mesh->size()-2; i+=2)
						{
							MPHVERTEX A = (*mesh)[i];
							MPHVERTEX B = (*mesh)[i+1];
							MPHVERTEX C = (*mesh)[i+2];
							MPHVERTEX D = (*mesh)[i+3];

							triangulatedMesh.push_back(A); triangulatedMesh.push_back(B); triangulatedMesh.push_back(C);
							triangulatedMesh.push_back(D); triangulatedMesh.push_back(C); triangulatedMesh.push_back(B);
						}
					}
					break;
				}
				m_iCurMeshType = -1;

				finalMesh->insert(finalMesh->end(), triangulatedMesh.begin(), triangulatedMesh.end());
				mesh->clear();
			}
			break;
	}

	*data_pp = data_p;
}

MetroidModelViewer::MPHVERTEX MetroidModelViewer::getCurrentExportTri(float vtx_state[3], float nrm_state[3], float uv_state[2], float col_state[3])
{
	MPHVERTEX tri;

	tri.pos.x = vtx_state[0];
	tri.pos.y = vtx_state[1];
	tri.pos.z = vtx_state[2];

	tri.normal.x = nrm_state[0];
	tri.normal.y = nrm_state[1];
	tri.normal.z = nrm_state[2];

	tri.uv.x = uv_state[0];
	tri.uv.y = uv_state[1];

	tri.color.x = col_state[0];
	tri.color.y = col_state[1];
	tri.color.z = col_state[2];

	return tri;
}
