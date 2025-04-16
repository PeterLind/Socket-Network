#include "game_pch.h"
#include "AIMimic.h"
#include "Engine.h"
#include "SelectorNode.h"
#include "DeathBehaviour.h"
#include "Decorator.h"
#include "SequenceNode.h"
#include "RandomSelectorNode.h"

#include "IdleBehaviour.h"
#include "ShootBehaviour.h"
#include "WalkSideWaysBehaviour.h"
#include "FindLineOfSightBehaviour.h"
#include "ComeWithinRangeBehaviour.h"
#include "SeperationBehaviour.h"

#include "StatComponent.h"
#include "WeaponComponent.h"
#include "MovementComponent.h"
#include "AnimationPlayerComponent.h"

#include "GameObject.h"
#include "Physics.h"
#include "Random.h"
#include "Navmesh.h"
#include "Scene.h"
#include "Animation.h"
#include "SceneManager.h"
#include "DataHandler.h"
#include "EnemyData.h"
#include "NetworkEventDispatcher.h"
#include "PostMaster.h"

namespace FOC
{
	AIMimic::AIMimic() : myEnemyData(&Engine::GetInstance()->GetDataHandler()->Get<EnemyData>())
	{
	}
	AIMimic::~AIMimic()
	{
		ZeroMemory(myMessage, NETMESSAGE_SIZE);
	}
	void AIMimic::SceneInit(Scene& aScene)
	{
		aScene;
		myMessage = new char[NETMESSAGE_SIZE];
		FOC::Engine::GetInstance()->GetNetworkApplication()->GetEventDispatcher()->Subscribe<char*>(FOC::NETWORK::PackageType::eENEMY_ACTION, [&](char* t) { UpdateParam(t); });
	}
	void AIMimic::Init()
	{
		myGameObject->GetScene().GetGameObjectPoolManager().MultiRegister(std::to_string(myGameObject->GetID()), myEnemyData->myWeapon.maxAmmo, FOC::GameObjectFactory::CreateEnemyProjectile);

		myStatComponent = myGameObject->GetComponent<StatComponent>();
		myMovementComponent = myGameObject->GetComponent<MovementComponent>();
		myWeaponComponent = myGameObject->GetComponent<WeaponComponent>();
		myNavmesh = &myGameObject->GetScene().GetNavmesh();

		myStatComponent->myStat = &myEnemyData->myStat;
		myWeaponComponent->myWeaponData = &myEnemyData->myWeapon;
		myMovementComponent->SetKineticParameters(&myEnemyData->myParams);
	}

	void AIMimic::Update(const float aDeltaTime)
	{
		auto isDead = myBehaviourTree.GetBlackboard().GetValue<bool>("IsDead");
		if(isDead)
			return;

		myClosestPlayer = CalculateClosestPlayer();
		RotateAIs();
		HandleNavmesh();
		myAnimPlayer->Update(aDeltaTime);
		myCurrentTarget = FindClosestTarget();
		myBehaviourTree.GetBlackboard().SetValue<GameObject *>("Target", myCurrentTarget);
		myBehaviourTree.GetBlackboard().SetValue<GameObject *>("Player", myClosestPlayer);
		if(myClosestPlayer)
		{
			myRangeFromPlayer = std::abs((myClosestPlayer->LocalPosition() - myGameObject->LocalPosition()).Length());
		}
	}
	void AIMimic::Link()
	{
		myAnimPlayer = myGameObject->GetComponent<AnimationPlayerComponent>();

		myAnimPlayer->Init();
		myAnimPlayer->AddAnimation("Assets/Animation/Enemy/SK_enemy_Idle.bakanim", "Idle");
		myAnimPlayer->AddAnimation("Assets/Animation/Enemy/SK_enemy_attack.bakanim", "Attack");
		myAnimPlayer->AddAnimation("Assets/Animation/Enemy/SK_enemy_Walk.bakanim", "Walk");
		myAnimPlayer->AddAnimation("Assets/Animation/Enemy/SK_enemy_Death.bakanim", "Death");
		myAnimPlayer->AddAnimation("Assets/Animation/Enemy/SK_enemy_reload.bakanim", "Reload");
		myAnimPlayer->PlayAnimation("Idle", true);


		// SUPER DUPER TEMP!!!!! ------------------------------------------------------------------------------------------------------------
		for (int i = 0; i < myOtherAIs.size(); i++)
		{
			myCurrentTarget = myClosestPlayer;
		}
		// ----------------------------------------------------------------------------------------------------------------------------------

		InitBehaviourTree();
	}

	GameObject* AIMimic::FindClosestTarget()
	{
		int closestIndex = -1;
		float closestDistance = std::numeric_limits<float>::max(); // Initialize with a large value

		for (int i = 0; i < myOtherAIs.size(); i++)
		{
			float currentDistance = (myOtherAIs[i]->LocalPosition() - myGameObject->LocalPosition()).Length();

			if (currentDistance < closestDistance)
			{
				closestIndex = i;
				closestDistance = currentDistance;
			}
		}

		if (closestIndex != -1 && closestDistance < myEnemyData->minRangeFromTarget)
		{
			return myOtherAIs[closestIndex];
		}
		else
		{
			return myClosestPlayer;
		}
	}

