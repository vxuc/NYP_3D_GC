/**
 CPlayer3D
 By: Toh Da Jun
 Date: Apr 2020
 */
#include "Player3D.h"

 // Include ShaderManager
#include "RenderControl/ShaderManager.h"

// Include ImageLoader
#include "System/ImageLoader.h"

//For allowing creating of Mesh 
#include "Primitives/MeshBuilder.h"

#include "../../MyMath.h"

#include <iostream>
using namespace std;

/**
 @brief Default Constructor
 */
CPlayer3D::CPlayer3D(void)
	: vec3Up(glm::vec3(0.0f, 1.0f, 0.0f))
	, vec3Right(glm::vec3(1.0f, 1.0f, 0.0f))
	, vec3WorldUp(glm::vec3(0.0f, 1.0f, 0.0f))
	, fYaw(-90.0f)
	, fPitch(0.0f)
	, fMouseSensitivity(0.1f)
	, cCamera(NULL)
	, cTerrain(NULL)
	, cPrimaryWeapon(NULL)
	, cSecondaryWeapon(NULL)
	, iCurrentWeapon(0)
	, slideTimer(1.f)
	, velocity(0.f)
	, minSpeed(0.f)
	, maxSpeed(.1f)
	, addSprintSpeed(0.f)
	, addSprintVelocity(0.f)
	, addCrouchSpeed(0.f)
	, addCrouchVelocity(0.f)
	, addSlideSpeed(0.f)
	, addSlideVelocity(0.f)
	, addCounterSlideSpeed(-10.f)
	, totalVelocity(0.f)
{
	// Set the default position so it is above the ground
	vec3Position = glm::vec3(0.0f, 0.5f, 0.0f);

	// Update the player vectors
	UpdatePlayerVectors();
}

/**
 @brief Constructor with vectors
 @param position A const glm::vec3 variable which contains the position of the camera
 @param up A const glm::vec3 variable which contains the up direction of the camera
 @param yaw A const float variable which contains the yaw of the camera
 @param pitch A const float variable which contains the pitch of the camera
 */
CPlayer3D::CPlayer3D(	const glm::vec3 vec3Position,
						const glm::vec3 vec3Front,
						const float fYaw,
						const float fPitch)
	: vec3Up(glm::vec3(0.0f, 1.0f, 0.0f))
	, vec3Right(glm::vec3(1.0f, 1.0f, 0.0f))
	, vec3WorldUp(glm::vec3(0.0f, 1.0f, 0.0f))
	, fYaw(fYaw)
	, fPitch(fPitch)
	, fMouseSensitivity(0.1f)
	, cCamera(NULL)
	, cTerrain(NULL)
	, cPrimaryWeapon(NULL)
	, cSecondaryWeapon(NULL)
	, iCurrentWeapon(0)
	, slideTimer(1.f)
	, velocity(0.f)
	, minSpeed(0.f)
	, maxSpeed(.1f)
	, addSprintSpeed(0.f)
	, addSprintVelocity(0.f)
	, addCrouchSpeed(0.f)
	, addCrouchVelocity(0.f) 
	, addSlideSpeed(0.f)
	, addSlideVelocity(0.f)
	, addCounterSlideSpeed(-10.f)
	, totalVelocity(0.f)
{
	mesh = NULL;

	this->vec3Position = vec3Position;
	this->vec3Front = vec3Front;
	this->fYaw = fYaw;
	this->fPitch = fPitch;

	// Update the player vectors
	UpdatePlayerVectors();
}

/**
 @brief Destructor
 */
CPlayer3D::~CPlayer3D(void)
{
	if (cSecondaryWeapon)
	{
		delete cSecondaryWeapon;
		cSecondaryWeapon = NULL;
	}

	if (cPrimaryWeapon)
	{
		delete cPrimaryWeapon;
		cPrimaryWeapon = NULL;
	}

	if (cTerrain)
	{
		// We set it to NULL only since it was declared somewhere else
		cTerrain = NULL;
	}
	
	if (cCamera)
	{
		// We set it to NULL only since it was declared somewhere else
		cCamera = NULL;
	}
}

