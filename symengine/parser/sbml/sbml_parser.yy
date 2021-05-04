%require "3.0"
%define api.pure full
%define api.value.type {struct SymEngine::SBMLSTYPE}
%define api.prefix {sbml}
%param {SymEngine::SbmlParser &p}

/*
// Uncomment this to enable parser tracing:
%define parse.trace
%printer { fprintf(yyo, "%s", $$.c_str()); } <string>
%printer { std::cerr << *$$; } <basic>
*/


%code requires // *.h
{

#include "symengine/parser/sbml/sbml_parser.h"

}

%code // *.cpp
{
#include "symengine/pow.h"
#include "symengine/logic.h"
#include "symengine/parser/sbml/sbml_parser.h"

    using SymEngine::add;
    using SymEngine::Basic;
    using SymEngine::Boolean;
    using SymEngine::Eq;
    using SymEngine::Ge;
    using SymEngine::Gt;
    using SymEngine::Le;
    using SymEngine::Lt;
    using SymEngine::mul;
    using SymEngine::Ne;
    using SymEngine::one;
    using SymEngine::pow;
    using SymEngine::RCP;
    using SymEngine::rcp_static_cast;
    using SymEngine::set_boolean;
    using SymEngine::sub;
    using SymEngine::vec_basic;
    using SymEngine::vec_boolean;

#include "symengine/parser/sbml/sbml_tokenizer.h"

    int yylex(SymEngine::SBMLSTYPE * yylval, SymEngine::SbmlParser & p)
    {
        return p.m_tokenizer.lex(*yylval);
    } // ylex

    void yyerror(SymEngine::SbmlParser & p, const std::string &msg)
    {
        throw SymEngine::ParseError(msg);
    }

// Force YYCOPY to not use memcopy, but rather copy the structs one by one,
// which will cause C++ to call the proper copy constructors.
#define YYCOPY(Dst, Src, Count)                                                \
    do {                                                                       \
        YYPTRDIFF_T yyi;                                                       \
        for (yyi = 0; yyi < (Count); yyi++)                                    \
            (Dst)[yyi] = (Src)[yyi];                                           \
    } while (0)

} // code

%token<string> IDENTIFIER
%token<string> NUMERIC
%token END_OF_FILE 0

%left AND OR
%left EQ '<' '>' LE GE NE
%left '+' '-'
%left '*' '/' '%'
%right UMINUS UPLUS '!'
%left '^' '@'
%nonassoc '('

%type<basic> st_expr
%type<basic> expr
%type<basic_vec> expr_list
%type<basic> leaf
%type<basic> func

%start st_expr

%%
st_expr : expr
{
    $$ = $1;
    p.res = $$;
};

expr : expr '+' expr
{
    $$ = add($1, $3);
}
| expr '-' expr
{
    $$ = sub($1, $3);
}
| expr '*' expr
{
    $$ = mul($1, $3);
}
| expr '/' expr
{
    $$ = div($1, $3);
}
| expr '%' expr
{
    $$ = p.modulo($1, $3);
}
| expr '^' expr
{
    $$ = pow($1, $3);
}
| expr '@' expr
{
    $$ = pow($1, $3);
}
| expr '<' expr
{
    $$ = rcp_static_cast<const Basic>(Lt($1, $3));
}
| expr '>' expr
{
    $$ = rcp_static_cast<const Basic>(Gt($1, $3));
}
| expr NE expr
{
    $$ = rcp_static_cast<const Basic>(Ne($1, $3));
}
| expr LE expr
{
    $$ = rcp_static_cast<const Basic>(Le($1, $3));
}
| expr GE expr
{
    $$ = rcp_static_cast<const Basic>(Ge($1, $3));
}
| expr EQ expr
{
    $$ = rcp_static_cast<const Basic>(Eq($1, $3));
}
| expr OR expr
{
    set_boolean s;
    s.insert(rcp_static_cast<const Boolean>($1));
    s.insert(rcp_static_cast<const Boolean>($3));
    $$ = rcp_static_cast<const Basic>(logical_or(s));
}
| expr AND expr
{
    set_boolean s;
    s.insert(rcp_static_cast<const Boolean>($1));
    s.insert(rcp_static_cast<const Boolean>($3));
    $$ = rcp_static_cast<const Basic>(logical_and(s));
}
| '(' expr ')'
{
    $$ = $2;
}
| '-' expr %prec UMINUS
{
    $$ = neg($2);
}
| '+' expr %prec UPLUS
{
    $$ = $2;
}
| '!' expr
{
    $$ = rcp_static_cast<const Basic>(
        logical_not(rcp_static_cast<const Boolean>($2)));
}
| leaf
{
    $$ = rcp_static_cast<const Basic>($1);
};

leaf : IDENTIFIER
{
    $$ = p.parse_identifier($1);
}
| NUMERIC
{
    $$ = p.parse_numeric($1);
}
| func
{
    $$ = $1;
};

func : IDENTIFIER '(' expr_list ')'
{
    $$ = p.functionify($1, $3);
}
|
IDENTIFIER '(' ')'
{
    $$ = p.functionify($1);
}

expr_list :

    expr_list ',' expr
{
    $$ = $1; // TODO : should make copy?
    $$.push_back($3);
}
| expr
{
    $$ = vec_basic(1, $1);
};
