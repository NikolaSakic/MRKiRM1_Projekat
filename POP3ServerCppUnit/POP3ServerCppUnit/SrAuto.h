#pragma ones

#ifndef _Sr_AUTO_H_
#define _Sr_AUTO_H_

#include <fsm.h>
#include <fsmsystem.h>

#include "../kernel/stdMsgpc16pl16.h"
#include "NetFSM.h"

#define NUM_USERS 4


class SrAuto : public FiniteStateMachine {

	// for FSM
	StandardMessage StandardMsgCoding;

	MessageInterface *GetMessageInterface(uint32 id);
	void	SetDefaultHeader(uint8 infoCoding);
	void	SetDefaultFSMData();
	void	NoFreeInstances();
	void	Reset();
	uint8	GetMbxId();
	uint8	GetAutomate();
	uint32	GetObject();
	void	ResetData();

public:
	SrAuto();
	~SrAuto();

	void Initialize();

	void FSM_Sr_Idle();
	void FSM_Sr_Authorising_username();
	void FSM_Sr_Authorising_pass();
	void FSM_Sr_Mail_Check();
	void FSM_Sr_Sending();
	void FSM_Sr_Deleting();
	void FSM_Sr_Disconnect();
	void FSM_Sr_Last_State();

	void Start();

	//bool FSMMsg_2_NetMsg();
	void NetMsg_2_FSMMsg(const char* apBuffer, uint16 anBufferLength);

protected:
	static DWORD WINAPI ServerListener(LPVOID);

	int m_MessageCount;
	char m_UserName[20];
	char m_Password[20];

	SOCKET m_Server_Socket;
	SOCKET m_Client_Socket;
	struct sockaddr_in server;
	struct sockaddr_in client;

	HANDLE m_hThread;
	DWORD m_nThreadID;

};

#endif /* _Sr_AUTO_H */