#include"lets-talk.h"

int main(int argc, char *argv[])
{
        printf("Welcome to Lets-Talk!\n");

        int MY_Port, OtherMachine_Port;
        char *IP_Addr;
        Sending_List = List_create();
        Receving_List = List_create();

        if (Sending_List == NULL || Receving_List == NULL)
        {

                printf("Cant Allocate memory for Lists Terminating\n");
                return -1;
        }

        pthread_mutex_init(&SendingList_Mutex, NULL);  // Fast Mutex
        pthread_mutex_init(&RecevingList_Mutex, NULL); // Fast Mutex

        // List1_Mut = PTHREAD_MUTEX_INITIALIZER;
        // pthread_mutex_t List2_Mut = PTHREAD_MUTEX_INITIALIZER; // Fast Mutex
        Terminate = false;
        Message_Sent = false;

        if (argc == 4)
        {
                MY_Port = atoi(argv[1]);
                otherMachinePort=OtherMachine_Port = atoi(argv[3]);
                IP_Addr = argv[2];
        }
        else
        {
                printf("Missing Arguments! Usage: <local port> <remote host> <remote port>\n");
                return -1;
        }
	printf("Type in your messages now!\n");


        //Client addr is the socket at which Current Machine will Recieve Messages 
        //Server_addr is the One at which the Current Machine will send Messages too 

        //Creating UDP Socket

        ClientSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if (ClientSock < 0 || ServerSock < 0)
        {
                perror("Could Not Create Socket. Error!!!\n");
                return -1;
        }

        // For Receving
        Server_addr.sin_family = AF_INET;
        Server_addr.sin_port = htons(OtherMachine_Port);
        Server_addr.sin_addr.s_addr = inet_addr(IP_Addr); // bind your socket to localhost

        // For Sending
        Client_addr.sin_family = AF_INET;
        Client_addr.sin_port = htons(MY_Port);
        Client_addr.sin_addr.s_addr = inet_addr(IP_Addr); // bind your socket to localhost

        // Bind My port
        if (bind(ClientSock, (struct sockaddr *)&Client_addr, sizeof(Client_addr)) < 0)
        {
                perror("Bind Failed. Error!!!\n");
                return -1;
        }

    

        // Create 4 Threads Now
        pthread_create(&ThreadArray[0], NULL, FirstThreadFunc, NULL);
        pthread_create(&ThreadArray[1], NULL, SecondThreadFuc, NULL);
        pthread_create(&ThreadArray[2], NULL, ThirdThreadFuc, NULL);
        pthread_create(&ThreadArray[3], NULL, FourthThread, NULL);

        pthread_join(ThreadArray[0], NULL); // wait
        exit(0);
}


//-----------------------_FUNCTION_IMPLEMENTATIONS_--------------------






void FreeMemory() // Free any memory Used by lists
{
        while (List_count(Sending_List) > 0)
        {
                free(List_trim(Sending_List));
        }

        while (List_count(Receving_List) > 0)
        {
                free(List_trim(Receving_List));
        }
}

// This function will tell if the other machine is online
void OnlineStatus()
{
        char command[100], PortNum[10];
        sprintf(PortNum,"%d",otherMachinePort);
        strcpy(command,"lsof -i -P -n | grep -c ");
        strncat(command,PortNum,strlen(PortNum));

   FILE * in = popen(command,"r");
        char c = getc(in);
        if (c=='1')
        {
                printf("Online\n");
                fflush(stdout);
        }
        else{
                printf("Offline\n");
                fflush(stdout);
        }
        
        fclose(in);

}

// This function will Encrypt the message to send
void Encrypt(char *Message)
{
        int strlenght = strlen(Message);

        for (int i = 0; i < strlenght; i++)
        {
                Message[i] = (Message[i] + KEY) % 256;
        }
}

void Decrypt(char *Message)
{

        int strlenght = strlen(Message);

        for (int i = 0; i < strlenght; i++)
        {
                Message[i] = Message[i] - KEY;
        }
}

