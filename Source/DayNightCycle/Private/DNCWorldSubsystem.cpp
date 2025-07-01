#include "DNCWorldSubsystem.h"

#include "DNCFunctionLibrary.h"

#include <Engine/Canvas.h>

#if !UE_BUILD_SHIPPING
static FAutoConsoleCommandWithWorldAndArgs CVarSetTimeSeconds(
    TEXT( "DayNightSystem.SetTimeSeconds" ),
    TEXT( "Sets the time (in seconds) of the day cycle system" ),
    FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(
        []( const TArray< FString > & params, UWorld * world ) {
            if ( params.Num() != 1 )
            {
                return;
            }

            const auto seconds = FCString::Atoi( *params[ 0 ] );
            world->GetSubsystem< UDNCWorldSubsystem >()->SetTime( FDNCTime::FromSeconds( seconds ) );
        } ),
    ECVF_Cheat );

static FAutoConsoleCommandWithWorldAndArgs CVarSetTimeHoursMinutes(
    TEXT( "DayNightSystem.SetTimeHoursMinutes" ),
    TEXT( "Sets the time (in Hours Minutes) of the day cycle system" ),
    FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(
        []( const TArray< FString > & params, UWorld * world ) {
            if ( params.Num() != 2 )
            {
                return;
            }

            const auto hours = FCString::Atoi( *params[ 0 ] );
            const auto minutes = FCString::Atoi( *params[ 1 ] );

            world->GetSubsystem< UDNCWorldSubsystem >()->SetTime( FDNCTime( hours, minutes ) );
        } ),
    ECVF_Cheat );
#endif

namespace
{
    const auto DayInSecondsTime = FDNCTimeStamp( UDNCFunctionLibrary::TotalSecondsInADay() );
}

UDNCWorldSubsystem::UDNCWorldSubsystem() :
    CurrentPeriod( nullptr ),
    NextPeriod( nullptr ),
    DilatedTimeSpeed( 1.0f ),
    TimeSpeed( 1.0f ),
    bIsPaused( false ),
    TransitionState( ETransitionState::WaitingToTransition )
{
}

void UDNCWorldSubsystem::Tick( float delta_time )
{
    Super::Tick( delta_time );

    CurrentTime += delta_time * DilatedTimeSpeed;

    TriggerTransitionEvents( delta_time );

    auto use_last_period_if_no_period_matches = false;

    if ( CurrentTime > DayInSecondsTime )
    {
        CurrentTime -= DayInSecondsTime;
        use_last_period_if_no_period_matches = true;
    }

    UpdateCurrentPeriod( use_last_period_if_no_period_matches );
    UpdateObserversWithCurrentTime();
}

bool UDNCWorldSubsystem::IsTickable() const
{
    return !bIsPaused && DayDefinition != nullptr && !DayDefinition->Periods.IsEmpty();
}

void UDNCWorldSubsystem::StartCycle( UDNCDayDefinition * day_definition )
{
    if ( day_definition == nullptr )
    {
        return;
    }

    if ( day_definition->Periods.IsEmpty() )
    {
        return;
    }

    DayDefinition = day_definition;

    TimeSpeed = static_cast< float >( DayInSecondsTime.Seconds ) / DayDefinition->FullDayDuration.ToSeconds();
    DilatedTimeSpeed = TimeSpeed;
    DayDefinition->Periods.Sort();

    const auto time = PendingCurrentTime.Get( FDNCTimeStamp( DayDefinition->CycleStart ) );

    ForEachDayObserver( [ this ]( auto & observer ) {
        observer->SetWorld( GetWorld() );
    } );

    for ( auto period : DayDefinition->Periods )
    {
        ForEachPeriodObserver( period, [ this ]( auto & observer ) {
            observer->SetWorld( GetWorld() );
        } );
    }

    ForEachDayObserver( [ this ]( const auto & observer ) {
        observer->OnCycleStarted( DayDefinition );
    } );

    SetTime( time );

    if ( day_definition->bStartsPaused )
    {
        Pause();
    }
}

