//================ Copyright (c) 2018, PG, All rights reserved. =================//
//
// Purpose:		first person camera movement
//
// $NoKeywords: $
//===============================================================================//

#ifndef MPHPLAYER_H
#define MPHPLAYER_H

#include "cbase.h"

class Camera;

class MPHPlayer
{
public:
	MPHPlayer();
	virtual ~MPHPlayer();

	void update(bool rotateWithMouse);

	void onKeyDown(KeyboardEvent &key);
	void onKeyUp(KeyboardEvent &key);

	inline Camera *getCamera() const {return m_camera;}

private:
	void onFovChange(UString oldValue, UString newValue);

	Camera *m_camera;
	Vector3 m_vVelocity;

	bool m_bLeft;
	bool m_bRight;
	bool m_bForward;
	bool m_bBackward;

	bool m_bSprinting;
	bool m_bCrouching;
	bool m_bJumping;

	bool m_bJumpUpDownToggle;
};

#endif
