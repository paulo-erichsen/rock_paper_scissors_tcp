/******************************************************************************
* Program:
*    Lab RockClient, Rock/Paper/Scissors with Sockets - Client Code
* Author:
*    Paulo Fagundes
* Summary:
*    The Client attempts to connect to the game server. When the connection is
*    stablished, the server sends instructions to the Client, which the client
*    interprets them and then acts according to the commands sent.
*    See the Constants to learn about the commands, and see run() to
*    understand how the client reads from the server and responds to it.
******************************************************************************/
// #include <cstdlib>
// #include <netinet/in.h> // sockaddr_in
// #include <sys/socket.h>
// #include <sys/types.h>

#include <cstring> // memcpy, bcopy, strcmp
#include <iostream> // cout
#include <netdb.h> // gethostbyname
#include <sstream> // stringstream
#include <unistd.h> // gethostname, close

#include "constants.h"
#include "helpers.h"
#include "client.h"

using namespace std;

/******************************************************************************
* MAIN
* argv: host_name, port_number
******************************************************************************/
int main(int argc, char** argv)
{
   char host[MAXLEN];
   int port;

   parseClientArgs(argc, argv, host, port);

   Client client(host, port);
   client.run();

   return 0;
}

/******************************************************************************
* Client Constructor - attempts to connect to hostname on the given port number
******************************************************************************/
Client::Client(char* hostname, int port)
{
   socketFD = connectToServer(hostname, port);
   playerName = new char[MAXLEN];
   opponentName = new char[MAXLEN];
   results = new int[3]; // wins/losses/draws
   fill(results, results + 3, 0); // set the elements of results to '0'
}

/******************************************************************************
* Client Destructor
******************************************************************************/
Client::~Client()
{
   delete [] playerName;
   delete [] opponentName;
   delete [] results;

   // make sure to close the socket
   shutdown(socketFD, SHUT_RDWR);
   close(socketFD);
}

/******************************************************************************
* run() - loop forever, getting commands from the server, and acting according
*         to those commands.
******************************************************************************/
void Client::run()
{
   int opCode = -1;
   char buffer[MAXLEN];
   bool gameIsSet = false; // true after name is prompted and opponent given
   bool isFirstRound = true;
   bool running = true;

   while(running)
   {
      if (isFirstRound && gameIsSet)
         updateDisplay();

      // read the command from the server
      read_data(socketFD, buffer);
      opCode = parseCommand(buffer);
      switch(opCode)
      {
         case NAME:
            handlePlayerName();
            break;
         case OPNT:
            handleOpponentName();
            gameIsSet = true;
            break;
         case ROUND:
            handleRoundOption(isFirstRound);
            break;
         case RWIN:
         case RLOSS:
         case RDRAW:
            handleRoundResult(opCode);
            break;
         case PDC:
            running = false;
            cout << "Server closed by Player, or server error.\n";
            cout << "\nYour final results: ( W / L / D )" << endl
                 << "                      " << results[WINS] << " / "
                 << results[LOSSES] << " / "
                 << results[DRAWS] << endl;
            break;
         default:
            cout << "Failed to parse the command from the Server!\n";
      }
   }
}

/******************************************************************************
* connectToServer() - attempts to establish a connection between the client
*                     and the server. If successful, saves the Server's
*                     socket File Descriptor. If it fails, exit the program.
******************************************************************************/
int Client::connectToServer(char* hostname, int port)
{
   int socketFD;
   struct sockaddr_in socketAddress;
   struct hostent* hostEntry;

   socketFD = socket(AF_INET, SOCK_STREAM, 0); // domain, type, default prtocol
   if (socketFD == ERROR_BAD)
   {
      exitErr("error on creating socket");
   }

   if ((hostEntry = gethostbyname(hostname)) == NULL)
   {
      exitErr("can't find host");
   }

   socketAddress.sin_family = AF_INET;
   // memcpy(&(socketAddress.sin_addr),
   //        hostEntry->h_addr_list[0], hostEntry->h_length);
   bcopy((char*)hostEntry->h_addr, (char*)& socketAddress.sin_addr,
         hostEntry->h_length);
   socketAddress.sin_port = htons(port);

   if (connect(socketFD, (struct sockaddr *)&socketAddress,
               sizeof(socketAddress)) != ERROR_OK)
   {
      exitErr("Failed to connect to the Server");
   }

   cout << "Successfully connected to the Server\n";
   return socketFD;
}

