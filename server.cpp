/******************************************************************************
* Program:
*    Lab RockSrv, Rock/Paper/Scissors with Sockets - Server Code
* Author:
*    Paulo Fagundes
* Summary:
*    The server waits until players(clients) connect to it and then, when there
*    are 2*n players connected, it starts a game of Rock / Paper / Scissors
*    for the 2 clients.
*    The Protocol used in this assignment:
*    - The Server sends commands to the client such as "NAME", "OPNT", "ROUND",
*      and others. These commands are defined in the constants.
*    - The Client in turn listens for such commands, and act upon them.
*    - For instance, Server sends GET_NAME ("NAME"). The Client reads that in,
*      understands the command, prompts the user for a user name, then sends
*      that data back to the Server.
*    - Pretty much every command the server gives to the client is executed
*      from the loop found in play(). See play() and run() in the Client's side
******************************************************************************/
// #include <cstdlib>
// #include <sys/socket.h>
// #include <sys/types.h>

#include <cstring>  // memcpy
#include <iostream> // cout
#include <netdb.h>  // gethostbyname, hostent
#include <netinet/in.h> // sockaddr_in
#include <sstream> // stringstream
#include <string>   // pop_back
#include <unistd.h> // gethostname
#include <vector>

#include "constants.h"
#include "helpers.h"
#include "server.h"

using namespace std;


/******************************************************************************
* MAIN
* argv: port number
******************************************************************************/
int main(int argc, char** argv)
{
   int port = DEFAULT_PORT;

   if (argc > 1)
   {
      // convert the argument to an integer
      // Note: could use atoi(), but it can give segmentation faults...
      stringstream ss(argv[1]);
      ss >> port;

      if (!ss)
      {
         port = DEFAULT_PORT; // set it again so that the port won't be '0'
         cout << "Failed to set the given port number. Using default port.\n";
      }
   }

   Server server(port);
   server.run();

   return 0;
}

/******************************************************************************
* Server constructor
******************************************************************************/
Server::Server(int port)
{
   openSocket(port);
}

/******************************************************************************
* Server destructor
******************************************************************************/
Server::~Server()
{
   // make sure to close the socket
   shutdown(socketFD, SHUT_RDWR);
   close(socketFD);
}

/******************************************************************************
* run() - runs the server. Loops forever, listening for new player players to
*         connect. When at least 2 players are available to play, start the
*         game for them, then resume listening for new incoming connections
******************************************************************************/
void Server::run()
{
   vector<Player*> players; // a vector of all connected players
   vector<Player*> idle; // a vector of idle players

   while(true)
   {
      // let players connect
      Player *player = NULL;
      player = getPlayer(); // let a player connect
      if (player)
         players.push_back(player);

      // if there are 2 players connected, but not playing, make them play
      getIdlePlayers(players, idle);
      if (idle.size() > 1)
      {
         // get the players that are not playing and make them "play"
         Player* p1 = idle.front();
         Player* p2 = idle.back();
         p1->isPlaying = true;
         p2->isPlaying = true;

         // fork the process, such that the server can keep listening for new
         // players....
         // NOTE: could multi-thread instead of fork... (might do that for T2)
         int pid = fork();
         if (pid == 0)
            play(p1, p2);
         else if (pid == ERROR_BAD)
         {
            cout << "FAILURE! Failed to fork the process\n";
         }
      }
   }

   // once done running, deallocate each one of the allocated players
   idle.clear();
   deletePlayers(players);
}

