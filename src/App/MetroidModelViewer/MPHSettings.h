//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		model viewer settings window
//
// $NoKeywords: $
//===============================================================================//

#ifndef MPHSETTINGS_H
#define MPHSETTINGS_H

#include "CBaseUIWindow.h"

class MetroidModelViewer;
class ConVar;

class CBaseUIButton;
class CBaseUICheckbox;
class CBaseUILabel;
class CBaseUISlider;

class MPHSettings : public CBaseUIWindow
{
public:
	MPHSettings(MetroidModelViewer *mmv);
	virtual ~MPHSettings() {;}

	void update();

	void updateAlpha(bool alpha);
	void updateTextured(bool textured);
	void updateLightmapped(bool lightmapped);
	void updateFilter(bool filtering);
	void updateCaptureMouse(bool captured);

private:
	void onOpenClicked();
	void onOpenClickedReal();
	void onExportClicked();
	void onScaleChange(CBaseUISlider *slider);
	void onFovChange(CBaseUISlider *slider);
	void onWireframeClicked();
	void onTexturedChange(CBaseUICheckbox *checkbox);
	void onLightmappedChange(CBaseUICheckbox *checkbox);
	void onFilterChange(CBaseUICheckbox *checkbox);
	void onAlphaChange(CBaseUICheckbox *checkbox);
	void onAnimChange(CBaseUICheckbox *checkbox);
	void onCullingChange(CBaseUICheckbox *checkbox);
	void onDepthFuncChange(CBaseUICheckbox *checkbox);
	void onSensitivityChange(CBaseUISlider *slider);
	void onCaptureMouseChange(CBaseUICheckbox *checkbox);
	void onNoclipClicked();
	void onFullscreenClicked();

	MetroidModelViewer *m_modelViewer;

	CBaseUICheckbox *m_alpha;
	CBaseUISlider *m_scale;
	CBaseUILabel *m_scaleLabel;
	CBaseUILabel *m_fovLabel;
	ConVar *m_fov_ref;
	ConVar *m_mph_model_scale_ref;
	CBaseUICheckbox *m_textured;
	CBaseUICheckbox *m_lightmapped;
	ConVar *m_mousespeed_ref;
	CBaseUICheckbox *m_filtering;
	CBaseUICheckbox *m_captureMouse;

	bool m_bIsFirstOpenFileTry;
	int m_iWaitingOpenFileTry;
};

#endif
