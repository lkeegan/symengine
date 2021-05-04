#include <tinyxml2.h>
#include <cstring>
#include <symengine/parser/mathml_parser.h>
#include <symengine/real_double.h>
#include <symengine/real_mpfr.h>
#include <symengine/ntheory_funcs.h>
#include <symengine/parser.h>

namespace SymEngine
{

class MathmlInfixVisitor : public tinyxml2::XMLVisitor
{
private:
    std::size_t depth{0};
    std::vector<char> op;
    std::string sep{};
    std::vector<bool> print_op;
    std::map<std::string, std::string> func_names;
    std::map<std::string, std::string> constant_names;
    std::map<std::string, char> ops;

public:
    std::stringstream s;
    MathmlInfixVisitor() : op(1024, ','), print_op(1024, false)
    {
        ops["plus"] = '+';
        ops["minus"] = '-';
        ops["times"] = '*';
        ops["divide"] = '/';

        func_names["power"] = "pow";
        func_names["arcsin"] = "asin";
        func_names["arccos"] = "acos";
        func_names["arcsec"] = "asec";
        func_names["arccsc"] = "acsc";
        func_names["arctan"] = "atan";
        func_names["arccot"] = "acot";
        func_names["arcsinh"] = "asinh";
        func_names["arccsch"] = "acsch";
        func_names["arccosh"] = "acosh";
        func_names["arctanh"] = "atanh";
        func_names["arccoth"] = "acoth";
        func_names["arcsech"] = "asech";
        func_names["lt"] = "Lt";
        func_names["leq"] = "Le";
        func_names["gt"] = "Gt";
        func_names["geq"] = "Geq";
        func_names["eq"] = "Eq";
        func_names["and"] = "And";
        func_names["or"] = "Or";
        func_names["xor"] = "Xor";
        func_names["not"] = "Not";

        constant_names["true"] = "True";
        constant_names["false"] = "False";
        constant_names["pi"] = "pi";
        constant_names["exponentiale"] = "E";
        constant_names["eulergamma"] = "EulerGamma";
    }

    bool VisitEnter(const tinyxml2::XMLElement &element,
                    const tinyxml2::XMLAttribute *attribute) override
    {
        std::string name{element.Name()};
        std::cout << "Enter " << name << std::endl;
        if (attribute != nullptr) {
            std::cout << "  - " << attribute->Name() << std::endl;
        }
        if (name == "apply") {
            // next element is operator
            return true;
        }
        if (name == "cn") {
            // number
            const auto *att{attribute};
            while (att != nullptr) {
                if (strcmp(att->Name(), "type") == 0) {
                    if (strcmp(attribute->Value(), "complex-cartesian") == 0) {
                        sep = " + I*";
                    } else if (strcmp(attribute->Value(), "rational") == 0) {
                        sep = "/";
                    }
                }
                att = att->Next();
            }
            return true;
        }
        if (name == "ci") {
            // symbol
            return true;
        }
        if (name == "sep") {
            // separator in complex/rational number
            print_op[depth] = false;
            s << sep;
            return true;
        }
        // built-in constant
        auto ic = constant_names.find(name);
        if (ic != constant_names.cend()) {
            s << ic->second;
            return true;
        }
        // built-in function
        if (print_op[depth]) {
            s << ' ' << op[depth] << ' ';
        }
        print_op[depth] = true;
        ++depth;
        std::cout << "depth " << depth << std::endl;
        auto iop = ops.find(name);
        if (iop != ops.cend()) {
            op[depth] = iop->second;
            s << "(";
            return true;
        }
        op[depth] = ',';
        auto i_func = func_names.find(name);
        if (i_func != func_names.cend()) {
            s << i_func->second << "(";
            return true;
        }
        // otherwise assume user-defined function for now
        s << element.Name() << "(";
        return true;
    }

    bool VisitExit(const tinyxml2::XMLElement &element) override
    {
        std::string name{element.Name()};
        std::cout << "Exit " << name << std::endl;
        if (name == "apply") {
            print_op[depth] = false;
            --depth;
            s << ")";
        }
        return true;
    }
    bool Visit(const tinyxml2::XMLText &text) override
    {
        if (print_op[depth]) {
            s << ' ' << op[depth] << ' ';
        }
        print_op[depth] = true;
        s << text.Value();
        return true;
    }
};

RCP<const Basic> parse_mathml(const std::string &xml)
{
    MathmlParser p{};
    return p.parse(xml);
}

RCP<const Basic> MathmlParser::parse(const std::string &xml)
{
    tinyxml2::XMLDocument doc;
    std::cout << xml << std::endl;
    doc.Parse(xml.c_str());
    MathmlInfixVisitor v;
    doc.Accept(&v);
    std::string expr{v.s.str()};
    std::cout << "EXPR: " << expr << std::endl;
    return SymEngine::parse(expr);
}

MathmlParser::MathmlParser() = default;

} // namespace SymEngine