void UDNCWorldSubsystem::Pause()
{
    bIsPaused = true;
}

void UDNCWorldSubsystem::Resume()
{
    bIsPaused = false;
}

UDNCPeriod * UDNCWorldSubsystem::GetCurrentPeriod() const
{
    return CurrentPeriod;
}

FDNCTimeStamp UDNCWorldSubsystem::GetCurrentPeriodRemainingTime( const EDNCPeriodTransitionUnits time_unit ) const
{
    auto current_period_elapsed_time = CurrentTime - CurrentPeriod->StartTime;

    if ( current_period_elapsed_time.Seconds < 0 )
    {
        current_period_elapsed_time = ( DayInSecondsTime - CurrentPeriod->StartTime ) + CurrentTime;
    }

    auto time_left = ( CurrentPeriod->RealTimeDuration * DilatedTimeSpeed ) - current_period_elapsed_time;

    switch ( time_unit )
    {
        case EDNCPeriodTransitionUnits::Game:
        {
            return time_left;
        }
        case EDNCPeriodTransitionUnits::RealWorld:
        {
            return time_left / DilatedTimeSpeed;
        }
        default:
        {
            checkNoEntry();
        }
        break;
    }
    return {};
}

void UDNCWorldSubsystem::SetTime( const FDNCTime & new_time )
{
    SetTime( FDNCTimeStamp( new_time ) );
}

void UDNCWorldSubsystem::SetTime( const FDNCTimeStamp & time_stamp )
{
    CurrentTime = time_stamp;
    UpdateCurrentPeriod( true );
}

TStatId UDNCWorldSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT( UDNCWorldSubsystem, STATGROUP_Tickables );
}

void UDNCWorldSubsystem::ShowDebugInfo( UCanvas * canvas )
{
    if ( !canvas )
    {
        return;
    }

    auto & display_debug_manager = canvas->DisplayDebugManager;

    if ( DayDefinition == nullptr )
    {
        display_debug_manager.SetDrawColor( FColor::Red );
        display_debug_manager.DrawString( TEXT( "No Day Definition is assigned." ) );
        return;
    }

    display_debug_manager.SetDrawColor( FColor::Yellow );
    display_debug_manager.DrawString( TEXT( "Day Definition : " ) );
    display_debug_manager.SetDrawColor( FColor::White );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Day duration : %s" ), *DayDefinition->FullDayDuration.ToString() ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Day start time : %s" ), *DayDefinition->CycleStart.ToString() ) );

    display_debug_manager.SetDrawColor( FColor::Yellow );
    display_debug_manager.DrawString( TEXT( "Current time : " ) );
    display_debug_manager.SetDrawColor( FColor::White );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Time Speed : %f" ), DilatedTimeSpeed ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Current Time: %f ( %s )" ), CurrentTime.Seconds, *FDNCTime::FromSeconds( CurrentTime.Seconds ).ToString() ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Current Period: %s" ), *GetNameSafe( CurrentPeriod ) ) );
}

void UDNCWorldSubsystem::Serialize( FArchive & archive )
{
    archive << CurrentTime;

    if ( archive.IsLoading() )
    {
        if ( DayDefinition != nullptr )
        {
            UpdateCurrentPeriod( true );
        }
        else
        {
            PendingCurrentTime = CurrentTime;
        }
    }
}

void UDNCWorldSubsystem::UpdateCurrentPeriod( bool use_last_period_if_no_period_matches )
{
    const auto & periods = DayDefinition->Periods;

    for ( auto index = periods.Num() - 1; index >= 0; --index )
    {
        const auto & period = periods[ index ];

        if ( CurrentTime > period->StartTime )
        {
            SetCurrentPeriod( index );
            return;
        }
    }

    if ( use_last_period_if_no_period_matches && periods.Num() > 0 )
    {
        SetCurrentPeriod( periods.Num() - 1 );
    }
}

