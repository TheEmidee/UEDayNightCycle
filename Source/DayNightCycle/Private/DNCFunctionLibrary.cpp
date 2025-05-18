#include "DNCFunctionLibrary.h"

#include "DNCTypes.h"

int UDNCFunctionLibrary::DNCTimeToSeconds( const FDNCTime & time )
{
    return time.ToSeconds();
}

FDNCTimeStamp UDNCFunctionLibrary::DNCTimeToTimeStamp( const FDNCTime & time )
{
    return FDNCTimeStamp( time );
}

FDNCTimeStamp UDNCFunctionLibrary::SecondsToTimeStamp( float seconds )
{
    return FDNCTimeStamp( seconds );
}

FString UDNCFunctionLibrary::DNCTimeToString( const FDNCTime & time )
{
    return time.ToString();
}

int UDNCFunctionLibrary::TotalSecondsInADay()
{
    return FUnitConversion::Convert( 1, EUnit::Days, EUnit::Seconds );
}

FText UDNCFunctionLibrary::DNCTimeStampToText( const FDNCTimeStamp & time_stamp )
{
    return time_stamp.ToText();
}
