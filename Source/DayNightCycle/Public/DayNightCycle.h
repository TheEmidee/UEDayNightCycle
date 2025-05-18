#pragma once

#include <CoreMinimal.h>

class FDayNightCycleModule : public IModuleInterface
{
public:
    void StartupModule() override;
    void ShutdownModule() override;

private:
    static void OnShowDebugInfo( AHUD * hud, UCanvas * canvas, const FDebugDisplayInfo & display_info, float & y_line, float & y_position );
};
