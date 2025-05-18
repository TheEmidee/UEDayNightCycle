#pragma once

#include <CoreMinimal.h>
#include <GameplayTagContainer.h>
#include <UObject/NoExportTypes.h>

#include "DNCTypes.generated.h"

class UDNCDayObserver;
class UDNCPeriodObserver;

USTRUCT( BlueprintType )
struct FDNCTime
{
    GENERATED_BODY()

    FDNCTime();
    FDNCTime( int hour, int minutes );

    int ToSeconds() const;
    FString ToString() const;

    FDNCTime & operator+=( const FDNCTime & other );

    static FDNCTime FromSeconds( int seconds );
    static FDNCTime FromSeconds( float seconds );

    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = true, UIMin = 0, UIMax = 23, ClampMin = 0, ClampMax = 23, ForceUnits = "Hours" ) )
    int Hours;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = true, UIMin = 0, UIMax = 59, ClampMin = 0, ClampMax = 59, ForceUnits = "Minutes" ) )
    int Minutes;
};

FORCEINLINE int FDNCTime::ToSeconds() const
{
    return FUnitConversion::Convert( Hours, EUnit::Hours, EUnit::Seconds ) + FUnitConversion::Convert( Minutes, EUnit::Minutes, EUnit::Seconds );
}

FORCEINLINE bool operator<( const FDNCTime & left, const FDNCTime & right )
{
    if ( left.Hours < right.Hours )
    {
        return true;
    }

    if ( left.Hours > right.Hours )
    {
        return false;
    }

    return left.Minutes < right.Minutes;
}

FORCEINLINE FDNCTime operator+( const FDNCTime & left, const FDNCTime & right )
{
    return FDNCTime::FromSeconds( left.ToSeconds() + right.ToSeconds() );
}

FORCEINLINE FDNCTime operator-( const FDNCTime & left, const FDNCTime & right )
{
    return FDNCTime::FromSeconds( left.ToSeconds() - right.ToSeconds() );
}

FORCEINLINE FDNCTime operator*( const FDNCTime & time, float speed )
{
    return FDNCTime::FromSeconds( time.ToSeconds() * speed );
}

FORCEINLINE FDNCTime & FDNCTime::operator+=( const FDNCTime & other )
{
    *this = FromSeconds( ToSeconds() + other.ToSeconds() );
    return *this;
}

USTRUCT( BlueprintType )
struct FDNCTimeStamp
{
    GENERATED_BODY()

    FDNCTimeStamp() = default;
    explicit FDNCTimeStamp( float seconds );
    explicit FDNCTimeStamp( const FDNCTime & time );
    FText ToText() const;

    FDNCTimeStamp & operator=( const FDNCTime & time );
    FDNCTimeStamp & operator+( const FDNCTimeStamp & other );

    UPROPERTY( BlueprintReadOnly )
    float Seconds = 0.0f;
};

FORCEINLINE FDNCTimeStamp & FDNCTimeStamp::operator=( const FDNCTime & time )
{
    Seconds = time.ToSeconds();
    return *this;
}

FORCEINLINE FDNCTimeStamp & FDNCTimeStamp::operator+( const FDNCTimeStamp & other )
{
    Seconds += other.Seconds;
    return *this;
}

FORCEINLINE FDNCTimeStamp operator-( const FDNCTime & time, const FDNCTimeStamp & time_stamp )
{
    return FDNCTimeStamp( time.ToSeconds() - time_stamp.Seconds );
}

FORCEINLINE FDNCTimeStamp operator-( const FDNCTimeStamp & time_stamp, const FDNCTime & time )
{
    return FDNCTimeStamp( time_stamp.Seconds - time.ToSeconds() );
}

FORCEINLINE FDNCTimeStamp operator/( FDNCTimeStamp & time_stamp, float speed )
{
    time_stamp.Seconds /= speed;
    return time_stamp;
}

FORCEINLINE FDNCTimeStamp & operator+=( FDNCTimeStamp & timestamp, float seconds )
{
    timestamp.Seconds += seconds;
    return timestamp;
}

FORCEINLINE FDNCTimeStamp & operator-=( FDNCTimeStamp & timestamp, const FDNCTimeStamp & other )
{
    timestamp.Seconds -= other.Seconds;
    return timestamp;
}

FORCEINLINE bool operator==( const FDNCTimeStamp & left, const FDNCTimeStamp & right )
{
    return left.Seconds == right.Seconds;
}

FORCEINLINE bool operator!=( const FDNCTimeStamp & left, const FDNCTimeStamp & right )
{
    return !operator==( left, right );
}

FORCEINLINE bool operator<( const FDNCTimeStamp & left, const FDNCTimeStamp & right )
{
    return left.Seconds < right.Seconds;
}

FORCEINLINE bool operator<=( const FDNCTimeStamp & left, const FDNCTimeStamp & right )
{
    return left.Seconds <= right.Seconds;
}

FORCEINLINE bool operator>( const FDNCTimeStamp & left, const FDNCTimeStamp & right )
{
    return left.Seconds > right.Seconds;
}

FORCEINLINE bool operator>=( const FDNCTimeStamp & left, const FDNCTimeStamp & right )
{
    return left.Seconds >= right.Seconds;
}

FORCEINLINE bool operator<( const FDNCTimeStamp & left, const FDNCTime & time )
{
    return left.Seconds < time.ToSeconds();
}

FORCEINLINE bool operator<=( const FDNCTimeStamp & left, const FDNCTime & time )
{
    return left.Seconds <= time.ToSeconds();
}

FORCEINLINE bool operator>( const FDNCTimeStamp & left, const FDNCTime & time )
{
    return left.Seconds > time.ToSeconds();
}

