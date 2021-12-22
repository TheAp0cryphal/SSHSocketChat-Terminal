
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.c"
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/socket.h> // Needed for socket creating and binding
#include <arpa/inet.h>  //intranet_addr

#define Array_size 4000 // Each Message Can be of Size 4000 - 1 Characters long
#define KEY 3           // Will be used to Encrypt and Decrypt Data

bool Terminate, Message_Sent; // This flag will signal Termination when set true
char Buffer[Array_size];      // First Thread will read input from Keyboard into this thread
pthread_mutex_t SendingList_Mutex, RecevingList_Mutex;
int ServerSock, ClientSock, otherMachinePort;
List *Sending_List;  // Will be used by Keyboard and Sender thread
List *Receving_List; // will be used by Reciver and printer thread
struct sockaddr_in Server_addr, Client_addr;
pthread_t ThreadArray[4]; // we have total 4 Threads


void FreeMemory(); // Free any memory Used by lists
void OnlineStatus(); // This function will tell if the other machine is online or not
void Encrypt(char *Message);// This function will be Encrypt the message to send
void Decrypt(char *Message);
void Allocate_Memory_and_to_List(char *buffer, List *list);
void *FirstThreadFunc();
void *SecondThreadFuc();
void *ThirdThreadFuc();
void *FourthThread();