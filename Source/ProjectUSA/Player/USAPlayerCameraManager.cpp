// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/USAPlayerCameraManager.h"

#include "Camera/USACameraShakeBase.h"

UCameraShakeBase* AUSAPlayerCameraManager::StartCameraShake
(TSubclassOf<UCameraShakeBase> ShakeClass, 
	float Scale, 
	ECameraShakePlaySpace PlaySpace, 
	FRotator UserPlaySpaceRot)
{
// To determine the layer
	UUSACameraShakeBase* USACameraShakeBase 
		= Cast <UUSACameraShakeBase>(ShakeClass->GetDefaultObject());

	int8 CameraShakeBaseLayerIndex = -1;

	if (USACameraShakeBase != nullptr)
	{
// Get Layer
		CameraShakeBaseLayerIndex 
			= USACameraShakeBase->GetCameraShakeBaseLayerIndex();

// If it is a valid layer
		if (CameraShakeBaseLayerIndex >= 0)
		{
// If a key corresponding to the corresponding layer exists in the map
			if (CurrentCameraShakeBaseMap.Contains(CameraShakeBaseLayerIndex))
			{
// When an existing CameraShake exists that corresponds to the layer of the requested CameraShake
				if (CurrentCameraShakeBaseMap[CameraShakeBaseLayerIndex] != nullptr)
				{
// Stop existing CameraShake
					StopCameraShake(CurrentCameraShakeBaseMap[CameraShakeBaseLayerIndex], true);
				}
			}
// If there is no key corresponding to the requested CameraShake Layer
			else
			{
				CurrentCameraShakeBaseMap.Add({ CameraShakeBaseLayerIndex , nullptr });
			}
		}
	}

// Run CameraShake
	UCameraShakeBase* InstCameraShakeBase 
		= Super::StartCameraShake(ShakeClass, Scale, PlaySpace, UserPlaySpaceRot);

// Update currently running CameraShake
	if (CameraShakeBaseLayerIndex >= 0)
	{
		CurrentCameraShakeBaseMap[CameraShakeBaseLayerIndex] = InstCameraShakeBase;
	}

	return InstCameraShakeBase;
}
