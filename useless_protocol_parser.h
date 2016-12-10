/*
 * useless_protocol_parser.h
 *
 *  Created on: 10 дек. 2016 г.
 *      Author: gorec
 */

#ifndef USELESS_NL_CORE_USELESS_PROTOCOL_PARSER_H_
#define USELESS_NL_CORE_USELESS_PROTOCOL_PARSER_H_

#include <list>
#include <string>

class UselessProtocolParser{
public:
	static int PrepareMsgData(std::list<std::string> &src_strings, std::string &result_data);
	static int ParseMsg(std::string &src_data, std::list<std::string> &result_strings);
	static int WithdrawPid(std::string &msg);
	static int InjectPid(std::string &msg, uint32_t pid);
private:

};

#endif /* USELESS_NL_CORE_USELESS_PROTOCOL_PARSER_H_ */
