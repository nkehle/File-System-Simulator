/* Noa Kehle
 * Asgn2 Submission
 * 10/16 */

#ifndef FS_SIMULATOR_H
#define FS_SIMULATOR_H

#include <stdio.h>    // used for reading and writing
#include <stdlib.h>   // used for printing
#include <stdint.h>   // for int conversion
#include <string.h>   // for string memset
#include <unistd.h>   // for the chdir
#include <stdbool.h>  // while true

void executeCommand(char *inStr, int *current, int *parent, FILE *inodesList, int *inodesPtr);
void createFile(char type, int *parent, FILE *currentPtr, char name[], FILE *inodesList, int *inodesPtr);
void ls(int *currentPtr);
void cd(int *currentPtr, char *target, int *parentPtr, FILE *inodesList);
int checkExists(char type, int *currentPtr, char *target, char *buffer1);
int countInodes();
char *uint32_to_str(uint32_t i);
uint32_t strToInt(const char *str);
void parseCommand(char *inStr, char *tmp1, char *tmp2, int *args, int *jptr);

#endif
