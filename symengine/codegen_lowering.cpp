#include <symengine/codegen_lowering.h>

#include <symengine/constants.h>
#include <symengine/mul.h>
#include <symengine/visitor.h>

namespace SymEngine
{

namespace
{

RCP<const Basic> lower_codegen_function(const Basic &x)
{
    switch (x.get_type_code()) {
        case SYMENGINE_COT:
            return div(one,
                       tan(down_cast<const OneArgFunction &>(x).get_arg()));
        case SYMENGINE_CSC:
            return div(one,
                       sin(down_cast<const OneArgFunction &>(x).get_arg()));
        case SYMENGINE_SEC:
            return div(one,
                       cos(down_cast<const OneArgFunction &>(x).get_arg()));
        case SYMENGINE_ACOT:
            return atan(
                div(one, down_cast<const OneArgFunction &>(x).get_arg()));
        case SYMENGINE_ACSC:
            return asin(
                div(one, down_cast<const OneArgFunction &>(x).get_arg()));
        case SYMENGINE_ASEC:
            return acos(
                div(one, down_cast<const OneArgFunction &>(x).get_arg()));
        case SYMENGINE_CSCH:
            return div(one,
                       sinh(down_cast<const OneArgFunction &>(x).get_arg()));
        case SYMENGINE_SECH:
            return div(one,
                       cosh(down_cast<const OneArgFunction &>(x).get_arg()));
        case SYMENGINE_COTH:
            return div(one,
                       tanh(down_cast<const OneArgFunction &>(x).get_arg()));
        case SYMENGINE_ACSCH:
            return asinh(
                div(one, down_cast<const OneArgFunction &>(x).get_arg()));
        case SYMENGINE_ASECH:
            return acosh(
                div(one, down_cast<const OneArgFunction &>(x).get_arg()));
        case SYMENGINE_ACOTH:
            return atanh(
                div(one, down_cast<const OneArgFunction &>(x).get_arg()));
        default:
            return x.rcp_from_this();
    }
}

class LowerCodegenExpr : public BaseVisitor<LowerCodegenExpr, TransformVisitor>
{
public:
    using TransformVisitor::bvisit;

    void bvisit(const OneArgFunction &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        RCP<const Basic> rewritten_fn
            = eq(*newarg, *farg) ? x.rcp_from_this() : x.create(newarg);

        result_ = lower_codegen_function(*rewritten_fn);
    }
};

} // namespace

RCP<const Basic> lower_codegen_expr(const RCP<const Basic> &x)
{
    LowerCodegenExpr visitor;
    return visitor.apply(x);
}

RCP<const Basic> lower_codegen_expr(const Basic &x)
{
    return lower_codegen_expr(x.rcp_from_this());
}

} // namespace SymEngine
