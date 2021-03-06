/**
 CSolidObjectManager
 By: Toh Da Jun
 Date: Apr 2020
 */

#include "SolidObjectManager.h"

// Include CCollisionManager
#include "Primitives/CollisionManager.h"

#include "../Entities/Player3D.h"
#include "../Entities/Car3D.h"

#include "../CameraEffects/CameraEffectsManager.h"
#include "../CameraEffects/HitMarker.h"

// Include CCameraEffectsManager
//#include "../CameraEffects/CameraEffectsManager.h"

#include <iostream>
using namespace std;

/**
 @brief Default Constructor
 */
CSolidObjectManager::CSolidObjectManager(void)
	: model(glm::mat4(1.0f))
	, view(glm::mat4(1.0f))
	, projection(glm::mat4(1.0f))
	, cProjectileManager(NULL)
{
}

/**
 @brief Destructor
 */
CSolidObjectManager::~CSolidObjectManager(void)
{
	// We won't delete this since it was created elsewhere
	cProjectileManager = NULL;

	// Remove all CSolidObject
	std::list<CSolidObject*>::iterator it = lSolidObject.begin(), end = lSolidObject.end();
	while (it != end)
	{
		// Delete if done
		delete *it;
		it = lSolidObject.erase(it);
	}
}

/**
 @brief Initialise this class instance
 @return A bool variable
 */
bool CSolidObjectManager::Init(void)
{
	lSolidObject.clear();

	cProjectileManager = CProjectileManager::GetInstance();

	return true;
}

/**
 @brief Set model
 @param model A const glm::mat4 variable containing the model for this class instance
 */
void CSolidObjectManager::SetModel(const glm::mat4 model)
{
	this->model = model;
}

/**
 @brief Set view
 @param view A const glm::mat4 variable containing the model for this class instance
 */
void CSolidObjectManager::SetView(const glm::mat4 view)
{
	this->view = view;
}

/**
 @brief Set projection
 @param projection A const glm::mat4 variable containing the model for this class instance
 */
void CSolidObjectManager::SetProjection(const glm::mat4 projection)
{
	this->projection = projection;
}

/**
 @brief Add a CSolidObject* to this class instance
 @param cSolidObject The CSolidObject* variable to be added to this class instance
 */
void CSolidObjectManager::Add(CSolidObject* cSolidObject)
{
	lSolidObject.push_back(cSolidObject);
}

/**
 @brief Remove a CSolidObject* from this class instance
 @param cSolidObject The CSolidObject* variable to be erased from this class instance
 @return A bool variable
 */
bool CSolidObjectManager::Erase(CSolidObject* cSolidObject)
{
	// Find the entity's iterator
	std::list<CSolidObject*>::iterator findIter = std::find(lSolidObject.begin(), lSolidObject.end(), cSolidObject);

	// Delete the entity if found
	if (findIter != lSolidObject.end())
	{
		// Delete the CSolidObject
		//delete *findIter;
		// Go to the next iteration after erasing from the list
findIter = lSolidObject.erase(findIter);
return true;
	}
	// Return false if not found
	return false;
}

/**
 @brief Collision Check for a CSolidObject*
 @param cSolidObject The CSolidObject* variable to be checked
 @return A bool variable
 */
bool CSolidObjectManager::CollisionCheck(CSolidObject* cSolidObject)
{
	std::list<CSolidObject*>::iterator it, end;
	end = lSolidObject.end();
	for (it = lSolidObject.begin(); it != end; ++it)
	{
		// Check for collisions between the 2 entities
		if (CCollisionManager::BoxBoxCollision(cSolidObject->GetPosition() + cSolidObject->boxMin,
			cSolidObject->GetPosition() + cSolidObject->boxMax,
			(*it)->GetPosition() + (*it)->boxMin,
			(*it)->GetPosition() + (*it)->boxMax) == true)
		{
			// Rollback the cSolidObject's position
			cSolidObject->RollbackPosition();
			// Rollback the NPC's position
			(*it)->RollbackPosition();
			if ((*it)->GetType() == CSolidObject::TYPE::NPC)
			cout << "** Collision between this Entity and a NPC ***" << endl;
			else if ((*it)->GetType() == CSolidObject::TYPE::OTHERS)
			cout << "** Collision between this Entity and an OTHERS ***" << endl;
			else if ((*it)->GetType() == CSolidObject::TYPE::STRUCTURE)
			cout << "** Collision between this Entity and a STRUCTURE ***" << endl;
			else if ((*it)->GetType() == CSolidObject::TYPE::PROJECTILE)
			cout << "** Collision between this Entity and a PROJECTILE ***" << endl;
			// Quit this loop since a collision has been found
			break;
		}
	}

	return true;
}

