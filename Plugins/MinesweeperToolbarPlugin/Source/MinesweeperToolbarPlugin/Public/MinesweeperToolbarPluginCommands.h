// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "MinesweeperToolbarPluginStyle.h"

class FMinesweeperToolbarPluginCommands : public TCommands<FMinesweeperToolbarPluginCommands>
{
public:

	FMinesweeperToolbarPluginCommands()
		: TCommands<FMinesweeperToolbarPluginCommands>(TEXT("MinesweeperToolbarPlugin"), NSLOCTEXT("Contexts", "MinesweeperToolbarPlugin", "MinesweeperToolbarPlugin Plugin"), NAME_None, FMinesweeperToolbarPluginStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};