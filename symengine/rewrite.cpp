#include <symengine/visitor.h>
#include <symengine/basic.h>
#include <symengine/logic.h>

namespace SymEngine
{

class RewriteAsExp : public BaseVisitor<RewriteAsExp, TransformVisitor>
{
public:
    using TransformVisitor::bvisit;

    RewriteAsExp() : BaseVisitor<RewriteAsExp, TransformVisitor>() {}

    void bvisit(const Sin &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        auto expo = mul(I, newarg);
        auto a = exp(expo);
        auto b = exp(neg(expo));
        result_ = div(sub(a, b), mul(integer(2), I));
    }

    void bvisit(const Cos &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        auto expo = mul(I, newarg);
        auto a = exp(expo);
        auto b = exp(neg(expo));
        result_ = div(add(a, b), integer(2));
    }

    void bvisit(const Tan &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        auto expo = mul(I, newarg);
        auto a = exp(expo);
        auto b = exp(neg(expo));
        result_ = div(sub(a, b), mul(I, add(a, b)));
    }

    void bvisit(const Cot &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        auto expo = mul(I, newarg);
        auto a = exp(expo);
        auto b = exp(neg(expo));
        result_ = div(mul(I, add(a, b)), sub(a, b));
    }

    void bvisit(const Csc &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        auto expo = mul(I, newarg);
        auto a = exp(expo);
        auto b = exp(neg(expo));
        result_ = div(mul(I, integer(2)), sub(a, b));
    }

    void bvisit(const Sec &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        auto expo = mul(I, newarg);
        auto a = exp(expo);
        auto b = exp(neg(expo));
        result_ = div(integer(2), add(a, b));
    }

    void bvisit(const Sinh &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        result_ = div(sub(exp(newarg), exp(neg(newarg))), integer(2));
    }

    void bvisit(const Cosh &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        result_ = div(add(exp(newarg), exp(neg(newarg))), integer(2));
    }

    void bvisit(const Tanh &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        auto pos_exp = exp(newarg);
        auto neg_exp = exp(neg(newarg));
        result_ = div(sub(pos_exp, neg_exp), add(pos_exp, neg_exp));
    }

    void bvisit(const Csch &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        auto pos_exp = exp(newarg);
        auto neg_exp = exp(neg(newarg));
        result_ = div(integer(2), sub(pos_exp, neg_exp));
    }

    void bvisit(const Sech &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        auto pos_exp = exp(newarg);
        auto neg_exp = exp(neg(newarg));
        result_ = div(integer(2), add(pos_exp, neg_exp));
    }

    void bvisit(const Coth &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        auto pos_exp = exp(newarg);
        auto neg_exp = exp(neg(newarg));
        result_ = div(add(pos_exp, neg_exp), sub(pos_exp, neg_exp));
    }
};

RCP<const Basic> rewrite_as_exp(const RCP<const Basic> &x)
{
    RewriteAsExp b;
    return b.apply(x);
}

class RewriteAsSin : public BaseVisitor<RewriteAsSin, TransformVisitor>
{
public:
    using TransformVisitor::bvisit;

    RewriteAsSin() : BaseVisitor<RewriteAsSin, TransformVisitor>() {}

    void bvisit(const Cos &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        result_ = sin(unevaluated_expr(add(newarg, div(pi, integer(2)))));
    }

    void bvisit(const Tan &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        result_ = div(mul(integer(2), pow(sin(newarg), integer(2))),
                      sin(mul(integer(2), newarg)));
    }

    void bvisit(const Cot &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        result_ = div(sin(mul(integer(2), newarg)),
                      mul(integer(2), pow(sin(newarg), integer(2))));
    }

    void bvisit(const Csc &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        result_ = div(integer(1), sin(newarg));
    }

    void bvisit(const Sec &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        result_ = div(integer(1),
                      sin(unevaluated_expr(add(newarg, div(pi, integer(2))))));
    }
};

RCP<const Basic> rewrite_as_sin(const RCP<const Basic> &x)
{
    RewriteAsSin b;
    return b.apply(x);
}

class RewriteAsCos : public BaseVisitor<RewriteAsCos, TransformVisitor>
{
public:
    using TransformVisitor::bvisit;

    RewriteAsCos() : BaseVisitor<RewriteAsCos, TransformVisitor>() {}

    void bvisit(const Sin &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        result_ = cos(unevaluated_expr(sub(newarg, div(pi, integer(2)))));
    }

