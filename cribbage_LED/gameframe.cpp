#include "gameframe.h"

Game::Turn::Turn()
{
	this->plr = 0;
	this->score = 0;
}
Game::Queue::Queue()
{
	// don't need to init the turns b/c it's done in the Turn constructor?

}
