#include <ostream>
#include <sstream>

#include "STLOutStream.h"

namespace ers {

/** This class streams an issue into an human readable string. 
  * It is uses by the issue class to display itself and to put it into standard stl streams. 
  * @author Matthias Wiesmann
  * @version 1.0
  */
    
class HumanStream  : public STLOutStream {
protected:
    void serialize(const std::string &key, const std::string &value) ;
    void serialize_start(const Issue *i) ; 
    void serialize_end(const Issue *i) ; 
    void serialize_separator(const Issue *i) ;
public:
    HumanStream() ;
    std::string str();        /**< \brief returns the string for the stream */
} ; // human_stream 

} // namespace ers

