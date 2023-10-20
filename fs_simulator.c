/* Noa Kehle
 * Asgn2 Submission
 * 10/9/23 */

#include "fs_simulator.h"

int main(int argc, char **argv){
  int inodesCnt, current = 0, parent = 0;
  int *inodesPtr = &inodesCnt, *currentPtr = &current, *parentPtr = &parent;
  FILE *root, *inodesList;

  /* Initial ARG checking */ 
  if(argc > 1){                                  // if there is a given argument
    root = fopen(argv[1], "r");
    if((root == NULL)){                          // checks to make sure the given file is valid
      printf("%s\n", "Error: root does not exist");
      return(EXIT_FAILURE);
    }
  } else {
    printf("%s\n", "Error: Invalid number or aguments");
    return(EXIT_FAILURE);
  }
  chdir(argv[1]);                                 // go into the right folder 
  inodesList = fopen("inodes_list", "ab+");       // open the master list for appending and reading
  inodesCnt = countInodes();                      // get the initial count of how many total inodes there are

  while(true){
    printf("%s", "> ");                           // input prompt
    char inStr[39];                               // length 39 for 32 + ' ' + 5 + \0 and \n *Must reset each time
    fgets(inStr, sizeof(inStr), stdin);  
    executeCommand(inStr, currentPtr, parentPtr, inodesList, inodesPtr);
  }
  return 0;
}

/* Executes the proper command from the user input */
void executeCommand(char *inStr, int *currentPtr, int *parentPtr, FILE *inodesList, int *inodesPtr){
  char *parentInodeName = uint32_to_str(*currentPtr); 
  FILE *parentInode = fopen(parentInodeName, "ab");
  free(parentInodeName);
  int j = 0, args = 0;
  char linefeed = 0x0A;
  char tmp1[6] = {'\0'};
  char tmp2[34] = {'\0'};
  
  parseCommand(inStr, tmp1, tmp2, &args, &j);
  

  /* switch case for finding the correct command */
  if ((strcmp(tmp1, "ls") == 0)) {                        // list the contents
    ls(currentPtr);
  } else if((strcmp(tmp1, "exit") == 0)){                 // exit program
    exit(0);
  } else if ((strcmp(tmp1, "cd") == 0)) {                 // change directory
    cd(currentPtr, tmp2, parentPtr, inodesList);
  } else if ((strcmp(tmp1, "touch") == 0)){               // create a file
    tmp2[j] = linefeed; // replace the 00 with a 0a
    createFile('f', currentPtr, parentInode, tmp2, inodesList, inodesPtr);
  } else if ((strcmp(tmp1, "mkdir") == 0 )){              // create a directory
    createFile('d', currentPtr, parentInode, tmp2, inodesList, inodesPtr);
  } else {
    printf("%s\n", "Error: Not a valid command");
  }
  fclose(parentInode);                                
}

/* Parse command split inStr by space and list args */
void parseCommand(char *inStr, char *tmp1, char *tmp2, int *args, int *jptr){
  int i = 0, j = 0;

  /* Gather the first part of the command */
  while((inStr[i] != ' ') && (inStr[i] != '\n') && (inStr[i] != '\0')){  
    tmp1[i] = inStr[i];
    i++;
  } tmp1[i] = '\0';                                     // null terminate the string
  i++;                                                  // skip the first space
  
  /* Gather the second part of the command*/
  while((inStr[i] != '\n') && (inStr[i] != '\0') && (j <= 32)){      
    if(inStr[i] == ' ') {                               // if there is another space then that means too many arguments
      args++;
    } tmp2[j] = inStr[i];
    j++;
    i++;
  }     
  /* name is a full 32 chars, need to clear excess to avoid buffer overflow */
  if (j >= 32 && inStr[i] != '\n'){                     // the name is a full 32 chars and there is no newline
    int c;
    while ((c = fgetc(stdin)) != '\n' && c != EOF) {    // discard the excess values to avoid a buffer overflow
    } tmp2[j+1] = '\0';                                 // if the name buffer is full then terminate after the 32 chars at space 33
  }
  *jptr = j;
  return;
}

/* creates a file (inode) of the given type */
void createFile(char type, int *currentPtr, FILE *parentInode, char *name, FILE *inodesList, int *inodesPtr){ 
  FILE *newInode;
  char *inode = uint32_to_str(*inodesPtr);        // convert the int to a string for the name of the inode file
  char buffer1[33] = {'\0'}; 

  if(*inodesPtr < 1024){                          // check the size of the file system
    int tmp = checkExists(type, currentPtr, name, buffer1);
    if (tmp > 0){
      printf("%s\n", "Error: File already exists");
      free(inode);
      return;
    } 
    newInode = fopen(inode, "wb");                // create the new inode
    free(inode);

    /* for getting the name of the file */
    int nameLen = strlen(name);
    char buffer[32] = {'\0'};
    for(int i = 0; i < nameLen; i++){
      buffer[i] = name[i];
    }
    fwrite(inodesPtr, 4, 1, inodesList);          // add to master inodes_list
    fwrite(inodesPtr, 4, 1, parentInode);         // add to the parent node
    /* Create a FILE */
    if(type == 'f'){
      fwrite(buffer, 1, nameLen, newInode);       // file name stored as ascii text followed by null
      fwrite(&type, 1, 1, inodesList);            // writes 'f' to the master inodes_list
      buffer[nameLen - 1] = '\0';                 // remove the newline for writing to the parent 
      fwrite(buffer, 1, 32, parentInode);         // write to the parent dir
    } else { /* Create a DIRECTORY */
      char cd[32] = ".";
      char pd[32] = "..";
      fwrite(inodesPtr, 4, 1, newInode);          // write current directory integer
      fwrite(&cd, 32, 1, newInode);               // write name -> .
      fwrite(currentPtr, 4, 1, newInode);         // write  parent directory integer
      fwrite(&pd, 32, 1, newInode);               // write name -> ..                            
      fwrite(&type, 1, 1, inodesList);            // writes 'd' to the master inodes_list
      fwrite(buffer, 1, 32, parentInode);         // write to the parent inode
    }
    
    (*inodesPtr)++;                               // increment inodes counter
    fclose(newInode);                             // close file
  } else {
    printf("%s\n", "Error: Not enough space left in the system");
   return;
  }
  return;
}

