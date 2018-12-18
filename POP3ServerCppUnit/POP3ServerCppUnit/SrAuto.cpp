#include <stdio.h>

#include "const.h"
#include "SrAuto.h"

#define StandardMessageCoding 0x00

/*
 *3. broj vremenskih kontrola
 *4. maks stanja za taj automat
 *5. maks f-ja prelaza za jedno stanje
*/
SrAuto::SrAuto() : FiniteStateMachine(SR_AUTOMATE_TYPE_ID, SR_AUTOMATE_MBX_ID, 0, 7, 1) {
}

SrAuto::~SrAuto() {
}

/* This function actually connnects the ClAutoe with the mailbox. */
uint8 SrAuto::GetMbxId() {
	return SR_AUTOMATE_MBX_ID;
}

uint32 SrAuto::GetObject() {
	return GetObjectId();
}

MessageInterface *SrAuto::GetMessageInterface(uint32 id) {
	return &StandardMsgCoding;
}

void SrAuto::SetDefaultHeader(uint8 infoCoding) {
	SetMsgInfoCoding(infoCoding);
	SetMessageFromData();
}

void SrAuto::SetDefaultFSMData() {
	SetDefaultHeader(StandardMessageCoding);
}

void SrAuto::NoFreeInstances() {
	printf("[%d] SrAuto::NoFreeInstances()\n", GetObjectId());
}

void SrAuto::Reset() {
	printf("[%d] SrAuto::Reset()\n", GetObjectId());
}


void SrAuto::Initialize() {
	SetState(FSM_SR_IDLE);	
	
	//intitialization message handlers
	InitEventProc(FSM_SR_IDLE, MSG_Set_All, (PROC_FUN_PTR)&SrAuto::FSM_Sr_Idle );
	InitEventProc(FSM_SR_AUTHORISING_USERNAME, MSG_MSG, (PROC_FUN_PTR)&SrAuto::FSM_Sr_Authorising_username );
	InitEventProc(FSM_SR_AUTHORISING_PASS, MSG_MSG, (PROC_FUN_PTR)&SrAuto::FSM_Sr_Authorising_pass );
	InitEventProc(FSM_SR_MAIL_CHECK, MSG_MSG, (PROC_FUN_PTR)&SrAuto::FSM_Sr_Mail_Check );
	InitEventProc(FSM_SR_SENDING, MSG_MSG, (PROC_FUN_PTR)&SrAuto::FSM_Sr_Sending );
	InitEventProc(FSM_SR_DELETING, MSG_MSG, (PROC_FUN_PTR)&SrAuto::FSM_Sr_Deleting );
	InitEventProc(FSM_SR_DISCONNECT, MSG_MSG, (PROC_FUN_PTR)&SrAuto::FSM_Sr_Disconnect );
	
}

void SrAuto::FSM_Sr_Idle(){
	int c;

    //Create socket
    m_Server_Socket = socket(AF_INET , SOCK_STREAM , 0);
    if (m_Server_Socket == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    //Bind
    if( bind(m_Server_Socket,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return;
    }
    puts("bind done");

	//Listen
    listen(m_Server_Socket , 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    //accept connection from an incoming client
    m_Client_Socket = accept(m_Server_Socket, (struct sockaddr *)&client, (int*)&c);
    if (m_Client_Socket < 0)
    {
        perror("accept failed");
        return;
    }
    puts("Connection accepted");

	/*TODO: send +OK send*/
	

	SetState(FSM_SR_AUTHORISING_USERNAME);

	/* Then, start the thread that will listen on the the newly created socket. */
	m_hThread = CreateThread(NULL, 0, ServerListener, (LPVOID) this, 0, &m_nThreadID); 
	if (m_hThread == NULL) {
		/* Cannot create thread.*/
		closesocket(m_Server_Socket);
		m_Server_Socket = INVALID_SOCKET;
		return ;
	}

}



void SrAuto::FSM_Sr_Authorising_username(){
	int user_valid = 0;

	char* data = new char[255];
	uint8* buffer = GetParam(PARAM_DATA);
	uint16 size = buffer[2];

	memcpy(data,buffer + 4,size);
	data[size]=0;
	printf("%s",data);

	for(int i; i < NUM_USERS; i++)
	{
		if(strcmp(data + 5, users[i]) == 0) //provera user-a
		{
			user_valid = 1;
			INDEKS_USERA = i;
			break;
		}
	}

	char *message;
	if(user_valid == 1)
	{
		//Send some data
		strcpy(message, "+OK\r\n\0");
		if( send(m_Client_Socket , message , strlen(message), 0) < 0)
		{
			puts("Send failed");
			return;
		}

		SetState(FSM_SR_AUTHORISING_PASS);

	}else{
		strcpy(message, "-ERROR\r\n\0");
		if( send(m_Client_Socket , message , strlen(message), 0) < 0)
		{
			puts("Send failed");
			return;
		}

		SetState(FSM_SR_IDLE);
	}
	
}


void SrAuto::FSM_Sr_Authorising_pass(){
	int pass_valid = 0;

	char* data = new char[255];
	uint8* buffer = GetParam(PARAM_DATA);
	uint16 size = buffer[2];

	memcpy(data,buffer + 4,size);
	data[size]=0;
	printf("%s",data);

	if(strcmp(data + 5, users[INDEKS_USERA]) == 0) //provera user-a
	{
		pass_valid = 1;
	}
	
	char *message;
	if(pass_valid == 1)
	{
		//Send some data
		strcpy(message, "+OK\r\n\0");
		if( send(m_Client_Socket , message , strlen(message), 0) < 0)
		{
			puts("Send failed");
			return;
		}

		SetState(FSM_SR_MAIL_CHECK);

	}else{
		strcpy(message, "-ERROR\r\n\0");
		if( send(m_Client_Socket , message , strlen(message), 0) < 0)
		{
			puts("Send failed");
			return;
		}

		SetState(FSM_SR_IDLE);
	}
	
}

void SrAuto::FSM_Sr_Mail_Check(){
	char* data = new char[255];
	uint8* buffer = GetParam(PARAM_DATA);
	uint16 size = buffer[2];

	memcpy(data,buffer + 4,size);
	data[size]=0;
	printf("%s",data);


}

void SrAuto::NetMsg_2_FSMMsg(const char* apBuffer, uint16 anBufferLength) {
	
	PrepareNewMessage(0x00, MSG_MSG);
	SetMsgToAutomate(SR_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	AddParam(PARAM_DATA,anBufferLength,(uint8 *)apBuffer);
	SendMessage(SR_AUTOMATE_MBX_ID);
	
}


DWORD SrAuto::ServerListener(LPVOID param) {
	SrAuto* pParent = (SrAuto*) param;
	int nReceivedBytes;
	char* buffer = new char[255];

	
	/* Receive data from the network until the socket is closed. */ 
	do {
		nReceivedBytes = recv(pParent->m_Client_Socket, buffer, 255, 0);
		if (nReceivedBytes == 0)
		{
			printf("Disconnected from server!\n");
			pParent->FSM_Sr_Disconnect();
			break;
		}
		if (nReceivedBytes < 0) {
			printf("error\n");
			DWORD err = WSAGetLastError();
			break;
		}
		pParent->NetMsg_2_FSMMsg(buffer, nReceivedBytes);

		Sleep(1000); 
			
	} while(1);

	

	delete [] buffer;
	return 1;
}


// Automat sam sebi salje poruku za start sistema.
void SrAuto::Start(){

	PrepareNewMessage(0x00, MSG_Set_All);
	SetMsgToAutomate(SR_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	SendMessage(SR_AUTOMATE_MBX_ID);

}