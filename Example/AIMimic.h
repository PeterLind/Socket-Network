#include "IController.h"
#include "BehaviorTree.h"
#include "Math/Vector.h"
#include "EventMessage.h"

namespace FOC
{
	class Collider;
	class Navmesh;
	class InputController;
	class AnimationPlayerComponent;
	struct EnemyData;

	enum class BehaviourValues : char
	{
		Idle = 1,
		LineOfSight = 2,
		ComeWithinRange = 3,
		Shoot = 4,
		Seperation = 5,
		SideWalk = 6,
		Death = 7,
	};

	class AIMimic : public IController
	{
	public:
		AIMimic();
		~AIMimic();
		void SceneInit(Scene& aScene) override;
		void Init() override;
		void Update(const float aDeltaTime) override;
		void Link() override;

		void AddAIs(GameObject* anAI);
		void UpdateParam(char* t);
	private:
		GameObject* FindClosestTarget();
		GameObject* CalculateClosestPlayer();
		void RotateAIs();
		void HandleNavmesh();
		void InitBehaviourTree();

		BehaviorTree myBehaviourTree;
		Navmesh* myNavmesh;
		GameObject* myClosestPlayer;
		GameObject* myCurrentTarget;
		std::vector<GameObject*> myOtherAIs;
		AnimationPlayerComponent* myAnimPlayer;
		float myRangeFromPlayer;
		float myRangeFromTargets;

		BehaviourValues myActiveBehaviour;

		EnemyData* myEnemyData;

		char* myMessage;

		int myPlayerID;

	};
}