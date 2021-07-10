// Copyright Epic Games, Inc. All Rights Reserved.

#include "MinesweeperToolbarPlugin.h"
#include "MinesweeperToolbarPluginStyle.h"
#include "MinesweeperToolbarPluginCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

static const FName MinesweeperToolbarPluginTabName("MinesweeperToolbarPlugin");

#define LOCTEXT_NAMESPACE "FMinesweeperToolbarPluginModule"

void FMinesweeperToolbarPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FMinesweeperToolbarPluginStyle::Initialize();
	FMinesweeperToolbarPluginStyle::ReloadTextures();

	FMinesweeperToolbarPluginCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FMinesweeperToolbarPluginCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FMinesweeperToolbarPluginModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FMinesweeperToolbarPluginModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MinesweeperToolbarPluginTabName, FOnSpawnTab::CreateRaw(this, &FMinesweeperToolbarPluginModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FMinesweeperToolbarPluginTabTitle", "MinesweeperToolbarPlugin"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FMinesweeperToolbarPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FMinesweeperToolbarPluginStyle::Shutdown();

	FMinesweeperToolbarPluginCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MinesweeperToolbarPluginTabName);
}

TSharedRef<SDockTab> FMinesweeperToolbarPluginModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FMinesweeperToolbarPluginModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("MinesweeperToolbarPlugin.cpp"))
		);
	

	MineFieldBox->ClearChildren();
	
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				ConstructTopPanel().ToSharedRef()
			]
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Fill)
			.HAlign(HAlign_Fill)
			[
				ConstructBottomPanel().ToSharedRef()
			]
		];
}

void FMinesweeperToolbarPluginModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(MinesweeperToolbarPluginTabName);
}

void FMinesweeperToolbarPluginModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FMinesweeperToolbarPluginCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FMinesweeperToolbarPluginCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

void FMinesweeperToolbarPluginModule::ChangeWidth(const FText& InputText)
{
	MaxWidthX = InputText.ToString() == "" ? 1 : FMath::Clamp(FCString::Atoi(*InputText.ToString()), 1, 50);
	int FieldSize = MaxWidthX*MaxHeightY;
	AmountOfMines = FieldSize * MinePercentage;
	AmountOfMinesInputBox->SetText(FText::FromString(FString::FromInt(AmountOfMines)));
}

void FMinesweeperToolbarPluginModule::ChangeHeight(const FText& InputText)
{
	MaxHeightY = InputText.ToString() == "" ? 1 : FMath::Clamp(FCString::Atoi(*InputText.ToString()), 1, 50);
	int FieldSize = MaxWidthX*MaxHeightY;
	AmountOfMines = FieldSize * MinePercentage;
	AmountOfMinesInputBox->SetText(FText::FromString(FString::FromInt(AmountOfMines)));
}

void FMinesweeperToolbarPluginModule::ChangeMineAmount(const FText& InputText)
{
	int FieldSize = MaxWidthX*MaxHeightY;
	AmountOfMines = FMath::Clamp(FCString::Atoi(*InputText.ToString()), 0, FieldSize-1);
	AmountOfMinesInputBox->SetText(FText::FromString(FString::FromInt(AmountOfMines)));
	AmountOfMinesPercentageSlider->SetValue(100*AmountOfMines/FieldSize);
}

void FMinesweeperToolbarPluginModule::ChangeMineAmountPercentage(float InputValue)
{
	MinePercentage = InputValue/100;
	AmountOfMines = FMath::Floor(MaxWidthX*MaxHeightY*MinePercentage);
	AmountOfMinesInputBox->SetText(FText::FromString(FString::FromInt(AmountOfMines)));
}

