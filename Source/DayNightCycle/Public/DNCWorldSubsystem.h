#pragma once

#include "DNCTypes.h"

#include <CoreMinimal.h>
#include <Subsystems/WorldSubsystem.h>

#include "DNCWorldSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FDNCOnPeriodChangedDelegate, UDNCPeriod *, PreviousPeriod, UDNCPeriod *, NewPeriod );

UCLASS( DisplayName = "Day and Night Cycle System" )
class DAYNIGHTCYCLE_API UDNCWorldSubsystem final : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    UDNCWorldSubsystem();

    void Tick( float delta_time ) override;
    bool IsTickable() const override;

    UFUNCTION( BlueprintCallable )
    void StartCycle( UDNCDayDefinition * day_definition );

    UFUNCTION( BlueprintCallable )
    void Pause();

    UFUNCTION( BlueprintCallable )
    void Resume();

    UFUNCTION( BlueprintPure )
    UDNCPeriod * GetCurrentPeriod() const;

    UFUNCTION( BlueprintPure )
    FDNCTimeStamp GetCurrentPeriodRemainingTime( const EDNCPeriodTransitionUnits time_unit ) const;

    UFUNCTION( BlueprintCallable )
    void SetTime( const FDNCTime & new_time );

    void SetTime( const FDNCTimeStamp & time_stamp );

    TStatId GetStatId() const override;

    void ShowDebugInfo( UCanvas * canvas );

    void Serialize( FArchive & archive ) override;

private:
    enum class ETransitionState : uint8
    {
        WaitingToTransition,
        Transitioning
    };

    void UpdateCurrentPeriod( bool use_last_period_if_no_period_matches );
    void UpdateObserversWithCurrentTime();
    void SetCurrentPeriod( int period_index );
    void TriggerTransitionEvents( float delta_time );

    template < typename _CALLBACK_ >
    void ForEachDayObserver( _CALLBACK_ callback )
    {
        for ( auto & observer : DayDefinition->Observers )
        {
            callback( observer );
        }
    }

    template < typename _CALLBACK_ >
    void ForEachPeriodObserver( UDNCPeriod * period, _CALLBACK_ callback )
    {
        for ( auto & observer : period->Observers )
        {
            callback( observer );
        }
    }

    template < typename _CALLBACK_ >
    void ForEachDayAndCurrentPeriodObserver( _CALLBACK_ callback )
    {
        ForEachPeriodObserver( CurrentPeriod, callback );
        ForEachDayObserver( callback );
    }

    UPROPERTY()
    TObjectPtr< UDNCDayDefinition > DayDefinition;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    TObjectPtr< UDNCPeriod > CurrentPeriod;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    TObjectPtr< UDNCPeriod > NextPeriod;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    FDNCTimeStamp CurrentTime;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    float DilatedTimeSpeed;

    UPROPERTY( BlueprintAssignable )
    FDNCOnPeriodChangedDelegate OnPeriodChanged;

    float TimeSpeed;
    bool bIsPaused;
    bool bCanTransition;
    FDNCTimeStamp NextTransitionStartTime;
    FDNCTimeStamp TransitionDuration;
    FDNCTimeStamp TransitionTime;
    TOptional< FDNCTimeStamp > PendingCurrentTime;
    ETransitionState TransitionState;
};