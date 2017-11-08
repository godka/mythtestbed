#include "VirtualP2PServer.hh"

VirtualP2PServer::VirtualP2PServer()
{
	InitSock();
	mrunning = false;
	_peermap.clear();
}


VirtualP2PServer::~VirtualP2PServer()
{
}

int VirtualP2PServer::InitSock()
{
#ifdef WIN32
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){
		printf("Windows sockets 2.2 startup");
		return 1;
	}
	else{
		printf("init winsock32 success\n");
	}
#endif
	return 0;
}

int VirtualP2PServer::Stop(){
	mrunning = false;
	return 0;
}

#define RECV_LOOP_COUNT 1
int VirtualP2PServer::recv_within_time(int fd, char *buf, size_t buf_n, struct sockaddr* addr, socklen_t *len, unsigned int sec, unsigned usec)
{
#if 1
	struct timeval tv;
	fd_set readfds;
	int i = 0;
	unsigned int n = 0;
	for (i = 0; i < RECV_LOOP_COUNT; i++)
	{
		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);
		tv.tv_sec = sec;
		tv.tv_usec = usec;
		select(fd + 1, &readfds, NULL, NULL, &tv);
		if (FD_ISSET(fd, &readfds))
		{
			if ((n = recvfrom(fd, buf, buf_n, 0, addr, len)) >= 0)
			{
				return n;
			}
		}
	}
	return -1;
#else
	return recvfrom(fd, buf, buf_n, 0, addr, len);
#endif
}

int VirtualP2PServer::Start()
{
	StartThreadStatic(this);
	//pthread_create(&pid, NULL, StartThreadStatic, this);
	return 0;
}

void VirtualP2PServer::AddAttrs(sockaddr_in* sender,unsigned long time_recv,int bytes_recv)
{
	unsigned long tick = mythTickCount();
	long queuing_delay = (long)tick - (long)time_recv;

	char tmp[64] = { 0 };
	sprintf(tmp, "%s.%d", inet_ntoa(sender->sin_addr), sender->sin_port);
	//printf("add attrs,%s,%lu,%d\n", tmp,time_recv,bytes_recv);
	PeerStatus* status = _peermap[tmp];
	if (!status){
		status = new PeerStatus(tmp);
		_peermap[tmp] = status;
	}
	status->Set(time_recv,bytes_recv);
}

void VirtualP2PServer::udpsend(sockaddr_in* addr, cJSON* msg){
	char* msgstr = cJSON_PrintUnformatted(msg);
	sendto(PrimaryUDP, msgstr, strlen(msgstr), 0, (sockaddr*) addr, sizeof(sockaddr));

}
void VirtualP2PServer::udpsend(char* ip, int port, cJSON* msg){
	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	char* msgstr = cJSON_PrintUnformatted(msg);
	sendto(PrimaryUDP, msgstr, strlen(msgstr), 0, (sockaddr*) &addr, sizeof(sockaddr));
}
void VirtualP2PServer::hostsend(cJSON* msg){
	udpsend(HOSTIP, HOSTPORT, msg);
}
int VirtualP2PServer::StartThread(){
	PrimaryUDP = socket(AF_INET, SOCK_DGRAM, 0);
	if (PrimaryUDP < 0){
		printf("create socket error");
		return 1;
	}
	sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(23583);
	local.sin_addr.s_addr = INADDR_ANY;
	int nResult = bind(PrimaryUDP, (sockaddr*) &local, sizeof(sockaddr));
	mrunning = true;
	sockaddr_in sender;
	int dwSender = sizeof(sender);
	char recvbuf[1501] = {0};
	printf("Start Loop! Port:%d\n", 23583);
	unsigned long tick = mythTickCount();
	unsigned long basictick = tick;
	while (mrunning){
		memset(recvbuf,0,1500);
		int ret = recv_within_time(PrimaryUDP, (char *) &recvbuf, 1500, (sockaddr *) &sender, (socklen_t*) &dwSender,0,10);
		if (ret > 0){
			//printf("received: %s\n",recvbuf);
			if (recvbuf[0] == '{' && recvbuf[ret - 1] == '}'){
				cJSON* root = cJSON_Parse(recvbuf);
				if (root){
					//cJSON* ret;
					cJSON* req = cJSON_GetObjectItem(root, "time");
					//int _starttime = 0;
					if (req){
						int _starttime = req->valueint;
						int _recvbytes = ret;
						this->AddAttrs(&sender,_starttime,_recvbytes);
					}
				}
				else{
					//maybe in p2p model
					printf("json parse error!\n");
				}
			}
		}
		unsigned long now = mythTickCount();
		if(now - tick > 100){//100ms
			for(auto &p:_peermap){
				long queuing_delay = 0;
				int bytes = 0;
				p.second->Get(&queuing_delay,&bytes);
				std::string filename = p.first + ".log";
				FILE* file = fopen(filename.c_str(),"a+");
				fprintf(file,"%lu,%ld,%d\n",now - basictick,queuing_delay,bytes);
				fclose(file);
			}
			tick = now;
		}
	}
	return 0;
}