void Allocate_Memory_and_to_List(char *buffer, List *list)
{

        // we are going to Dynamically allocate memory using calloc
        // Copy Message into from buffer and all of it to the list

        char *Message = (char *)calloc(Array_size, sizeof(char));
        memcpy(Message, buffer, Array_size);

        Encrypt(Message);
        if (List_prepend(list, Message) == -1)
        {
                printf("Adding Message to List Failed Terminating\n");
                exit(-1);
        }

        //Clear the buffer
        memset(buffer, '\0', Array_size);
}

void *FirstThreadFunc() // Getting Input from Keyboard
{
        while (true)
        {
                memset(Buffer, '\0', sizeof(Buffer));
                fgets(Buffer, sizeof(Buffer), stdin); // Read message from keyboard from Keyboard
                //Add Message to the List
                if (memcmp("!status",Buffer,strlen("!status"))==0)
                {
                        OnlineStatus();
                        continue;
                }
                

                pthread_mutex_lock(&SendingList_Mutex);
                Allocate_Memory_and_to_List(Buffer, Sending_List);
                //printf((char*)List_curr(Sending_List));
                pthread_mutex_unlock(&SendingList_Mutex);
        }

        // We Terminate Signal
}

void *SecondThreadFuc() // Will Extract a Message From Reciving List and Display it onScreen
{
        // Extract a Message from the Reciving List and Display it on screen

        while (!Terminate)
        {
                pthread_mutex_lock(&RecevingList_Mutex);
                char *Message = List_trim(Receving_List);

                if (Message != NULL)
                {
                        if (memcmp(Message, "!exit", strlen("!exit")) == 0)
                        {
                                // Time to exit
                                Terminate = true;
                        }
                        printf("%s", Message);
                        fflush(stdout);
                        free(Message); // Free the Node Memory as well
                }
                pthread_mutex_unlock(&RecevingList_Mutex);
                //return;
        }

        pthread_mutex_lock(&SendingList_Mutex);
        pthread_mutex_lock(&RecevingList_Mutex);
        pthread_cancel(ThreadArray[1]);
        pthread_cancel(ThreadArray[0]);
        pthread_cancel(ThreadArray[3]);

        FreeMemory();
        //List_free(Sending_List,);  Need to Free memory here

        pthread_mutex_unlock(&SendingList_Mutex);
        pthread_mutex_unlock(&RecevingList_Mutex);
        pthread_exit(NULL);
}

void *ThirdThreadFuc() // Recieved UDP Datagram from other Machine and Add to Receving message List
{
        int StructSize = sizeof(Server_addr);

        while (true)
        {
                char *Message = (char *)calloc(Array_size, sizeof(char));
                if (recvfrom(ClientSock, Message, Array_size, 0, (struct sockaddr *)&Server_addr, &StructSize) < 0)
                {
                        perror("Receive Failed. Error!!!!!\n");
                        exit(-1);
                }
                pthread_mutex_lock(&RecevingList_Mutex);
                Decrypt(Message);
                List_prepend(Receving_List, Message);
                pthread_mutex_unlock(&RecevingList_Mutex);
                //return;
        }

        //Receive the message back from the server
}

void *FourthThread() // Send Message to Other Machine
{
        while (!Terminate)
        {
                pthread_mutex_lock(&SendingList_Mutex);
                char *Message = List_trim(Sending_List);

                if (Message != NULL)
                {

                        if (sendto(ClientSock, Message, Array_size, 0, (struct sockaddr *)&Server_addr, sizeof(Server_addr)) < 0)
                        {
                                perror("Send Failed. Error!!!!\n");
                                exit(-1);
                        }
                        Decrypt(Message);
                        if (memcmp(Message, "!exit", strlen("!exit")) == 0)
                        {
                                // Time to exit
                                Terminate = true;
                        }

                        //free(Message->pItem); // Free the Node Memory as well
                }
                pthread_mutex_unlock(&SendingList_Mutex);
                //return;
        }

        pthread_mutex_lock(&SendingList_Mutex);
        pthread_mutex_lock(&RecevingList_Mutex);
        pthread_cancel(ThreadArray[0]);
        pthread_cancel(ThreadArray[1]);
        pthread_cancel(ThreadArray[2]);

        //List_free(Sending_List,);  Need to Free memory here
        FreeMemory();

        pthread_mutex_unlock(&SendingList_Mutex);
        pthread_mutex_unlock(&RecevingList_Mutex);
        pthread_exit(NULL);
}

