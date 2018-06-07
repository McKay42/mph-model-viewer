//================ Copyright (c) 2018, PG, All rights reserved. =================//
//
// Purpose:		first person camera movement
//
// $NoKeywords: $
//===============================================================================//

#include "MPHPlayer.h"

#include "Engine.h"
#include "ConVar.h"
#include "Camera.h"
#include "Keyboard.h"
#include "Mouse.h"

ConVar mousespeed("mousespeed", 1.4f);
ConVar fov("fov", 85.0f);

MPHPlayer::MPHPlayer()
{
	m_camera = new Camera();
	m_camera->setFov(fov.getFloat());

	fov.setCallback( fastdelegate::MakeDelegate(this, &MPHPlayer::onFovChange) );

	m_bLeft = false;
	m_bRight = false;
	m_bForward = false;
	m_bBackward = false;

	m_bSprinting = false;
	m_bCrouching = false;
	m_bJumping = false;

	m_bJumpUpDownToggle = true;
}

MPHPlayer::~MPHPlayer()
{
	SAFE_DELETE(m_camera);
}

void MPHPlayer::update(bool rotateWithMouse)
{
	// rotation
	if (rotateWithMouse)
	{
		Vector2 diff = engine->getMouse()->getRawDelta();
		engine->getMouse()->setPos( Vector2((int)(engine->getScreenWidth()/2), (int)(engine->getScreenHeight()/2)) );

		float degreesPerDot = 0.02199f; // Source Engine value here
		if (diff.x != 0.0f)
			m_camera->rotateY(-diff.x*degreesPerDot*mousespeed.getFloat());
		if (diff.y != 0.0f)
			m_camera->rotateX(-diff.y*degreesPerDot*mousespeed.getFloat());
	}

	// translation
	// noclip movement
	{
		const float noclipSpeed = (16.5f * (engine->getKeyboard()->isShiftDown() ? 3.0f : 1.0f) * (engine->getKeyboard()->isControlDown() ? 0.2f : 1)) * 0.4f;
		const float noclipAccelerate = 20.0f;
		const float friction = 10.0f;

		Vector3 wishdir;
		wishdir += (m_bForward ? m_camera->getViewDirection() : Vector3());
		wishdir -= (m_bBackward ? m_camera->getViewDirection() : Vector3());
		wishdir += (m_bLeft ? m_camera->getViewRight() : Vector3());
		wishdir -= (m_bRight ? m_camera->getViewRight() : Vector3());
		wishdir += (m_bJumping ? (m_bJumpUpDownToggle ? -Vector3(0, 1, 0) : Vector3(0, 1, 0)) : Vector3());
		wishdir.normalize();

		float wishspeed = noclipSpeed;

		// friction (deccelerate)
		float spd = m_vVelocity.length();
		if (spd > 0.00000001f)
		{
			// only apply friction once we "stop" moving (special case for noclip mode)
			if (wishdir.length() == 0.0f)
			{
				// multiply speed into friction: this keeps the decceleration time constant, and independent of the current speed
				float drop = spd * friction * engine->getFrameTime();

				float newSpeed = spd - drop;
				if (newSpeed < 0.0f)
					newSpeed = 0.0f;

				newSpeed /= spd;
				m_vVelocity *= newSpeed;
			}
		}
		else
			m_vVelocity.zero();

		// accelerate
		{
			float addspeed = wishspeed;
			if (addspeed > 0.0f)
			{
				float accelspeed = noclipAccelerate * engine->getFrameTime() * wishspeed;

				if (accelspeed > addspeed)
					accelspeed = addspeed;

				m_vVelocity += accelspeed * wishdir;
			}
		}

		// clamp to max speed
		if (m_vVelocity.length() > noclipSpeed)
			m_vVelocity.setLength(noclipSpeed);
	}

	m_camera->setPos(m_camera->getPos() + m_vVelocity * engine->getFrameTime());
}

void MPHPlayer::onKeyDown(KeyboardEvent &key)
{
	if (key == KEY_W)
		m_bForward = true;
	if (key == KEY_A)
		m_bLeft = true;
	if (key == KEY_S)
		m_bBackward = true;
	if (key == KEY_D)
		m_bRight = true;

	if (key == KEY_SHIFT)
		m_bSprinting = true;
	if (key == KEY_CONTROL)
		m_bCrouching = true;

	if (key == KEY_SPACE && !m_bJumping)
	{
		m_bJumping = true;
		m_bJumpUpDownToggle = !m_bJumpUpDownToggle;
	}
}

void MPHPlayer::onKeyUp(KeyboardEvent &key)
{
	if (key == KEY_W)
		m_bForward = false;
	if (key == KEY_A)
		m_bLeft = false;
	if (key == KEY_S)
		m_bBackward = false;
	if (key == KEY_D)
		m_bRight = false;

	if (key == KEY_SHIFT)
		m_bSprinting = false;
	if (key == KEY_CONTROL)
		m_bCrouching = false;

	if (key == KEY_SPACE)
		m_bJumping = false;
}

void MPHPlayer::onFovChange(UString oldValue, UString newValue)
{
	m_camera->setFov(newValue.toFloat());
}
