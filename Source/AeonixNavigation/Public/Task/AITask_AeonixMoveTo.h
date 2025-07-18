// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Subsystem/AeonixSubsystem.h"

#include <AeonixNavigation/Public/Data/AeonixDefines.h>

#include <Runtime/AIModule/Classes/Navigation/PathFollowingComponent.h>
#include <Runtime/AIModule/Classes/Tasks/AITask.h>
#include <Runtime/Core/Public/HAL/ThreadSafeBool.h>

#include "AITask_AeonixMoveTo.generated.h"

class AAIController;
class UAeonixNavAgentComponent;
struct FAeonixNavigationPath;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAeonixMoveTaskCompletedSignature, TEnumAsByte<EPathFollowingResult::Type>, Result, AAIController*, AIController);

UCLASS()
class AEONIXNAVIGATION_API UAITask_AeonixMoveTo : public UAITask
{
	GENERATED_BODY()

public:
	UAITask_AeonixMoveTo(const FObjectInitializer& ObjectInitializer);

	/** tries to start move request and handles retry timer */
	void ConditionalPerformMove();

	/** prepare move task for activation */
	void SetUp(AAIController* Controller, const FAIMoveRequest& InMoveRequest, bool aUseAsyncPathfinding);

	EPathFollowingResult::Type GetMoveResult() const { return MoveResult; }
	bool WasMoveSuccessful() const { return MoveResult == EPathFollowingResult::Success; }

	UFUNCTION(BlueprintCallable, Category = "AI|Tasks", meta = (AdvancedDisplay = "AcceptanceRadius,StopOnOverlap,AcceptPartialPath,bUsePathfinding,bUseContinuosGoalTracking", DefaultToSelf = "Controller", BlueprintInternalUseOnly = "TRUE", DisplayName = "Aeonix Move To Location or Actor"))
	static UAITask_AeonixMoveTo* AeonixAIMoveTo(AAIController* Controller, FVector GoalLocation, bool aUseAsyncPathfinding, AActor* GoalActor = nullptr,
		float AcceptanceRadius = -1.f, EAIOptionFlag::Type StopOnOverlap = EAIOptionFlag::Default, bool bLockAILogic = true, bool bUseContinuosGoalTracking = false);

	UE_DEPRECATED(4.12, "This function is now depreacted, please use version with FAIMoveRequest parameter")
	void SetUp(AAIController* Controller, FVector GoalLocation, AActor* GoalActor = nullptr, float AcceptanceRadius = -1.f, EAIOptionFlag::Type StopOnOverlap = EAIOptionFlag::Default);

	/** Allows custom move request tweaking. Note that all MoveRequest need to
	*	be performed before PerformMove is called. */
	FAIMoveRequest& GetMoveRequestRef() { return MoveRequest; }

	/** Switch task into continuous tracking mode: keep restarting move toward goal actor. Only pathfinding failure or external cancel will be able to stop this task. */
	void SetContinuousGoalTracking(bool bEnable);

	void TickTask(float DeltaTime) override;

	UFUNCTION()
	void OnPathFindComplete(EAeonixPathFindStatus Status);

protected:
	void LogPathHelper();
	
	bool bUseAsyncPathfinding;

	UPROPERTY(BlueprintAssignable)
	FGenericGameplayTaskDelegate OnRequestFailed;

	UPROPERTY(BlueprintAssignable)
	FAeonixMoveTaskCompletedSignature OnMoveFinished;

	/** parameters of move request */
	UPROPERTY()
	FAIMoveRequest MoveRequest;

	/** handle of path following's OnMoveFinished delegate */
	FDelegateHandle PathFinishDelegateHandle;

	/** handle of path's update event delegate */
	FDelegateHandle PathUpdateDelegateHandle;

	/** handle of active ConditionalPerformMove timer  */
	FTimerHandle MoveRetryTimerHandle;

	/** handle of active ConditionalUpdatePath timer */
	FTimerHandle PathRetryTimerHandle;

	/** request ID of path following's request */
	FAIRequestID MoveRequestID;

	/** currently followed path */
	FNavPathSharedPtr Path;

	FAeonixNavigationPath* AeonixPath;

	TScriptInterface<UAeonixSubsystem> AeonixSubsystem;

	TEnumAsByte<EPathFollowingResult::Type> MoveResult;
	bool bUseContinuousTracking{false};

	virtual void Activate() override;
	virtual void OnDestroy(bool bOwnerFinished) override;

	virtual void Pause() override;
	virtual void Resume() override;

	/** finish task */
	void FinishMoveTask(EPathFollowingResult::Type InResult);

	/** stores path and starts observing its events */
	void SetObservedPath(FNavPathSharedPtr InPath);

	//FPathFollowingRequestResult myResult;

	FAeonixPathfindingRequestResult Result;

	UPROPERTY()
	TObjectPtr<UAeonixNavAgentComponent> NavComponent;

	void CheckPathPreConditions();

	void RequestPathSynchronous();
	void RequestPathAsync();

	void RequestMove();

	void ResetPaths();

	/** remove all delegates */
	virtual void ResetObservers();

	/** remove all timers */
	virtual void ResetTimers();

	/** tries to update invalidated path and handles retry timer */
	void ConditionalUpdatePath();

	/** start move request */
	virtual void PerformMove();

	/** event from followed path */
	virtual void OnPathEvent(FNavigationPath* InPath, ENavPathEvent::Type Event);

	/** event from path following */
	virtual void OnRequestFinished(FAIRequestID RequestID, const FPathFollowingResult& Result);
};
