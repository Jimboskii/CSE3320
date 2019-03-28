/*
  Name: Hunter Nghiem
  ID:   1001275883

  Name: Nhan Lam
  ID:   1001506478

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
#define MAX_NUM_ARGUMENTS 10    // Mav shell only supports ten arguments

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

//function to find the logical block address offset.
int LBAToOffset(int32_t sector)
{
  return ((sector - 2) * BPB_BytsPerSec) + (BPB_BytsPerSec * BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec);
}

int root_address;

//the structure of the entries
typedef struct __attribute__((__packed__)) DirectoryEntry{
  char      DIR_Name[11];
  uint8_t   DIR_Attr;
  uint8_t   Unused1[8];
  uint16_t  DIR_FirstClusterHigh;
  uint8_t   Unused2[4];
  uint16_t  DIR_FirstClusterLow;
  uint32_t  DIR_FileSize;
}DirectoryEntry;

DirectoryEntry directory[16];
char formatName[16][12];

FILE *root_file_system;
bool isFATOpen = false;
char * formatToken;
char* fileName;
char* ext;
char* token;
char TempName[16][12];
int address;
char readin[1];

int main()
{
  //char points to cmd_str to alloc the block for the command MAX_COMMAND_SIZE
  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {

    // Print out the msh prompt
    printf ("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int token_count = 0;


    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;

    //makes duplicate of the original string so that way it will keep
    //original as you parse the other strings
    char *working_str  = strdup( cmd_str );


    // Tokenize the input stringswith whitespace used as the delimiter
    while (((arg_ptr = strsep(&working_str, WHITESPACE)) != NULL) && (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);

        if (token[0][0] == '\n')
        {
          break;
        }

        else if(strlen(token[token_count]) == 0)
        {
          memset(token[token_count],'\0',MAX_COMMAND_SIZE);
          //made so the token will not skip one token count,
          //cause while you set the null terminator to 0, it should take the NULL
          //out of the token, which will also take out the null after entering inputs
          //also taking the token count to -- to eliminate the null and then increase it after
          //to safely get a token that will not produce a null
          token_count--;
        }
        token_count++;
      }
      //NULL terminate array for exec functions
      token[token_count] = NULL;
      if (token[0] == NULL)
      {
        //do nothing
      }
      else if(token[0] != NULL)
      {
        //---------------quit and exit function used to terminate program----------
        if(!strcmp(token[0],"quit") || !strcmp(token[0] , "exit"))
        {
          return 0;
        }
////////////// Opening a File
        else if(!strcmp(token[0],"open") && isFATOpen == false)
        {
          root_file_system = fopen(token[1],"rb");
          isFATOpen = true;
          printf("File %s opened.\n",token[1]);

          if(root_file_system == NULL)
          {
            perror("Error in opening the image: ");
          }
          else
          {
            //fseeks and read of all the components for the fat32
            fseek(root_file_system,3,SEEK_SET);
            fread(&BS_OEMName,8,1,root_file_system);

            fseek(root_file_system,11,SEEK_SET);
            fread(&BPB_BytsPerSec,2,1,root_file_system);

            fseek(root_file_system,13,SEEK_SET);
            fread(&BPB_SecPerClus,1,1,root_file_system);

            fseek(root_file_system,14,SEEK_SET);
            fread(&BPB_RsvdSecCnt,2,1,root_file_system);

            fseek(root_file_system,16,SEEK_SET);
            fread(&BPB_NumFATs,1,1,root_file_system);

            fseek(root_file_system,17,SEEK_SET);
            fread(&BPB_RootEntCnt,2,1,root_file_system);

            fseek(root_file_system,71,SEEK_SET);
            fread(&BS_VolLab,11,1,root_file_system);

            fseek(root_file_system,36,SEEK_SET);
            fread(&BPB_FATSz32,4,1,root_file_system);

            fseek(root_file_system,44,SEEK_SET);
            fread(&BPB_RootClus,4,1,root_file_system);

            //equation to find the root address
            root_address = (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec) + (BPB_RsvdSecCnt * BPB_BytsPerSec);
            fseek(root_file_system, root_address, SEEK_SET);
            fread(directory, 32 * 16, 1, root_file_system);

            int i;
            for (i = 0; i < 16; i++)
            {
              //checking the correct attribute of the file names, if it does not match it is garbage
              if (directory[i].DIR_Attr == 0x01 || directory[i].DIR_Attr == 0x10 || directory[i].DIR_Attr == 0x20)
              {
                  memcpy(formatName[i], directory[i].DIR_Name, 11);
                  formatName[i][11] = '\0';
                  memcpy(TempName[i],formatName[i],12);
              }
            }
          }
        }
        //funtion to close the fat32 file
        else if(!strcmp(token[0],"close"))
        {
          isFATOpen = false;
          fclose(root_file_system);
          printf("Close Successful.\n");
        }
        //this else if only is entered if the fat32 file is open
        else if (isFATOpen)
        {
					//information function to find the hex and dec of each componemts of the fat32
          if(!strcmp(token[0],"info")&&isFATOpen)
          {
            printf("BPB_BytsPerSec: %x in hexadecimal and %d in base 10.\n", BPB_BytsPerSec, BPB_BytsPerSec);
            printf("BPB_SecPerClus: %x in hexadecimal and %d in base 10.\n", BPB_SecPerClus, BPB_SecPerClus);
            printf("BPB_RsvdSecCnt: %x in hexadecimal and %d in base 10.\n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);
            printf("BPB_NumFATs   : %x in hexadecimal and %d in base 10.\n", BPB_NumFATs, BPB_NumFATs);
            printf("BPB_FATSz32   : %x in hexadecimal and %d in base 10.\n", BPB_FATSz32, BPB_FATSz32);
          }
					//stat function that retrieves the attribute and cluster number of a filename or directory
          if(!strcmp(token[0],"stat") && isFATOpen && token[1]!=NULL)
          {
            //This part of the code is the checking of each character of the input and checking it with every file
            //and matching it with that file.
            //This structure is repeated for the function get, read, and cd.
            bool fileNotFound;
            int i;
            for(i =0; i <16 ; i++)
            {
              fileName = strtok(TempName[i], " \t");
              ext = strtok(NULL, " \t");

              if(ext != NULL)
              {
                //concatating the file name with a period and an extention.
                char text[] = ".";
                strcat(text,ext);
                strcat(fileName,text);
              }
              if(fileName != NULL)
              {
                strcpy(TempName[i],fileName);
              }
              char capitalName[16];
              strcpy (capitalName,token[1]);
              char * txt = capitalName;
              while(*txt)
              {
                //changing the txt to uppercase to check with the files.
                *txt = toupper(*txt);
                txt++;
              }
              if(token[1] == NULL)
              {
                printf("Error: File not found.");
              }
              if(strcmp(capitalName,TempName[i]) == 0)
              {
                printf("Attribute: %d\n", directory[i].DIR_Attr);
                printf("Cluster Number: %d\n", directory[i].DIR_FirstClusterLow);
                fileNotFound = false;
              }
            }
            if(fileNotFound)
            {
              //printf("Error: File not found.\n");
              fileNotFound = true;
            }
          }
					//The get function that retrieves the file from the Fat 32 file and place it in
          //the current working directory
          if(!strcmp(token[0],"get")&&isFATOpen)
          {
            int i;
            for(i =0; i <16 ; i++)
            {
              fileName = strtok(TempName[i], " \t");
              ext = strtok(NULL, " \t");

              if(ext != NULL)
              {
                char text[] = ".";
                strcat(text,ext);
                strcat(fileName,text);
              }
              if(fileName != NULL)
              {
                strcpy(TempName[i],fileName);
              }
              char capitalName[16];
              strcpy (capitalName,token[1]);
              //take the file names, set them to uppercase and then puts spaces in between , so that way we will be able to find
              //the files that are in the fat32.img
              char * txt = capitalName;
              while(*txt)
              {
                *txt = toupper(*txt);
                txt++;
              }

              //writes to the FAT, so that way we can get to file we want to get, then save it to the new file system
              if(strcmp(capitalName,TempName[i]) == 0)
              {
                address = LBAToOffset(directory[i].DIR_FirstClusterLow);
                FILE *new_file_system2 = fopen(capitalName, "w");

                int size_of_file = directory[i].DIR_FileSize;
                fseek(root_file_system, address, SEEK_SET);

                int i;
                for ( i = 0; i < size_of_file  ; i++)
                {
                fread(&readin, 1, 1, root_file_system);
                fprintf(new_file_system2, "%c", readin[0]);
                }

                fclose(new_file_system2);
              }
            }
          }
					//The cd funtion changes the current working directory to the given directory
          if(!strcmp(token[0],"cd")&&isFATOpen)
          {

            int i;
            for(i =0; i <16 ; i++)
            {
              fileName = strtok(TempName[i], " \t");
              ext = strtok(NULL, " \t");

              if(ext != NULL)
              {
                char text[] = ".";
                strcat(text,ext);
                strcat(fileName,text);
              }
              if(fileName != NULL)
              {
                strcpy(TempName[i],fileName);
              }
              char capitalName[16];
              strcpy (capitalName,token[1]);
              char * txt = capitalName;
              while(*txt)
              {
                *txt = toupper(*txt);
                txt++;
              }
              if(strcmp(capitalName,TempName[i]) == 0)
              {
                int newaddress = 0;
                if (directory[i].DIR_FirstClusterLow == 0)
                {
                  newaddress = root_address;
                }
                else
                {
                  newaddress = LBAToOffset(directory[i].DIR_FirstClusterLow);
                }

                fseek(root_file_system, newaddress, SEEK_SET);

                for(i =0; i < 16; i++)
                {
                  fread(&directory[i], sizeof(DirectoryEntry), 1, root_file_system);
                }

                for (i = 0; i < 16; i++)
                {
                      memcpy(formatName[i], directory[i].DIR_Name, 11);
                      formatName[i][11] = '\0';
                      memcpy(TempName[i],formatName[i],12);
                }
              }
            }
          }
					//The ls list the current directory contents
          if(!strcmp(token[0],"ls")&&isFATOpen)
          {
            int i;
            for (i = 0; i < 16; i++)
            {
              //filtering the files to get rid of garbage
              if (directory[i].DIR_Attr == 0x01 || directory[i].DIR_Attr == 0x10 || directory[i].DIR_Attr == 0x20)
              {
                if((formatName[i][0] >= 'a' && formatName[i][0] <= 'z') || (formatName[i][0] >= 'A' && formatName[i][0] <= 'Z'))
                {
                  char formatted[12];
                  formatted[11] = '\0';
                  memcpy(formatted, directory[i].DIR_Name, 11);
                  printf("%s\n", formatted);
                }
              }
            }
          }
					//The read function reads from the given file at a giving position and specified the
          //position parameter and output the number of bytes specified.
          if(!strcmp(token[0],"read")&&isFATOpen)
          {
            int i;
            for(i =0; i <16 ; i++)
            {
              fileName = strtok(TempName[i], " \t");
              ext = strtok(NULL, " \t");

              if(ext != NULL)
              {
                char text[] = ".";
                strcat(text,ext);
                strcat(fileName,text);
              }
              if(fileName != NULL)
              {
                strcpy(TempName[i],fileName);
              }
              char capitalName[16];
              strcpy (capitalName,token[1]);
              char * txt = capitalName;
              while(*txt)
              {
                *txt = toupper(*txt);
                txt++;
              }
              if(strcmp(capitalName,TempName[i]) == 0)
              {
                address = LBAToOffset(directory[i].DIR_FirstClusterLow);
                if(token[2] == NULL)
                {
                  fseek(root_file_system, address, SEEK_SET);
                }
                if(token[2] != NULL)
                {
                  fseek(root_file_system, address+atoi(token[2]), SEEK_SET);
                }
                int i;
                for ( i = 0; i < atoi(token[3])  ; i++)
                {
                fread(&readin, 1, 1, root_file_system);
                printf("%d\n", readin[0]);
                }
              }
          }
        }
        //The volume function prints the volume name of the FAT file.
        if(!strcmp(token[0],"volume") && isFATOpen)
        {
          if(strlen(BS_VolLab) == 0)
          {
            printf("Error: volume name not found.");
          }
          else
          {
            printf("Volume Name is '%s'\n", BS_VolLab);
          }
        }
      }
      free( working_str );
    }
  }
    return EXIT_SUCCESS;
}
