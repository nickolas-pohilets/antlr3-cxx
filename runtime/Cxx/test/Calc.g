grammar Calc;

options
{
    language=Cxx;
}

@parser::includes
{
#include <unordered_map>
#include <string>
#include <sstream>

struct parser_context
{
	std::unordered_map<antlr3::String, int> vars;
	std::stringstream ss;
};

}

@parser::context
{
    parser_context* c;
}

prog: stat+ EOF;
stat: expr NEWLINE
	  {
	      c->ss << $expr.value << std::endl;
	  }
	| ID '=' expr NEWLINE
	  {
	      c->vars.insert(std::make_pair($ID.text, $expr.value));
	  }
	| NEWLINE
	  {
	  }
	;

expr returns [int value]
    @init { $value = 0; }
	: e1=multExpr
      {
          $value = $e1.value;
      }
      ( '+' e2=multExpr
        {
            $value += $e2.value;
        }
      | '-' e2=multExpr
        {
            $value -= $e2.value;
        }
      )*
	;

multExpr returns [int value]
    @init { $value = 0; }
	: e1=atom
	  {
	      $value = $e1.value;
	  }
	( '*' e2=atom
	  {
	      $value *= $e2.value;
	  }
	)*
	;

atom returns [int value]
    @init { $value = 0; }
	: INT
	  {
	      $value = atoi(antlr3::toUTF8($INT.text).c_str());
	  }
	| ID
	  {
	      antlr3::String name = $ID.text;
	      auto it = c->vars.find(name);
	  	  if(it == c->vars.end())
	  	  {
	  	      c->ss << "ERROR: Unknown variable \"" << antlr3::toUTF8(name) << "\"" << std::endl;
              $value = 0;
	  	  }
	  	  else
	  	  {
	  	      $value = it->second;
	  	  }
	  }
	| '(' expr ')'
	  {
	      $value = $expr.value;
	  }
	;

ID: ('a'..'z'|'A'..'Z')+ ;
INT: ('0'..'9')+ ;
NEWLINE: '\r'? '\n';
WS: (' '|'\t')+ { $channel=antlr3::TokenHiddenChannel; };