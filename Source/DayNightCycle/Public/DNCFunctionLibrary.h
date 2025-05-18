#pragma once

#include "DNCTypes.h"

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "DNCFunctionLibrary.generated.h"

struct FDNCTime;

UCLASS()
class DAYNIGHTCYCLE_API UDNCFunctionLibrary final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintPure )
    static int DNCTimeToSeconds( const FDNCTime & time );

    UFUNCTION( BlueprintPure )
    static FDNCTimeStamp DNCTimeToTimeStamp( const FDNCTime & time );

    UFUNCTION( BlueprintPure )
    static FDNCTimeStamp SecondsToTimeStamp( float seconds );

    UFUNCTION( BlueprintPure )
    static FString DNCTimeToString( const FDNCTime & time );

    UFUNCTION( BlueprintPure )
    static int TotalSecondsInADay();

    UFUNCTION( BlueprintPure )
    static FText DNCTimeStampToText( const FDNCTimeStamp & time_stamp );
};
