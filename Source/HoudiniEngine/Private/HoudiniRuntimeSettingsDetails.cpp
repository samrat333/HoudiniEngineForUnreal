/*
 * PROPRIETARY INFORMATION.  This software is proprietary to
 * Side Effects Software Inc., and is not to be reproduced,
 * transmitted, or disclosed in any way without written permission.
 *
 * Produced by:
 *      Mykola Konyk
 *      Side Effects Software Inc
 *      123 Front Street West, Suite 1401
 *      Toronto, Ontario
 *      Canada   M5J 2M2
 *      416-504-9876
 *
 */

#include "HoudiniEnginePrivatePCH.h"


float
FHoudiniRuntimeSettingsDetails::RowValueWidgetDesiredWidth = 350;


float
FHoudiniRuntimeSettingsDetails::RowFullWidgetDesiredWidth = 400;


TSharedRef<IDetailCustomization>
FHoudiniRuntimeSettingsDetails::MakeInstance()
{
	return MakeShareable(new FHoudiniRuntimeSettingsDetails);
}


FHoudiniRuntimeSettingsDetails::FHoudiniRuntimeSettingsDetails()
{

}


FHoudiniRuntimeSettingsDetails::~FHoudiniRuntimeSettingsDetails()
{

}


void
FHoudiniRuntimeSettingsDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Create basic categories.
	DetailBuilder.EditCategory("Cooking", TEXT(""), ECategoryPriority::Important);
	DetailBuilder.EditCategory("CollisionGeneration", TEXT(""), ECategoryPriority::Important);

	// Create Plugin Information category.
	{
		static const FName InformationCategory = TEXT("Plugin Information");
		IDetailCategoryBuilder& InformationCategoryBuilder = DetailBuilder.EditCategory(InformationCategory);

		// Add built Houdini version.
		CreateHoudiniEntry(LOCTEXT("HInformationBuilt", "Built against Houdini"),
						   InformationCategoryBuilder, HAPI_VERSION_HOUDINI_MAJOR, HAPI_VERSION_HOUDINI_MINOR,
						   HAPI_VERSION_HOUDINI_BUILD, HAPI_VERSION_HOUDINI_PATCH);

		// Add built against Houdini Engine version.
		CreateHoudiniEngineEntry(LOCTEXT("HEngineInformationBuilt", "Built against Houdini Engine"),
								 InformationCategoryBuilder, HAPI_VERSION_HOUDINI_ENGINE_MAJOR,
								 HAPI_VERSION_HOUDINI_ENGINE_MINOR, HAPI_VERSION_HOUDINI_ENGINE_API);

		// Add running against Houdini version.
		{
			int32 RunningMajor = 0;
			int32 RunningMinor = 0;
			int32 RunningBuild = 0;
			int32 RunningPatch = 0;

			if(FHoudiniApi::IsHAPIInitialized())
			{
				// Retrieve version numbers for running Houdini.
				FHoudiniApi::GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_MAJOR, &RunningMajor);
				FHoudiniApi::GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_MINOR, &RunningMinor);
				FHoudiniApi::GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_BUILD, &RunningBuild);
				FHoudiniApi::GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_PATCH, &RunningPatch);
			}

			CreateHoudiniEntry(LOCTEXT("HInformationRunning", "Running against Houdini"),
							   InformationCategoryBuilder, RunningMajor, RunningMinor, RunningBuild, RunningPatch);
		}

		// Add running against Houdini Engine version.
		{
			int32 RunningEngineMajor = 0;
			int32 RunningEngineMinor = 0;
			int32 RunningEngineApi = 0;

			if(FHoudiniApi::IsHAPIInitialized())
			{
				// Retrieve version numbers for running Houdini Engine.
				FHoudiniApi::GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_ENGINE_MAJOR, &RunningEngineMajor);
				FHoudiniApi::GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_ENGINE_MINOR, &RunningEngineMinor);
				FHoudiniApi::GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_ENGINE_API, &RunningEngineApi);
			}

			CreateHoudiniEngineEntry(LOCTEXT("HEngineInformationRunning", "Running against Houdini Engine"),
									 InformationCategoryBuilder, RunningEngineMajor, RunningEngineMinor,
									 RunningEngineApi);
		}

		// Add path of libHAPI.
		{
			FString HFSPath = HOUDINI_ENGINE_HFS_PATH;
			if(!HFSPath.IsEmpty())
			{
				HFSPath += TEXT("/bin");
				CreateHAPILibraryPathEntry(HFSPath, InformationCategoryBuilder);
			}
			else
			{
				FString LibHAPILocation = TEXT("Not Found");

#if PLATFORM_WINDOWS

				FString HoudiniRegistryLocation = FString::Printf(TEXT("Software\\Side Effects Software\\Houdini %d.%d.%d"),
					HAPI_VERSION_HOUDINI_MAJOR, HAPI_VERSION_HOUDINI_MINOR, HAPI_VERSION_HOUDINI_BUILD);
				FString HoudiniInstallationPath;

				if(FWindowsPlatformMisc::QueryRegKey(HKEY_LOCAL_MACHINE, *HoudiniRegistryLocation, TEXT("InstallPath"), HoudiniInstallationPath))
				{
					LibHAPILocation = FString::Printf(TEXT("%s/bin"), *HoudiniInstallationPath);
				}

#elif PLATFORM_MAC

				FString HoudiniLocation = FString::Printf(TEXT("/Library/Frameworks/Houdini.framework/Versions/%d.%d.%d/Libraries"), 
					HAPI_VERSION_HOUDINI_MAJOR, HAPI_VERSION_HOUDINI_MINOR, HAPI_VERSION_HOUDINI_BUILD);
				FString CheckLibHAPILocation = HoudiniLocation + TEXT("/libHAPI.dylib");

				if(FPaths::FileExists(CheckLibHAPILocation))
				{
					LibHAPILocation = HoudiniLocation;
				}

#endif

				CreateHAPILibraryPathEntry(LibHAPILocation, InformationCategoryBuilder);
			}
		}
	}
}


