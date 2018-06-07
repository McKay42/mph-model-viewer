//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		model viewer settings window
//
// $NoKeywords: $
//===============================================================================//

#include "MPHSettings.h"

#include "Engine.h"
#include "Environment.h"
#include "ConVar.h"

#include "MetroidModelViewer.h"

#include "CBaseUIContainer.h"
#include "CBaseUIButton.h"
#include "CBaseUILabel.h"
#include "CBaseUICheckbox.h"
#include "CBaseUISlider.h"

MPHSettings::MPHSettings(MetroidModelViewer *mmv) : CBaseUIWindow(10, 10, 190, 200, "Model Viewer (TAB)")
{
	m_modelViewer = mmv;
	m_bIsFirstOpenFileTry = true;
	m_iWaitingOpenFileTry = 0;

	setResizeable(false);

	m_mph_model_scale_ref = convar->getConVarByName("mph_model_scale");
	m_fov_ref = convar->getConVarByName("fov");
	m_mousespeed_ref = convar->getConVarByName("mousespeed");

	int xPos = 6;
	int yPos = 6;
	int yGap = 6;
	float groupSeparationMult = 4.0f;
	int sliderBlockSize = 25;

	// open a model
	CBaseUIButton *loadButton = new CBaseUIButton(xPos, yPos, 100, 24, "", "Open");
	loadButton->setSizeToContent(5, 6);
	loadButton->setClickCallback( fastdelegate::MakeDelegate(this, &MPHSettings::onOpenClicked) );

	// export currently loaded model
	CBaseUIButton *exportButton = new CBaseUIButton(xPos, loadButton->getPos().y + loadButton->getSize().y + yGap, 100, 24, "", "Export .dae to /exports/");
	exportButton->setTextColor(0xff00ff00);
	exportButton->setTextDarkColor(0xff005500);
	exportButton->setSizeToContent(5, 6);
	exportButton->setClickCallback( fastdelegate::MakeDelegate(this, &MPHSettings::onExportClicked) );

	// model scale slider
	m_scale = new CBaseUISlider(xPos, exportButton->getPos().y + exportButton->getSize().y + yGap, 170, 24, "");
	m_scale->setBounds(1.0f, 144.0f);
	m_scale->setAnimated(false);
	m_scale->setValue(m_mph_model_scale_ref->getFloat(), false);
	m_scale->setBlockSize(sliderBlockSize, sliderBlockSize);
	m_scale->setOrientation(true);
	m_scale->setKeyDelta(1.0f);
	m_scale->setChangeCallback( fastdelegate::MakeDelegate(this, &MPHSettings::onScaleChange) );

	m_scaleLabel = new CBaseUILabel(m_scale->getPos().x + m_scale->getSize().x + 8, m_scale->getPos().y, 100, 24, "", UString::format("Scale: %.1f", m_mph_model_scale_ref->getFloat()));
	m_scaleLabel->setDrawBackground(false);
	m_scaleLabel->setDrawFrame(false);



	// wireframe toggle
	CBaseUIButton *wireframe = new CBaseUIButton(xPos, m_scale->getPos().y + m_scale->getSize().y + (int)(yGap*groupSeparationMult), 100, 24, "", "Wireframe (Q)");
	wireframe->setSizeToContent(5, 6);
	wireframe->setClickCallback( fastdelegate::MakeDelegate(this, &MPHSettings::onWireframeClicked) );

	// textures
	m_textured = new CBaseUICheckbox(xPos, wireframe->getPos().y + wireframe->getSize().y + yGap, 110, 24, "", "Textured (E)");
	m_textured->setChecked(true);
	m_textured->setSizeToContent(2, 6);
	m_textured->setChangeCallback( fastdelegate::MakeDelegate(this, &MPHSettings::onTexturedChange) );

	// lightmaps
	m_lightmapped = new CBaseUICheckbox(xPos, m_textured->getPos().y + m_textured->getSize().y + yGap, 100, 24, "", "Lightmapped (L)");
	m_lightmapped->setChecked(true);
	m_lightmapped->setSizeToContent(2, 6);
	m_lightmapped->setChangeCallback( fastdelegate::MakeDelegate(this, &MPHSettings::onLightmappedChange) );

	// texture filtering
	m_filtering = new CBaseUICheckbox(xPos, m_lightmapped->getPos().y + m_lightmapped->getSize().y + yGap, 100, 24, "", "Texture filtering (F)");
	m_filtering->setChecked(true);
	m_filtering->setSizeToContent(2, 6);
	m_filtering->setChangeCallback( fastdelegate::MakeDelegate(this, &MPHSettings::onFilterChange) );

	// alpha toggle
	m_alpha = new CBaseUICheckbox(xPos, m_filtering->getPos().y + m_filtering->getSize().y + yGap, 100, 24, "", "Experimental alpha (B)");
	m_alpha->setChecked(true);
	m_alpha->setSizeToContent(2, 6);
	m_alpha->setChangeCallback( fastdelegate::MakeDelegate(this, &MPHSettings::onAlphaChange) );

	// anim
	CBaseUICheckbox *animatedTextures = new CBaseUICheckbox(xPos, m_alpha->getPos().y + m_alpha->getSize().y + yGap, 100, 24, "", "Experimental anim.");
	animatedTextures->setChecked(true);
	animatedTextures->setSizeToContent(2, 6);
	animatedTextures->setChangeCallback( fastdelegate::MakeDelegate(this, &MPHSettings::onAnimChange) );

	// culling toggle
	CBaseUICheckbox *culling = new CBaseUICheckbox(xPos, animatedTextures->getPos().y + animatedTextures->getSize().y + yGap, 110, 24, "", "Backface culling");
	culling->setChecked(true);
	culling->setSizeToContent(2, 6);
	culling->setChangeCallback( fastdelegate::MakeDelegate(this, &MPHSettings::onCullingChange) );

	// glDepthFunc toggle
	CBaseUICheckbox *depthFunc = new CBaseUICheckbox(xPos, culling->getPos().y + culling->getSize().y + yGap, 110, 24, "", "glDepthFunc(GL_LEQUAL)");
	depthFunc->setChecked(false);
	depthFunc->setSizeToContent(2, 6);
	depthFunc->setChangeCallback( fastdelegate::MakeDelegate(this, &MPHSettings::onDepthFuncChange) );

	// fov slider
	CBaseUISlider *fovSlider = new CBaseUISlider(xPos, depthFunc->getPos().y + depthFunc->getSize().y + yGap, 100, 24, "");
	fovSlider->setBounds(50.0f, 170.0f);
	fovSlider->setAnimated(false);
	fovSlider->setBlockSize(sliderBlockSize, sliderBlockSize);
	fovSlider->setValue(m_fov_ref->getFloat(), false);
	fovSlider->setOrientation(true);
	fovSlider->setKeyDelta(5.0f);
	fovSlider->setChangeCallback( fastdelegate::MakeDelegate(this, &MPHSettings::onFovChange) );

	m_fovLabel = new CBaseUILabel(fovSlider->getPos().x + fovSlider->getSize().x + 8, fovSlider->getPos().y, 100, 24, "", UString::format("FOV: %i", m_fov_ref->getInt()));
	m_fovLabel->setDrawBackground(false);
	m_fovLabel->setDrawFrame(false);



	// mouse sensitivity
	CBaseUISlider *sensitivity = new CBaseUISlider(xPos, (int)fovSlider->getPos().y + (int)fovSlider->getSize().y + (int)(yGap*groupSeparationMult), 100, 24, "");
	sensitivity->setBounds(0.05f, 3.0f);
	sensitivity->setAnimated(false);
	sensitivity->setValue(m_mousespeed_ref->getFloat(), false);
	sensitivity->setBlockSize(sliderBlockSize, sliderBlockSize);
	sensitivity->setOrientation(true);
	sensitivity->setKeyDelta(0.1f);
	sensitivity->setChangeCallback( fastdelegate::MakeDelegate(this, &MPHSettings::onSensitivityChange) );

	CBaseUILabel *sensitivityLabel = new CBaseUILabel(sensitivity->getPos().x + sensitivity->getSize().x + 8, sensitivity->getPos().y, 100, 24, "", "Sensitivity");
	sensitivityLabel->setDrawBackground(false);
	sensitivityLabel->setDrawFrame(false);

	// mouse lock
	m_captureMouse = new CBaseUICheckbox(xPos, sensitivity->getPos().y + sensitivity->getSize().y + yGap + 1, 110, 24, "", "Lock Cursor (C)");
	m_captureMouse->setChecked(false);
	m_captureMouse->setSizeToContent(2, 6);
	m_captureMouse->setChangeCallback( fastdelegate::MakeDelegate(this, &MPHSettings::onCaptureMouseChange) );

	// noclip
	CBaseUIButton *toggleNoclip = new CBaseUIButton(xPos, m_captureMouse->getPos().y + m_captureMouse->getSize().y + yGap, 100, 24, "", "Toggle Noclip (N)");
	toggleNoclip->setVisible(false);
	toggleNoclip->setSizeToContent(5, 6);
	toggleNoclip->setClickCallback( fastdelegate::MakeDelegate(this, &MPHSettings::onNoclipClicked) );

	// fullscreen
	CBaseUIButton *fullscreenToggle = new CBaseUIButton(xPos, toggleNoclip->getPos().y + toggleNoclip->getSize().y + yGap, 100, 24, "", "Toggle Fullscreen");
	fullscreenToggle->setSizeToContent(5, 6);
	fullscreenToggle->setClickCallback( fastdelegate::MakeDelegate(this, &MPHSettings::onFullscreenClicked) );

	getContainer()->addBaseUIElement(loadButton);
	getContainer()->addBaseUIElement(exportButton);
	getContainer()->addBaseUIElement(m_scale);
	getContainer()->addBaseUIElement(m_scaleLabel);
	getContainer()->addBaseUIElement(fovSlider);
	getContainer()->addBaseUIElement(m_fovLabel);
	getContainer()->addBaseUIElement(wireframe);
	getContainer()->addBaseUIElement(m_textured);
	getContainer()->addBaseUIElement(m_lightmapped);
	getContainer()->addBaseUIElement(culling);
	getContainer()->addBaseUIElement(depthFunc);
	getContainer()->addBaseUIElement(m_alpha);
	getContainer()->addBaseUIElement(animatedTextures);
	getContainer()->addBaseUIElement(m_filtering);
	getContainer()->addBaseUIElement(sensitivity);
	getContainer()->addBaseUIElement(sensitivityLabel);
	getContainer()->addBaseUIElement(m_captureMouse);
	getContainer()->addBaseUIElement(toggleNoclip);
	getContainer()->addBaseUIElement(fullscreenToggle);

	setSizeToContent(6,6);
}

