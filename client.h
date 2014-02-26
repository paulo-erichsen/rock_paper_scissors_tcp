#ifndef CLIENT_H
#define CLIENT_H

/******************************************************************************
* Client Class
******************************************************************************/
class Client
{
   public:
      Client(char* hostname, int port);
      ~Client();
      void run();

   private:
      int socketFD; // the Server's socket File Descriptor
      char* playerName;
      char* opponentName;
      int* results;

      // socket functionality
      int connectToServer(char* hostname, int port);

      // client-server interaction / game functionality
      int parseCommand(char* cmd);
      void handlePlayerName();
      void handleOpponentName();
      void handleRoundOption(bool& isFirstRound);
      void handleRoundResult(int resultType);
      void updateDisplay();
      bool isValidOption(char option);
      void displayOptions();
};

#endif