FReply FMinesweeperToolbarPluginModule::BuildMineField()
{
	bLost = false;
	MineFieldBox->ClearChildren();
	MineField.Init(TSharedPtr<SButton>(), MaxHeightY*MaxWidthX);
	int MinesRemainingToPlace = AmountOfMines;

	for (int PosY = 0; PosY < MaxHeightY; PosY++)
	{
		TSharedPtr<SHorizontalBox> FieldRow;
		MineFieldBox->AddSlot()
		.AutoHeight()
		[
			SAssignNew(FieldRow, SHorizontalBox)
		];

		for (int PosX = 0; PosX < MaxWidthX; PosX++)
		{
			TSharedPtr<SButton> ButtonField;
			
			FieldRow->AddSlot()
			.HAlign(HAlign_Fill)
			[
				SNew(SBox)
				.WidthOverride(25)
				.HeightOverride(25)
				[
					SAssignNew(ButtonField, SButton)
					.Text(FText::FromString(" "))
					.Tag(FName(""))
					.OnClicked(FOnClicked::CreateRaw(this, &FMinesweeperToolbarPluginModule::ClickOnMineField, PosX, PosY))
				]
			];
			
			MineField[MaxWidthX*PosY+PosX] = ButtonField;
		}
	}

	// Put Mines everywhere
	for (;MinesRemainingToPlace > 0;)
	{
		int RandX = FMath::RandRange(0, MaxWidthX-1);
		int RandY = FMath::RandRange(0, MaxHeightY-1);

		if (!HasMineOnField(RandX, RandY))
		{	
			MineField[MaxWidthX*RandY+RandX]->SetTag(FName("Mine"));
			MinesRemainingToPlace--;
		}
	}
	return FReply::Handled();
}

FReply FMinesweeperToolbarPluginModule::ClickOnMineField(int PosX, int PosY)
{
	if (bLost)
	{
		FText OutputMessage = FText::FromString("You already lost! Generate a new Field to Play Again.");
		FMessageDialog::Open(EAppMsgType::Ok, OutputMessage);
		return FReply::Handled();
	}

	TSharedPtr<SButton> CurrentField = MineField[MaxWidthX*PosY+PosX];
	if (HasMineOnField(PosX, PosY))
	{
		bLost = true;
		RevealAllMines();

		// Create Window About Loss
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("You Lose!!!"));
	}
	else
	{
		int SurroundingMines = CountNeighbouringMines(PosX, PosY);
		if (SurroundingMines > 0)
		{
			CurrentField->SetTag(FName("Safe"));
			MakeYellowCounterButton(CurrentField, SurroundingMines);
		}
		else
		{
			ExpandNeighbourhood(PosX, PosY);
		}
	}

	return FReply::Handled();
}

void FMinesweeperToolbarPluginModule::ExpandNeighbourhood(int PosX, int PosY)
{
	if (OutOfBoundsX(PosX, PosY) || OutOfBoundsY(PosX, PosY) || HasMineOnField(PosX,PosY) ||
		MineField[MaxWidthX*PosY+PosX]->GetTag() == FName("Safe"))
		return;

	TSharedPtr<SButton> CurrentField = MineField[MaxWidthX*PosY+PosX];
	int SurroundingMines = CountNeighbouringMines(PosX, PosY);
	CurrentField->SetTag(FName("Safe"));

	if (SurroundingMines > 0)
	{
		MakeYellowCounterButton(CurrentField, SurroundingMines);
	}
	else
	{
		for (int i = -1; i < 2; i++)
		{
			for (int j = -1; j < 2; j++)
			{
				ExpandNeighbourhood(PosX+i, PosY+j);
			}
		}
		CurrentField->SetBorderBackgroundColor(FColor::Green);
	}
}

int FMinesweeperToolbarPluginModule::CountNeighbouringMines(int PosX, int PosY)
{
	int SurroundingMines = 0;

	for (int i = -1; i < 2; i++)
		{
			for (int j = -1; j < 2; j++)
			{
				int FieldToCheck = MaxWidthX*(PosY+i)+(PosX+j);
				if (!OutOfBoundsX(PosX+j, PosY+i) && !OutOfBoundsY(PosX+j, PosY+i))
					SurroundingMines += MineField[FieldToCheck]->GetTag() == FName("Mine") ? 1 : 0;
			}
		}

	return SurroundingMines;
}

void FMinesweeperToolbarPluginModule::MakeYellowCounterButton(TSharedPtr<SButton> RefButton, int SurroundingMines)
{
	TSharedPtr<STextBlock> NumberOfMines = SNew(STextBlock);
	NumberOfMines->SetText(FText::FromString(FString::FromInt(SurroundingMines)));
	RefButton->SetBorderBackgroundColor(FColor::Yellow);
	RefButton->SetContent(NumberOfMines.ToSharedRef());
}

bool FMinesweeperToolbarPluginModule::HasMineOnField(int PosX, int PosY)
{
	return MineField[MaxWidthX*PosY+PosX]->GetTag() == FName("Mine");
}

