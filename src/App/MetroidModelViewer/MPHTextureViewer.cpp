//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		texture browser
//
// $NoKeywords: $
//===============================================================================//

#include "MPHTextureViewer.h"

#include "ResourceManager.h"
#include "Engine.h"

#include "CBaseUIContainer.h"
#include "CBaseUIImage.h"
#include "CBaseUIButton.h"
#include "CBaseUILabel.h"
#include "CBaseUIScrollView.h"

#include "OpenGLHeaders.h"



//********************************//
//	Image class without blending  //
//********************************//

class MPHBaseUIImage : public CBaseUIImage
{
public:
	MPHBaseUIImage(UString imageResourceName, float xPos, float yPos, float xSize, float ySize, UString name) : CBaseUIImage(imageResourceName, xPos, yPos, xSize, ySize, name) {;}

	void draw(Graphics *g)
	{
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_BLEND);
		{
			CBaseUIImage::draw(g);
		}
		glPopAttrib();
	}
};



MPHTextureViewer::MPHTextureViewer() : CBaseUIWindow(30, 30, 170, 800, "Texture Viewer")
{
	m_iLastY = 6;

	m_view = new CBaseUIScrollView(0, 0, getContainer()->getSize().x, getContainer()->getSize().y, "");
	m_view->setDrawFrame(false);
	m_view->setDrawBackground(false);
	m_view->setHorizontalScrolling(false);
	m_view->setVerticalScrolling(true);

	getContainer()->addBaseUIElement(m_view);
}

void MPHTextureViewer::addTexture(Image *tex, UString name)
{
	if (tex == NULL) return;

	CBaseUIImage *img = new MPHBaseUIImage("", 6, m_iLastY, tex->getWidth()+1, tex->getHeight()+1, name);
	img->setDrawFrame(true);
	img->setDrawBackground(false);
	img->setBackgroundColor(0xff00ff00);
	img->setScaleToFit(false);
	img->setImage(tex);

	CBaseUILabel *lab = new CBaseUILabel(img->getPos().x + img->getSize().x+6, img->getPos().y+img->getSize().y/2, 100, 28, name, name);
	lab->setSizeToContent(3, 6);
	lab->setDrawBackground(false);
	lab->setDrawFrame(false);
	lab->setPosY(img->getPos().y+img->getSize().y/2 - lab->getSize().y/2);

	if (lab->getPos().x + lab->getSize().x > m_vSize.x)
	{
		int xDiff = (lab->getPos().x + lab->getSize().x + 6) - m_vSize.x;
		setPosX(m_vPos.x - xDiff);
		setSizeX(lab->getPos().x + lab->getSize().x + 6);
	}

	m_view->getContainer()->addBaseUIElement(img);
	m_view->getContainer()->addBaseUIElement(lab);
	m_view->setScrollSizeToContent();

	m_textures.push_back(img);

	m_iLastY += tex->getHeight() + 6;
}

void MPHTextureViewer::clearTextures()
{
	m_textures.clear();
	m_view->clear();

	m_iLastY = 6;
	setPosX(m_vPos.x + (m_vSize.x - 138));
	setSizeX(138);

	CBaseUIButton *exportButton = new CBaseUIButton(6, m_iLastY, 100, 24, "", "Export all to /exports/");
	exportButton->setClickCallback( fastdelegate::MakeDelegate(this, &MPHTextureViewer::onExport) );
	exportButton->setTextColor(0xff00ff00);
	exportButton->setTextDarkColor(0xff005500);
	exportButton->setDrawBackground(false);
	exportButton->setSizeToContent(5, 5);

	if (exportButton->getPos().x + exportButton->getSize().x > m_vSize.x)
	{
		int xDiff = (exportButton->getPos().x + exportButton->getSize().x + 6) - m_vSize.x;
		setPosX(m_vPos.x - xDiff);
		setSizeX(exportButton->getPos().x + exportButton->getSize().x + 6);
	}

	m_view->getContainer()->addBaseUIElement(exportButton);
	m_view->setScrollSizeToContent();

	m_iLastY += exportButton->getSize().y + 6;
}

void MPHTextureViewer::onExport()
{
	if (m_textures.size() < 1)
	{
		debugLog("Nothing to export!\n");
		return;
	}

	debugLog("Starting texture export...\n");
	for (int i=0; i<m_textures.size(); i++)
	{
		CBaseUIImage *curImage = m_textures[i];
		debugLog("   Exporting %s ...\n", curImage->getName().toUtf8());
		curImage->getImage()->writeToFile("exports/");
	}
	debugLog("Done.\n");
}

void MPHTextureViewer::onResized()
{
	CBaseUIWindow::onResized();
	m_view->setSize(getContainer()->getSize().x, getContainer()->getSize().y);
}
