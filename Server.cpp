#include <boost/filesystem.hpp>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <strings.h>
#include <fstream>
#include <vector>
#include "Serializer.h"
class Server {
    
     int _sockId, _newSockId, _portNo,_count;
     std::string _fileName,_dirName;
     socklen_t _cliLen;
     char _buffer[4096];
     struct sockaddr_in _server_addr, _client_addr;
     std::vector<std::thread> v;
     std::thread _th;

     public:
    
          Server(int portNo_);
          ~Server ();
          bool setUp();
          bool start();
          void threadStuff(int sockid);
          void getData(Serializer &ob, size_t writePos);
          bool createDir();
          void writeFile(std::string &data);
    
  
};

Server::Server(int portNo_): _portNo(portNo_), _count(0)
{
    std::cout << "Server Constructor"  << std::endl;
    _sockId =  socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == _sockId)
    {
        std::cout << "Socket Creation failed" <<std::endl;
    }
}

Server::~Server ()
{
    if (0 == close(_sockId))
    {
        std::cout << "Server Connection Closed Successfully" << std::endl;
    }
    else
    {
        std::cout << "Error in closing connection" << std::endl;
    }
}


bool Server :: setUp()
{
    
    _server_addr.sin_family = AF_INET;
    _server_addr.sin_port  = htons(_portNo);
    _server_addr.sin_addr.s_addr = INADDR_ANY;
    
    // Binding the socket 
    if (-1 != bind(_sockId, (struct sockaddr *) &_server_addr, sizeof(_server_addr)))
    {
        
        return true;
    }
    else
    {
        
        return false;
    }
}

bool Server :: start()
{
    
    if ( -1 != listen(_sockId, 7))
    {
        // After successful listen
        
        while(1)
        {
            std::cout << "Listening For Clients..."  << std::endl;
            _cliLen =  sizeof(_client_addr);
            _newSockId =  accept(_sockId, (struct sockaddr *) &_client_addr, &_cliLen);
            if (-1 == _newSockId)
            {
                std::cout << "Error on accept" << std::endl;
                return false;
            }
            
            
            v.push_back(std::thread(&Server::threadStuff, this, _newSockId));
            
            //  for(auto &t : v)
            // {
            //     t.join();
            // } 
            
        }
    }
    else
    {
        return false;
    }
    return true;
}


void Server ::  threadStuff(int sockid)
{
    std::cout << "Cilent Connected"  << std::endl;
    unsigned int readSize;
    std::string newfile;
    // Receving section
    Serializer ob;
    while( (readSize =  recv(sockid, _buffer, 4096, 0) ) )
    {
        
        std::cout << "Received Size : "  << readSize <<std::endl;

        /*
        std::string temp;
        temp.assign(_buffer, readSize);
        ob.appendStr(temp);
        */

        for (int i=0; i<readSize; i++)
             ob.appendChar(_buffer[i]);

        
        //std::cout <<"XXX: "<< _buffer[5] <<std::endl;
        bzero(_buffer,sizeof(_buffer));
        
    }

    std::cout << "Recevier Buffer Size : " << ob.getBufSize() <<std::endl;
    
    std::pair<bool, unsigned int> bufPair = ob.consumeAndGetInt(1);

    if (bufPair.first)
    {
        std::cout << "Size : "  << bufPair.second  << std::endl;
        std::pair<bool, std::string> bufPair1 = ob.consumeAndGetStr(5,bufPair.second);
        _dirName = bufPair1.second;
        std::cout << "Directory Name : " << _dirName << std::endl;
        createDir();
        std::cout << "CurrentPos : " <<  5+bufPair.second << std::endl;
        getData(ob, 5+bufPair.second);
    }
   
    std::cout << "Recevier Buffer Size : " << ob.getBufSize() <<std::endl;
                
}

void Server::getData(Serializer &ob, size_t writePos)
{
    std::cout << "WRITE_POSITION : " << writePos  << std::endl;
    
    if (writePos>=ob.getBufSize())
    {
        std::cout << "return worked" << std::endl;
        return;
    }
    else
    {
        std::cout << "looping" << std::endl;
        while(writePos < ob.getBufSize())
        {
            std::pair<bool, char> bufChar = ob.consumeAndGetChar(writePos);
            if (bufChar.first)
            {
                if (bufChar.second=='F')
                {
                    ++writePos;
                    std::cout << "CurrentPos : " <<  writePos << std::endl;
                    std::pair<bool, int> bufFSize = ob.consumeAndGetInt(writePos);
                    writePos+=4;
                    std::cout << "FileName Size: "  << bufFSize.second  << std::endl;
                   
                    std::pair<bool, std::string> bufPairStr = ob.consumeAndGetStr(writePos,bufFSize.second);
                    writePos+= bufFSize.second; // adding the length of filename
                    _fileName = bufPairStr.second;
                    std::pair<bool, int> bufSize = ob.consumeAndGetInt(writePos);
                    writePos+=4;
                    
                    std::pair<bool, std::string> bufPairdata = ob.consumeAndGetStr(writePos,bufSize.second);
                     std::cout << "Buf : " <<  bufPairdata.second << std::endl;
                    writePos+= bufSize.second;
                    std::cout << "WritePos = " << writePos << std::endl;
                    writeFile(bufPairdata.second);
                    getData(ob, writePos);
                     
                }
                else if (bufChar.second == 'C')
                {
                    // ++writePos;
                    return ;
                    
                }
                else
                {
                    // std::cout << "XX: " << bufChar.second << std::endl;
                }
                
            }
        }
    }
    
}

bool Server :: createDir()
{
    std::string mypath = "";
    mypath=_dirName;
    boost::filesystem::path p(mypath);
    if (boost::filesystem::create_directories(p))
    {
        std::cout << "Directory created successfully "  << std::endl;
        return true;
    }
    else
    {
        std::cout << "Directory creation failed "  << std::endl;
        return false;
    }
}

void Server :: writeFile(std::string &data)
{
    strcpy(_buffer, data.c_str());
    std::string name =  _dirName + "/"+ _fileName;
    std::cout << "home/zerocool/workspace/Network App/" << name << std::endl; 
    std::ofstream myfile("/home/zerocool/workspace/Network App/"+name, std::ios::out | std::ios::app | std::ios::binary);
    if (myfile.is_open())
    {
                
        myfile << data;
        
        
        bzero(_buffer,4095);
        
        myfile.close();
        std::cout << "File Created Succssfully" << std::endl; 
    }
    else
    {
        std::cout << "File failed to open"  << std::endl;

    }
    
    
    
}


int main ()
{
    int portNo;
    std::cout << "Enter the port no : " << std::endl;
    std::cin >> portNo;
    Server server(portNo);
    if (server.setUp())
    {
        if (!server.start())
        {
             std::cout << "Server crashed"  << std::endl;
        }
    }
    else
    {
        std::cout << "Server setUp failed"  << std::endl;
    }
    
    return 0;
}
