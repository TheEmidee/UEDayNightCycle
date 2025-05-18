#include "DayNightCycle.h"

#include "DNCWorldSubsystem.h"

#include <Engine/Canvas.h>
#include <GameFramework/HUD.h>

#define LOCTEXT_NAMESPACE "FDayNightCycleModule"

void FDayNightCycleModule::StartupModule()
{
    if ( !IsRunningDedicatedServer() )
    {
#if ENABLE_DRAW_DEBUG
        AHUD::OnShowDebugInfo.AddStatic( &FDayNightCycleModule::OnShowDebugInfo );
#endif
    }
}
void FDayNightCycleModule::ShutdownModule()
{
}

void FDayNightCycleModule::OnShowDebugInfo( AHUD * hud, UCanvas * canvas, const FDebugDisplayInfo & /*display_info*/, float & /*y_line*/, float & /*y_position*/ )
{
    static const FName NAME_DayNightCycle( "DayNightCycle" );

    if ( canvas != nullptr )
    {
        if ( hud->ShouldDisplayDebug( NAME_DayNightCycle ) )
        {
            auto & display_debug_manager = canvas->DisplayDebugManager;
            display_debug_manager.SetFont( GEngine->GetSmallFont() );
            display_debug_manager.SetDrawColor( FColor::Yellow );
            display_debug_manager.DrawString( TEXT( "DAY NIGHT CYCLE" ) );

            if ( auto * system = hud->GetWorld()->GetSubsystem< UDNCWorldSubsystem >() )
            {
                system->ShowDebugInfo( canvas );
            }
        }
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FDayNightCycleModule, DayNightCycle )