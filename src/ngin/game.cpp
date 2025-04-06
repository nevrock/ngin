#include <ngin/game.h>

std::string Game::state_;         // Define and initialize the static state variable
Event Game::eventState_;          // Define the static eventState variable
std::shared_ptr<Lex> Game::game_; 
bool Game::running_;         // Define the static eventState variable
