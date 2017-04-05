#pragma once

#define CMD_JOIN					('0' << 8) | 'J'
#define CMD_LOGIN					('0' << 8) | 'L'
#define CMD_LOGOUT					('1' << 8) | 'L'
#define CMD_CREATE_ROOM				('R' << 8) | 'C'
#define CMD_ENTER_ROOM				('R' << 8) | 'E'
#define CMD_LEAVE_ROOM				('R' << 8) | 'L'
#define CMD_START_ROOM				('R' << 8) | 'S'
#define CMD_GET_ROOM_LIST			('L' << 8) | 'R'

#define CMD_FRIEND_FIND_ID			('I' << 8) | 'F'
#define CMD_FRIEND_FIND_NAME		('N' << 8) | 'F'
#define CMD_SAVE_PROFILE_IMAGE		('I' << 8) | 'S'
#define CMD_LOAD_PROFILE_IMAGE		('I' << 8) | 'L'

#define CMD_GET_USER_INFO			('I' << 8) | 'U'
#define CMD_FRIEND_ADD				('A' << 8) | 'F'
#define CMD_GET_FRIEND_LIST			('F' << 8) | 'G'
#define CMD_KEY_PACKET				('I' << 8) | 'K'
#define CMD_GET_USER_LIST_INROOM	CMD_KEY_PACKET

#define CMD_FRIEND_ADD_SUCCESS		('0' << 8) | 'F'
#define CMD_FRIEND_ADD_FAIL			('9' << 8) | 'F'
#define CMD_JOIN_SUCCESS			('S' << 8) | 'J'
#define CMD_LOGIN_SUCCESS			('S' << 8) | 'L'
#define CMD_JOIN_FAIL				('F' << 8) | 'J'
#define CMD_LOGIN_FAIL				('F' << 8) | 'L'
#define CMD_DB_SUCCESS				('D' << 8) | 'S'
#define CMD_DB_FAIL					('E' << 8) | 'E'
#define CMD_NOT_FOUND_CMD			('3' << 8) | '0'

#define CMD_STAGE_RESULT			('0' << 8) | 'R'
#define CMD_CONNECT_STAGE			('S' << 8) | 'C'
#define CMD_LOGED_IN				('1' << 8) | '1'
#define CMD_POS_STATUS_PACKET		('S' << 8) | 'S'
#define CMD_GET_POS_STATUS_PACKET		('S' << 8) | 'G'


//////////////////////////////////////////////////////////////////
#define CMD_SET_VOICE_PACKET		('V' << 8) | 'S'
#define CMD_GET_VOICE_PACKET		('V' << 8) | 'G'
#define CMD_START_VOICE				('V' << 8) | 'V'



#define CMD_VOTE_START				('S' << 8) | 'V'
#define CMD_VOTE_AGREE				('A' << 8) | 'V'
#define CMD_VOTE_DISAGREE			('D' << 8) | 'V'
#define CMD_VOTE_NULLITY			('N' << 8) | 'V'
#define CMD_GET_VOTE_RESULT			('R' << 8) | 'V'