/* checks to see if a file exists
 * -1 if it does NOT exist
 * integer coresponding to the inode if it exists  */
int checkExists(char type, int *currentPtr, char *target, char *buffer1){
  char *currentName = uint32_to_str(*currentPtr);     // convert the name into a int to get the access the file
  FILE *currentDir = fopen(currentName, "rb");        // open the file
  free(currentName);
  int found = 0, inode = 0;

  fseek(currentDir, 4, SEEK_SET);                   // skip the first integer starting from the beggining
  while(found != 1){
    fread(buffer1, 1, 32, currentDir);              // read the name into the buffer
    if(strlen(target) == 32){
      buffer1[32] = '\0';
    } if (type == 'f') {
      buffer1[strlen(target)-1] = '\n';             // add a newline for checking existence of single files
    } 
    if(strcmp(buffer1, target) == 0){               // it exists!
      /* get the coresponding inode */
      fseek(currentDir, -36, SEEK_CUR);             // go back to get the inode 
      fread(&inode, 4, 1, currentDir);
      break;

     } else if(feof(currentDir)){                   // if we've reached the end of the file -> does not exist!
       inode = -1;
       break;
     }
     fseek(currentDir, 4, SEEK_CUR);                // skip the next int
  }
  fclose(currentDir);                               // close the file when done 
  return inode;
}

/* displays the files in the current dirrectory */
void ls(int *currentPtr){
  char *currentName = uint32_to_str(*currentPtr);
  FILE *currentDir = fopen(currentName, "rb");
  free(currentName);
  int buffer1 = 0, i = 0;
  char buffer2[32];

  fseek(currentDir, 0, SEEK_END);                 // go to the end of the file
  int length = ftell(currentDir);                 // get the length
  fseek(currentDir, 0, SEEK_SET);                 // go back to the beggining

  while(ftell(currentDir) < length){
    fread(&buffer1, 4, 1, currentDir);            // fill the buffer with the int
    fread(buffer2, 1, 32, currentDir);            // fill the buffer with the name
    printf("%d%c%s\n", buffer1, ' ', buffer2);    
    i++;
  }
  fclose(currentDir);                             // close the file when done 
}


/* change into the target directory */
void cd(int *currentPtr, char *target, int *parentPtr, FILE *inodesList){
  char buffer1[33];                                           // length 32 for the name + null and newline`
  int buffer2 = 0, ford = 0, inode;                           // for the inode
  
  inode = checkExists('d', currentPtr, target, buffer1);      // check to see if it exits
  if(inode == -1){                                            // if the file did not exist
    printf("%s\n", "Error: File does not exist");
    return;
  }

  /* check the master inodes list to see if its a file */ 
  fseek(inodesList, 0, SEEK_SET);                       // start searching from the beggining 

  while(ford != 1){
     fread(&buffer2, 4, 1, inodesList);                 // check the first 4 bytes (int) to see if it matches 
     if(buffer2 == inode){                              // -> its a file
       if(fgetc(inodesList) == 'f'){
        ford = 1;
        printf("%s\n", "Error: Cannot cd into a file");
       } else {                                         // -> its a directory
        *parentPtr = *currentPtr;                       // set the parent to what you were previously in 
        *currentPtr = inode;                            // set the current to the new target
        ford = 1;
       }
    } fseek(inodesList, 1, SEEK_CUR);                   // skip the letter
  }
  fseek(inodesList, 0, SEEK_CUR);  
}

/* Counts the number of inodes currently in the master inodes_list */
int countInodes(){
  FILE *file = fopen("inodes_list", "rb");
  fseek(file, 0, SEEK_END);                               // finds the end of file
  int bytes = ftell(file);                                // returns that position 
  int res = bytes/5;                                      // 5 bytes per inode
  fclose(file);
  return res;
}

/* Converts a uint32 into a string for the inode names */
char *uint32_to_str(uint32_t i) {
   int length = snprintf(NULL, 0, "%lu", (unsigned long)i);   // pretend to print to a string to determine length
   char* str = malloc(length + 1);                            // allocate space for the actual string
   snprintf(str, length + 1, "%lu", (unsigned long)i);        // print to string
   return str;
}

/* Converts a sting to the uint32 */
uint32_t strToInt(const char *str) {                  
    char *endPtr;
    unsigned long ul = strtoul(str, &endPtr, 10);
    return (uint32_t)ul;
}


