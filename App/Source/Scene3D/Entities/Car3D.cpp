/**
 CCar3D
 By: Jordan Low
 Date: Nov 2021
 */
#include "Car3D.h"

 // Include ShaderManager
#include "RenderControl/ShaderManager.h"

 // Include LoadOBJ
#include "System/LoadOBJ.h"

// Include ImageLoader
#include "System/ImageLoader.h"

//For allowing creating of Mesh 
#include "Primitives/MeshBuilder.h"

#include "Inputs/MouseController.h"

#include "../../MyMath.h"

#include <iostream>
using namespace std;

/**
 @brief Default Constructor
 */
CCar3D::CCar3D(void)
	: vec3Up(glm::vec3(0.0f, 1.0f, 0.0f))
	, vec3Right(glm::vec3(1.0f, 1.0f, 0.0f))
	, vec3WorldUp(glm::vec3(0.0f, 1.0f, 0.0f))
	, fYaw(-90.0f)
	, fPitch(0.0f)
	, cCamera(NULL)
	, cTerrain(NULL)
	, cPlayer3D(NULL)
	, velocity(0)
	, maxSpeed(4)
	, accel(0)
	, currSpeed(0)
	, torque(0)
	, torqueSpeed(80)
	, tiltAngle(0)
	, thirdPersonView(true)
{
	// Set the default position so it is above the ground
	vec3Position = glm::vec3(0.0f, 0.5f, 0.0f);
}

/**
 @brief Constructor with vectors
 @param position A const glm::vec3 variable which contains the position of the camera
 @param up A const glm::vec3 variable which contains the up direction of the camera
 @param yaw A const float variable which contains the yaw of the camera
 @param pitch A const float variable which contains the pitch of the camera
 */
CCar3D::CCar3D(	const glm::vec3 vec3Position,
						const glm::vec3 vec3Front,
						const float fYaw,
						const float fPitch)
	: vec3Up(glm::vec3(0.0f, 1.0f, 0.0f))
	, vec3Right(glm::vec3(1.0f, 1.0f, 0.0f))
	, vec3WorldUp(glm::vec3(0.0f, 1.0f, 0.0f))
	, fYaw(fYaw)
	, fPitch(fPitch)
	, cCamera(NULL)
	, cTerrain(NULL)
	, cPlayer3D(NULL)
	, velocity(0)
	, maxSpeed(4)
	, accel(0)
	, currSpeed(0)
	, torque(0)
	, torqueSpeed(80)
	, tiltAngle(0)
	, thirdPersonView(true)
{
	mesh = NULL;

	this->vec3Position = vec3Position;
	this->vec3Front = vec3Front;
	this->fYaw = fYaw;
	this->fPitch = fPitch;
}

/**
 @brief Destructor
 */
CCar3D::~CCar3D(void)
{
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
bool CCar3D::Init(void)
{
	// Call the parent's Init()
	CSolidObject::Init();

	// Set the type
	SetType(CEntity3D::TYPE::CAR);

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<ModelVertex> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;

	std::string file_path = "Models/tank.obj";
	bool success = CLoadOBJ::LoadOBJ(file_path.c_str(), vertices, uvs, normals, true);
	if (!success)
	{
		cout << "Unable to load Models/racer.obj" << endl;
		return false;
	}

	CLoadOBJ::IndexVBO(vertices, uvs, normals, index_buffer_data, vertex_buffer_data);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &IBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(ModelVertex), &vertex_buffer_data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);
	iIndicesSize = index_buffer_data.size();
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)(sizeof(glm::vec3) + sizeof(glm::vec3)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

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
	cPhysics3D.SetStatus(CPhysics3D::STATUS::IDLE);
	cPhysics3D.SetGravity(glm::vec3(0.f));

	fMovementSpeed = 3000.f;
	return true;
}

/**
 @brief Set model
 @param model A const glm::mat4 variable containing the model for this class instance
 */
void CCar3D::SetModel(const glm::mat4 model)
{
	this->model = model;
}

/**
 @brief Set view
 @param view A const glm::mat4 variable containing the model for this class instance
 */
void CCar3D::SetView(const glm::mat4 view)
{
	this->view = view;
}

/**
 @brief Set projection
 @param projection A const glm::mat4 variable containing the model for this class instance
 */
void CCar3D::SetProjection(const glm::mat4 projection)
{
	this->projection = projection;
}

