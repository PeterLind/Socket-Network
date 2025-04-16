#include "game_pch.h"
#include "NetworkController.h"
#include <Engine.h>
#include "NetworkEventDispatcher.h"
#include "GameObject.h"
#include "MovementComponent.h"
#include "PlayerData.h"
#include "DataHandler.h"
#include "Scene.h"
#include <GameObjectFactory.h>
#include "PostMaster.h"

namespace FOC
{
	NetworkController::NetworkController() : myPlayerData(&Engine::GetInstance()->GetDataHandler()->Get<PlayerData>())
	{
	}
	NetworkController::~NetworkController()
	{
	}

	void NetworkController::Init()
	{
		FOC::Engine::GetInstance()->GetNetworkApplication()->GetEventDispatcher()->Subscribe<char*>(FOC::NETWORK::PackageType::ePLAYER_ACTION, [&](char* t) { UpdateParamaters(t); });

		myStatComponent = myGameObject->GetComponent<StatComponent>();
		myMovementComponent = myGameObject->GetComponent<MovementComponent>();
		myWeaponComponent = myGameObject->GetComponent<WeaponComponent>();

		myStatComponent->myStat = &myPlayerData->myStat;
		myWeaponComponent->myWeaponData = &myPlayerData->myWeapon;
		myMovementComponent->SetKineticParameters(&myPlayerData->myParams);

	}

	void NetworkController::SceneInit(Scene& aScene)
	{
		aScene;
		myCollectedPosition = myGameObject->myTransform.myPosition;
	}

	void NetworkController::Update(const float aDeltaTime)
	{
		myGameObject->GetComponent<MovementComponent>()->SetPosition(myCollectedPosition);
		myGameObject->myTransform.myRotation.y = myRotationY - FMath::Pi;
		if (myWeaponCode == (int)WeaponReturnCode::Fireing)
		{
			//myBulletDirection.x -= FMath::Pi;
			myWeaponComponent->Fire(myGameObject->LocalPosition(), myBulletDirection);
			FOC::NETWORK::MessagePackage package(FOC::NETWORK::PackageType::eACK);
			FOC::Engine::GetInstance()->GetNetworkApplication()->SendTo(package);
			//std::cout << "Other player Shot" << std::endl;
		}
		aDeltaTime;
	}
	void NetworkController::UpdateParamaters(char* aPackageMessage)
	{
		FOC::NETWORK::DataBuilder builder;
		builder
			.Read<Vector3f>(&myCollectedPosition, aPackageMessage)
			.Read<float>(&myRotationY, aPackageMessage)
			.Read<int>(&myWeaponCode, aPackageMessage)
			.Read<Vector3f>(&myBulletDirection, aPackageMessage);
	}
}