/**
 @brief Initialise this class instance
 @return true is successfully initialised this class instance, else false
 */
bool CPlayer3D::Init(void)
{
	// Call the parent's Init()
	CSolidObject::Init();

	// Set the type
	SetType(CEntity3D::TYPE::PLAYER);

	// Generate and bind the VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//CS: Create the Quad Mesh using the mesh builder
	mesh = CMeshBuilder::GenerateBox(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
		1.0f, 1.0f, 1.0f);

	// load and create a texture 
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene3D_Player.tga", false);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene3D_Player.tga" << endl;
		return false;
	}

	// Store the handler to the CTerrain
	cTerrain = CTerrain::GetInstance();
	// Update the y-axis position of the player
	vec3Position.y = cTerrain->GetHeight(vec3Position.x, vec3Position.z);

	// Set the Physics to fall status by default
	cPhysics3D.SetStatus(CPhysics3D::STATUS::FALL);

	fMovementSpeed = 10.f;
	return true;
}

/**
 @brief Set model
 @param model A const glm::mat4 variable containing the model for this class instance
 */
void CPlayer3D::SetModel(const glm::mat4 model)
{
	this->model = model;
}

/**
 @brief Set view
 @param view A const glm::mat4 variable containing the model for this class instance
 */
void CPlayer3D::SetView(const glm::mat4 view)
{
	this->view = view;
}

/**
 @brief Set projection
 @param projection A const glm::mat4 variable containing the model for this class instance
 */
void CPlayer3D::SetProjection(const glm::mat4 projection)
{
	this->projection = projection;
}

/**
@brief Returns the view matrix calculated using Euler Angles and the LookAt Matrix
@return A glm::mat4 variable which contains the view matrix
*/
glm::mat4 CPlayer3D::GetViewMatrix(void) const
{
	return glm::lookAt(vec3Position, vec3Position + vec3Front, vec3Up);
}

/**
 @brief Attach a camera to this class instance
 @param cCamera A CCamera* variable which contains the camera
 */
void CPlayer3D::AttachCamera(CCamera* cCamera)
{
	// Set the camera to the player
	this->cCamera = cCamera;

	// Update the camera's attributes with the player's attributes
	if (cCamera)
	{
		cCamera->vec3Position = vec3Position;
		cCamera->vec3Front = vec3Front;
		cCamera->vec3Up = vec3Up;
		cCamera->vec3Right = vec3Right;
		cCamera->fYaw = fYaw;
		cCamera->fPitch = fPitch;
	}
}

/**
 @brief Check if a camera ia attached to this class instance
 @return true if a camera is attached, else false
 */
bool CPlayer3D::IsCameraAttached(void)
{
	if (cCamera)
		return true;
	return false;
}

/**
 @brief Set Weapon to this class instance
 @param iSlot A const int variable which contains the weapon info to check for. 0 == Primary, 1 == Secondary
 @param cWeaponInfo A CWeaponInfo* variable which contains the weapon info
 */
void CPlayer3D::SetWeapon(const int iSlot, CWeaponInfo* cWeaponInfo)
{
	if (iSlot == 0)
		cPrimaryWeapon = cWeaponInfo;
	else if (iSlot == 1)
		cSecondaryWeapon = cWeaponInfo;
}

 /**
 @brief Get Weapon of this class instance
 @param iSlot A const int variable which contains the weapon info to check for. 0 == Primary, 1 == Secondary
 */
CWeaponInfo* CPlayer3D::GetWeapon(void) const
{
	if (iCurrentWeapon == 0)
		return cPrimaryWeapon;
	else if (iCurrentWeapon == 1)
		return cSecondaryWeapon;

	return NULL;
}

/**
 @brief Set current weapon
 @param iSlot A const int variable which contains the weapon info to check for. 0 == Primary, 1 == Secondary
 */
