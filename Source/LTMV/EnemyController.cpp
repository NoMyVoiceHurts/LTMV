// Fill out your copyright notice in the Description page of Project Settings.

#include "LTMV.h"
#include "EnemyController.h"

#include "EnemyCharacter.h"


AEnemyController::AEnemyController(const FObjectInitializer& OI) : Super(OI) {
    _SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));

    SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception Component")));
    GetAIPerceptionComponent()->bAutoActivate = true;
    GetAIPerceptionComponent()->ConfigureSense(*_SightConfig);
    GetAIPerceptionComponent()->SetDominantSense(_SightConfig->GetSenseImplementation());

    GetAIPerceptionComponent()->OnPerceptionUpdated.AddDynamic(this, &AEnemyController::PerceptionUpdated);
    GetAIPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyController::TargetPerceptionUpdated);
}

void AEnemyController::Possess(APawn* InPawn) {
    AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(InPawn);
    if (EnemyCharacter) {
        Super::Possess(InPawn);

        ApplySenses(EnemyCharacter->_SightRange,
                    EnemyCharacter->_HearingRange,
                    EnemyCharacter->_VisionAngleDegrees);
    }
}

void AEnemyController::ApplySenses(float SightRange, float HearingRange, float VisionAngleDegrees) {
    /* Sight */
    _SightConfig->SightRadius = SightRange;
    _SightConfig->LoseSightRadius = SightRange + 20.0f;
    _SightConfig->PeripheralVisionAngleDegrees = VisionAngleDegrees;
    _SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    _SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    _SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    GetAIPerceptionComponent()->ConfigureSense(*_SightConfig);
    GetAIPerceptionComponent()->SetDominantSense(_SightConfig->GetSenseImplementation());
}

void AEnemyController::PerceptionUpdated(TArray<AActor*> Actors) {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "PerceptionUpdated");
}

void AEnemyController::TargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus) {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "OnTargetPerceptionUpdated");
}