/**
@brief Returns the view matrix calculated using Euler Angles and the LookAt Matrix
@return A glm::mat4 variable which contains the view matrix
*/
glm::mat4 CCar3D::GetViewMatrix(void) const
{
	return glm::lookAt(vec3Position, vec3Position + vec3Front, vec3Up);
}

CPhysics3D CCar3D::GetPhysics()
{
	return cPhysics3D;
}

void CCar3D::ProcessMovement(double dElapsedTime)
{
	//reset the accel to 0
	accel = 0;

	//add friction to currSpeed
	float friction = currSpeed * -1.f;
	currSpeed += friction * dElapsedTime;

	//reset the torque to 0 if made 1 full round
	//if (torque >= 360 || torque <= -360)
	//	torque = 0;

	//find the rotation result
	glm::mat4 rotationMatrix = glm::mat4(1.0f);
	rotationMatrix = glm::rotate(rotationMatrix, glm::radians(torque), glm::vec3(0, 1, 0));
	glm::vec4 rotationResult = rotationMatrix * glm::vec4(0, 0, -1, 0);

	//calc the new front, right and up vector
	vec3Front = glm::vec3(rotationResult.x, rotationResult.y, rotationResult.z);
	vec3Right = glm::normalize(glm::cross(vec3Front, vec3WorldUp));
	vec3Up = glm::normalize(glm::cross(vec3Right, vec3Front));

	////clamp the yaw and pitch
	if (cPlayer3D)
	{
		cPlayer3D->fYaw = Math::Clamp(cPlayer3D->fYaw, -115.f, -65.f);
		cPlayer3D->fPitch = Math::Clamp(cPlayer3D->fPitch, -20.f, 0.f);

		cPlayer3D->UpdatePlayerVectors();

		//set yaw and pitch to car's torque rotation
		fYaw = -torque + cPlayer3D->fYaw;
		fPitch = cPlayer3D->fPitch;

		//process for player inputs
		ProcessCarInputs(dElapsedTime);
	}


	currSpeed += accel * dElapsedTime;
	velocity = vec3Front * currSpeed * (float)dElapsedTime;

	//max cap the velocity
	if (glm::length(velocity) > 0.5f)
		velocity = glm::normalize(velocity) * 0.5f;

	//predict the next pos
	glm::vec3 predictedPos = vec3Position;
	predictedPos += velocity;

	//Find the tilt angle for the car
	float fCheckHeight = cTerrain->GetHeight(predictedPos.x, predictedPos.z) + fHeightOffset - vec3Position.y;
	float xzAxis = glm::length(glm::vec2(vec3Position.x, vec3Position.z) - glm::vec2(predictedPos.x, predictedPos.z));
	tiltAngle = glm::degrees(atan2f(fCheckHeight, xzAxis));

	//set the current pos to the predicted pos
	vec3Position = predictedPos;
}

CPlayer3D* CCar3D::GetPlayer3D()
{
	return cPlayer3D;
}

void CCar3D::SetWeapon(CWeaponInfo* cWeaponInfo)
{
	cWeapon = cWeaponInfo;
}

float CCar3D::GetCurrSpeed()
{
	return currSpeed;
}

CWeaponInfo* CCar3D::GetWeapon(void) const
{
	return cWeapon;
}

/**
 @brief Update the elapsed time
 @param dt A const double variable containing the elapsed time since the last frame
 @return A bool variable
 */
