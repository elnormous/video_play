//
//  native_play
//

#include "Player.h"

ouzel::Engine engine;
std::unique_ptr<Player> player;

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
        player.reset(new Player(args[1]));
    }
}
