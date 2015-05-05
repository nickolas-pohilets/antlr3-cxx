lexer grammar Filter;

options
{
    language=Cxx;
    filter=true;
}

ID: ('a'..'z'|'A'..'Z')+ ;
INT: ('0'..'9')+ ;
NEWLINE: '\r'? '\n';
WS: (' '|'\t')+ { $channel=antlr3::TokenHiddenChannel; };