bool CCar3D::Update(const double dElapsedTime)
{
	if (!cPlayer3D)
	{
		ProcessMovement(dElapsedTime);
		if (glm::distance(vec3Position, CPlayer3D::GetInstance()->GetPosition()) < 5)
		{
			if (CKeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_E))
			{
				cPlayer3D = CPlayer3D::GetInstance();
				cCamera = CCamera::GetInstance();
				cPlayer3D->isDriving = true;
				cPlayer3D->SetVehicleWeapon(this->cWeapon);
			}
		}
	}
	else
	{
		cout << "SPEED: " << currSpeed << std::endl;
		if (!bStatus) {
			cPlayer3D->SetPosition(vec3Position + vec3Right * 2.f);
			cPlayer3D->isDriving = false;
			cPlayer3D->SetVehicleWeapon(nullptr);
			cPlayer3D = NULL;
			cCamera = NULL;
			return true;
		}
		cPlayer3D->SetPosition(vec3Position);
		StorePositionForRollback();
		ProcessMovement(dElapsedTime);

		vec3Position += vec3Vel * (float)dElapsedTime;

		float frictionX = vec3Vel.x * -1.f;
		vec3Vel.x += frictionX * (float)dElapsedTime;

		float frictionY = vec3Vel.y * -1.f;
		vec3Vel.y += frictionY * (float)dElapsedTime;

		float frictionZ = vec3Vel.z * -1.f;
		vec3Vel.z += frictionZ * (float)dElapsedTime;

		if (!cPlayer3D)
			return false;

		if (cWeapon != nullptr)
		{
			if (cWeapon->GetAutoFire())
			{
				if (CMouseController::GetInstance()->IsButtonDown(CMouseController::BUTTON_TYPE::LMB))
				{
					bool fired = cWeapon->Discharge(vec3Position + glm::vec3(0, vec3Scale.y * 0.75f, 0), vec3Front, (CSolidObject*)this);

					//if (fired) //if first bullet is fired, apply recoil
					//	cPlayer3D->ApplyRecoil(cWeapon);
				}
			}
			else
			{
				if (CMouseController::GetInstance()->IsButtonPressed(CMouseController::BUTTON_TYPE::LMB))
				{
					bool fired = cWeapon->Discharge(vec3Position + glm::vec3(0, vec3Scale.y * 0.75f, 0), vec3Front, (CSolidObject*)this);

					//if (fired) //if first bullet is fired, apply recoil
					//	cPlayer3D->ApplyRecoil(cWeapon);
				}
			}
			if (CKeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_R))
			{
				cWeapon->Reload();
			}
		}
	}

	// If the camera is attached to this player, then update the camera
	if (cCamera)
	{
		glm::vec3 camOffset = vec3Up * 2.f + vec3Front * -5.f;
		if (!thirdPersonView)
			camOffset = vec3Up * 0.1f;
		cCamera->vec3Position = vec3Position + camOffset;
		cCamera->vec3Front = vec3Front;
		cCamera->vec3Up = vec3Up;
		cCamera->vec3Right = vec3Right;
		cCamera->fYaw = fYaw;
		cCamera->fPitch = fPitch;
	}

	// Constraint the player's position
	Constraint();

	CSolidObject::Update(dElapsedTime);

	return true;
}

/**
 @brief PreRender Set up the OpenGL display environment before rendering
 */
void CCar3D::PreRender(void)
{
	CSolidObject::PreRender();
}

/**
 @brief Render Render this instance
 */
void CCar3D::Render(void)
{
	if (!bStatus)
		return;
	model = glm::rotate(model, glm::radians(torque / 2), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(tiltAngle), glm::vec3(1, 0, 0));
	CSolidObject::Render();
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CCar3D::PostRender(void)
{
	CSolidObject::PostRender();
}

/**
 @brief Constraint the player's position
 */
void CCar3D::Constraint(void)
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


void CCar3D::ProcessCarInputs(double dElapsedTime)
{
	//exit car
	if (CKeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_E))
	{
		cPlayer3D->SetPosition(vec3Position + vec3Right * 2.f);
		cPlayer3D->isDriving = false;
		cPlayer3D->SetVehicleWeapon(nullptr);
		cPlayer3D = NULL;
		cCamera = NULL;
		return;
	}
	//change cam view
	if (CKeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_TAB))
	{
		thirdPersonView = !thirdPersonView;
	}
	if (CKeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_A))
	{
		float dir = 1;
		if (currSpeed < 0)
			dir = -1;

		if (currSpeed > 0.5f || currSpeed < -0.5f)
			torque += torqueSpeed * dir * (float)dElapsedTime;
	}
	if (CKeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_S))
	{
		accel = -fMovementSpeed * dElapsedTime;
	}
	if (CKeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_D))
	{
		float dir = 1;
		if (currSpeed < 0)
			dir = -1;

		if (currSpeed > 0.5f || currSpeed < -0.5f)
			torque -= torqueSpeed * dir * (float)dElapsedTime;
	}
	if (CKeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_W))
	{
		accel = fMovementSpeed * dElapsedTime;
	}
}

/**
 @brief Print Self
 */
void CCar3D::PrintSelf(void)
{
	cout << "CPlayer3D::PrintSelf()" << endl;
	cout << "========================" << endl;
}