/**
 @brief Update the elapsed time
 @param dt A const double variable containing the elapsed time since the last frame
 @return A bool variable
 */
bool CSolidObjectManager::Update(const double dElapsedTime)
{
	enemyCount = 0;
	std::list<CSolidObject*>::iterator it, end;
	// Update all CSolidObject
	end = lSolidObject.end();
	for (it = lSolidObject.begin(); it != end; ++it)
	{
		if ((*it)->GetType() == CEntity3D::TYPE::NPC && (*it)->GetStatus())
			enemyCount++;
		(*it)->Update(dElapsedTime);
	}

	if (enemyCount == 0)
		allEnemyDied = true;

	std::cout << "COIUNT: " << enemyCount << std::endl;

	return true;
}

/**
 @brief Collision Check for all entities and projectiles
 @return A bool variable
 */
bool CSolidObjectManager::CheckForCollision(void)
{
	bool bResult = false;
	bool isWallRun = false;
	float hitAngle = 0;

	std::list<CSolidObject*>::iterator it, end;
	std::list<CSolidObject*>::iterator it_other;

	// Check for collisions among entities
	end = lSolidObject.end();
	for (it = lSolidObject.begin(); it != end; ++it)
	{
		// If the entity is not active, then skip it
		if ((*it)->GetStatus() == false)
			continue;

		for (it_other = lSolidObject.begin(); it_other != end; ++it_other)
		{
			// If the entity is not active, then skip it
			if ((*it_other)->GetStatus() == false)
				continue;

			// If the 2 entities to check are the same, then skip this iteration
			if (it_other == it)
				continue;

			// Check for collisions between the 2 entities
			if (CCollisionManager::BoxBoxCollision((*it)->GetPosition() + (*it)->boxMin,
				(*it)->GetPosition() + (*it)->boxMax,
				(*it_other)->GetPosition() + (*it_other)->boxMin,
				(*it_other)->GetPosition() + (*it_other)->boxMax) == true)
			{
				if ((*it)->GetType() == CSolidObject::TYPE::PLAYER &&
					(*it_other)->GetType() == CSolidObject::TYPE::CAR)
				{
					if (!CPlayer3D::GetInstance()->isDriving)
						(*it)->RollbackPosition();
					cout << "** Collision between Player and an Entity ***" << endl;
					break;
				}
				else if ((*it)->GetType() == CSolidObject::TYPE::PLAYER &&
					(*it_other)->GetType() == CSolidObject::TYPE::AMMO)
				{
					//increase player ammo
					CPlayer3D::GetInstance()->GetWeapon()->SetTotalRound(CPlayer3D::GetInstance()->GetWeapon()->GetTotalRound() + 30);
					(*it_other)->SetStatus(false);
					cout << "** Collision between Player and Ammo ***" << endl;
					break;
				}
				// Check if a movable entity collides with another movable entity
				else if (
					(((*it)->GetType() >= CSolidObject::TYPE::PLAYER) &&
						((*it)->GetType() <= CSolidObject::TYPE::CAR))
					&&
					(((*it_other)->GetType() >= CSolidObject::TYPE::NPC) &&
						((*it_other)->GetType() <= CSolidObject::TYPE::OTHERS))
					)
				{

					if ((*it)->GetType() == CSolidObject::TYPE::PLAYER)
					{
						(*it)->RollbackPosition();
						(*it_other)->RollbackPosition();
						if (CPlayer3D::GetInstance()->GetWeapon()->isMeleeAttacking)
						{
							//player melee attack
							(*it_other)->SetHealth((*it_other)->GetHealth() - CPlayer3D::GetInstance()->GetWeapon()->GetDamage());
							if ((*it_other)->GetHealth() <= 0)
								(*it_other)->SetStatus(false);
						}
					}
					else
					{
						//set enemy vel relative to the car's front
						CCar3D* c = (CCar3D*)(*it);
						(*it_other)->SetVel((*it_other)->GetVel() + (*it)->GetFront() * c->GetCurrSpeed());
						(*it_other)->SetHealth((*it_other)->GetHealth() - c->GetCurrSpeed());
						if ((*it_other)->GetHealth() <= 0)
							(*it_other)->SetStatus(false);
					}

					//calculate hitmarker angle
					glm::vec3 dist = (*it)->GetFront() - (*it_other)->GetFront();
					hitAngle = atan2f(dist.z, dist.x);
					bResult = true;
					cout << "** Collision between Player and an Entity ***" << endl;
					break;
				}
				else if (
						(((*it)->GetType() >= CSolidObject::TYPE::NPC) &&
						((*it)->GetType() <= CSolidObject::TYPE::OTHERS))
						&&
						(((*it_other)->GetType() >= CSolidObject::TYPE::NPC) &&
						((*it_other)->GetType() <= CSolidObject::TYPE::OTHERS))
					)
				{
					(*it)->RollbackPosition();
					(*it_other)->RollbackPosition();
					cout << "** Collision between 2 Entities ***" << endl;
					break;
				}
				// Check if a movable entity collides with a non-movable entity
				if (
						(((*it)->GetType() >= CSolidObject::TYPE::PLAYER) &&
							((*it)->GetType() <= CSolidObject::TYPE::OTHERS))
						&&
						((*it_other)->GetType() == CSolidObject::TYPE::STRUCTURE))
				{
					if (((*it)->GetType() == CSolidObject::TYPE::PLAYER))
					{
						(*it)->RollbackPosition();

					}
					else if ((*it)->GetType() == CSolidObject::TYPE::CAR)
					{
						CCar3D* c = (CCar3D*)(*it);
						(*it)->SetVel((*it)->GetVel() - (*it)->GetFront() * c->GetCurrSpeed());
					}

					/*if (bResult)
						CCameraEffectsManager::GetInstance()->Get("BloodScreen")->SetStatus(true);*/
					//float dotPdt = glm::dot((*it_other)->GetFront(), (*it)->GetFront()); //wall's front dot player's front
					//std::cout << "WALLFRONT: " << (*it_other)->GetFront().x << " " << (*it_other)->GetFront().y << " " << (*it_other)->GetFront().z <<
					//	" PLAYERFRONT: " << (*it)->GetFront().x << " " << (*it)->GetFront().y << " " << (*it)->GetFront().z << std::endl;
					//std::cout << "DOT: " << dotPdt << std::endl;
					//CPlayer3D* cPlayer3D = dynamic_cast<CPlayer3D*>(*it);
					//if ((dotPdt > 0.8f || dotPdt < -0.8f) && cPlayer3D->GetPhysics().GetStatus() != CPhysics3D::STATUS::IDLE) //check if player is looking perpendicular to the wall's front
					//{
					//	isWallRun = true; //tilt cam using this bool
					//	std::cout << "Wall running" << std::endl;
					//	cPlayer3D->GetPhysics().SetStatus(CPhysics3D::STATUS::WALL_RUN);
					//	(*it)->SetPosition((*it)->GetPosition() + glm::vec3((*it)->GetFront().x / 10, 0, (*it)->GetFront().z / 10)); //move the player towards the direction of the wall's front
					//}
					cout << "** Collision between Entity and Structure ***" << endl;
					break;
				}
			}
		}
	}

	// Check for collisions between entities and projectiles
	end = lSolidObject.end();
	for (it = lSolidObject.begin(); it != end; ++it)
	{
		// If the entity is not active, then skip it
		if ((*it)->GetStatus() == false)
			continue;

		for (unsigned int i = 0; i < cProjectileManager->vProjectile.size(); i++)
		{
			// If the entity is not active, then skip it
			if ((cProjectileManager->vProjectile[i])->GetStatus() == false)
				continue;

			// Use ray tracing to check for collisions between the 2 entities
			if (CCollisionManager::RayBoxCollision((*it)->GetPosition() + (*it)->boxMin,
				(*it)->GetPosition() + (*it)->boxMax,
				(cProjectileManager->vProjectile[i])->GetPreviousPosition(),
				(cProjectileManager->vProjectile[i])->GetPosition()))
			{
				if ((*it)->GetType() == CSolidObject::TYPE::PLAYER)
				{
					// If this projectile is fired by the player, then skip it
					if ((cProjectileManager->vProjectile[i])->GetSource() == (*it))
						continue;
					(cProjectileManager->vProjectile[i])->SetStatus(false);

					//Damage the player health using the projectile damage
					(*it)->SetHealth((*it)->GetHealth() - cProjectileManager->vProjectile[i]->GetDamage());
					if ((*it)->GetHealth() <= 0)
						(*it)->SetStatus(false);
					
					cout << "** RayBoxCollision between Player and Projectile ***" << endl;
					glm::vec3 dist = (*it)->GetFront() - (cProjectileManager->vProjectile[i])->GetSource()->GetFront();
					hitAngle = atan2f(dist.z, dist.x);
					bResult = true;
					break;
				}
				else if ((*it)->GetType() == CSolidObject::TYPE::NPC)
				{
					// If this projectile is fired by the NPC, then skip it
					if ((cProjectileManager->vProjectile[i])->GetSource() == (*it))
						continue;
					(cProjectileManager->vProjectile[i])->SetStatus(false);
					//Damage the player health using the projectile damage
					(*it)->SetHealth((*it)->GetHealth() - cProjectileManager->vProjectile[i]->GetDamage());
					if ((*it)->GetHealth() <= 0)
						(*it)->SetStatus(false);
					std::cout << "HEALTH: " << (*it)->GetHealth() << std::endl;

					cout << "** RayBoxCollision between NPC and Projectile ***" << endl;
					break;
				}
				else if ((*it)->GetType() == CSolidObject::TYPE::STRUCTURE || (*it)->GetType() == CSolidObject::TYPE::CAR)
				{
					(cProjectileManager->vProjectile[i])->SetStatus(false);

					//Damage the player health using the projectile damage
					(*it)->SetHealth((*it)->GetHealth() - cProjectileManager->vProjectile[i]->GetDamage());
					if ((*it)->GetHealth() <= 0)
						(*it)->SetStatus(false);

					cout << "** RayBoxCollision between Structure and Projectile ***" << endl;
					break;
				}
			}

			// Check for collisions between the 2 entities
			if (CCollisionManager::BoxBoxCollision((*it)->GetPosition() + (*it)->boxMin,
				(*it)->GetPosition() + (*it)->boxMax,
				(cProjectileManager->vProjectile[i])->GetPosition() + (cProjectileManager->vProjectile[i])->boxMin,
				(cProjectileManager->vProjectile[i])->GetPosition() + (cProjectileManager->vProjectile[i])->boxMax) == true)
			{
				if ((*it)->GetType() == CSolidObject::TYPE::PLAYER)
				{
					// If this projectile is fired by the player, then skip it
					if ((cProjectileManager->vProjectile[i])->GetSource() == (*it))
						continue;

					(cProjectileManager->vProjectile[i])->SetStatus(false);

					//Damage the player health using the projectile damage
					(*it)->SetHealth((*it)->GetHealth() - cProjectileManager->vProjectile[i]->GetDamage());
					if ((*it)->GetHealth() <= 0)
						(*it)->SetStatus(false);

					cout << "** BoxBoxCollision between Player and Projectile ***" << endl;
					glm::vec3 dist = (*it)->GetFront() - (cProjectileManager->vProjectile[i])->GetSource()->GetFront();
					hitAngle = atan2f(dist.z, dist.x);
					bResult = true;
					break;
				}
				else if ((*it)->GetType() == CSolidObject::TYPE::NPC)
				{
					(cProjectileManager->vProjectile[i])->SetStatus(false);

					//Damage the player health using the projectile damage
					(*it)->SetHealth((*it)->GetHealth() - cProjectileManager->vProjectile[i]->GetDamage());
					if ((*it)->GetHealth() <= 0)
						(*it)->SetStatus(false);

					cout << "** BoxBoxCollision between NPC and Projectile ***" << endl;
					break;
				}
				else if ((*it)->GetType() == CSolidObject::TYPE::STRUCTURE)
				{
					(cProjectileManager->vProjectile[i])->SetStatus(false);

					//Damage the player health using the projectile damage
					(*it)->SetHealth((*it)->GetHealth() - cProjectileManager->vProjectile[i]->GetDamage());
					if ((*it)->GetHealth() <= 0)
						(*it)->SetStatus(false);

					cout << "** BoxBoxCollision between Structure and Projectile ***" << endl;
					break;
				}
			}
		}
	}
	
	if (bResult == true)
	{
		CCameraEffectsManager::GetInstance()->Get("HitMarker")->SetStatus(true);
		((CHitMarker*)(CCameraEffectsManager::GetInstance()->Get("HitMarker")))->angle = glm::degrees(hitAngle);
	}

	return true;
}

/**
 @brief CleanUp all CEntity3Ds which are marked for deletion
 */
void CSolidObjectManager::CleanUp(void)
{
	std::list<CSolidObject*>::iterator it, end;
	it = lSolidObject.begin();
	end = lSolidObject.end();
	while (it != end)
	{
		if ((*it)->IsToDelete())
		{
			// Delete the CSolidObject
			delete *it;
			// Go to the next iteration after erasing from the list
			it = lSolidObject.erase(it);
		}
		else
		{
			// Go to the next iteration
			++it;
		}
	}
}

/**
 @brief Render this class instance
 */
void CSolidObjectManager::Render(void)
{
	// Render all entities
	std::list<CSolidObject*>::iterator it, end;
	end = lSolidObject.end();
	for (it = lSolidObject.begin(); it != end; ++it)
	{
		(*it)->SetView(view);
		(*it)->SetProjection(projection);
		(*it)->PreRender();
		(*it)->Render();
		(*it)->PostRender();
	}
}