void MPHSettings::update()
{
	CBaseUIWindow::update();
	if (!m_bVisible) return;

	// HACKHACK: open the dialog delayed, to give the engine enough time to exit fullscreen mode
	m_iWaitingOpenFileTry--;
	if (m_iWaitingOpenFileTry == 1)
	{
		m_iWaitingOpenFileTry = 0;
		onOpenClickedReal();
	}
}

void MPHSettings::onOpenClicked()
{
	m_iWaitingOpenFileTry = 30;
	engine->disableFullscreen();
}

void MPHSettings::onOpenClickedReal()
{
	UString selectedFile = env->openFileWindow("Metroid Prime Hunters model (*.bin)\0*.bin\0All Files (*.*)\0*.*\0\0", "Open File", m_bIsFirstOpenFileTry ? "mph\\models\\" : "");
	if (selectedFile.length() > 4)
	{
		m_bIsFirstOpenFileTry = false;
		m_modelViewer->loadMapRaw(selectedFile);
	}
}

void MPHSettings::onExportClicked()
{
	m_modelViewer->exportPlyModel();
}

void MPHSettings::onScaleChange(CBaseUISlider *slider)
{
	m_mph_model_scale_ref->setValue(slider->getFloat());

	if (slider->getFloat() >= 100.0f || slider->getFloat() <= 1.0f)
		m_scaleLabel->setText(UString::format("Scale: %i", m_mph_model_scale_ref->getInt()));
	else
		m_scaleLabel->setText(UString::format("Scale: %.1f", m_mph_model_scale_ref->getFloat()));
}

