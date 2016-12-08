/*
 * uselessnl_core.cpp
 *
 *  Created on: 7 дек. 2016 г.
 *      Author: gorec
 */

#include "uselessnl_core.h"
#include <linux/netlink.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

UselessNLCore::UselessNLCore(){
	thread_addr.valid=false;
	thread_addr.pid=0;
	thread_addr.sock_fd=0;
}

UselessNLCore::~UselessNLCore(){
	if(thread_addr.valid){
		close(thread_addr.sock_fd);
	}
}

int UselessNLCore::Init(int uniq_thread_id){
	int ret=0;
	struct sockaddr_nl src_addr;
	uint32_t thread_id=pthread_self();
	thread_addr.pid=((( uniq_thread_id&0x3f ) ) << 16) | getpid();
	thread_addr.sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USELESS_P);
	if (thread_addr.sock_fd < 0){
		ret=1;
	}
	if(ret==0){
		memset(&src_addr, 0, sizeof(src_addr));
		src_addr.nl_family = AF_NETLINK;
		src_addr.nl_pid = thread_addr.pid; /* self pid */
		ret=bind(thread_addr.sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));
	}
	if(ret==0){
		thread_addr.valid=true;
	}
	else{
		thread_addr.pid=0;
		close(thread_addr.sock_fd);
	}
	return thread_addr.pid;
}

int UselessNLCore::SendMsg(UselessNLMsg &useless_msg){
	struct sockaddr_nl dest_addr;
	int ret=0;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	struct msghdr msg;

	assert(useless_msg.data_len<=NL_MAX_PAYLOAD);
	assert(thread_addr.valid);

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0; /* For Linux Kernel */
	dest_addr.nl_groups = 0; /* unicast */

	nlh = (struct nlmsghdr *) ::operator new(NLMSG_SPACE(useless_msg.data_len));

	memset(nlh, 0, NLMSG_SPACE(useless_msg.data_len));
	nlh->nlmsg_len = NLMSG_SPACE(useless_msg.data_len);
	nlh->nlmsg_pid = thread_addr.pid;
	nlh->nlmsg_flags = 0;
	nlh->nlmsg_type=useless_msg.type;
	if(useless_msg.data){
		memcpy(NLMSG_DATA(nlh), useless_msg.data,useless_msg.data_len);
	}

	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control=0;
	msg.msg_controllen=0;

	ret=sendmsg(thread_addr.sock_fd, &msg, 0);
	delete nlh;
	return ret;
}

int UselessNLCore::RecvMsg(UselessNLMsg &useless_msg){
	struct sockaddr_nl dest_addr;
	int ret=0;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	struct msghdr msg;

	assert(useless_msg.data_len>=NL_MAX_PAYLOAD);
	assert(thread_addr.valid);

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0; /* For Linux Kernel */
	dest_addr.nl_groups = 0; /* unicast */

	nlh = (struct nlmsghdr *)::operator new(NLMSG_SPACE(NL_MAX_PAYLOAD));

	memset(nlh, 0, NLMSG_SPACE(NL_MAX_PAYLOAD));
	nlh->nlmsg_len = NLMSG_SPACE(NL_MAX_PAYLOAD);
	nlh->nlmsg_pid = thread_addr.pid;
	nlh->nlmsg_flags = 0;

	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control=0;
	msg.msg_controllen=0;

	/* Read message from kernel */
	ret=recvmsg(thread_addr.sock_fd, &msg, 0);
	if(ret!=-1){
		useless_msg.data_len=nlh->nlmsg_len-NLMSG_HDRLEN;
		useless_msg.type=nlh->nlmsg_type;
		if(ret>0 && useless_msg.data && useless_msg.data_len>0){
			memcpy(useless_msg.data,NLMSG_DATA(nlh),useless_msg.data_len);
		}
	}
	delete nlh;
	return ret;
}

int UselessNLCore::GetErrorCode(UselessNLMsg &msg){
	int code=-1;
	if(msg.type==NLMSG_ERROR){
		if(msg.data!=0 && msg.data_len>=sizeof(struct nlmsgerr)){
			struct nlmsgerr *error=(struct nlmsgerr *)msg.data;		//todo return header
			code=error->error;
		}
	}
	return code;
}

int UselessNLCore::GetErrorMsg(UselessNLMsg &msg, std::string &err_str){
	int ret=0;
	int error_code=GetErrorCode(msg);
	if(error_code!=-1){
		switch(error_code){
			case ERROR_NO:
				err_str.assign("ACK received");
				break;
			case ERROR_WRONG_MSG_TYPE:
				err_str.assign("Error: wrong message type");
				break;
			case ERROR_DAEMON_UNREACHABLE:
				err_str.assign("Error: daemon unreachable");
				break;
			case ERROR_TRANSFER_FAILED:
				err_str.assign("Error: transfer failed");
				break;
			default:
				err_str.assign("Unknown error");
				break;
		}
	}
	else{
		ret=1;
	}
	return ret;
}


