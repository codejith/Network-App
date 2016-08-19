#include <boost/filesystem.hpp>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <fstream>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstdlib>
#include "Serializer.h"

class Client {
    int                      _clientSock;
    int                      _portNo;
    char                     _buffer[4096];
    std::string              _clientName;
    std::string              _path;
    struct hostent          *_server;
    struct sockaddr_in       _server_addr;
    boost::filesystem::path _p;
public:
    Client(const int portNo_, const std::string name_, const boost::filesystem::path p_);
    ~Client();
    
    bool setUp();
    bool start();
    bool operation();
    void setPath(boost::filesystem::path &p);
    void pathFinder();
    bool sendBuffer(Serializer &ob);
    void startSession(Serializer & ob);
    bool fileCopy(Serializer &ob);
    void setClientName(std::string name);
};

Client:: Client(const int portNo_, const std::string name_, const boost::filesystem::path p_):_portNo(portNo_), _clientName(name_),_p(p_)
{
        _clientSock = socket (AF_INET, SOCK_STREAM, 0);
        _server = gethostbyname("127.0.0.1");
        if (-1 == _clientSock)
        {
            std::cout << "Socket Creation failed" <<std::endl;    
        }
    }


Client::~Client ()
{
    if (0 == close(_clientSock))
    {
        std::cout << "Client Connection Closed Successfully" << std::endl;
    }
    else
    {
        std::cout << "Error in closing connection" << std::endl;
    }
    
}


void Client :: setClientName(std::string name_)
{
    _clientName = name_;
}


void Client :: setPath(boost::filesystem::path &p)
{
    _path = p.string();
}

bool Client :: setUp()
{
        
    _server_addr.sin_family = AF_INET;
    _server_addr.sin_port = htons (_portNo);
    bcopy((char *)_server->h_addr, (char *)&_server_addr.sin_addr.s_addr, _server->h_length);
    // _server_addr.sin_addr.s_addr = INADDR_ANY;
    if (-1 == connect(_clientSock, (struct sockaddr *) &_server_addr, sizeof(_server_addr)))
    {
        std::cout << "Connection Error"  << std::endl;
        return false;
    }
    
    if (!start())
    {
        std::cout << "Operational Error "  << std::endl;
    }
    
    
    return true;
}


bool Client :: start()
{
    char choice;
    int ans;
    do
    {
        
        std::cout << "1 . Open a new connection" << std::endl;
        std::cout << "2 . Exit" << std::endl;
        std::cout << "Enter a choice : ";
        std::cin >> ans;
        switch(ans)
        {
        case 1 :
        {
            Serializer ob;
            startSession(ob);
            if (!sendBuffer(ob))
            {
                std::cout << "Sending failed" << std::endl;
            }
            else
            {
                std::cout << "Sending successful" << std::endl;
            }
            break;
        }
        case 2: exit(0);
            
            
        default: std::cout << "Invalid Choice "  << std::endl;
            break;
        }
        std::cout << "Do you want to close connection (Y/N)"  << std::endl;
        std::cin >> choice;
        
    }while (choice == 'Y' || choice == 'y');
    
    
    
    return true;
}

bool Client :: sendBuffer(Serializer &ob)
{
    std::size_t readSize = 0;
    std::cout  << "Sending Buffer Size = " << ob.getBufSize() << std::endl;
    
    while (readSize < ob.getBufSize())
    {
        std::pair<size_t, const char *> bufPair = ob.getBufferAt(readSize, 4096);
        if (0 > send(_clientSock, bufPair.second, bufPair.first, 0))
        {
            std::cout << "Sending failed"  << std::endl;
            return false;
        }
        
        std::cout  << "readSize = " << readSize << std::endl;
        readSize += bufPair.first;
    }
    
    return true;
}

void Client :: startSession(Serializer & ob)
{   
    char choice;
    ob.serializeBuf(_clientName, "start");
    do
    {
        if (fileCopy(ob))
        {
            std::cout << "Filecopy called"  << std::endl;
        }
         else
        {
            std::cout << "Filecopy not  called"  << std::endl;
        }
         
         std::pair<bool,std::string> pairBuf = ob.consumeAndGetStr(0, ob.getBufSize());
         
         std::cout << pairBuf.second << std::endl;
         
         std::cout << "Do you want to continue (Y/N)"  << std::endl;
        std::cin >> choice;
        
    }while (choice == 'Y' || choice == 'y');
    
    ob.serializeBuf(_clientName, "end");
}


bool Client :: fileCopy(Serializer &ob)
{
    setPath(_p);
    pathFinder();
    std::string name, line;
    std::cout << "Enter the filename : "  << std::endl;
    std::cin >> name;
    
    std::ifstream readFile (_path+name, std::ios::in | std::ios::binary);
    std::cout << "GGG : "  << _path+name  <<std::endl;
    if (readFile.is_open())
    {
        // concatinate _path and name
        ob.serializeFile(_path, name);
        ob.appendFile(readFile);
        
    }
    else
    {
        std::cout << "Failed to open file " << std::endl;
    }
    
    readFile.close();
    return true;
}

void Client ::  pathFinder()
{
    
    try
    {
        
        //  std::cout << "AAAA : " << temp <<std::endl;
        if (boost::filesystem::exists(_p))
        {
            if (boost::filesystem::is_regular_file(_p))
            {
                //Open up and write function 
            }
            else if(boost::filesystem::is_directory(_p))
            {
                std::cout << _p  <<" is a directory nd contains : " << std::endl;
               copy(boost::filesystem::directory_iterator(_p),
                    boost::filesystem::directory_iterator(),
                    std::ostream_iterator<boost::filesystem::directory_entry>(std::cout, "\n") ); 
            }
            else
            {
                std::cout << "FileSystem or Directory does not exist" << std::endl;
            }
        }
        else
        {
            std::cout << "Invalid file path" << std::endl;    
        }
    }
    catch(const boost::filesystem::filesystem_error &er)
    {
        std::cout << er.what() << std::endl;
    }

}




int main(int argc, char * argv[])
{
    boost::filesystem::path p(argv[2]);
    std::string name  = argv[1];
    std::cout << name;
    int portNo;
    std::cout << "Enter the port no : " << std::endl;
    std::cin >> portNo;
    
    
    Client client(portNo,name,p);
    client.setClientName(name);
    if (client.setUp())
    {
        if (!client.start())
        {
            std::cout << "Client crashed"  << std::endl;
        }
    }
    else
    {
        std::cout << "Client setUp failed"  << std::endl;
    }
    
    return 0;
}