	GameObject* AIMimic::CalculateClosestPlayer()
	{
		GameObject* choosenPlayer;
		GameObject* player = Engine::GetInstance()->GetSceneManager()->GetCurrentScene()->GetPlayer();
		GameObject* network = Engine::GetInstance()->GetSceneManager()->GetCurrentScene()->GetNetworkPlayer();
		if (!player || !network) return nullptr;

		float currentPlayerDistance = (player->LocalPosition() - myGameObject->LocalPosition()).Length();
		float currentNetworkDistance = (network->LocalPosition() - myGameObject->LocalPosition()).Length();

		if (currentPlayerDistance < currentNetworkDistance)
		{
			choosenPlayer = player;
		}
		else
		{
			choosenPlayer = network;
		}

		return choosenPlayer;
	}

	void AIMimic::AddAIs(GameObject* anAI)
	{
		myOtherAIs.push_back(anAI);
	}

	void AIMimic::UpdateParam(char* t)
	{
		std::memcpy(myMessage, t, NETMESSAGE_SIZE);
		FOC::NETWORK::DataBuilder networkMessageBuilder; // This helps structure a message that you want to send
		int aId;
		networkMessageBuilder.Read<BehaviourValues>(&myActiveBehaviour, t).Read<int>(&aId, t);
		myBehaviourTree.GetBlackboard().SetValue<char*>("NetworkData", myMessage);
		if (myGameObject->GetID() == aId)
		{
			myBehaviourTree.Update(Engine::GetInstance()->GetDeltaTime(), &networkMessageBuilder);
		}
	}

	void AIMimic::RotateAIs()
	{
		if (myClosestPlayer)
		{

			FOC::Vector3f direction = (myClosestPlayer->LocalPosition() - myGameObject->LocalPosition()).GetNormalized();
			float angle = -atan2(direction.z, direction.x);
			myGameObject->myTransform.myRotation.y = angle - 90 * FOC::FMath::DegToRad;
		}
	}

	void AIMimic::HandleNavmesh()
	{
		if (myNavmesh)
		{

			if (!myNavmesh->IsInsideNavmesh(myGameObject->myTransform.myPosition))
			{
				Vector2f range = { 50,50 };
				Optional<Vector3f> pos = myNavmesh->FindClosestPointInNavmesh(myGameObject->LocalPosition(), range);
				if (pos)
				{
					myGameObject->LocalPosition() = pos.myValue;
				}
			}
		}
	}

	void AIMimic::InitBehaviourTree()
	{
		myBehaviourTree.GetBlackboard().SetValue<StatComponent*>("StatComponent", myStatComponent);
		myBehaviourTree.GetBlackboard().SetValue<WeaponComponent*>("WeaponComponent", myWeaponComponent);
		myBehaviourTree.GetBlackboard().SetValue<MovementComponent*>("MovementComponent", myMovementComponent);
		myBehaviourTree.GetBlackboard().SetValue<GameObject*>("GameObject", myGameObject);
		myBehaviourTree.GetBlackboard().SetValue<AnimationPlayerComponent*>("Anim", myAnimPlayer);
		myBehaviourTree.GetBlackboard().SetValue<GameObject*>("Target", myCurrentTarget);
		myBehaviourTree.GetBlackboard().SetValue<EnemyData*>("EnemyData", myEnemyData);
		myBehaviourTree.GetBlackboard().SetValue<bool>("Mimic", true);
		myBehaviourTree.GetBlackboard().SetValue<bool>("IsDead", false);


		SelectorNode* selector = myBehaviourTree.SetRoot<SelectorNode>();

		auto idle = selector->AddNode<IdleBehaviour>(myBehaviourTree.GetBlackboard());
		idle->AddDecorator<DecoratorStateEqual<BehaviourValues>>(myBehaviourTree.GetBlackboard(), myActiveBehaviour, BehaviourValues::Idle);

		auto find = selector->AddNode<FindLineOfSightBehaviour>(myBehaviourTree.GetBlackboard());
		find->AddDecorator<DecoratorStateEqual<BehaviourValues>>(myBehaviourTree.GetBlackboard(), myActiveBehaviour, BehaviourValues::LineOfSight);

		auto come = selector->AddNode<ComeWithinRangeBehaviour>(myBehaviourTree.GetBlackboard());
		come->AddDecorator<DecoratorStateEqual<BehaviourValues>>(myBehaviourTree.GetBlackboard(), myActiveBehaviour, BehaviourValues::ComeWithinRange);

		auto shoot = selector->AddNode<ShootBehaviour>(myBehaviourTree.GetBlackboard());
		shoot->AddDecorator<DecoratorStateEqual<BehaviourValues>>(myBehaviourTree.GetBlackboard(), myActiveBehaviour, BehaviourValues::Shoot);

		auto seperate = selector->AddNode<SeperationBehaviour>(myBehaviourTree.GetBlackboard());
		seperate->AddDecorator<DecoratorStateEqual<BehaviourValues>>(myBehaviourTree.GetBlackboard(), myActiveBehaviour, BehaviourValues::Seperation);

		auto sideWalk = selector->AddNode<WalkSideWaysBehaviour>(myBehaviourTree.GetBlackboard());
		sideWalk->AddDecorator<DecoratorStateEqual<BehaviourValues>>(myBehaviourTree.GetBlackboard(), myActiveBehaviour, BehaviourValues::SideWalk);

		auto death = selector->AddNode<DeathBehaviour>(myBehaviourTree.GetBlackboard());
		death->AddDecorator<DecoratorStateEqual<BehaviourValues>>(myBehaviourTree.GetBlackboard(), myActiveBehaviour, BehaviourValues::Death);
	}
}