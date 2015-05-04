grammar P2;
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

a : A EOF { c << tokenStream()->toString();} ;
A : '-' I ;
I : '0'..'9'+ ;
WS : (' '|'\n') {$channel=antlr3::TokenHiddenChannel;} ;