void CPlayer3D::SetCurrentWeapon(const int iSlot)
{
	iCurrentWeapon = iSlot;
}

/**
 @brief Discharge weapon
 @param iSlot A const int variable which contains the weapon info to check for. 0 == Primary, 1 == Secondary
 @return A bool variable
 */
bool CPlayer3D::DischargeWeapon(void)
{
	if ((iCurrentWeapon == 0) && (cPrimaryWeapon))
	{
		bool fired = cPrimaryWeapon->Discharge(vec3Position, vec3Front, (CSolidObject*)this);

		if (fired) //if first bullet is fired, apply recoil
			ApplyRecoil(cPrimaryWeapon->GetMinRecoil(), cPrimaryWeapon->GetMaxRecoil());

		return fired;
	}
	else if ((iCurrentWeapon == 1) && (cSecondaryWeapon))
	{
		bool fired = cSecondaryWeapon->Discharge(vec3Position, vec3Front, (CSolidObject*)this);
		
		if (fired)
			ApplyRecoil(cSecondaryWeapon->GetMinRecoil(), cSecondaryWeapon->GetMaxRecoil());

		return fired;
	}
	return NULL;
}

/**
 @brief Set to Jump
 */
void CPlayer3D::SetToJump(void)
{
	if (cPhysics3D.GetStatus() == CPhysics3D::STATUS::IDLE)
	{
		cPhysics3D.SetStatus(CPhysics3D::STATUS::JUMP);
		cPhysics3D.SetInitialVelocity(glm::vec3(0.0f, 1.2f, 0.0f));
	}
}

float CPlayer3D::GetTotalVelocity()
{
	return totalVelocity;
}

CPhysics3D CPlayer3D::GetPhysics()
{
	return cPhysics3D;
}

/**
 @brief Processes input received from any keyboard-like input system as player movements. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
 @param direction A const Player_Movement variable which contains the movement direction of the camera
 @param deltaTime A const float variable which contains the delta time for the realtime loop
 */
void CPlayer3D::ProcessMovement(const PLAYERMOVEMENT direction, const float deltaTime)
{
	switch (activeState)
	{
	case PLAYER_STATE::WALK:
		addSprintSpeed = -50.f;
		ResetMovementValues(PLAYER_STATE::CROUCH);
		break;
	case PLAYER_STATE::SPRINT:
		addSprintSpeed = 50.f;
		ResetMovementValues(PLAYER_STATE::CROUCH);
		break;
	case PLAYER_STATE::CROUCH:
		//if crouch vel to the max, then stop sliding
		if (addCrouchVelocity >= 0.1f && enableSliding)
		{
			enableSliding = false;
		}
		else
		{
			//if total vel allows sliding and 
			if (totalVelocity >= 0.2f && enableSliding)
				addCrouchSpeed = 50.f;
			else
			{
				addSprintSpeed = -10.f;
				addCrouchSpeed = -10.f;
			}
		}
		break;
	}

	float accel = fMovementSpeed * deltaTime;
	velocity += accel * deltaTime;
	velocity = Math::Clamp(velocity, -maxSpeed, maxSpeed);

	float addedSprintAccel = addSprintSpeed * deltaTime;
	addSprintVelocity += addedSprintAccel * deltaTime;
	addSprintVelocity = Math::Clamp(addSprintVelocity, 0.f, .1f);

	float addedCrouchAccel = addCrouchSpeed * deltaTime;
	addCrouchVelocity += addedCrouchAccel * deltaTime;
	addCrouchVelocity = Math::Clamp(addCrouchVelocity, -.05f, .1f);

	totalVelocity = velocity + addSprintVelocity + addCrouchVelocity;
	std::cout << "FINAL VEL: " << totalVelocity << std::endl;

	if (direction == PLAYERMOVEMENT::FORWARD)
		vec3Position += vec3Front * totalVelocity;
	if (direction == PLAYERMOVEMENT::BACKWARD)
		vec3Position -= vec3Front * totalVelocity;
	if (direction == PLAYERMOVEMENT::LEFT)
		vec3Position -= vec3Right * totalVelocity;
	if (direction == PLAYERMOVEMENT::RIGHT)
		vec3Position += vec3Right * totalVelocity;
}

