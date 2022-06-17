/*
* Game engine foundation
* @author: Professor Dave Churchill at Memorial University and Justin Goding
* @version date: 2/4/2022
* */

#include <SFML/Graphics.hpp>

#include "Game_Engine.h"

int main()
{
    Game_Engine g("assets.txt");
    g.run();
}