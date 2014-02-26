#include <cstdlib>  // exit
#include <cstring>  // strlen
#include <iostream> // cout
#include <unistd.h> // read, write
#include "helpers.h"
#include "constants.h" // ERROR_BAD

/******************************************************************************
* HELPERS / UTILS
******************************************************************************/
// output the message and then exit with the error code
void exitErr(std::string msg)
{
   std::cout << msg << std::endl;
   exit(ERROR_BAD);
}

// read_data - reads data from the socket stream
int read_data (int fd , char* buffer )
{
   // temp is a char that represents the length of the message being sent.
   //   temp is converted to an integer to get the actual integer number
   char temp;
   int i = 0;
   int length = 0;

   // 1st character = Get the Length of the Message
   if ( read ( fd , &temp , 1 ) < 0 )
   {
      exit (-3) ;
   }
   length = temp ;

   // read the actual message. Reads $length chars
   while ( i < length )
   {
      if ( i < ( i+= read (fd , & buffer [i], length - i)))
      {
         exit (-3);
      }
   }
   return i; /* Return size of char* */
}

// write_data - writes data to the socket stream
int write_data ( int fd , const char* message )
{
   char temp;
   int length = 0;

   // get the length of the message and store it into a char
   length = strlen ( message ) + 1; // +1 to account for the '\0'
   temp = length ;

   // send a 1 byte (1 char) containing the length of the message
   if( write (fd , &temp , 1) < 0 )
   {
      exit (-2) ;
   }

   // now send the actual message
   if( write (fd , message , length ) < 0 )
   {
      exit (-2) ;
   }

   return length; // returns the length of the message that was sent
}

// reads the arguments from the client and assigns the host and the port
// to the appropriate parameters
void parseClientArgs(int argc, char** argv, char* host, int& port)
{
   // argv[1] should be hostname, argv[2] should be port
   if (argc > 2)
   {
      strcpy(host, argv[1]);
      port = atoi(argv[2]);
      if (!port)
         exitErr("Invalid port number");
   }
   else
   {
      std::cout << "Usage: " << argv[0] << " HOSTNAME PORT\n";
      exit(ERROR_BAD);
   }
}
