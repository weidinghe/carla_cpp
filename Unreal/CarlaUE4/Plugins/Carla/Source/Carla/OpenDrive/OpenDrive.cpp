﻿// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/OpenDrive/OpenDrive.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Misc/FileHelper.h"
#include "Carla/Game/CarlaStatics.h"
#include "GenericPlatform/GenericPlatformProcess.h"

#include "Runtime/Core/Public/HAL/FileManagerGeneric.h"
#include "Misc/FileHelper.h"

FString UOpenDrive::FindPathToXODRFile(const FString &InMapName){

  FString MapName = InMapName;

#if WITH_EDITOR
    {
      // 在编辑器中游玩时，地图名称会多出一个前缀，这里我们将其移除。
      FString CorrectedMapName = MapName;
      constexpr auto PIEPrefix = TEXT("UEDPIE_0_");
      CorrectedMapName.RemoveFromStart(PIEPrefix);
      MapName = CorrectedMapName;
    }
#endif // WITH_EDITOR

  MapName += TEXT(".xodr");

  const FString DefaultFilePath =
      FPaths::ProjectContentDir() +
      TEXT("Carla/Maps/OpenDrive/") +
      MapName;

  auto &FileManager = IFileManager::Get();

  if (FileManager.FileExists(*DefaultFilePath))
  {
    return DefaultFilePath;
  }

  TArray<FString> FilesFound;
  FileManager.FindFilesRecursive(
      FilesFound,
      *FPaths::ProjectContentDir(),
      *MapName,
      true,
      false,
      false);

  return FilesFound.Num() > 0 ? FilesFound[0u] : FString{};
}

FString UOpenDrive::GetXODR(const UWorld *World)
{
  auto MapName = World->GetMapName();

  // 在编辑器中游玩时，地图名称会多出一个前缀，这里我们将其移除。
  #if WITH_EDITOR
  {
    FString CorrectedMapName = MapName;
    constexpr auto PIEPrefix = TEXT("UEDPIE_0_");
    CorrectedMapName.RemoveFromStart(PIEPrefix);
    MapName = CorrectedMapName;
  }
  #endif // WITH_EDITOR

  ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(World);

  auto MapDir = GameMode->GetFullMapPath();
  const auto FolderDir = MapDir + "/OpenDrive/";
  const auto FileName = MapDir.EndsWith(MapName) ? "*" : MapName;

  // 查找地图中所有.xodr和.bin文件。
  TArray<FString> Files;
  IFileManager::Get().FindFilesRecursive(Files, *FolderDir, *FString(FileName + ".xodr"), true, false, false);

  FString Content;

  if (!Files.Num())
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to find OpenDrive file for map '%s'"), *MapName);
  }
  else if (FFileHelper::LoadFileToString(Content, *Files[0]))
  {
    UE_LOG(LogTemp, Log, TEXT("Loaded OpenDrive file '%s'"), *Files[0]);
  }
  else
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to load OpenDrive file '%s'"), *Files[0]);
  }

  return Content;
}

FString UOpenDrive::LoadXODR(const FString &MapName)
{
  const auto FilePath = FindPathToXODRFile(MapName);

  FString Content;

  if (FilePath.IsEmpty())
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to find OpenDrive file for map '%s'"), *MapName);
  }
  else if (FFileHelper::LoadFileToString(Content, *FilePath))
  {
    UE_LOG(LogTemp, Log, TEXT("Loaded OpenDrive file '%s'"), *FilePath);
  }
  else
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to load OpenDrive file '%s'"), *FilePath);
  }

  return Content;
}

FString UOpenDrive::GetXODRByPath(FString XODRPath, FString MapName){

  // 在编辑器中游玩时，地图名称会多出一个前缀，这里我们将其移除。
  #if WITH_EDITOR
  {
    FString CorrectedMapName = MapName;
    constexpr auto PIEPrefix = TEXT("UEDPIE_0_");
    CorrectedMapName.RemoveFromStart(PIEPrefix);
    MapName = CorrectedMapName;
  }
  #endif // WITH_EDITOR

  FString FileName = XODRPath.EndsWith(MapName) ? "*" : MapName;
  FString FolderDir = XODRPath;
  FolderDir.RemoveFromEnd(MapName + ".xodr");

  // 查找地图中所有.xodr和.bin文件。
  TArray<FString> Files;
  IFileManager::Get().FindFilesRecursive(Files, *FolderDir, *FString(FileName + ".xodr"), true, false, false);

  FString Content;

  if (!Files.Num())
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to find OpenDrive file for map '%s'"), *MapName);
  }
  else if (FFileHelper::LoadFileToString(Content, *Files[0]))
  {
    UE_LOG(LogTemp, Log, TEXT("Loaded OpenDrive file '%s'"), *Files[0]);
  }

  return Content;
}

UOpenDriveMap *UOpenDrive::LoadOpenDriveMap(const FString &MapName)
{
  UOpenDriveMap *Map = nullptr;
  auto XODRContent = LoadXODR(MapName);
  if (!XODRContent.IsEmpty())
  {
    Map = NewObject<UOpenDriveMap>();
    Map->Load(XODRContent);
  }
  return Map;
}

UOpenDriveMap *UOpenDrive::LoadCurrentOpenDriveMap(const UObject *WorldContextObject)
{
  UWorld *World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
  return World != nullptr ?
      LoadOpenDriveMap(World->GetMapName()) :
      nullptr;
}
