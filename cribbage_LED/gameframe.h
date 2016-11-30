#ifndef GAME_H
#define GAME_H

#include "crib_globals.h"

// state machine with common features (Players, Queue for turn history, etc)
// implemented as a parent class with children for each state and static
// common features
namespace Game
{
	class Frame;
	class State;
	class Player
	{
	public:
		Player();
		void setNumPlayers();
		Player* next();
	private:
		unsigned score;	// player's current score
		char pNum;	// unique player ID number
	};

	class Turn
	{
	public:
		Turn();
		Turn(Player* plr, unsigned score); // I don't know if this will be useful...
		Player* plr;
		unsigned score;
	};
	class Queue
	{
		Queue();
		void addTurn(Player *plr, unsigned score);
		void undo();
		void redo();
	private:
		unsigned currTurn;
		Turn turns[MAX_TURNS*MAX_PLAYERS]; // allocate space for a command history
	};
	class State
	{
	public:
		virtual void handleInput(Game::Frame& gameFrame);
		virtual void update(Game::Frame& gameFrame);
	};
	class Frame
	{
	public:
		virtual void handleInput();
	private:
		// individual player registry
		Player players[MAX_PLAYERS];
	};
}
#endif