void
FHoudiniRuntimeSettingsDetails::CreateHoudiniEntry(const FText& EntryName, IDetailCategoryBuilder& DetailCategoryBuilder,
												   int32 VersionMajor, int32 VersionMinor, int32 VersionBuild,
												   int32 VersionPatch)
{
	FDetailWidgetRow& Row = DetailCategoryBuilder.AddCustomRow(TEXT(""));

	Row.NameWidget.Widget = SNew(STextBlock)
							.Text(EntryName)
							.Font(IDetailLayoutBuilder::GetDetailFont());

	TSharedRef<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox);

	{
		TSharedRef<SNumericEntryBox<int32> > NumericEntryBox = SNew(SNumericEntryBox<int32>);
		HorizontalBox->AddSlot().Padding(0, 0, 5, 0)
		[
			SAssignNew(NumericEntryBox, SNumericEntryBox<int32>)
			.AllowSpin(false)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Value(VersionMajor)
		];
		NumericEntryBox->SetEnabled(false);
	}

	{
		TSharedRef<SNumericEntryBox<int32> > NumericEntryBox = SNew(SNumericEntryBox<int32>);
		HorizontalBox->AddSlot().Padding(0, 0, 5, 0)
		[
			SAssignNew(NumericEntryBox, SNumericEntryBox<int32>)
			.AllowSpin(false)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Value(VersionMinor)
		];
		NumericEntryBox->SetEnabled(false);
	}

	{
		TSharedRef<SNumericEntryBox<int32> > NumericEntryBox = SNew(SNumericEntryBox<int32>);
		HorizontalBox->AddSlot().Padding(0, 0, 5, 0)
		[
			SAssignNew(NumericEntryBox, SNumericEntryBox<int32>)
			.AllowSpin(false)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Value(VersionBuild)
		];
		NumericEntryBox->SetEnabled(false);
	}

	{
		TSharedRef<SNumericEntryBox<int32> > NumericEntryBox = SNew(SNumericEntryBox<int32>);
		HorizontalBox->AddSlot().Padding(0, 0, 5, 0)
		[
			SAssignNew(NumericEntryBox, SNumericEntryBox<int32>)
			.AllowSpin(false)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Value(VersionPatch)
		];
		NumericEntryBox->SetEnabled(false);
	}

	Row.ValueWidget.Widget = HorizontalBox;
	Row.ValueWidget.MinDesiredWidth(FHoudiniRuntimeSettingsDetails::RowValueWidgetDesiredWidth);
}


