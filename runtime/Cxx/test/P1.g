grammar P1;
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

a : A { c << tokenStream()->toString();} ;
A : '\\' 't' {setText("\t");} ;
WS : (' '|'\n') {$channel=antlr3::TokenHiddenChannel;} ;
