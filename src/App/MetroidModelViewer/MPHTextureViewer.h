//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		texture browser
//
// $NoKeywords: $
//===============================================================================//

#ifndef MPHTEXTUREVIEWER_H
#define MPHTEXTUREVIEWER_H

#include "CBaseUIWindow.h"

class Image;
class CBaseUIScrollView;
class CBaseUIImage;

class MPHTextureViewer : public CBaseUIWindow
{
public:
	MPHTextureViewer();
	virtual ~MPHTextureViewer() {;}

	void addTexture(Image *tex, UString name);
	void clearTextures();

	void onResized();

private:
	void onExport();

	CBaseUIScrollView *m_view;
	std::vector<CBaseUIImage*> m_textures;

	int m_iLastY;
};

#endif
