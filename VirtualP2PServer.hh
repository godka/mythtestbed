#pragma once
#ifdef WIN32
#include <WinSock2.h>
#pragma comment(lib, "ws2_32")
typedef int socklen_t;
#else
#include <wchar.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include "cJSON.h"
#define HOSTIP "121.42.136.168"
#define HOSTPORT 23584
class PeerStatus
{
  public:
	PeerStatus(std::string tag)
	{
		_last_queuing_delay = ~0;
		_delay = 0;
		_bytes = 0;
		_index = 0;
		_tag = tag;
	}
	void Set(long queuing_delay, int bytes)
	{
		long _real_delay = 0;
		if(_last_queuing_delay != ~0){
			_real_delay = queuing_delay - _last_queuing_delay;
		}
		_last_queuing_delay = queuing_delay;
		_delay += _real_delay;
		_bytes += bytes;
		_index++;
	}

	void Get(long *delay, int *bytes)
	{
		if (_index > 0)
		{
			_delay /= _index;
			//_bytes /= _index;
			*delay = _delay;
			*bytes = _bytes;
		}
		else
		{
			*delay = 0;
			*bytes = 0;
		}
		_delay = 0;
		_bytes = 0;
		_index = 0;
	}

  private:
	long _last_queuing_delay;
	long _delay;
	int _bytes;
	int _index;
	std::string _tag;
};
class VirtualP2PServer
{
  public:
	static VirtualP2PServer *CreateNew()
	{
		return new VirtualP2PServer();
	}
	VirtualP2PServer();
	~VirtualP2PServer();
	static void *StartThreadStatic(void *data)
	{
		if (data)
		{
			VirtualP2PServer *tmp = (VirtualP2PServer *)data;
			tmp->StartThread();
		}
		return NULL;
	}

	int Start();
	int Stop();

  private:
	unsigned long mythTickCount()
	{
#ifdef WIN32
		return GetTickCount();
#else
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#endif
	}
	int InitSock();

  protected:
	void AddAttrs(sockaddr_in* sender,unsigned long time_recv,int bytes_recv);

	void udpsend(char *ip, int port, cJSON *msg);
	void udpsend(sockaddr_in *addr, cJSON *msg);
	void hostsend(cJSON *msg);

	int StartThread();
#ifdef WIN32
	SOCKET PrimaryUDP;
#else
	int PrimaryUDP;
#endif
	bool mrunning;
	int recv_within_time(int fd, char *buf, size_t buf_n, struct sockaddr *addr, socklen_t *len, unsigned int sec, unsigned usec);
	std::map<std::string, PeerStatus *> _peermap;
};
