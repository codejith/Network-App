#ifndef SERIAL
#define SERIAL


#include <string>
class Serializer
{
  public :

    /*Sending side */
    void appendChar(char ch_);
    void appendInt(unsigned int num_);
    void serializeBuf(std::string &clientName_,  std::string pos_);
    void serializeFile(std::string path_,std::string filename_);
    void appendFile(std::ifstream &file_);
    void appendStr(const std::string &str_);
    std::size_t getBufSize();
    std::pair<size_t, const char *> getBufferAt(std::size_t startPos_, std::size_t size_); // size_ = send size

    /*Receving side*/

    void readCharBuf(const char * temp);
    std::pair<bool, unsigned int> consumeAndGetInt(size_t index_);
    std::pair<bool, char> consumeAndGetChar(size_t index_);
    std::pair<bool, std::string> consumeAndGetStr(int startPos_, int size_);
    
  private :
    std::string                    _buf;
    std::size_t                    _size;    // actual buffer size
    std::size_t                    _pos;

};

#endif