void MPHSettings::onFovChange(CBaseUISlider *slider)
{
	m_fov_ref->setValue(slider->getFloat());
	m_fovLabel->setText(UString::format("FOV: %i", slider->getInt()));
}

void MPHSettings::onCullingChange(CBaseUICheckbox *checkbox)
{
	m_modelViewer->setCulling(checkbox->isChecked());
}

void MPHSettings::onWireframeClicked()
{
	m_modelViewer->toggleWireframe();
}

void MPHSettings::onTexturedChange(CBaseUICheckbox *checkbox)
{
	m_modelViewer->setTextured(checkbox->isChecked());
}
void MPHSettings::updateTextured(bool textured)
{
	m_textured->setChecked(textured);
}

void MPHSettings::onLightmappedChange(CBaseUICheckbox *checkbox)
{
	m_modelViewer->setLightmapped(checkbox->isChecked());
}
void MPHSettings::updateLightmapped(bool lightmapped)
{
	m_lightmapped->setChecked(lightmapped);
}

void MPHSettings::onAlphaChange(CBaseUICheckbox *checkbox)
{
	m_modelViewer->setAlpha(checkbox->isChecked());
}
void MPHSettings::updateAlpha(bool alpha)
{
	m_alpha->setChecked(alpha);
}

void MPHSettings::onFilterChange(CBaseUICheckbox *checkbox)
{
	m_modelViewer->setTextureFiltering(checkbox->isChecked());
}
void MPHSettings::updateFilter(bool filtering)
{
	m_filtering->setChecked(filtering);
}

void MPHSettings::onAnimChange(CBaseUICheckbox *checkbox)
{
	m_modelViewer->setAnim(checkbox->isChecked());
}

void MPHSettings::onDepthFuncChange(CBaseUICheckbox *checkbox)
{
	m_modelViewer->setDepthFuncLequal(checkbox->isChecked());
}

void MPHSettings::onSensitivityChange(CBaseUISlider *slider)
{
	m_mousespeed_ref->setValue(slider->getFloat());
}

void MPHSettings::onCaptureMouseChange(CBaseUICheckbox *checkbox)
{
	m_modelViewer->setCaptureMouse(checkbox->isChecked());
}
void MPHSettings::updateCaptureMouse(bool captured)
{
	m_captureMouse->setChecked(captured);
}

void MPHSettings::onNoclipClicked()
{
	m_modelViewer->toggleNoclip();
}

void MPHSettings::onFullscreenClicked()
{
	engine->toggleFullscreen();
}
