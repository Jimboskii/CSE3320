#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <ctype.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10     // Mav shell only supports ten arguments

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

struct directoryList
{
	int32_t directoryAddress;
	struct directoryList *next; //pointer to next node in directoryList
	struct directoryList *prev; //pointer to prevoius node in directoryList
}*head, *newNode;

char BS_OEMName[8];
int16_t BPB_BytsPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATs;
int16_t BPB_RootEntCnt;
char BS_VolLab[11];
int32_t BPB_FATSz32;
int32_t BPB_RootClus;

int32_t FirstDataSector = 0;
int32_t FirstSectorofCluster = 0;

FILE *fp;
FILE *outfile;
struct DirectoryEntry dir[16];

int32_t RootDirSectors = 0;
int32_t currentWD = 0;
int32_t previousWD = 0;

struct directoryList* allocateNode();
void appendNode(struct directoryList** head, struct directoryList* newNode);
void freeNodeList(struct directoryList* head);
void printFileInfo();
void ls(char* lsStatus);
void cd(struct directoryList** head, char* path);
void stat(char * str);
void volume();
void readFile(char * str, char * p, char * b);
void get(char* str);
int LBAToOffset(int32_t sector);
int16_t NextLB(uint32_t sector);

int main()
{
	char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
	/*
		need to keep track if a file is already opened.
		If a file is already opened then the system should output that one is opened.
		If a file is not yet opened, it should prompt the user to open a file so that 
		the rest of the funcitons can be used
	*/
	int fileStatus = 0;


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

	    int token_count = 0;                                 
	                                                           
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

	    /*in the case that the user only presses enter, the program should skip
	      the rest of the code and print another prompt line
	    */
	    if(token[0] == NULL)
	    {
	      continue;
	    }

	    /*if the user types in quit or exit the program will exit
	      Note: Not yet necessary to call fork function since these next 
	            action don't need to run in the child process
	    */
	    else if(strcmp(token[0], "quit") == 0 || strcmp(token[0], "exit") == 0)
	    {
	      //freeNodeList(head);
	      exit(0);
	    }

	    /*
			If the input wasn't quit or enter, the user cannot yet use any of the 
			functions to access FAT32 system, so the system should prompt user to 
			open a FAT32 img file
	    */
	    else if(strcmp(token[0], "open") != 0 && !fileStatus)
	    {
	    	printf("Error: File system image must be opened first.\n");
	    	continue;
	    }

	    else if(strcmp(token[0], "open") == 0)
	    {
	    	if(fileStatus)
	    	{
	    		printf("Error: File system image already open.\n");
	    		continue;
	    	}
	    	else
	    	{
		    	if((fp = fopen(token[1], "r")) == NULL)
		    	{
		    		printf("Error: File system image not found.\n");
		    		continue;
		    	}
		    	printf("file was opened..\n");
		    	fileStatus = 1;

		    	/*
					once file is opened, the user can read data from file

					seek and read the variables from the Boot Sector and
					the BPB Structure

					SEEK_SET is used as a reference to the offset. In this case,
					to start from the beginning
		    	*/
		    	fseek(fp, 3, SEEK_CUR);
		    	fread(&BS_OEMName, 1, 8, fp);
		    	//printf("BS_OEMName: %s\n", BS_OEMName);
		    	fread(&BPB_BytsPerSec, 1, 2, fp);
		    	//printf("BPB_BytsPerSec: %d\n", BPB_BytsPerSec);
		    	fread(&BPB_SecPerClus, 1, 1, fp);
		    	//printf("BPB_SecPerClus: %d\n", BPB_SecPerClus);
		    	fread(&BPB_RsvdSecCnt, 1, 2, fp);
		    	//printf("BPB_RsvdSecCnt: %d\n", BPB_RsvdSecCnt);
		    	fread(&BPB_NumFATs, 1, 1, fp);
		    	//printf("BPB_NumFATs: %d\n", BPB_NumFATs);
		    	fread(&BPB_RootEntCnt, 1, 2, fp);
		    	//printf("BPB_RootEntCnt: %d\n", BPB_RootEntCnt);

				fseek(fp, 36, SEEK_SET);
		    	fread(&BPB_FATSz32, 1, 4, fp);
		    	//printf("BPB_FATSz32: %d\n", BPB_FATSz32);

		    	fseek(fp, 44, SEEK_SET);
		    	fread(&BPB_RootClus, 1, 4, fp);
		    	//printf("BPB_RootClus: %d\n", BPB_RootClus);

		    	fseek(fp, 71, SEEK_SET);
		    	fread(BS_VolLab, 1, 11, fp);
		    	//printf("BS_VolLab: %s\n", BS_VolLab);

		    	/*
					Note: Root Directory contains 16 32-byte records found in the struct dir[]
						We also need to keep track of the previous and current directories for when 
						we change directories. Since The root directory is the start of the directory it can
						be set to both current and previous as default.

					Root directory is at the first cluster and the address can be found 
					with the following formula:
		    	*/
		    	RootDirSectors = (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec)
		    						+ (BPB_RsvdSecCnt * BPB_BytsPerSec);
				currentWD = RootDirSectors;
		    	previousWD = currentWD;

		    	newNode = allocateNode();
		    	newNode->directoryAddress = currentWD;
		    	newNode->next = NULL;
		    	newNode->prev = NULL;

		    	if(head == NULL)
		    	{
		    		head = newNode;
		    	}
		    	else
		    	{
		    		appendNode(&head, newNode);
		    	}

		    	fseek(fp, currentWD, SEEK_SET);
		    	int i;
		    	for(i = 0; i < 16; i++)
		    	{
		    		/*
						for each record, read in the data and populate the record
		    		*/
		    		fread(&dir[i], 32, 1, fp);
		    	}
		    }
	    }

	    else if(strcmp(token[0], "close") == 0)
	    {
	     	if(fileStatus)
	     	{
	     		fclose(fp);
	     		fileStatus = 0;

	     		//if file is closed then it should free the memory allocated by the list
	     		freeNodeList(head);
	     	}
	     	else
	     	{
	     		printf("Error: File system not open.\n");
	     	}
	    }
	    /*
			Once file is opened, the file status will be set to "true"
			Only then can the other commands be accessible
	    */
	    if(fileStatus)
	    {
	    	if(strcmp(token[0], "info") == 0)
	    	{
	    		printFileInfo();
	    		continue;
	    	}

	    	else if(strcmp(token[0], "stat") == 0)
	    	{
	    		if(token[1] == NULL)
	    		{
	    			printf("Error: number of function inputs is not met...\n");
	    			continue;
	    		}
	    		else
	    		{
	    			stat(token[1]);
	    			continue;
	    		}
	    	}

	    	else if(strcmp(token[0], "get") == 0)
	    	{
	    		if(token[1] == NULL)
	    		{
	    			printf("Error: number of function inputs is not met...\n");
	    			continue;
	    		}
	    		else
	    		{
	    			get(token[1]);
	    			continue;
	    		}
	    	}

	    	else if(strcmp(token[0], "cd") == 0)
	    	{
	    		if(token[1] == NULL)
	    		{
	    			printf("Error: number of function inputs is not met...\n");
	    			continue;
	    		}
	    		else
	    		{
	    			cd(&head, token[1]);
	    			continue;
	    		}
	    	}

	    	else if(strcmp(token[0], "ls") == 0)
	    	{
	    		/*
					Program supports listing "." and ".." as path name for function ls()
	    		*/
	    		char* lsStatus = malloc(9 * sizeof(char));
	    		if(token[1] != NULL && (strcmp(token[1], "..") == 0 || strcmp(token[1], ".") == 0))
	    		{
	    			strcpy(lsStatus,"previous");
	    			ls(lsStatus);
	    		}	
	    		else
	    		{
	    			strcpy(lsStatus,"current");
	    			ls(lsStatus);
	    		}
	    		free(lsStatus);
	    		continue;
	    	}

	    	else if(strcmp(token[0], "read") == 0)
	    	{
	    		if(token[1] == NULL || token[2] == NULL || token[3] == NULL)
	    		{
	    			printf("Error: number of function inputs is not met...\n");
	    			continue;
	    		}
	    		else
	    		{
	    			readFile(token[1], token[2], token[3]);
	    			continue;
	    		}
	    	}

	    	else if(strcmp(token[0], "volume") == 0)
	    	{
	    		volume();
	    		continue;
	    	}
	    	else
	    	{
	    		if(strcmp(token[0], "open") != 0)
	    		{
	    			printf("Error: command %s not found.\n", token[0]);
	    		}
	    		continue;
	    	}
	    }

	    free( working_root ); 

	    /*
	      I/O is buffered. Its important to clear it out using fflush()
	    */
	    fflush(NULL);
	}

	return 0;
}

