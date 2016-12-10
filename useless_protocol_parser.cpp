/*
 * useless_protocol_parser.cpp
 *
 *  Created on: 10 дек. 2016 г.
 *      Author: gorec
 */

#include "useless_nl_config.h"
#include "useless_protocol_parser.h"
#include <stdint.h>

int UselessProtocolParser::PrepareMsgData(std::list<std::string> &src_strings, std::string &result_data){
	int ret=0;
	result_data.clear();
	for(auto &it:src_strings){
		if(it.length()>NL_MAX_STR_LEN){
			ret=1;
			break;
		}
		uint32_t str_len=(uint32_t)it.length();
		result_data.append((char*)&str_len,sizeof(str_len));
		result_data.append(it);
	}
	if(result_data.length()>NL_MAX_PAYLOAD){
		ret=1;
	}
	return ret;
}

int UselessProtocolParser::ParseMsg(std::string &src_data, std::list<std::string> &result_strings){
	int ret=0;
	int msg_len=src_data.length();
	uint32_t cur_strlen=0;
	int offset=0;

	if(msg_len>NL_MAX_PAYLOAD){
		ret=1;
	}
	if(ret==0){
		while(offset<msg_len){
			cur_strlen=*((uint32_t *)(src_data.data()+offset));
			if(cur_strlen>NL_MAX_STR_LEN){
				ret=1;
				break;
			}
			offset+=sizeof(uint32_t);
			result_strings.push_back(std::string(src_data.data()+offset,cur_strlen));
			offset+=cur_strlen;
		}
	}

	if(offset>msg_len){
		ret=1;
	}
	return ret;
}

int UselessProtocolParser::WithdrawPid(std::string &msg){
	uint32_t pid=0;
	if(msg.length()>sizeof(pid)){
		pid=*((uint32_t *)msg.data());
		msg.erase(0,sizeof(pid));
	}
	return pid;
}

int UselessProtocolParser::InjectPid(std::string &msg, uint32_t pid){
	int ret=1;
	if(msg.length()<=(NL_MAX_PAYLOAD-sizeof(pid))){
		msg.insert(0,(char*)(&pid),sizeof(pid));
		ret=0;
	}
	return ret;
}
