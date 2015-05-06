grammar CalcAST;

options
{
    language=Cxx;
    output=AST;
}

prog: stat+ EOF;

stat: expr NEWLINE -> expr
	| ID '=' expr NEWLINE -> ^('=' ID expr)
	| NEWLINE ->
	;

expr
    scope { int bazz; double foo; }
    : multExpr ( ('+'^ | '-'^) multExpr )*
    ;

multExpr
	: atom ('*'^ atom)*
    ;

atom
	: INT
	| ID
	| '('! expr ')'!
	;

ID: ('a'..'z'|'A'..'Z')+ ;
INT: ('0'..'9')+ ;
NEWLINE: '\r'? '\n';
WS: (' '|'\t')+ { $channel=antlr3::TokenHiddenChannel; };