struct directoryList* allocateNode()
{
    struct directoryList* node;
    node = malloc(sizeof(struct directoryList));
    return node;
}

/*
	Parameters: Takes a reference to a pointer (pointer to pointer) to the head of a list
				and an int that will be inserted as a new node to the end of the list
*/
void appendNode(struct directoryList** head, struct directoryList* newNode)
{
	struct directoryList* temp = *head;

	while(temp->next != NULL)
	{
		temp = temp->next;
	}
	temp->next = newNode;
	newNode->prev = temp;
}

/*
	Function to free the node passed a a parameter
*/
void deallocateNode(struct directoryList* node)
{
	free(node);
}

/*
	Function to deallocate memory for the directory list
*/
void freeNodeList(struct directoryList* head)
{
	struct directoryList* temp;
    
    if (head == NULL)
    {
        printf("List is empty");
        return;
    }
    while (head != NULL)
    {
        temp = head->next;
        free(head);
        head = temp;
    }
}

/*
	Function prints the information on the file system
*/
void printFileInfo()
{
	printf("Information about the file system:\n");
	printf("\nBPB_BytsPerSec in hex: %X base 10: %d\n", BPB_BytsPerSec, BPB_BytsPerSec);
	printf("BPB_SecPerClus in hex: %X base 10: %d\n", BPB_SecPerClus, BPB_SecPerClus);
	printf("BPB_RsvdSecCnt in hex: %X base 10: %d\n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);
	printf("BPB_NumFATs in hex: %X base 10: %d\n", BPB_NumFATs, BPB_NumFATs);
	printf("BPB_FATSz32 in hex: %X base 10: %d\n", BPB_FATSz32, BPB_FATSz32);
}

/*
	Parameters : The current sector number that points to a block of data
	Returns    : The value of the address for that block of data
	Description: Finds the starting address of a block of data given the sector number
				 corresponding to that data block
*/
int LBAToOffset(int32_t sector)
{
	return ((sector - 2) * BPB_BytsPerSec) + (BPB_BytsPerSec * BPB_RsvdSecCnt)
			+ (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec);
}

/*
	Purpose: Given a logical block address, look up into the first FAT and return 
	the logical block address of the block in the file. If there is no further blocks
	then return -1
*/
int16_t NextLB(uint32_t sector)
{
	uint32_t FATAddress = (BPB_BytsPerSec * BPB_RsvdSecCnt) + (sector * 4);
	int16_t val;
	fseek(fp, FATAddress, SEEK_SET);
	fread(&val, 2, 1, fp);
	return val;
}

/*
	Purpose: Read from given file name at the specified position 
	Output: number of bytes specified
*/
void readFile(char * str, char * p, char * b)
{
	int i;
	int position = atoi(p);
	int bytes = atoi(b);
	for(i = 0; i < strlen(str); i++)
	{
		str[i] = toupper(str[i]);
	}

	for(i = 0; i < 16; i++)	
	{
		char * content;
		char * token;
		char fileName[9];
		char extension[4];
		char * file;
		/*
			check to see if the content is hidden or not. If so then it will skip
		*/
		if(dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20)
		{
			content = malloc(12 * sizeof(char));
			memcpy(content, dir[i].DIR_Name, 11);
			content[11] = '\0';

			strncpy(fileName, content, 8);
			fileName[8] = '\0';
			token = strtok(fileName, " ");
			
			strncpy(extension, content+8, 3);
			extension[3] = '\0';
			
			if(extension[0] == ' ')
			{
				file = (char *) malloc(strlen(fileName) + 1);
				strcpy(file, fileName);
			}
			else
			{
				file = (char *) malloc(strlen(fileName) + strlen(extension) + 2);
				strcpy(file, fileName);
				strcat(file, ".");
				strcat(file, extension);	
			}
			if(strcmp(file, str) == 0)
			{
				/*
					if the filename is found then, we get the the first n bytes of the file and 
					print the number as the output
				*/
				char * buffer = malloc((bytes + 1) * sizeof(char));
				uint16_t cluster = dir[i].DIR_FirstClusterLow;
				int workingDir = LBAToOffset(cluster);
				fseek(fp, workingDir, SEEK_SET);

				fseek(fp, position, SEEK_CUR);
				fread(buffer, bytes, 1, fp);
				buffer[bytes] = '\0';

				printf("Output: %s\n", buffer);
				free(buffer);
				return;
			}
			free(file);
			free(content);
		}
	}
	printf("Error: File not found.\n");
}

/*
	Purpose: Prints the volume name of the file system image. If there is a volume name, it will be found in
	the root directory. If there is no volume name, your program shall output “Error: volume name
	not found.”

	Since char BS_VolLab[11] is found as a global variable there won't be a need to pass it as a parameter
*/
void volume()
{
	if(BS_VolLab[0] == '\0')
	{
		printf("Error: volume name not found.\n");
	}
	else
	{
		printf("volume: %s\n", BS_VolLab);
	}
}

/*
	Purpose: This command shall print the attributes and starting cluster number of the file or directory name.
	If the parameter is a directory name then the size shall be 0. If the file or directory does not exist
	then your program shall output “Error: File not found”.
*/
void stat(char * str)
{
	int i;
	for(i = 0; i < strlen(str); i++)
	{
		str[i] = toupper(str[i]);
	}

	for(i = 0; i < 16; i++)	
	{
		char * content;
		char * token;
		char fileName[9];
		char extension[4];
		char * file;
		/*
			check to see if the content is hidden or not. If so then it will skip
		*/
		if(dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20)
		{
			content = malloc(12 * sizeof(char));
			memcpy(content, dir[i].DIR_Name, 11);
			content[11] = '\0';

			strncpy(fileName, content, 8);
			fileName[8] = '\0';
			token = strtok(fileName, " ");
			
			strncpy(extension, content+8, 3);
			extension[3] = '\0';
			
			if(extension[0] == ' ')
			{
				file = (char *) malloc(strlen(fileName) + 1);
				strcpy(file, fileName);
			}
			else
			{
				file = (char *) malloc(strlen(fileName) + strlen(extension) + 2);
				strcpy(file, fileName);
				strcat(file, ".");
				strcat(file, extension);	
			}
			if(strcmp(file, str) == 0)
			{
				/*
					if file is found, function should print the attributes and starting cluster numner 
					of the file or directory name.

					If str is a directory name then size shall be 0
				*/
				printf("%s \n\tAttribute: %x \n\tCluster High: %d \n\tCluster Low: %d \n\tFileSize: %d bytes\n", str, dir[i].DIR_Attr, dir[i].DIR_FirstClusterHigh, dir[i].DIR_FirstClusterLow, dir[i].DIR_FileSize);
		    	return;
			}
			free(file);
			free(content);
		}
	}
	printf("Error: File not found.\n");
}

/*
	Parameters: Name of File to be copied
	Purpose: File will search and compare the name of file. If found it will copy data 
			 into current working directory with same name 
*/
void get(char* str)
{
	int i;
	for(i = 0; i < strlen(str); i++)
	{
		str[i] = toupper(str[i]);
	}
	for(i = 0; i < 16; i++)	
	{
		char * content;
		char * token;
		char fileName[9];
		char extension[4];
		char * file;
		/*
			check to see if the content is hidden or not. If so then it will skip
		*/
		if(dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20)
		{
			content = malloc(12 * sizeof(char));
			memcpy(content, dir[i].DIR_Name, 11);
			content[11] = '\0';

			strncpy(fileName, content, 8);
			fileName[8] = '\0';
			token = strtok(fileName, " ");
			
			strncpy(extension, content+8, 3);
			extension[3] = '\0';
			
			if(extension[0] == ' ')
			{
				file = (char *) malloc(strlen(fileName) + 1);
				strcpy(file, fileName);
			}
			else
			{
				file = (char *) malloc(strlen(fileName) + strlen(extension) + 2);
				strcpy(file, fileName);
				strcat(file, ".");
				strcat(file, extension);	
			}
			if(strcmp(file, str) == 0)
			{
				/*
					if file is found, function should open a file with same name in 
					current working directory
				*/
				if((outfile = fopen(file, "w+")) == NULL)
	    		{
	    			printf("Error: File could not be opened.\n");
	    		}

		    	printf("output file was opened..\n");
				uint32_t fileSize = dir[i].DIR_FileSize;
				uint16_t cluster = dir[i].DIR_FirstClusterLow;
				int workingDir = LBAToOffset(cluster);
				char buffer[512];

				//fseek(fp, workingDir, SEEK_SET);

				while(fileSize > 512)
				{		
					fseek(fp, workingDir, SEEK_SET);
					fread(buffer, 512, 1, fp);
					fwrite(buffer, 512, 1, outfile);
					fileSize = fileSize - 512;
					cluster = NextLB(cluster);
					workingDir = LBAToOffset(cluster);
				}

				fseek(fp, workingDir, SEEK_SET);
				fread(buffer, fileSize, 1, fp);
				fwrite(buffer, fileSize, 1, outfile);

		    	fclose(outfile);
		    	return;
			}
			free(file);
			free(content);
		}
	}
	printf("Error: File not found.\n");
}

/*
	Since File *fp and its attributes are all global, it is not necessary to pass a pointer
	to the file.

	Purpose: To print the directory contents.
	Description: Supports listing "." and ".." or "../". It will also not list deleted files or system 
				 volume names.

	Note: show attributes in hex are: 0x01, 0x10, or 0x20
		meaning some files/directories are hidden
*/
void ls(char* lsStatus)
{
	/*
		based on the address passed the function should search and read into the directory
		struct again
	*/
	int i;
	
	if(strcmp(lsStatus, "previous") == 0)
	{
		fseek(fp, previousWD, SEEK_SET);
		for(i = 0; i < 16; i++)
		{
			/*
				for each record, read in the data and populate the record
			*/
			fread(&dir[i], 32, 1, fp);
		}
	}
	

	char * contentName;
	char * token;
	char fileName[9];
	char extension[4];
	char * file;
	for(i = 0; i < 16; i++)
	{
		/*
			check to see if the content is hidden or not. If so then it will skip
		*/
		if(dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20)
		{
			/*
				DIR_Name has the name as well as garbage at the end of the string,
				so we need to cut off the garbage from the end.

				Note: Strings end with /0 character so n + 1 space is needed for the string
			*/

			contentName = malloc(12 * sizeof(char));
			memcpy(contentName, dir[i].DIR_Name, 11);
			contentName[11] = '\0';
			if((contentName[0] != 0x2E) && (contentName[0] != -0x1B))
			{
				strncpy(fileName, contentName, 8);
				fileName[8] = '\0';
				token = strtok(fileName, " ");
				//printf("%s\n", fileName);
				
				strncpy(extension, contentName+8, 3);
				extension[3] = '\0';
				//printf("%s\n", extension);
				
				if(extension[0] == ' ')
				{
					file = (char *) malloc(strlen(fileName) + 1);
					strcpy(file, fileName);
					printf("%s\n", file);
				}
				else
				{
					file = (char *) malloc(strlen(fileName) + strlen(extension) + 2);
					strcpy(file, fileName);
					strcat(file, ".");
					strcat(file, extension);
					printf("%s\n", file);	
				}
				free(file);
			}
			free(contentName);
		}
	}

	/*
		Since ls() does not change directories, it should reset to the original current
		directory
	*/
	if(strcmp(lsStatus, "previous") == 0)
	{
		fseek(fp, currentWD, SEEK_SET);
		for(i = 0; i < 16; i++)
		{
			/*
				for each record, read in the data and populate the record
			*/
			fread(&dir[i], 32, 1, fp);
		}
	}
}

void cd(struct directoryList** head, char* path)
{
	/*
		move to the end of the list to get the most recent entry,
		If the previous node is NULL then it means you are at the root entry and 
		this should reset the current and previous directories to the root entry
	*/
	int i;
	if(path == NULL)
	{
		struct directoryList* current = *head;
		struct directoryList* next;

		if(current->next == NULL)
		{
			currentWD = RootDirSectors;
	    	previousWD = currentWD;

	    	newNode = allocateNode();
	    	newNode->directoryAddress = currentWD;
	    	newNode->next = NULL;
	    	newNode->prev = NULL;

	    	*head = newNode;

	    	fseek(fp, currentWD, SEEK_SET);
	    	for(i = 0; i < 16; i++)
	    	{
	    		/*
					for each record, read in the data and populate the record
	    		*/
	    		fread(&dir[i], 32, 1, fp);
	    	}
			return;
		}
		else
		{
			while(current != NULL)
			{
				next = current->next;
				free(current);
				current = current->next;
			}

			currentWD = RootDirSectors;
	    	previousWD = currentWD;

	    	newNode = allocateNode();
	    	newNode->directoryAddress = currentWD;
	    	newNode->next = NULL;
	    	newNode->prev = NULL;

	    	*head = newNode;

			fseek(fp, currentWD, SEEK_SET);	

			for(i = 0; i < 16; i++)
			{
				/*
					for each record, read in the data and populate the record
				*/
				fread(&dir[i], 32, 1, fp);
			}
			return;	
		}
	}

	/*
		check to see if the path is for the previous directory
		If it is then delete the most recent node and makes its previous node the
		current working directory
	*/
	if(strcmp(path, "..") == 0 || strcmp(path, "../") == 0 || strcmp(path, ".") == 0)
	{
		struct directoryList* current = *head;
		struct directoryList* next;
		/*
			if head->next == NULL then there are no more nodes in the struct list
			and the root directory should be added back in 
		*/
		if(current->next == NULL)
		{
			currentWD = RootDirSectors;
	    	previousWD = currentWD;

	    	newNode = allocateNode();
	    	newNode->directoryAddress = currentWD;
	    	newNode->next = NULL;
	    	newNode->prev = NULL;

	    	*head = newNode;

	    	fseek(fp, currentWD, SEEK_SET);
	    	for(i = 0; i < 16; i++)
	    	{
	    		/*
					for each record, read in the data and populate the record
	    		*/
	    		fread(&dir[i], 32, 1, fp);
	    	}
			return;
		}
		/*
			if it has not reached the end, the function should remove the last node and 
			make the previous node before it the new "directory"
		*/
		else
		{
			while(current != NULL && current->next != NULL)
			{
				next = current;
				//free(current);
				current = current->next;
			}
			free(next->next);
			next->next = NULL;

			currentWD = next->directoryAddress;
			fseek(fp, currentWD, SEEK_SET);	

			for(i = 0; i < 16; i++)
			{
				/*
					for each record, read in the data and populate the record
				*/
				fread(&dir[i], 32, 1, fp);
			}
			return;	
		}
	}
	else
	{
		/*
			Search each directory an compare to see if the path matches an entry
			If so, it will become the new current working directory
			
			convert input string to all uppercase so it can be compared easier
		*/
		char * token;
		char * contentName;
		for(i = 0; i < strlen(path); i++)
		{
			path[i] = toupper(path[i]);
		}

		for(i = 0; i < 16; i++)
		{
			/*
				check to see if the content is hidden or not. If so then it will skip
			*/
			if(dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20)
			{
				contentName = malloc(12 * sizeof(char));
				memcpy(contentName, dir[i].DIR_Name, 11);
				contentName[11] = '\0';
				token = strtok(contentName, " ");

				if(strcmp(token, path) == 0)
				{
					previousWD = currentWD;
					currentWD = LBAToOffset(dir[i].DIR_FirstClusterLow);

					newNode = allocateNode();
			    	newNode->directoryAddress = currentWD;
			    	newNode->next = NULL;
			    	newNode->prev = NULL;
					appendNode(head, newNode);
					fseek(fp, currentWD, SEEK_SET);	
					for(i = 0; i < 16; i++)
					{
						/*
							for each record, read in the data and populate the record
						*/
						fread(&dir[i], 32, 1, fp);
					}
					return;
				}
				free(contentName);
			}
		}
	}
	printf("Directory %s was not found.\n", path);
}


