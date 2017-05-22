//
//  native_play
//

#include "PlayerLibav.h"

ouzel::Engine engine;
std::unique_ptr<PlayerLibav> player;

void ouzelMain(std::vector<std::string> const& args)
{
    if (args.size() < 2)
    {
        ouzel::Log() << "Too few arguments";
        return;
    }

    ouzel::Settings settings;
    settings.size = ouzel::Size2(800.0f, 600.0f);
    settings.resizable = true;

    if (engine.init(settings))
    {
        player.reset(new PlayerLibav(args[1]));
    }
}
