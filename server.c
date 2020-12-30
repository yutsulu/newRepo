//张若
//2020111456
//网络软件设计

#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>

#define PORT 23
#define BACKLOG 5

int main() {
	int socketFD=0,remoteFD=0;
	struct sockaddr_in localAddr={0}; 
	struct sockaddr_in remoteAddr={0};
	char buf[4096]={0} ;
	
	if((socketFD=socket(AF_INET,SOCK_STREAM,0))<0){
		printf("创建套接字失败！\n");
		return 0;
	}
	//构建本机地址结构体 
	localAddr.sin_family=AF_INET;
	localAddr.sin_port=htons(PORT);  //host to network short，将主机字节序转换成网络字节序 
	localAddr.sin_addr.s_addr=htonl(INADDR_ANY);  //host to network long，INADDR_ANY绑定到0.0.0.0 
	//绑定地址结构体和新创建的套接字 
	if((bind(socketFD,&localAddr,sizeof(localAddr)))<0){
		printf("绑定失败！\n");
		return 0; 
	} 
	
	//监听请求,BACKLOG为请求队列最大数 
	if(listen(socketFD,BACKLOG)<0) {
		printf("监听失败！\n")
		return 0; 
	}
	
	//接收请求
	if((remoteFD=accept(socketFD,&remoteAddr,sizeof(remoteAddr)))<0) {
		printf("接收失败！\n");
		return 0; 
	}else{
		printf("接收成功！\n");
		send(remoteFD,"连接成功",sizeof("连接成功"),0);
	}
	//接收客户端数据 
	if(recvLen=recv(remoteFD,buf,sizeof(buf),0)<=0){
		printf("接收失败或对端连接关闭！\n");
	} else{
		printf("recvLen:%d\n",recvLen);
		printf("buf:%s\n",buf);
	}
	close(remoteFD);
	close(socketFD);
	return 0;
	
}