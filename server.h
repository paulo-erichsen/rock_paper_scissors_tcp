#ifndef CLIENT_H
#define CLIENT_H

#include "constants.h"

/******************************************************************************
* the Player struct
******************************************************************************/
struct Player
{
   bool isPlaying;    // flag indicating if that player is Playing a game
   int clientFD;      // client File Descriptor / Socket Descriptor
   char name[MAXLEN]; // the name of the player
};

/******************************************************************************
* Server Class
******************************************************************************/
class Server
{
   public:
      Server(int = DEFAULT_PORT);
      ~Server();
      void run();

   private:
      int socketFD; // the welcome socket File Descriptor

      // socket functionality
      void openSocket(int port); // opens the server welcome socket
      Player* getPlayer(); // accept() ~ listens for new players

      // game processing methods
      void play(Player* p1, Player* p2);
      void getIdlePlayers(const std::vector<Player*>& players,
                                std::vector<Player*>& idle);
      void deletePlayers(std::vector<Player*>& players);
      int getRoundResult(char p1Choice, char p2Choice);
      void buildVerboseResult(char p1Choice, char p2Choice,
                              char* buffer, int result);
      std::string getVerboseChoice(char choice);
      int flip(int result);
};

#endif