    void bvisit(const Tan &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        result_ = div(cos(unevaluated_expr(sub(newarg, div(pi, integer(2))))),
                      cos(newarg));
    }

    void bvisit(const Cot &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        result_ = div(cos(newarg),
                      cos(unevaluated_expr(sub(newarg, div(pi, integer(2))))));
    }

    void bvisit(const Csc &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        result_ = div(integer(1),
                      cos(unevaluated_expr(sub(newarg, div(pi, integer(2))))));
    }

    void bvisit(const Sec &x)
    {
        auto farg = x.get_arg();
        auto newarg = apply(farg);
        result_ = div(integer(1), cos(newarg));
    }
};

RCP<const Basic> rewrite_as_cos(const RCP<const Basic> &x)
{
    RewriteAsCos b;
    return b.apply(x);
}

class RewriteAsStandardMath
    : public BaseVisitor<RewriteAsStandardMath, TransformVisitor>
{
public:
    using TransformVisitor::bvisit;

    RewriteAsStandardMath()
        : BaseVisitor<RewriteAsStandardMath, TransformVisitor>()
    {
    }

    void bvisit(const Add &x)
    {
        vec_basic newargs;
        bool changed = false;
        for (const auto &arg : x.get_args()) {
            auto newarg = apply(arg);
            changed = changed or newarg != arg;
            newargs.push_back(newarg);
        }
        result_ = changed ? add(newargs) : x.rcp_from_this();
    }

    void bvisit(const Mul &x)
    {
        vec_basic newargs;
        bool changed = false;
        for (const auto &arg : x.get_args()) {
            auto newarg = apply(arg);
            changed = changed or newarg != arg;
            newargs.push_back(newarg);
        }
        result_ = changed ? mul(newargs) : x.rcp_from_this();
    }

    void bvisit(const MultiArgFunction &x)
    {
        auto args = x.get_args();
        vec_basic newargs;
        bool changed = false;
        for (const auto &arg : args) {
            auto newarg = apply(arg);
            changed = changed or newarg != arg;
            newargs.push_back(newarg);
        }
        result_ = changed ? x.create(newargs) : x.rcp_from_this();
    }

    void bvisit(const Piecewise &x)
    {
        PiecewiseVec new_pairs;
        bool changed = false;
        for (const auto &branch_cond : x.get_vec()) {
            auto branch = branch_cond.first;
            auto cond = branch_cond.second;
            auto cond_basic = rcp_static_cast<const Basic>(cond);
            auto new_branch = apply(branch);
            auto new_cond = apply(cond_basic);
            changed = changed or new_branch != branch or new_cond != cond_basic;
            new_pairs.push_back(
                {new_branch, rcp_static_cast<const Boolean>(new_cond)});
        }
        result_ = changed ? piecewise(new_pairs) : x.rcp_from_this();
    }

    void bvisit(const Cot &x)
    {
        result_ = div(one, tan(apply(x.get_arg())));
    }

    void bvisit(const Csc &x)
    {
        result_ = div(one, sin(apply(x.get_arg())));
    }

    void bvisit(const Sec &x)
    {
        result_ = div(one, cos(apply(x.get_arg())));
    }

    void bvisit(const ASec &x)
    {
        result_ = acos(div(one, apply(x.get_arg())));
    }

    void bvisit(const ACsc &x)
    {
        result_ = asin(div(one, apply(x.get_arg())));
    }

    void bvisit(const ACot &x)
    {
        result_ = atan(div(one, apply(x.get_arg())));
    }

    void bvisit(const Csch &x)
    {
        result_ = div(one, sinh(apply(x.get_arg())));
    }

    void bvisit(const Sech &x)
    {
        result_ = div(one, cosh(apply(x.get_arg())));
    }

    void bvisit(const Coth &x)
    {
        result_ = div(one, tanh(apply(x.get_arg())));
    }

    void bvisit(const ACsch &x)
    {
        result_ = asinh(div(one, apply(x.get_arg())));
    }

    void bvisit(const ACoth &x)
    {
        result_ = atanh(div(one, apply(x.get_arg())));
    }

    void bvisit(const ASech &x)
    {
        result_ = acosh(div(one, apply(x.get_arg())));
    }
};

RCP<const Basic> rewrite_as_standard_math(const RCP<const Basic> &x)
{
    RewriteAsStandardMath b;
    return b.apply(x);
}

} // namespace SymEngine
