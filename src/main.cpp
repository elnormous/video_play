//
//  native_play
//

#include "Application.h"

ouzel::Engine engine;
Application application;

void ouzelMain(std::vector<std::string> const& args)
{
    ouzel::Settings settings;
    settings.size = ouzel::Size2(800.0f, 600.0f);
    settings.resizable = true;

    engine.init(settings);

    application.begin();
}
