grammar P4;
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

idList : ids+=ID (',' ids+=ID)* { c << $ids.size(); } ;
ID : ('_'|'a'..'z'|'A'..'Z')('_'|'a'..'z'|'A'..'Z'|'0'..'9')* ;
WS : (' '|'\n') {$channel=antlr3::TokenHiddenChannel;} ;