void
FHoudiniRuntimeSettingsDetails::CreateHoudiniEngineEntry(const FText& EntryName,
														 IDetailCategoryBuilder& DetailCategoryBuilder,
														 int32 VersionMajor, int32 VersionMinor, int32 VersionApi)
{
	FDetailWidgetRow& Row = DetailCategoryBuilder.AddCustomRow(TEXT(""));

	Row.NameWidget.Widget = SNew(STextBlock)
							.Text(EntryName)
							.Font(IDetailLayoutBuilder::GetDetailFont());

	TSharedRef<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox);

	{
		TSharedRef<SNumericEntryBox<int32> > NumericEntryBox = SNew(SNumericEntryBox<int32>);
		HorizontalBox->AddSlot().Padding(0, 0, 5, 0)
		[
			SAssignNew(NumericEntryBox, SNumericEntryBox<int32>)
			.AllowSpin(false)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Value(VersionMajor)
		];
		NumericEntryBox->SetEnabled(false);
	}

	{
		TSharedRef<SNumericEntryBox<int32> > NumericEntryBox = SNew(SNumericEntryBox<int32>);
		HorizontalBox->AddSlot().Padding(0, 0, 5, 0)
		[
			SAssignNew(NumericEntryBox, SNumericEntryBox<int32>)
			.AllowSpin(false)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Value(VersionMinor)
		];
		NumericEntryBox->SetEnabled(false);
	}

	{
		TSharedRef<SNumericEntryBox<int32> > NumericEntryBox = SNew(SNumericEntryBox<int32>);
		HorizontalBox->AddSlot().Padding(0, 0, 5, 0)
		[
			SAssignNew(NumericEntryBox, SNumericEntryBox<int32>)
			.AllowSpin(false)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Value(VersionApi)
		];
		NumericEntryBox->SetEnabled(false);
	}

	Row.ValueWidget.Widget = HorizontalBox;
	Row.ValueWidget.MinDesiredWidth(FHoudiniRuntimeSettingsDetails::RowValueWidgetDesiredWidth);
}


void
FHoudiniRuntimeSettingsDetails::CreateHAPILibraryPathEntry(const FString& LibHAPIPath,
														   IDetailCategoryBuilder& DetailCategoryBuilder)
{
	FDetailWidgetRow& Row = DetailCategoryBuilder.AddCustomRow(TEXT(""));

	FString LibHAPIExtensionName = TEXT("Location of libHAPI");

#if PLATFORM_WINDOWS
	LibHAPIExtensionName += TEXT(".dll");
#elif PLATFORM_MAC
	LibHAPIExtensionName += TEXT(".dylib");
#elif PLATFORM_LINUX
	LibHAPIExtensionName += TEXT(".so");
#endif

	Row.NameWidget.Widget = SNew(STextBlock)
							.Text(LibHAPIExtensionName)
							.Font(IDetailLayoutBuilder::GetDetailFont());

	TSharedRef<STextBlock> TextBlock = SNew(STextBlock)
									   .Text(LibHAPIPath)
									   .Font(IDetailLayoutBuilder::GetDetailFont());

	TextBlock->SetEnabled(false);
	Row.ValueWidget.Widget = TextBlock;
	Row.ValueWidget.MinDesiredWidth(FHoudiniRuntimeSettingsDetails::RowValueWidgetDesiredWidth);
}

