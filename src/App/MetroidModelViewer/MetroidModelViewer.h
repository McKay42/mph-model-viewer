//================ Copyright (c) 2015, PG & mike260, All rights reserved. =================//
//
// Purpose:		metroid prime hunters model viewer
//
// $NoKeywords: $mph
//=========================================================================================//

// To download mike260's model viewer, go to http://filetrip.net/nds-downloads/utilities/download-dsgraph-1-0-f29517.html
// I don't take credit for the reverse-engineering of the texture formats and model format, this was all mike's work.
// You can do everything you want with this code

#ifndef METROIDMODELVIEWER_H
#define METROIDMODELVIEWER_H

#include "App.h"
#include "MPHStructs.h"

class ConVar;
class Camera;
class Image;

class CWindowManager;

class MPHSettings;
class MPHTextureViewer;

class MPHPlayer;

class MetroidModelViewer : public App
{
public:
	MetroidModelViewer();
	~MetroidModelViewer();

	virtual void draw(Graphics *g);
	virtual void update();

	virtual void onKeyDown(KeyboardEvent &key);
	virtual void onKeyUp(KeyboardEvent &key);
	virtual void onChar(KeyboardEvent &charCode);

	virtual void onResolutionChanged(Vector2 newResolution);

	virtual void onFocusLost();

	void renderModel(Header *hdr_p);
	void doRenderDlist(u32 *data_p, u32 len);
	void doRenderReg(u32 reg, u32 **data_pp, float vtx_state[3]);

	void setCaptureMouse(bool captureMouse);
	void setCulling(bool culling) {m_bCulling = culling;}
	void setAlpha(bool alpha) {m_bAlpha = alpha;}
	void setAnim(bool anim) {m_bAnim = anim;}
	void setTextured(bool textured) {m_bTextured = textured;}
	void setLightmapped(bool lightmapped) {m_bLightmapped = lightmapped;}
	void setTextureFiltering(bool filtering) {m_bTextureFiltering = filtering;}
	void setDepthFuncLequal(bool lequal) {m_bGLLequal = lequal;}

	void toggleWireframe() {m_iWireframe++; m_iWireframe %= 3;}
	void toggleNoclip();

	void exportPlyModel();

	void loadMap(UString args);
	void loadMapRaw(UString filepath);

	struct MPHTEXTURE
	{
		int id;
		bool alpha;
		Image *texture;
	};

	struct MPHVERTEX
	{
		Vector3 pos;
		Vector3 normal;
		Vector3 color;
		Vector2 uv;
	};

private:
	void buildTextures(Header *hdr_p);
	bool existsTexture(int texid);
	MPHTEXTURE *getTexture(int texid);

	void doExportDlist(u32 *data_p, u32 len, std::vector<MPHVERTEX> *mesh, std::vector<MPHVERTEX> *finalMesh);
	void doExportReg(u32 reg, u32 **data_pp, float vtx_state[3], float nrm_state[3], float uv_state[2], float col_state[3], std::vector<MPHVERTEX> *mesh, std::vector<MPHVERTEX> *finalMesh);
	MPHVERTEX getCurrentExportTri(float vtx_state[3], float nrm_state[3], float uv_state[2], float col_state[3]);

	// file
	UString m_sFileName;
	int m_iFileSize;
	u8 *m_rawModelFileData;
	u8 *m_rawTextureFileData;
	Header *m_modelHeader;

	std::vector<MPHTEXTURE> m_textures;

	bool m_bBuildTextures;
	bool m_bDebugTextureBuilding;
	bool m_bTextureDebugOverlay;

	// rendering
	int m_iWireframe;
	bool m_bGLLequal;
	bool m_bTextureFiltering;
	bool m_bTextured;
	bool m_bLightmapped;
	bool m_bCulling;
	bool m_bAlpha;
	bool m_bAnim;

	// camera/player
	bool m_bCaptureMouse;
	MPHPlayer *m_player;

	// debugging
	CWindowManager *m_windowManager;
	MPHTextureViewer *m_textureViewer;
	MPHSettings *m_settings;

	// exporting
	int m_iCurMeshType;
	bool m_bCurMeshActive;
};

#endif
