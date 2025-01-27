// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once


/**
 * when you modify this, please note that this information can be saved with instances
 * also DefaultEngine.ini [/Script/Engine.CollisionProfile] should match with this list
 **/

#define Pan_ObjectChannel_Weapon         ECC_GameTraceChannel6
#define Pan_ObjectChannel_Projectile		ECC_GameTraceChannel7
#define Pan_TraceChannel_AimAssist       ECC_GameTraceChannel1
#define Pan_TraceChannel_Interaction		ECC_GameTraceChannel8
		 
#define Pan_PhysicalMaterial_Default		SurfaceType_Default
#define Pan_PhysicalMaterial_Character	SurfaceType1
#define Pan_PhysicalMaterial_Rock		SurfaceType2
#define Pan_PhysicalMaterial_Wood		SurfaceType3
