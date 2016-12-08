/*
 * uselessnl_core.h
 *
 *  Created on: 7 дек. 2016 г.
 *      Author: gorec
 */

#ifndef USELESSNL_CORE_H_
#define USELESSNL_CORE_H_

#include "useless_nl_config.h"
#include <stdint.h>
#include <string>

class UselessNLMsg{
public:
	UselessNLMsg(uint16_t type){
		data_allocated=false;
		this->type=type;
		data=0;
		data_len=0;
	}
	UselessNLMsg(uint16_t type, int data_len){
		data_allocated=true;
		this->type=type;
		data=(char *) operator new(data_len);
		this->data_len=data_len;
	}
	~UselessNLMsg(){
		if(data_allocated){
			delete data;
		}
	}
	uint16_t type;
	char *data;
	int data_len;
private:
	bool data_allocated;
};

class UselessNLCore{
public:
	UselessNLCore();
	~UselessNLCore();
	int Init(int uniq_thread_id);
	int SendMsg(UselessNLMsg &useless_msg);
	int RecvMsg(UselessNLMsg &useless_msg);
	int GetErrorMsg(UselessNLMsg &msg,std::string &err_str);
	int GetErrorCode(UselessNLMsg &msg);
private:
	struct UselessNLAddr{
		int sock_fd;
		uint32_t pid;
		bool valid;
	};
	UselessNLAddr thread_addr;
};

#endif /* USELESSNL_CORE_H_ */
