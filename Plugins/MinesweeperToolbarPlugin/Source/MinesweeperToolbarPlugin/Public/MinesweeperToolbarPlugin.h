// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Input/SSlider.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FMinesweeperToolbarPluginModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();
	
private:

	void RegisterMenus();

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

	int MaxWidthX = 6;
	int MaxHeightY = 6;
	int AmountOfMines = 6;
	float MinePercentage = 1.0;
	bool bLost = false;

	TSharedPtr<SEditableTextBox> AmountOfMinesInputBox;
	TSharedPtr<SSlider> AmountOfMinesPercentageSlider; 
	TSharedPtr<SVerticalBox> MineFieldBox = SNew(SVerticalBox);
	TArray<TSharedPtr<SButton>> MineField = TArray<TSharedPtr<SButton>>();


	TSharedPtr<SHorizontalBox> ConstructTopPanel();
	TSharedPtr<SHorizontalBox> ConstructBottomPanel();

	void ExpandNeighbourhood(int PosX, int PosY);
	void MakeYellowCounterButton(TSharedPtr<SButton> RefButton, int SurroundingMines);
	void RevealAllMines();
	int CountNeighbouringMines(int PosX, int PosY);

	// Auxialiary BoolFunctions
	bool HasMineOnField(int PosX, int PosY);
	bool OutOfBoundsX(int PosX, int PosY);
	bool OutOfBoundsY(int PosX, int PosY);

	// Delegate for OnClicked
	FReply BuildMineField();
	FReply ClickOnMineField(int PosX, int PosY);

	// Delegate for OnTextChange
	void ChangeWidth(const FText& InputText);
	void ChangeHeight(const FText& InputText);
	void ChangeMineAmount(const FText& InputText);

	// Delegate for OnValueChange
	void ChangeMineAmountPercentage(float InputValue);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};
