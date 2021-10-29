/**
 CScene3D
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include SingletonTemplate
#include "DesignPatterns\SingletonTemplate.h"

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include CKeyboardController
#include "Inputs\KeyboardController.h"
// Include CMouseController
#include "Inputs\MouseController.h"

// Include CFPSCounter
#include "TimeControl\FPSCounter.h"

// Include CSoundController
#include "..\SoundController\SoundController.h"

// Include CCamera
#include "Camera.h"

// Include Skybox
#include "Skybox/SkyBox.h"

// Include Terrain
#include "Terrain/Terrain.h"

// Include CSolidObjectManager
#include "Entities/SolidObjectManager.h"

// Include CPlayer3D
#include "Entities/Player3D.h"

// Include CProjectileManager
#include "WeaponInfo/ProjectileManager.h"
#include "WeaponInfo/Pistol.h"
#include "WeaponInfo/AssaultRifle.h"
#include "WeaponInfo/SubmachineGun.h"

class CSettings;

class CScene3D : public CSingletonTemplate<CScene3D>
{
	friend CSingletonTemplate<CScene3D>;
public:
	// Init
	bool Init(void);

	// Update
	bool Update(const double dElapsedTime);

	// PreRender
	void PreRender(void);

	// Render
	void Render(void);

	// PostRender
	void PostRender(void);

protected:
	// The handler to the CSettings
	CSettings* cSettings;

	// Keyboard Controller singleton instance
	CKeyboardController* cKeyboardController;

	// Mouse Controller singleton instance
	CMouseController* cMouseController;

	// A transformation matrix for controlling where to render the entities
	glm::mat4 transform;

	// FPS Control
	CFPSCounter* cFPSCounter;

	// Handler to the CSoundController
	CSoundController* cSoundController;

	// Handler to the camera class
	CCamera* cCamera;

	// Handler to the skybox class
	CSkyBox* cSkybox;

	// Handler to the skybox class
	CTerrain* cTerrain;

	// Handler to the CSolidObjectManager class
	CSolidObjectManager* cSolidObjectManager;

	// Handler to the Player3D class
	CPlayer3D* cPlayer3D;

	// Handler to the CProjectileManager class
	CProjectileManager* cProjectileManager;

	// Constructor
	CScene3D(void);
	// Destructor
	virtual ~CScene3D(void);
};

