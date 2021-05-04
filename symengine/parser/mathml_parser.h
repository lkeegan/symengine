#ifndef SYMENGINE_PARSER_MATHMLPARSER_H
#define SYMENGINE_PARSER_MATHMLPARSER_H

#include <fstream>
#include <algorithm>
#include <memory>

#include <symengine/parser/tokenizer.h>
#include <symengine/add.h>
#include <symengine/pow.h>
#include <symengine/logic.h>

namespace SymEngine
{

class MathmlParser
{
public:
    RCP<const Basic> parse(const std::string &xml);
    explicit MathmlParser();
};

} // namespace SymEngine

#endif
