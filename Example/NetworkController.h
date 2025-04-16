#include "IController.h"
#include "Vector.h"

namespace FOC
{
	struct PlayerData;
	class NetworkController : public IController
	{
	public:
		NetworkController();
		~NetworkController() override;

		void Init() override;
		void SceneInit(Scene& aScene) override;
		void Update(const float aDeltaTime) override;

		void UpdateParamaters(char* t);
		bool myIsUpgraded;
	private:
		PlayerData* myPlayerData;
		FOC::Vector3f myCollectedPosition;
		float myRotationY;
		FOC::Vector3f myBulletDirection;
		int myWeaponCode;
	};
}