void UDNCWorldSubsystem::UpdateObserversWithCurrentTime()
{
    ForEachDayObserver( [ & ]( auto & observer ) {
        observer->OnCurrentTimeChanged( CurrentPeriod, CurrentTime );
    } );
}

void UDNCWorldSubsystem::SetCurrentPeriod( const int period_index )
{
    if ( DayDefinition->Periods.IsEmpty() )
    {
        return;
    }

    auto new_period = DayDefinition->Periods[ period_index ];

    if ( CurrentPeriod == new_period )
    {
        return;
    }

    if ( CurrentPeriod != nullptr )
    {
        if ( TransitionState == ETransitionState::Transitioning && NextPeriod != nullptr )
        {
            ForEachDayAndCurrentPeriodObserver( [ this ]( const auto & observer ) {
                observer->OnTransitionEnded( CurrentPeriod, NextPeriod, TransitionDuration );
            } );

            TransitionState = ETransitionState::WaitingToTransition;
        }

        ForEachDayAndCurrentPeriodObserver( [ this ]( const auto & observer ) {
            observer->OnPeriodEnded( CurrentPeriod );
        } );
    }

    auto old_period = CurrentPeriod;

    CurrentPeriod = new_period;

    OnPeriodChangedDelegate.Broadcast( old_period, new_period );

    if ( DayDefinition->Periods.Num() > 1 )
    {
        const auto next_period_index = period_index == DayDefinition->Periods.Num() - 1 ? 0 : period_index + 1;
        NextPeriod = DayDefinition->Periods[ next_period_index ];

        auto delta = NextPeriod->StartTime - CurrentPeriod->StartTime;

        if ( delta.ToSeconds() < 0 )
        {
            delta = FDNCTime( 24, 0 ) - CurrentPeriod->StartTime + NextPeriod->StartTime;
        }

        DilatedTimeSpeed = delta.ToSeconds() / CurrentPeriod->RealTimeDuration.ToSeconds();

        switch ( CurrentPeriod->TransitionDurationUnits )
        {
            case EDNCPeriodTransitionUnits::Game:
            {
                TransitionDuration = CurrentPeriod->TransitionDuration;
            }
            break;
            case EDNCPeriodTransitionUnits::RealWorld:
            {
                TransitionDuration = FDNCTimeStamp( CurrentPeriod->TransitionDurationInSeconds * DilatedTimeSpeed );
            }
            break;
            default:
            {
                checkNoEntry();
            }
            break;
        }

        if ( TransitionDuration.Seconds > 0 )
        {
            NextTransitionStartTime = NextPeriod->StartTime - TransitionDuration;
        }
    }

    ForEachDayAndCurrentPeriodObserver( [ this ]( const auto & observer ) {
        observer->OnPeriodStarted( CurrentPeriod );
    } );
}

void UDNCWorldSubsystem::TriggerTransitionEvents( float delta_time )
{
    switch ( TransitionState )
    {
        case ETransitionState::WaitingToTransition:
        {
            if ( NextTransitionStartTime.Seconds > 0 &&
                 CurrentTime >= NextTransitionStartTime &&
                 NextPeriod != nullptr &&
                 CurrentTime.Seconds - NextTransitionStartTime.Seconds < NextPeriod->TransitionDurationInSeconds )
            {
                TransitionState = ETransitionState::Transitioning;
                TransitionTime.Seconds = 0.0f;

                ForEachDayAndCurrentPeriodObserver( [ this ]( const auto & observer ) {
                    observer->OnTransitionStarted( CurrentPeriod, NextPeriod, TransitionDuration );
                } );
            }
        }
        break;
        case ETransitionState::Transitioning:
        {
            TransitionTime.Seconds += delta_time * DilatedTimeSpeed;

            ForEachDayAndCurrentPeriodObserver( [ this ]( const auto & observer ) {
                observer->OnTransitionUpdate( CurrentPeriod, NextPeriod, TransitionTime, TransitionDuration );
            } );
        }
        break;
        default:
        {
            checkNoEntry();
            break;
        };
    }
}