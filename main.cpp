
#include "VirtualP2PServer.hh"
int main(int argc,char** argv){
	VirtualP2PServer* p2pserver = VirtualP2PServer::CreateNew();
	p2pserver->Start();
	return 0;
}

