#include <iostream>
#include <fstream>
#include <arpa/inet.h>
#include "Serializer.h"

void Serializer :: appendChar(char ch_)
{
    _buf = _buf + ch_;
}

void Serializer :: appendInt(unsigned int num_) 
{
    
    unsigned num1 = htonl(num_);
    std::cout << "Num1 : " << num1 << std::endl;
    const char* numChr = reinterpret_cast<const char*>(&num1);
    char abc[4] = {*numChr, *(numChr + 1), *(numChr + 2), *(numChr + 3)};

    std::string temp;
    temp.assign(abc, 4);
    appendStr(temp);
    
    /*for (int i = 0; i <4; ++i) // add char array directly to string
      appendChar(abc[i]);*/
}

void Serializer :: serializeBuf(std::string &clientName_,  std::string pos_)
{
    if ("start" == pos_)
    {
        int size1 = clientName_.size();
        appendChar('S');
        std::cout << "client size  : " << size1 << std::endl;
        appendInt(size1);
        appendStr(clientName_);
    }
    else
    {
        appendChar('C');

    }
}

void Serializer :: serializeFile(std::string path_, std::string filename_)
{

    unsigned int length = 0; 
    std::string line;

    std::ifstream readFile (path_+filename_, std::ios::in | std::ios::binary);

     if (readFile.is_open())
     {
         while( getline ( readFile, line ) )
         {
             std::cout <<  "X :" << line << std::endl;
             length += line.length()+1;
             
             
         }
         
         std::cout << "Length : " << length-1 << std::endl;
         
         appendChar('F');
         appendInt(filename_.size());
         appendStr(filename_);
         appendInt(length-1);
         
     }
     else
     {
         std::cout << "Failed to open file " << std::endl;
     }    
}

void Serializer::appendFile(std::ifstream &file)  
{
    std::string line;
    
    file.clear();
    file.seekg(0, std::ios::beg);
     while( getline ( file, line ) )
     {
         line = line + "\n";
         appendStr(line);
     } 
    /*if (line.empty())
    {
            line = "\n"; 
            appendStr(line);
    }*/
}

void Serializer :: appendStr(const std::string &str_)
{
    _buf = _buf + str_;
}

std::size_t Serializer :: getBufSize()
{
    return _buf.size();
}

std::pair<size_t, const char *> Serializer :: getBufferAt(std::size_t startPos_, std::size_t size_)
{
    const char* str = _buf.c_str();
    std::size_t readSize = startPos_ + size_ >= _buf.size() ? _buf.size() - startPos_ :  size_;
    return std::make_pair(readSize, (str + startPos_));

}

void Serializer :: readCharBuf(const char *temp)
{
	
    std::string val = std::string(temp);
    std::cout << "Size of Val : " << val.size() << std::endl;
    appendStr(val);
}

std::pair<bool, unsigned int> Serializer :: consumeAndGetInt(std::size_t index_) 
{
	if (index_ + 4 >_buf.size())
	{
		return std::make_pair(false, -1);
	}
	else
	{
		char buffer[4] = {_buf[index_],_buf[index_+1],_buf[index_+2],_buf[index_+3]};
		unsigned& num2 = reinterpret_cast<unsigned&>(buffer);
		unsigned int num = ntohl(num2) ;
		std::cout << "Converted Num : " << num << std::endl;
		return std::make_pair(true, num);
	}
}

std::pair<bool, char> Serializer :: consumeAndGetChar(std::size_t index_)
{
	if (index_ + 1 >_buf.size())
	{
		return std::make_pair(false, '$');
	}
	else
	{
		return std::make_pair(true, _buf[index_]);
	}
}

std::pair<bool, std::string> Serializer :: consumeAndGetStr(int startPos_, int size_)
{
	
	std::string data = _buf.substr (startPos_, size_);
	
	return std::make_pair(true, data);
     
}
