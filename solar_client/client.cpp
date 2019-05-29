#include "kNet.h"
#include "kNet/DebugMemoryLeakCheck.h"

using namespace kNet;

// Define a MessageID for our a custom message.
const message_id_t cHelloMessageID = 32;

BottomMemoryAllocator bma;
char com[100];
std::string mess;

int main(int argc, char **argv)
{
   	if (argc < 2)
   	{
      		std::cout << "Usage: " << argv[0] << " server-ip" << std::endl;
      		return 0;
   	}

	kNet::SetLogChannels(LogUser | LogInfo | LogError);
	EnableMemoryLeakLoggingAtExit();

   	Network network;
	const unsigned short cServerPort = 32000;
std::cout << "\ntest\n"<<std::endl;
   	//Ptr(MessageConnection) connection = network.Connect(argv[1], cServerPort, SocketOverUDP,  &listener);
   	Ptr(MessageConnection) connection = network.Connect(argv[1], atoi(argv[2]), SocketOverUDP,  NULL);
    //Ptr(MessageConnection) connection = network.Connect(argv[1], cServerPort, SocketOverUDP,  NULL);
  
	std::cin.getline(com,sizeof(com));
	while (com[0]!='X')
	{
        	if (connection)
        	{
                	connection->SendMessage(cHelloMessageID, true, true, 100, 0, com, strlen(com));
                	printf("message sent: [%s]\n",com);

        	}
		std::cin.getline(com,sizeof(com));
	}
   
   	return 0;
}

