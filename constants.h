/******************************************************************************
* CONSTANTS
******************************************************************************/

#ifndef CONSTANTS_H
#define CONSTANTS_H

const int DEFAULT_PORT = 6789;
const int ERROR_BAD = -1;
const int ERROR_OK = 0;
const int MAXLEN = 256; // size of the buffer

// gets rid of "deprecated conversion from string constant ... compiler warning
#pragma GCC diagnostic ignored "-Wwrite-strings"

// OPTIONS
#define ROCK    'r'
#define PAPER   'p'
#define SCISSOR 's'
#define QUIT    'q'

// commands
#define GET_NAME "NAME" // prompt for the name of the player
#define SET_OPPONENT "OPNT" // let player 'x' know who player 'y' is
#define TURN "ROUND" // start a new round ~ get input from players
#define WIN "RWIN"   // player 'x' won this round
#define LOSS "RLOSS" // player 'y' lost this round
#define DRAW "RDRAW" // this round was a tie
#define DC "PDC"     // one of the players disconnected or quit

// integer representation of the commands above
enum codes {
   NAME,
   OPNT,
   ROUND,
   RWIN,
   RLOSS,
   RDRAW,
   PDC
};

// used to access the array of integers for each player
enum results { WINS, LOSSES, DRAWS };

// the integer representing who won that round
enum roundResults { P1 = -1, TIE = 0, P2 = 1 };

#endif
