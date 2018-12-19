#ifndef _CONST_H_
#define _CONST_H_

#include "./kernelTypes.h"


//#define D_TESTING

const uint8 SR_AUTOMATE_TYPE_ID = 0x00;
const uint8 TEST_AUTOMATE_TYPE_ID = 0x01;

const uint8 SR_AUTOMATE_MBX_ID = 0x00;
const uint8 TEST_MBX_ID = 0x01;

//	Server states
enum ClStates {	FSM_SR_IDLE, 
				FSM_SR_AUTHORISING_USERNAME, 
				FSM_SR_AUTHORISING_PASS,
				FSM_SR_MAIL_CHECK,
				FSM_SR_SENDING,
				FSM_SR_DELETING,
				FSM_SR_DISCONNECT };

const uint16 MSG_Set_All = 0x0001;
const uint16 MSG_MSG = 0x0002;

#define ADRESS "10.81.35.43" /*"localhost"*/
#define PORT 110

#define PARAM_DATA 0x01
#define PARAM_Name 0x02
#define PARAM_Pass 0x03



#endif //_CONST_H_