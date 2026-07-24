#pragma once

#include "Stats/Stats.h"

DECLARE_STATS_GROUP(TEXT("Aura"), STATGROUP_Aura, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("AuraCharacterTick"), STAT_AuraCharacterTick, STATGROUP_Aura);
DECLARE_CYCLE_STAT(TEXT("AutoRun"),           STAT_AutoRun,           STATGROUP_Aura);

// Phase 2 - GAS core
DECLARE_CYCLE_STAT(TEXT("ExecCalcDamage"),   STAT_ExecCalcDamage,   STATGROUP_Aura);
DECLARE_CYCLE_STAT(TEXT("OnNativeInput"),    STAT_OnNativeInput,    STATGROUP_Aura);
DECLARE_CYCLE_STAT(TEXT("HoldToMove"),       STAT_HoldToMove,       STATGROUP_Aura);
DECLARE_CYCLE_STAT(TEXT("SetupNavPoints"),   STAT_SetupNavPoints,   STATGROUP_Aura);