/**
 @brief Processes input received from a mouse input system as player rotation. Expects the offset value in both the x and y direction.
 @param xoffset A const float variable which contains the x axis of the mouse movement
 @param yoffset A const float variable which contains the y axis of the mouse movement
 @param constrainPitch A const bool variable which indicates if the pitch will be constrained at upright positions
 */
void CPlayer3D::ProcessRotate(float fXOffset, float fYOffset, const bool constrainPitch)
{
	fXOffset *= fMouseSensitivity;
	fYOffset *= fMouseSensitivity;

	fYaw += fXOffset;
	fPitch += fYOffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (fPitch > 89.0f)
			fPitch = 89.0f;
		if (fPitch < -89.0f)
			fPitch = -89.0f;
	}

	// Update vec3Front, Right and vec3Up Vectors using the updated Euler angles
	UpdatePlayerVectors();
}

/**
 @brief Update the elapsed time
 @param dt A const double variable containing the elapsed time since the last frame
 @return A bool variable
 */
bool CPlayer3D::Update(const double dElapsedTime)
{
	if (cPrimaryWeapon)
		cPrimaryWeapon->Update(dElapsedTime);
	if (cSecondaryWeapon)
		cSecondaryWeapon->Update(dElapsedTime);

	// Update the Jump/Fall
	UpdateJumpFall(dElapsedTime);

	// If the camera is attached to this player, then update the camera
	if (cCamera)
	{
		cCamera->vec3Position = vec3Position;
		cCamera->vec3Front = vec3Front;
		cCamera->vec3Up = vec3Up;
		cCamera->vec3Right = vec3Right;
		cCamera->fYaw = fYaw;
		cCamera->fPitch = fPitch;
	}

	// Constraint the player's position
	Constraint();

	CSolidObject::Update(dElapsedTime);

	switch (activeState)
	{
	case PLAYER_STATE::REST:
		velocity = 0.f;
		break;
	case PLAYER_STATE::CROUCH:
		vec3Position.y -= 0.1f;
		break;
	default:
		break;
	}

	//std::cout << "Current Weapon: " << GetWeapon()->GetName() << std::endl;
	return true;
}

/**
 @brief PreRender Set up the OpenGL display environment before rendering
 */
void CPlayer3D::PreRender(void)
{
	CSolidObject::PreRender();
}

/**
 @brief Render Render this instance
 */