/******************************************************************************
* openSocket() - Opens the welcome socket and binds it to the given port
******************************************************************************/
void Server::openSocket(int port)
{
   struct sockaddr_in socketAddress;
   char hostname[MAXLEN];
   struct hostent* hostEntry;

   // create the socket
   socketFD = socket(AF_INET, SOCK_STREAM, 0); // domain, type, default prtocol

   // check if the socket got created appropriately
   if (socketFD == ERROR_BAD)
   {
      exitErr("error on creating socket");
   }

   // get the hostname and hostaddress (hostent) of the server
   gethostname(hostname, MAXLEN);
   hostEntry = gethostbyname(hostname);

   // builds the socket info - family, address, port number
   socketAddress.sin_family = AF_INET;
   // socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
   memcpy(&(socketAddress.sin_addr),
          hostEntry->h_addr_list[0], hostEntry->h_length);
   socketAddress.sin_port = htons(port);

   // binds the socket data to the socketFD
   if (bind(socketFD, (struct sockaddr *)&socketAddress,
            sizeof(socketAddress)) != ERROR_OK)
   {
      // handle the error
      exitErr("error on bind!");
   }

   // listen for connections on that port
   if (listen(socketFD, 1) != ERROR_OK)
   {
      // handle the error
      exitErr("error on listen!");
   }

   cout << "Socket opened successfully on port " << port << endl;
}

/******************************************************************************
* getPlayer() - listens, trying to accept() a connection from a new client.
*               In addition, when the client connects, prompt the client for
*                 the user name, saves that name and File Descriptor into the
*                 Player* and then return it.
******************************************************************************/
Player* Server::getPlayer()
{
   int clientFD = 0; // client File Descriptor / Socket Descriptor
   Player* player = NULL;

   // wait until a client connects
   clientFD = accept(socketFD, NULL, NULL);
   if (clientFD)
   {
      player = new Player;
      player->clientFD = clientFD;

      // let the Client know that we want a name from it
      write_data(player->clientFD, GET_NAME);

      // read the name from the client and store it into the Player*
      char buffer[MAXLEN] = "";
      read_data(player->clientFD, buffer);
      // TODO: validate the name he gave?? make sure there's a name?
      strcpy(player->name, buffer);
   }

   return player;
}

/******************************************************************************
* play() - starts and runs the game for the 2 given players
******************************************************************************/
void Server::play(Player* p1, Player* p2)
{
   // these buffers will store the inputs of the players
   char buffer1[MAXLEN] = "";
   char buffer2[MAXLEN] = "";

   // will store the result from the read operations
   int p1ReadResult;
   int p2ReadResult;

   // will store the player's inputs ~ r/p/s/q
   char p1Choice;
   char p2Choice;

   // will store the winner of this round
   int roundResult;

   cout << "--------------------------------------------\n";
   cout << "Starting a game - Process ID #" << getpid() << endl;
   cout << "Players: '" << p1->name << "' VS '" << p2->name << "'\n";

   // let the players know who their oponents are
   write_data(p1->clientFD, SET_OPPONENT);
   write_data(p2->clientFD, SET_OPPONENT);
   write_data(p1->clientFD, p2->name);
   write_data(p2->clientFD, p1->name);

   // loop, sending a TURN command to the players
   //  The TURN code means that the Server expects an input from the players.
   //  Valid Inputs are: r/p/s/q for Rock, Paper, Scissors, Quit
   // Then the server acts upon those results.
   while(true)
   {
      // ask the clients to send their inputs for this turn
      write_data(p1->clientFD, TURN);
      write_data(p2->clientFD, TURN);

      // read the player's inputs for this turn
      p1ReadResult = read_data(p1->clientFD, buffer1);
      p2ReadResult = read_data(p2->clientFD, buffer2);

      // store the player's inputs
      p1Choice = buffer1[0];
      p2Choice = buffer2[0];

      // if the inputs were either PAPER, ROCK or SCISSORS
      // then compute the results and send the results to the clients
      if (p1Choice != QUIT && p2Choice != QUIT &&
          p1ReadResult != ERROR_BAD && p2ReadResult != ERROR_BAD)
      {
         // compute the winner
         roundResult = getRoundResult(p1Choice, p2Choice);
         switch(roundResult)
         {
            case TIE :
               write_data(p1->clientFD, DRAW);
               write_data(p2->clientFD, DRAW);
               break;
            case P1 :
               write_data(p1->clientFD, WIN);
               write_data(p2->clientFD, LOSS);
               break;
            case P2 :
               write_data(p1->clientFD, LOSS);
               write_data(p2->clientFD, WIN);
               break;
            default:
               cout << "Error calculating the round result!\n";
         }
         // now that the DRAW / WIN LOSS was sent, build a nice string and
         // send it to both clients
         buildVerboseResult(p1Choice, p2Choice, buffer1, roundResult);
         buildVerboseResult(p2Choice, p1Choice, buffer2, flip(roundResult));
         write_data(p1->clientFD, buffer1);
         write_data(p2->clientFD, buffer2);
      }
      else
      {
         write_data(p1->clientFD, DC);
         write_data(p2->clientFD, DC);
      }
   }
}