FORCEINLINE bool operator>=( const FDNCTimeStamp & left, const FDNCTime & time )
{
    return left.Seconds >= time.ToSeconds();
}

FORCEINLINE FArchive & operator<<( FArchive & archive, FDNCTimeStamp & time )
{
    archive << time.Seconds;
    return archive;
}

UENUM()
enum class EDNCPeriodTransitionUnits : uint8
{
    RealWorld,
    Game
};

UCLASS( Blueprintable )
class DAYNIGHTCYCLE_API UDNCPeriod : public UDataAsset
{
    GENERATED_BODY()

public:
    UDNCPeriod() = default;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Meta = ( GameplayTagFilter = "DayNightCycle.Period" ) )
    FGameplayTag PeriodTag;

    // When does the period start?
    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    FDNCTime StartTime;

    // How much (real world) time this period should last?
    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    FDNCTime RealTimeDuration;

    UPROPERTY( EditAnywhere )
    EDNCPeriodTransitionUnits TransitionDurationUnits;

    // How much (game) time a transition lasts before switching to the next period
    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( EditCondition = "TransitionDurationUnits == EDNCPeriodTransitionUnits::Game" ) )
    FDNCTime TransitionDuration;

    // How much (real) time a transition lasts before switching to the next period
    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( EditCondition = "TransitionDurationUnits == EDNCPeriodTransitionUnits::RealWorld" ) )
    float TransitionDurationInSeconds;

    UPROPERTY( EditDefaultsOnly, Instanced, meta = ( ShowOnlyInnerProperties ) )
    TArray< TObjectPtr< UDNCPeriodObserver > > Observers;
};

FORCEINLINE bool operator<( const UDNCPeriod & left, const UDNCPeriod & right )
{
    return left.StartTime < right.StartTime;
}

UCLASS( BlueprintType )
class DAYNIGHTCYCLE_API UDNCDayDefinition final : public UDataAsset
{
    GENERATED_BODY()

public:
    UDNCDayDefinition();

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    FDNCTime FullDayDuration;

    // At what hour does the first cycle start
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    FDNCTime CycleStart;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    bool bStartsPaused;

    // The various periods of time to specify specific time dilation to make the time flow more or less fast
    UPROPERTY( EditDefaultsOnly, Category = "Observers" )
    TArray< TObjectPtr< UDNCPeriod > > Periods;

    UPROPERTY( EditDefaultsOnly, Instanced, Category = "Observers", meta = ( ShowOnlyInnerProperties ) )
    TArray< TObjectPtr< UDNCDayObserver > > Observers;
};

UCLASS( abstract, DefaultToInstanced, EditInlineNew, hidecategories = Object, CollapseCategories )
class DAYNIGHTCYCLE_API UDNCObserver : public UObject
{
    GENERATED_BODY()

public:
    void SetWorld( UWorld * world );
    UWorld * GetWorld() const override;

private:
    UPROPERTY()
    TObjectPtr< UWorld > World;
};

UCLASS( abstract, Blueprintable, DefaultToInstanced, EditInlineNew, hidecategories = Object, CollapseCategories )
class DAYNIGHTCYCLE_API UDNCDayObserver final : public UDNCObserver
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintImplementableEvent )
    void OnCycleStarted( UDNCDayDefinition * day_definition );

    // Return true if the event was processed and can be skipped until the next day cycle
    UFUNCTION( BlueprintImplementableEvent )
    bool OnCurrentTimeChanged( UDNCPeriod * period, const FDNCTimeStamp & time );

    UFUNCTION( BlueprintImplementableEvent )
    void OnPeriodStarted( UDNCPeriod * period );

    UFUNCTION( BlueprintImplementableEvent )
    void OnPeriodEnded( UDNCPeriod * period );

    UFUNCTION( BlueprintImplementableEvent )
    void OnTransitionStarted( UDNCPeriod * from_period, UDNCPeriod * to_period, FDNCTimeStamp transition_duration );

    UFUNCTION( BlueprintImplementableEvent )
    void OnTransitionEnded( UDNCPeriod * from_period, UDNCPeriod * to_period, FDNCTimeStamp transition_duration );

    UFUNCTION( BlueprintImplementableEvent )
    void OnTransitionUpdate( UDNCPeriod * from_period, UDNCPeriod * to_period, FDNCTimeStamp transition_time, FDNCTimeStamp transition_duration );
};

UCLASS( abstract, Blueprintable, DefaultToInstanced, EditInlineNew, hidecategories = Object, CollapseCategories )
class DAYNIGHTCYCLE_API UDNCPeriodObserver final : public UDNCObserver
{
    GENERATED_BODY()

public:
    // Return true if the event was processed and can be skipped until the next day cycle
    UFUNCTION( BlueprintImplementableEvent )
    bool OnCurrentTimeChanged( UDNCPeriod * period, const FDNCTimeStamp & time );

    UFUNCTION( BlueprintImplementableEvent )
    void OnPeriodStarted( UDNCPeriod * period );

    UFUNCTION( BlueprintImplementableEvent )
    void OnPeriodEnded( UDNCPeriod * period );

    UFUNCTION( BlueprintImplementableEvent )
    void OnTransitionStarted( UDNCPeriod * from_period, UDNCPeriod * to_period, FDNCTimeStamp transition_duration );

    UFUNCTION( BlueprintImplementableEvent )
    void OnTransitionEnded( UDNCPeriod * from_period, UDNCPeriod * to_period, FDNCTimeStamp transition_duration );

    UFUNCTION( BlueprintImplementableEvent )
    void OnTransitionUpdate( UDNCPeriod * from_period, UDNCPeriod * to_period, FDNCTimeStamp transition_time, FDNCTimeStamp transition_duration );
};