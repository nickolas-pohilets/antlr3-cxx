tree grammar EvalAST;

options
{
    language=Cxx;
    tokenVocab=CalcAST;
    ASTLabelType='antlr3::ItemPtr';
}

@postincludes {
#include <unordered_map>
#include <string>
#include <sstream>
}

@before_class {
struct parser_context2
{
	std::unordered_map<antlr3::String, int> vars;
	std::stringstream ss;
};
}

@declarations {
public:
    parser_context2* c;
}

prog
    : stat+
    ;

stat
    : expr
      {
           c->ss << $expr.value << std::endl;
      }
	| ^('=' ID expr)
      {
          c->vars.insert(std::make_pair($ID.text, $expr.value));
      }
	;

expr returns [int value]
    @init { $value = 0; }
    : ^('+' a=expr b=expr) { $value = a + b; }
    | ^('-' a=expr b=expr) { $value = a - b; }
    | ^('*' a=expr b=expr) { $value = a * b; }
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
    | INT
      {
          $value = atoi(antlr3::toUTF8($INT.text).c_str());
      }
    ;