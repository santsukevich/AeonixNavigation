﻿// Copyright 2024 Chris Ashworth



#include <AeonixEditor/Private/AenoixEditorDebugSubsystem.h>
#include <AeonixEditor/Private/AeonixPathDebugActor.h>

#include <AeonixNavigation/Public/Actor/AeonixBoundingVolume.h>
#include <AeonixNavigation/Public/Component/AeonixNavAgentComponent.h>
#include <AeonixEditor/AeonixEditor.h>

#include "Subsystem/AeonixSubsystem.h"

#include "EngineUtils.h"

void UAenoixEditorDebugSubsystem::UpdateDebugActor(AAeonixPathDebugActor* DebugActor)
{
	if (!DebugActor)
	{
		return;
	}
	
	if (DebugActor->DebugType == EAeonixPathDebugActorType::START)
	{
		StartDebugActor = DebugActor;
	}
	else
	{
		EndDebugActor = DebugActor;
	}
	
	 UAeonixSubsystem* AeonixSubsystem = DebugActor->GetWorld()->GetSubsystem<UAeonixSubsystem>();
	
	// If we've got a valid start and end target
	if (StartDebugActor && EndDebugActor && !bIsPathPending)
	{
		if (AAeonixBoundingVolume* Volume = AeonixSubsystem->GetMutableVolumeForAgent(StartDebugActor->NavAgentComponent))
		{
			Volume->UpdateBounds();
			FAeonixPathFindRequestCompleteDelegate& PathRequestCompleteDelegate = AeonixSubsystem->FindPathAsyncAgent(StartDebugActor->NavAgentComponent, EndDebugActor->GetActorLocation(), CurrentDebugPath);
			PathRequestCompleteDelegate.BindDynamic(this, &UAenoixEditorDebugSubsystem::OnPathFindComplete);
			bIsPathPending = true;
		}

	}
}

void UAenoixEditorDebugSubsystem::OnPathFindComplete(EAeonixPathFindStatus Status)
{
	if (Status == EAeonixPathFindStatus::Complete)
	{
		CurrentDebugPath.SetIsReady(true);
		bIsPathPending = false;
	}
	else if (Status == EAeonixPathFindStatus::Failed)
	{
		CurrentDebugPath.SetIsReady(false);
		bIsPathPending = false;
	}
	else
	{
		UE_LOG(AeonixEditor, Error, TEXT("Unhandled state"));
	}
}

void UAenoixEditorDebugSubsystem::Tick(float DeltaTime)
{
	if (!StartDebugActor)
	{
		return;
	}
	
	UAeonixSubsystem* AeonixSubsystem = StartDebugActor->GetWorld()->GetSubsystem<UAeonixSubsystem>();
	
	// If we've got a valid start and end target
	if (StartDebugActor && EndDebugActor && !bIsPathPending && !CurrentDebugPath.IsReady())
	{
		// this is just needed to deal with the lifetime of things in the editor world
		if (AAeonixBoundingVolume* Volume = AeonixSubsystem->GetMutableVolumeForAgent(StartDebugActor->NavAgentComponent))
		{
			Volume->UpdateBounds();
		}
		FAeonixPathFindRequestCompleteDelegate& PathRequestCompleteDelegate = AeonixSubsystem->FindPathAsyncAgent(StartDebugActor->NavAgentComponent, EndDebugActor->GetActorLocation(), CurrentDebugPath);
		PathRequestCompleteDelegate.BindDynamic(this, &UAenoixEditorDebugSubsystem::OnPathFindComplete);
		bIsPathPending = true;
	}
	
	if (StartDebugActor)
	{
		FlushPersistentDebugLines(StartDebugActor->GetWorld());
	}
	
	if (CurrentDebugPath.IsReady() && StartDebugActor)
	{
		CurrentDebugPath.DebugDraw(StartDebugActor->GetWorld(), AeonixSubsystem->GetVolumeForAgent(StartDebugActor->NavAgentComponent)->GetNavData());
	}
}

TStatId UAenoixEditorDebugSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAenoixEditorDebugSubsystem, STATGROUP_Tickables);
}

bool UAenoixEditorDebugSubsystem::IsTickable() const 
{
	return true;
}

bool UAenoixEditorDebugSubsystem::IsTickableInEditor() const 
{
	return true;
}

bool UAenoixEditorDebugSubsystem::IsTickableWhenPaused() const
{
	return true;
}
