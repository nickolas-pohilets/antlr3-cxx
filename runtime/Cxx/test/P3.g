grammar P3;
options { language=Cxx; }

@parser::includes
{
#include <antlr3/antlr3.hpp>
using namespace antlr3;
}

@parser::context
{
    StringStream c;
}

decl : type ID { c << "var " << $ID.text << ":" << $type.text; } ;
type : 'int' | 'float' ;
ID : ('_'|'a'..'z'|'A'..'Z')('_'|'a'..'z'|'A'..'Z'|'0'..'9')* ;
WS : (' '|'\n') {$channel=antlr3::TokenHiddenChannel;} ;
