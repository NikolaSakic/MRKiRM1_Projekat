#include <stdio.h>
#include <conio.h>
#include <time.h>

#include "const.h"
#include "UserAuto.h"

bool g_ProgramEnd = false;

const int height = 5;
const int width = 5;

#define StandardMessageCoding 0x00

UserAuto::UserAuto() : FiniteStateMachine(USER_AUTOMATE_TYPE_ID, USER_AUTOMATE_MBX_ID, 0, 3, 3) {
}

UserAuto::~UserAuto() {
}


uint8 UserAuto::GetAutomate() {
	return USER_AUTOMATE_TYPE_ID;
}

/* This function actually connnects the UserAuto with the mailbox. */
uint8 UserAuto::GetMbxId() {
	return USER_AUTOMATE_MBX_ID;
}

uint32 UserAuto::GetObject() {
	return GetObjectId();
}

MessageInterface *UserAuto::GetMessageInterface(uint32 id) {
	return &StandardMsgCoding;
}

void UserAuto::SetDefaultHeader(uint8 infoCoding) {
	SetMsgInfoCoding(infoCoding);
	SetMessageFromData();
}

void UserAuto::SetDefaultFSMData() {
	SetDefaultHeader(StandardMessageCoding);
}

void UserAuto::NoFreeInstances() {
	printf("[%d] UserAuto::NoFreeInstances()\n", GetObjectId());
}

void UserAuto::Reset() {
	printf("[%d] UserAuto::Reset()\n", GetObjectId());
}


void UserAuto::Initialize() {
	SetState(FSM_User_Idle);	
	
	//intitialization message handlers
	InitEventProc(FSM_User_Idle, MSG_Set_All, (PROC_FUN_PTR)&UserAuto::FSM_User_Idle_Set_All);
	InitEventProc(FSM_User_Connecting, MSG_User_Connected, (PROC_FUN_PTR)&UserAuto::FSM_User_Connecting_User_Connected);
	InitEventProc(FSM_User_Connecting, MSG_User_Connecton_Fail, (PROC_FUN_PTR)&UserAuto::FSM_User_Connecting_User_Connecton_Fail);
	InitEventProc(FSM_User_Connected, MSG_Mail, (PROC_FUN_PTR)&UserAuto::FSM_User_Connected_Mail);
	InitEventProc(FSM_User_Connected, MSG_User_Save_Mail, (PROC_FUN_PTR)&UserAuto::FSM_User_Connected_User_Save_Mail);
	InitEventProc(FSM_User_Connected, MSG_User_Disconnected, (PROC_FUN_PTR)&UserAuto::FSM_User_Connected_User_Disconnected);
}

void UserAuto::FSM_User_Idle_Set_All(){

	PrepareNewMessage(0x00, MSG_User_Check_Mail);
	SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	SendMessage(CL_AUTOMATE_MBX_ID);

	SetState(FSM_User_Connecting);
}

void getPosition(char c, char* key, int* position){
	
	int i;
	for(i = 0; i < strlen(key); i++)
	{
		if(key[i] == c)
			break;
	}

	position[0] = i / width;
	position[1] = i - position[0]*width;

}

void PlayFair(char* text)
{
	char key[] = "FTNABCDEGHIKLMOPQRSUVWXYZ\0";
	int position1[2];
	int position2[2];

	int size = strlen(text);
	if(size%2 == 1)
	{
		text[size] = 'Z';
		text[size+1] = '\0';
	}

	for(int i=0; i < strlen(text); i+=2)
	{
		getPosition(text[i], key, position1);
		getPosition(text[i+1], key, position2);

		if(position1[0] == position2[0] && position1[1] == position2[1]){
			text[i+1] = 'X';
		}
		else if(position1[0] == position2[0]){
			text[i] = key[position1[0]*width + (++position1[1]%width)];
			text[i+1] = key[position2[0]*width + (++position2[1]%width)];
		}else if (position1[1] == position2[1]){
			text[i] = key[(++position1[0]%height)*width + position1[1]];
			text[i+1] = key[(++position2[0]%height)*width + position2[1]];
		}
		else{
			text[i] = key[position1[0]*width + position2[1]];
			text[i+1] = key[position2[0]*width + position1[1]];
		}
	}
}

void UserAuto::FSM_User_Connecting_User_Connected(){

	char name[20];
	char pass[20];
	//printf("Username: ");
	//scanf("%s",name);
	strcpy(name,"NIKOLA\0");
	//printf("Pasword: ");
	//scanf("%s",pass);
	strcpy(pass,"ABCD\0");

	PlayFair(name);
	PlayFair(pass);

	PrepareNewMessage(0x00, MSG_User_Name_Password);
	SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	AddParam(PARAM_Name,strlen(name),(uint8*)name);
	AddParam(PARAM_Pass,strlen(pass),(uint8*)pass);
	SendMessage(CL_AUTOMATE_MBX_ID);

	SetState(FSM_User_Connected);

	m_File = fopen("temp.dat","w+b");
}

void UserAuto::FSM_User_Connecting_User_Connecton_Fail(){

	printf("Connection fail!!!\n");
	
	g_ProgramEnd = true;

	SetState(FSM_User_Idle);

}

void UserAuto::FSM_User_Connected_Mail(){

	char* data = new char[256];
	uint8* buffer = GetParam(PARAM_DATA);
	uint16 size = buffer[2];

	memcpy(data,buffer + 4,size);
	data[size]=0;
	fprintf(m_File,"%s",data);
	printf("%s",data);
	//printf("\n---------------------------------------------------------------------\n");

	//delete [] data;
}

void UserAuto::FSM_User_Connected_User_Save_Mail(){

	rewind(m_File);

	time_t ltime;
	time( &ltime );
    
	char* data = new char[1000];

	bool l_End = false;

	while( !feof(m_File) && !l_End){

		fgets(data,1000,m_File);

		char* temp = strstr(data,"Subject:");
		if( temp != NULL){
			l_End = true; // end of search for "Subject:"
			
			int i = 0 ,j = 0;
			temp += 9;

			while((j<20) && (temp[i] != '\r') ){
				if(isalpha(temp[i]) || isdigit(temp[i]) || isspace(temp[i])){
					m_FileName[j] = temp[i];
					j++;
				}
				i++;
			}
			memcpy(m_FileName+j,".dat\0",5);

			printf("Mail saved in file: %s\n",m_FileName);
		}

	}

	fclose(m_File);
	
	rename("temp.dat",m_FileName);

	m_File = fopen("temp.dat","w+b");
	
	delete [] data;
}

void UserAuto::FSM_User_Connected_User_Disconnected(){

	printf("User disconnected!\n");

	fclose(m_File);
		
	remove("temp.dat");

	g_ProgramEnd = true;

	SetState(FSM_User_Idle);
}

// Automat sam sebi salje poruku za start sistema.
void UserAuto::Start(){

	PrepareNewMessage(0x00, MSG_Set_All);
	SetMsgToAutomate(USER_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	SendMessage(USER_AUTOMATE_MBX_ID);

}
