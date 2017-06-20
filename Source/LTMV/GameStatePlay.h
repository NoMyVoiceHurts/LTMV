// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameState.h"
#include "GameStatePlay.generated.h"


UCLASS()
class LTMV_API AGameStatePlay : public AGameState {
    GENERATED_BODY()

public:
    bool _StatueWakeUp;
    TArray<TSharedPtr<const FUniqueNetId>> _UniqueNetIdArray;

    AGameStatePlay(const class FObjectInitializer& OI);

	FVector _puerta1_0;
	FVector _puerta1_1;
	FVector _puerta2_0;
	FVector _puerta2_1;
	FVector _puerta3_0;
	FVector _puerta3_1;
	FVector _puerta4_0;
	FVector _puerta4_1;
	FVector _point1_2;
	FVector _point2_3;
	FVector _point3_4;
	FVector _point4_1;

	FVector _point1;
	FVector _point2;

	TArray<bool> _doorStates;
	TArray<bool> _actualPath;
	TArray<bool> _beforePath;
	TArray<bool> _PatrolPoints;

	void updateDoors();

	/** Default crouched eye height */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		AActor* ActualZone;

	UFUNCTION(BlueprintCallable, Category = "EnemyCharacter", meta = (DisplayName = "Update AI zone"))
		void UpdateActualZone(AActor* zone);

private:
	void updateDoor(TActorIterator<AStaticMeshActor> _door, int value);
	void UpdatePatrolPoints(FVector pp1,FVector pp2, FVector pp3);
	TArray<bool> FindPath();

	ATargetPoint *_patrolPoint1;
	ATargetPoint *_patrolPoint2;
	ATargetPoint *_patrolPoint3;
	ACharacter *_enemy;
	int _actualZone;
};