bool FMinesweeperToolbarPluginModule::OutOfBoundsX(int PosX, int PosY)
{
	return PosX > MaxWidthX - 1 || PosX < 0;
}

bool FMinesweeperToolbarPluginModule::OutOfBoundsY(int PosX, int PosY)
{
	return PosY > MaxHeightY - 1 || PosY < 0;
}

TSharedPtr<SHorizontalBox> FMinesweeperToolbarPluginModule::ConstructTopPanel()
{
	TSharedPtr<SHorizontalBox> TopPanel = SNew(SHorizontalBox);
	TSharedRef<SHorizontalBox> WidthAndHeight = SNew(SHorizontalBox);
	TSharedRef<SHorizontalBox> MineAmountSettings = SNew(SHorizontalBox);

	WidthAndHeight->AddSlot()
	.HAlign(HAlign_Right)
	.VAlign(VAlign_Center)
	.AutoWidth()
	.Padding(10)
	[
		SNew(STextBlock)
		.Text(FText::FromString("Width:"))
	];

	WidthAndHeight->AddSlot()
	.Padding(10)
	[
		SNew(SEditableTextBox)
		.Text(FText::FromString("6"))
		.OnTextChanged(FOnTextChanged::CreateRaw(this, &FMinesweeperToolbarPluginModule::ChangeWidth))
	];					

	WidthAndHeight->AddSlot()
	.HAlign(HAlign_Right)
	.VAlign(VAlign_Center)
	.AutoWidth()
	.Padding(10)
	[
		SNew(STextBlock)
		.Text(FText::FromString("Height:"))
	];

	WidthAndHeight->AddSlot()
	.Padding(10)
	[
		SNew(SEditableTextBox)
		.Text(FText::FromString("6"))
		.OnTextChanged(FOnTextChanged::CreateRaw(this, &FMinesweeperToolbarPluginModule::ChangeHeight))
	];

	MineAmountSettings->AddSlot()
	.AutoWidth()
	.Padding(10)
	[
		SNew(SBox)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Amount of Mines"))
		]
	];

	MineAmountSettings->AddSlot()
	.AutoWidth()
	.Padding(10)
	[
		SAssignNew(AmountOfMinesInputBox, SEditableTextBox)
		.Text(FText::FromString("6"))
		.OnTextChanged(FOnTextChanged::CreateRaw(this, &FMinesweeperToolbarPluginModule::ChangeMineAmount))
	];

	MineAmountSettings->AddSlot()
	.HAlign(HAlign_Fill)
	[
		SAssignNew(AmountOfMinesPercentageSlider, SSlider)
		.OnValueChanged(FOnFloatValueChanged::CreateRaw(this, &FMinesweeperToolbarPluginModule::ChangeMineAmountPercentage))
		.MinValue(1)
		.MaxValue(99)
	];

	TopPanel->AddSlot()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Fill)
		.AutoHeight()
		[
			WidthAndHeight
		]
		+ SVerticalBox::Slot()
		.VAlign(VAlign_Top)
		[
			MineAmountSettings
		]
	];

	TopPanel->AddSlot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	.AutoWidth()
	.Padding(10)
	[
		SNew(SButton)
		.Text(FText::FromString("Generate new Minefield"))
		.OnClicked(FOnClicked::CreateRaw(this, &FMinesweeperToolbarPluginModule::BuildMineField))
	];

	return TopPanel;
}

TSharedPtr<SHorizontalBox> FMinesweeperToolbarPluginModule::ConstructBottomPanel()
{
	TSharedPtr<SHorizontalBox> BottomPanel = SNew(SHorizontalBox);

	BottomPanel->AddSlot()
	[
		SNew(SBorder)
		.BorderBackgroundColor(FColor::Yellow)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				MineFieldBox.ToSharedRef()
			]
		]
	];

	return BottomPanel;
}

void FMinesweeperToolbarPluginModule::RevealAllMines()
{
	for (TSharedPtr<SButton> Button : MineField)
	{
		if (Button->GetTag() == FName("Mine"))
		{
			TSharedPtr<STextBlock> NewText = SNew(STextBlock);
			NewText->SetText(FText::FromString("M"));
			Button->SetBorderBackgroundColor(FColor::Red);
			Button->SetContent(NewText.ToSharedRef());
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMinesweeperToolbarPluginModule, MinesweeperToolbarPlugin)