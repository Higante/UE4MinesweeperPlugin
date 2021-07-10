// Copyright Epic Games, Inc. All Rights Reserved.

#include "MinesweeperToolbarPluginCommands.h"

#define LOCTEXT_NAMESPACE "FMinesweeperToolbarPluginModule"

void FMinesweeperToolbarPluginCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "MinesweeperToolbarPlugin", "Bring up MinesweeperToolbarPlugin window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