void CPlayer3D::Render(void)
{
	// Don't render the player if the camera is attached to it
	if (cCamera)
		return;

	CSolidObject::Render();
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CPlayer3D::PostRender(void)
{
	CSolidObject::PostRender();
}

/**
 @brief Calculates the front vector from the Camera's (updated) Euler Angles
 */
void CPlayer3D::UpdatePlayerVectors(void)
{
	// Calculate the new vec3Front vector
	glm::vec3 front;
	front.x = cos(glm::radians(fYaw)) * cos(glm::radians(fPitch));
	front.y = sin(glm::radians(fPitch));
	front.z = sin(glm::radians(fYaw)) * cos(glm::radians(fPitch));
	vec3Front = glm::normalize(front);
	// Also re-calculate the Right and Up vector
	// Normalize the vectors, because their length gets closer to 0 the more 
	// you look up or down which results in slower movement.
	vec3Right = glm::normalize(glm::cross(vec3Front, vec3WorldUp));  
	vec3Up = glm::normalize(glm::cross(vec3Right, vec3Front));

	// If the camera is attached to this player, then update the camera
	if (cCamera)
	{
		cCamera->vec3Front = vec3Front;
		cCamera->vec3Up = vec3Up;
		cCamera->vec3Right = vec3Right;
		cCamera->fYaw = fYaw;
		cCamera->fPitch = fPitch;
	}
}

/**
 @brief Constraint the player's position
 */
void CPlayer3D::Constraint(void)
{
	// If the player is not jumping nor falling, then we snap his position to the terrain
	if (cPhysics3D.GetStatus() == CPhysics3D::STATUS::IDLE)
	{
		vec3Position.y = cTerrain->GetHeight(vec3Position.x, vec3Position.z) + fHeightOffset;
	}
	else
	{
		// If the player is jumping nor falling, then we only snap his position 
		// if the player's y-coordinate is below the cTerrain 
		float fCheckHeight = cTerrain->GetHeight(vec3Position.x, vec3Position.z) + fHeightOffset;

		// If the player is below the terrain, then snap to the terrain height
		if (fCheckHeight > vec3Position.y)
			vec3Position.y = fCheckHeight;
	}
}

/**
 @brief Update Jump or Fall
 @param deltaTime A const float variable which contains the delta time for the realtime loop
 */
void CPlayer3D::UpdateJumpFall(const double dElapsedTime)
{
	if (cPhysics3D.GetStatus() == CPhysics3D::STATUS::JUMP)
	{
		// Update the elapsed time to the physics engine
		cPhysics3D.AddElapsedTime((float)dElapsedTime);
		// Call the physics engine update method to calculate the final velocity and displacement
		cPhysics3D.Update(dElapsedTime);
		// Get the displacement from the physics engine and update the player position
		vec3Position = vec3Position + cPhysics3D.GetDisplacement();

		// If the player is still jumping and the initial velocity has reached zero or below zero, 
		// then it has reach the peak of its jump
		if ((cPhysics3D.GetStatus() == CPhysics3D::STATUS::JUMP) && (cPhysics3D.GetDeltaDisplacement().y <= 0.0f))
		{
			// Set status to fall
			cPhysics3D.SetStatus(CPhysics3D::STATUS::FALL);
		}
	}
	else if (cPhysics3D.GetStatus() == CPhysics3D::STATUS::FALL)
	{
		// Update the elapsed time to the physics engine
		cPhysics3D.AddElapsedTime((float)dElapsedTime);
		// Call the physics engine update method to calculate the final velocity and displacement
		cPhysics3D.Update(dElapsedTime);
		// Get the displacement from the physics engine and update the player position
		vec3Position = vec3Position + cPhysics3D.GetDisplacement();

		// Constaint the player's position to the terrain
		float fCheckHeight = cTerrain->GetHeight(vec3Position.x, vec3Position.z) + fHeightOffset;

		// Set the Physics to idle status
		if (fCheckHeight > vec3Position.y)
		{
			vec3Position.y = fCheckHeight;
			cPhysics3D.SetStatus(CPhysics3D::STATUS::IDLE);
		}
	}
}

/**
 @brief Apply Weapon Recoils
 @param minRecoil A vec2 variable for minRecoil
 @param maxRecoil A vec2 variable for maxRecoil
 */
void CPlayer3D::ApplyRecoil(glm::vec2 minRecoil, glm::vec2 maxRecoil)
{
	//Update the yaw and pitch based on the recoil
	fYaw += Math::RandFloatMinMax(minRecoil.x, maxRecoil.x);
	fPitch += Math::RandFloatMinMax(minRecoil.y, maxRecoil.y);
	UpdatePlayerVectors();
}

/**
 @brief Reset Movement Values
 @param state A PLAYER_STATE enum for which state to reset movement values
 */
void CPlayer3D::ResetMovementValues(PLAYER_STATE state)
{
	switch (state)
	{
	case PLAYER_STATE::SPRINT:
		addSprintSpeed = 0;
		addSprintVelocity = 0;
		break;
	case PLAYER_STATE::CROUCH:
		addCrouchSpeed = 0;
		addCrouchVelocity = 0;
		enableSliding = true;
		break;
	}
}

/**
 @brief Print Self
 */
void CPlayer3D::PrintSelf(void)
{
	cout << "CPlayer3D::PrintSelf()" << endl;
	cout << "========================" << endl;
}
