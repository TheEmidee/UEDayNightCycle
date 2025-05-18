#include "DNCTypes.h"

FDNCTimeStamp::FDNCTimeStamp( const float seconds ) :
    Seconds( seconds )
{
}

FDNCTimeStamp::FDNCTimeStamp( const FDNCTime & time ) :
    Seconds( time.ToSeconds() )
{
}

FText FDNCTimeStamp::ToText() const
{
    const auto minutes = FUnitConversion::Convert( Seconds, EUnit::Seconds, EUnit::Minutes );
    const auto secondes = FUnitConversion::Convert( FMath::Modulo( minutes, 1 ), EUnit::Minutes, EUnit::Seconds );

    FNumberFormattingOptions format_parameters;
    format_parameters.SetMinimumIntegralDigits( 2 );
    format_parameters.SetMaximumFractionalDigits( 0 );

    return FText::FormatOrdered( FText::FromString( "{0} : {1}" ), FText::AsNumber( FMath::TruncToInt( minutes ), &format_parameters ), FText::AsNumber( FMath::TruncToInt( secondes ), &format_parameters ) );
}

FDNCTime::FDNCTime() :
    Hours( 0 ),
    Minutes( 0 )
{
}

FDNCTime::FDNCTime( int hour, int minutes ) :
    Hours( hour ),
    Minutes( minutes )
{
}

FString FDNCTime::ToString() const
{
    return FString::Printf( TEXT( "%i:%i" ), Hours, Minutes );
}

FDNCTime FDNCTime::FromSeconds( int seconds )
{
    const auto hours = seconds / 3600;
    const auto hours_remainder = seconds % 3600;
    const auto minutes = hours_remainder / 60;

    return FDNCTime( hours, minutes );
}

FDNCTime FDNCTime::FromSeconds( float seconds )
{
    return FromSeconds( static_cast< int >( seconds ) );
}

UDNCDayDefinition::UDNCDayDefinition() :
    FullDayDuration( 0, 30 ),
    bStartsPaused( false )
{
}

void UDNCObserver::SetWorld( UWorld * world )
{
    World = world;
}

UWorld * UDNCObserver::GetWorld() const
{
    return World;
}