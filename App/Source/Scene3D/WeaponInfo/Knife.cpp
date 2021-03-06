/**
 CKnife
 By: Jordan Low
 Date: Oct 2021
 */
#include "Knife.h"

 // Include LoadOBJ
#include "System/LoadOBJ.h"

 // Include ImageLoader
#include "System\ImageLoader.h"

#include <iostream>
using namespace std;

/**
@brief Default Constructor
*/
CKnife::CKnife(void)
{
}

/**
@brief Default Destructor
*/
CKnife::~CKnife(void)
{
}

/**
 @brief Initialise this instance to default values
 */
bool CKnife::Init(void)
{
	// Call the parent's Init method
	CWeaponInfo::Init();

	// The name of the weapon
	name = "Knife";
	// The number of ammunition in a magazine for this weapon
	iMagRounds = 0;
	// The maximum number of ammunition for this magazine for this weapon
	iMaxMagRounds = 0;
	// The current total number of rounds currently carried by this player
	iTotalRounds = 0;
	// The max total number of rounds currently carried by this player
	iMaxTotalRounds = 0;
	// The number of bullets per click
	iBulletsPerClick = 1;
	iDamage = 5;

	// The time between shots
	dTimeBetweenShots = 0.02;
	// The time to reload
	dMaxReloadTime = 2.0f;
	// The time to equip
	dMaxEquipTime = 0.3f;
	// The elapsed time (between shots)
	dElapsedTime = 0.0;
	// Boolean flag to indicate if weapon can fire now
	bFire = true;
	// Boolean flag to indicate if weapon is full auto
	bAuto = false;
	// Bullet spread
	bulletSpread = 0.015f;
	// Recoil vectors
	minRecoil = glm::vec2(-0.1f, 0.1f);
	maxRecoil = glm::vec2(0.1f, 0.15f);

	crossHairType = CCrossHair::CROSSHAIR_DOT;

	// Call the parent's Init()
	CEntity3D::Init();

	// Set the type
	SetType(CEntity3D::TYPE::OTHERS);

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<ModelVertex> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;

	std::string file_path = "Models/Weapons/sword.obj";
	bool success = CLoadOBJ::LoadOBJ(file_path.c_str(), vertices, uvs, normals, true);
	if (!success)
	{
		cout << "Unable to load Models/Weapons/gun_type64_01.obj" << endl;
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
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Models/M4A1-s.tga", false);
	if (iTextureID == 0)
	{
		cout << "Unable to load Models/Weapons/map_gunType64_01_AO.png" << endl;
		return false;
	}

	return true;
}
