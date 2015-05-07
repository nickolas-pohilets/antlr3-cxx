grammar Calc;

options
{
    language=Cxx;
    backtrack=true;
    memoize=true;
    encoding='UTF8';
}

scope Foo {
    std::string str;
}

@parser::header_postincludes {
#include <unordered_map>
#include <string>
#include <sstream>
}

@parser::before_class {
struct parser_context
{
	std::unordered_map<antlr3::String, int> vars;
	std::stringstream ss;
};

struct Foo {};
}

@parser::declarations {
public:
    parser_context* c;
}

foo returns [std::vector<Foo> value]
    : prog
    ;

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

expr returns [int value=2, std::vector<Foo> bar = { Foo(), Foo() }]
    scope Foo;
    @init { $value = -1; $Foo::str = "expr"; }
	: e1=multExpr
      {
          (void)sizeof($Foo[-0]::str);
          $value = $e1.value;
      }
      ( '+' e2=multExpr
        {
            (void)sizeof($Foo[0]::str);
            $value += $e2.value;
        }
      | '-' e2=multExpr
        {
            $value -= $e2.value;
        }
      )*
	;

multExpr returns [int value=5]
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
    @init { $value = 0; assert(!$Foo::str.empty()); }
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

INFINITE: '∞';
WTF: '𤭢' ( ('\u0045' ('\u00B0'..'\u00B6')) | ('\u00D0' ('\u00BE'..'\u00BF')));
ID: ('a'..'z'|'A'..'Z')+ ;
INT: ('0'..'9')+ ;
NEWLINE: '\r'? '\n';
WS: (' '|'\t')+ { $channel=antlr3::TokenHiddenChannel; };