/******************************************************************************
* parseCommand() - returns the integer representation of the given command
******************************************************************************/
int Client::parseCommand(char* cmd)
{
   int opCode = -1;
   if (strcmp(cmd, "NAME") == 0)
      opCode = NAME;
   else if(strcmp(cmd, "OPNT") == 0)
      opCode = OPNT;
   else if(strcmp(cmd, "ROUND") == 0)
      opCode = ROUND;
   else if(strcmp(cmd, "RWIN") == 0)
      opCode = RWIN;
   else if(strcmp(cmd, "RLOSS") == 0)
      opCode = RLOSS;
   else if(strcmp(cmd, "RDRAW") == 0)
      opCode = RDRAW;
   else if(strcmp(cmd, "PDC") == 0)
      opCode = PDC;
   return opCode;
}

/******************************************************************************
* handlePlayerName() - prompts the player for a name and then sends it to the
*                      server
******************************************************************************/
void Client::handlePlayerName()
{
   // prompt user for his name, send it to the server
   cout << "What is your name?\n>";
   cin.getline(playerName, MAXLEN);

   write_data(socketFD, playerName);
}

/******************************************************************************
* handleOpponentName() - reads from the server the opponent's name and then
*                        displays it to the user
******************************************************************************/
void Client::handleOpponentName()
{
   read_data(socketFD, opponentName);
   cout << "Your opponent for this game is '" << opponentName << "'\n";
}

/******************************************************************************
* handleRoundOption() - allows the user to input his/her choice. Then send that
*                       choice to the server if it's a R/P/S/Q.
******************************************************************************/
void Client::handleRoundOption(bool& isFirstRound)
{
   char option = 'z';
   if (isFirstRound)
      displayOptions();

   // PROMPT
   while(!isValidOption(option))
   {
      cout << "Enter a single character:\n>";
      cin >> option;
      option = tolower(option);

      if (!isValidOption(option))
      {
         cout << "\nInvalid entry!\n";
         displayOptions();
      }
   }

   isFirstRound = false;

   if (option == 't')
   {
      updateDisplay();
      handleRoundOption(isFirstRound); // this recursion should be safe :)
   }
   else if (option == '?')
   {
      displayOptions();
      handleRoundOption(isFirstRound);
   }
   else
      write_data(socketFD, &option);    // Send the Option back to the server
}

/******************************************************************************
* handleRoundResult() - updates the score, reads from the server the result
*                       message and then displays it to the user
******************************************************************************/
void Client::handleRoundResult(int resultType)
{
   char buffer[MAXLEN];
   // set the resultType to contain the appropriate index
   if      (resultType == RWIN)  resultType = WINS;
   else if (resultType == RLOSS) resultType = LOSSES;
   else if (resultType == RDRAW) resultType = DRAWS;

   // update the score
   results[resultType]++;

   // get message on the result from the server and display it
   read_data(socketFD, buffer);
   cout << "Result: " << buffer << endl;
}

/******************************************************************************
* updateDisplay() - displays the game Statistics ~ names of players, score
******************************************************************************/
void Client::updateDisplay()
{
   cout << "--------------------------------------------\n";
   cout << "Player:   " << playerName << endl;
   cout << "Opponent: " << opponentName << endl;
   cout << "Your results: ( W / L / D )" << endl
        << "                " << results[WINS] << " / "
        << results[LOSSES] << " / "
        << results[DRAWS] << endl;
   cout << endl;
}

/******************************************************************************
* isValidOption()
******************************************************************************/
bool Client::isValidOption(char option)
{
   return (option == ROCK || option == PAPER || option == SCISSOR ||
           option == QUIT || option == 't'   || option == '?');
}

/******************************************************************************
* displayOptions()
******************************************************************************/
void Client::displayOptions()
{
   cout << "Options:\n"
        << " r - Rock\n"
        << " p - Paper\n"
        << " s - Scissors\n"
        << " t - Display stats of the game\n"
        << " ? - Display these options\n"
        << " q - Quit\n";
}