/******************************************************************************
* iterates through the vector of players, checking who isn't playing.
* fills the idle vector with players that are not playing
******************************************************************************/
void Server::getIdlePlayers(const vector<Player*>& players,
                            vector<Player*>& idle)
{
   idle.clear();
   for (vector<Player*>::const_iterator cit = players.begin();
        cit != players.end(); ++cit)
   {
      if (!((*cit)->isPlaying))
      {
         idle.push_back(*cit); // if this player isn't playing, then push it...
      }
   }
}

/******************************************************************************
* deletePlayers() - deallocates all the players that were previously allocated
******************************************************************************/
void Server::deletePlayers(vector<Player*>& players)
{
   for (vector<Player*>::iterator it = players.begin(); it != players.end(); ++it)
   {
      if (*it)
      {
         delete *it;
      }
   }
   players.clear();
}

/******************************************************************************
* given the inputs ROCK / PAPER / SCISSOR for each player, return
* the winner of that round.
* possible outputs are: P1 / TIE / P2
******************************************************************************/
int Server::getRoundResult(char p1Choice, char p2Choice)
{
   int result = -99;

   if (p1Choice == ROCK)
   {
      if      (p2Choice == ROCK)    result = TIE;
      else if (p2Choice == PAPER)   result = P2;
      else if (p2Choice == SCISSOR) result = P1;
   }
   else if (p1Choice == PAPER)
   {
      if      (p2Choice == ROCK)    result = P1;
      else if (p2Choice == PAPER)   result = TIE;
      else if (p2Choice == SCISSOR) result = P2;
   }
   else if (p1Choice == SCISSOR)
   {
      if      (p2Choice == ROCK)    result = P2;
      else if (p2Choice == PAPER)   result = P1;
      else if (p2Choice == SCISSOR) result = TIE;
   }
   return result;
}

/******************************************************************************
* buildVerboseResult() - builds a string
******************************************************************************/
void Server::buildVerboseResult(char p1Choice, char p2Choice,
                                char* buffer, int result)
{
   string msg;
   string choice1 = getVerboseChoice(p1Choice);
   string choice2 = getVerboseChoice(p2Choice);

   if (result == TIE)
      msg = choice1 + " TIES against " + choice2 + "! Round DRAW!\n";
   else if(result == P1)
      msg = choice1 + " beats " + choice2 + "! You WIN!\n";
   else if (result == P2)
      msg = choice1 + " is beaten by " + choice2 + "! You LOSE!\n";
   else
   {
      cout << "Invalid player choice! This should not have happened!\n";
      cout << "the choice was: " << result << endl;
   }

   strcpy(buffer, msg.c_str()); // put the string into the buffer
}

/******************************************************************************
* getVerboseChoice() - returns the string representation of the choice
*  'r' -> "ROCK"
*  'p' -> "PAPER
*  's' -> "SCISSORS"
******************************************************************************/
string Server::getVerboseChoice(char choice)
{
   if (choice == ROCK) return "ROCK";
   return (choice == PAPER ? "PAPER" : "SCISSORS");
}

/******************************************************************************
* flip() - flips the result - if P1 won, return P2, vice-versa...
******************************************************************************/
int Server::flip(int result)
{
   int flippedResult = -99;
   if (result == P1)       flippedResult = P2;
   else if (result == P2)  flippedResult = P1;
   else if (result == TIE) flippedResult = TIE;
   return flippedResult;
}
