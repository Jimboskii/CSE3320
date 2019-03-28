/*
Jimmy Pham
1001193667
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports five arguments



//Global variables
FILE *fp;
int ImageOpened = 0; //0 meaning that it is closeed
                     //1 meaning that it is opened

char       BS_OEMName[8];
int16_t    BPB_BytsPerSec;
int8_t     BPB_SecPerClus;
int16_t    BPB_RsvdSecCnt;
int8_t     BPB_NumFATs;
int16_t    BPB_RootEntCnt;
char       BS_VolLab[11];
int32_t    BPB_FATSz32;
int32_t    BPB_RootClus;

int32_t    RootDirSectors = 0;
int32_t    FirstDataSector = 0;
int32_t    FirstSectorofCluster = 0;




struct __attribute__((__packed__)) DirectoryEntry
{
  char DIR_Name[11];
  uint8_t DIR_Attr;
  uint8_t Unused1[8];
  uint16_t DIR_FirstClusterHigh;
  uint8_t Unused2[4];
  uint16_t DIR_FirstClusterLow;
  uint32_t DIR_FileSize;
};
struct DirectoryEntry dir[16];








int main()
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {
    // Print out the mfs prompt
    printf ("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;

    char *working_str  = strdup( cmd_str );

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality

/////Does nothing if there is no input
if(token[0] == NULL)
{
    continue;
}
    //////////Exits program
    if((strcmp("exit", token[0]) == 0)||(strcmp("quit", token[0]) == 0))
    {
        exit(0);
    }

    /////OPENING A FILE
    else if((strcmp("open", token[0]) == 0) && ImageOpened == 0)
    {
      //filename is user input
        // printf("here\n");
        fp = fopen(token[1], "r");
        if(fp != NULL)
        {
          ImageOpened = 1;
        }

        if(fp == NULL)
        {
          printf("Error: File system image not found.\n");
        }
    }


    ////////Closing a File
    else if((strcmp("close", token[0]) == 0) && (ImageOpened == 1))
    {

      if(fp == NULL)
      {
        perror("Error: File system not open.\n");
      }
      else if(fp != NULL)
      {
        ImageOpened = 0;
        fclose(fp);
        printf("File Closed\n");
      }





    free( working_root );

  }
}
return 0;
  }
