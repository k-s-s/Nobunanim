// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SafeCCDIKEditMode.h"

#include "AnimGraphNode_SafeCCDIK.h"
#include "IPersonaPreviewScene.h"
#include "Animation/DebugSkelMeshComponent.h"


void FSafeCCDIKEditMode::EnterMode(UAnimGraphNode_Base* InEditorNode, FAnimNode_Base* InRuntimeNode)
{
	RuntimeNode = static_cast<FAnimNode_SafeCCDIK*>(InRuntimeNode);
	GraphNode = CastChecked<UAnimGraphNode_SafeCCDIK>(InEditorNode);

	FNobunanimBaseEditMode::EnterMode(InEditorNode, InRuntimeNode);
}

void FSafeCCDIKEditMode::ExitMode()
{
	RuntimeNode = nullptr;
	GraphNode = nullptr;

	FNobunanimBaseEditMode::ExitMode();
}

FVector FSafeCCDIKEditMode::GetWidgetLocation() const
{
	EBoneControlSpace Space = RuntimeNode->EffectorLocationSpace;
	FVector Location = RuntimeNode->EffectorLocation;
	FBoneSocketTarget Target = RuntimeNode->EffectorTarget;

	USkeletalMeshComponent* SkelComp = GetAnimPreviewScene().GetPreviewMeshComponent();
	return ConvertWidgetLocation(SkelComp, RuntimeNode->ForwardedPose, Target, Location, Space);
}

UE::Widget::EWidgetMode FSafeCCDIKEditMode::GetWidgetMode() const
{
	USkeletalMeshComponent* SkelComp = GetAnimPreviewScene().GetPreviewMeshComponent();
	int32 TipBoneIndex = SkelComp->GetBoneIndex(RuntimeNode->TipBone.BoneName);
	int32 RootBoneIndex = SkelComp->GetBoneIndex(RuntimeNode->RootBone.BoneName);

	if (TipBoneIndex != INDEX_NONE && RootBoneIndex != INDEX_NONE)
	{
		return UE::Widget::WM_Translate;
	}

	return UE::Widget::WM_None;
}

void FSafeCCDIKEditMode::DoTranslation(FVector& InTranslation)
{
	USkeletalMeshComponent* SkelComp = GetAnimPreviewScene().GetPreviewMeshComponent();

	FVector Offset = ConvertCSVectorToBoneSpace(SkelComp, InTranslation, RuntimeNode->ForwardedPose, RuntimeNode->EffectorTarget, GraphNode->Node.EffectorLocationSpace);

	RuntimeNode->EffectorLocation += Offset;
	GraphNode->Node.EffectorLocation = RuntimeNode->EffectorLocation;
	GraphNode->SetDefaultValue(GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_SafeCCDIK, EffectorLocation), RuntimeNode->EffectorLocation);
}

void FSafeCCDIKEditMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	// draw line from root bone to tip bone if available
	if (RuntimeNode && RuntimeNode->DebugLines.Num() > 0)
	{
		USkeletalMeshComponent* SkelComp = GetAnimPreviewScene().GetPreviewMeshComponent();
		FTransform CompToWorld = SkelComp->GetComponentToWorld();

		PDI->DrawPoint(CompToWorld.TransformPosition(RuntimeNode->DebugLines[0]), FLinearColor::Red, 5, SDPG_Foreground);

		// no component space
		for (int32 Index = 1; Index < RuntimeNode->DebugLines.Num(); ++Index)
		{
			FVector Start = CompToWorld.TransformPosition(RuntimeNode->DebugLines[Index - 1]);
			FVector End = CompToWorld.TransformPosition(RuntimeNode->DebugLines[Index]);

			PDI->DrawLine(Start, End, FLinearColor::Red, SDPG_Foreground);
			PDI->DrawPoint(End, FLinearColor::White, 5, SDPG_Foreground);
		}

	}
#endif // #if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
}