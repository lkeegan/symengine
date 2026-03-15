#include <symengine/printers/codegen.h>
#include <symengine/codegen_lowering.h>
#include <symengine/printers.h>
#include <symengine/symengine_exception.h>

namespace SymEngine
{

namespace
{

std::string print_float_literal(double d)
{
    return print_double(d) + "f";
}

std::string print_function_name(const Function &x)
{
    static const std::vector<std::string> names_ = init_str_printer_names();
    const auto &name = names_[x.get_type_code()];
    if (name.empty()) {
        return "unknown";
    }
    return name;
}

template <typename Printer>
std::string print_function_call(Printer &printer, const std::string &name,
                                const vec_basic &args)
{
    std::ostringstream o;
    o << name << "(";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i != 0) {
            o << ", ";
        }
        o << printer.apply(args[i]);
    }
    o << ")";
    return o.str();
}

template <typename Printer>
std::string print_codegen_function(Printer &printer, const Function &x)
{
    switch (x.get_type_code()) {
        case SYMENGINE_SIN:
        case SYMENGINE_COS:
        case SYMENGINE_TAN:
        case SYMENGINE_ASIN:
        case SYMENGINE_ACOS:
        case SYMENGINE_ATAN:
        case SYMENGINE_ATAN2:
        case SYMENGINE_SINH:
        case SYMENGINE_COSH:
        case SYMENGINE_TANH:
        case SYMENGINE_ASINH:
        case SYMENGINE_ACOSH:
        case SYMENGINE_ATANH:
        case SYMENGINE_LOG:
        case SYMENGINE_FLOOR:
        case SYMENGINE_ERF:
        case SYMENGINE_ERFC:
            return print_function_call(printer, print_function_name(x),
                                       x.get_args());
        default:
            if (is_lowerable_codegen_function(x)) {
                return printer.apply(*lower_codegen_function(x));
            }
            throw SymEngineException("Code generation does not support "
                                     + print_function_name(x));
    }
}

} // namespace

void CodePrinter::bvisit(const Basic &x)
{
    throw SymEngineException("Not supported");
}
void CodePrinter::bvisit(const Complex &x)
{
    throw NotImplementedError("Not implemented");
}
void CodePrinter::bvisit(const Dummy &x)
{
    std::ostringstream s;
    s << x.get_name() << '_' << x.get_index();
    str_ = s.str();
}
void CodePrinter::bvisit(const Interval &x)
{
    std::string var = str_;
    std::ostringstream s;
    bool is_inf = eq(*x.get_start(), *NegInf);
    if (not is_inf) {
        s << var;
        if (x.get_left_open()) {
            s << " > ";
        } else {
            s << " >= ";
        }
        s << apply(x.get_start());
    }
    if (neq(*x.get_end(), *Inf)) {
        if (not is_inf) {
            s << " && ";
        }
        s << var;
        if (x.get_right_open()) {
            s << " < ";
        } else {
            s << " <= ";
        }
        s << apply(x.get_end());
    }
    str_ = s.str();
}
void CodePrinter::bvisit(const Contains &x)
{
    x.get_expr()->accept(*this);
    x.get_set()->accept(*this);
}
void CodePrinter::bvisit(const Piecewise &x)
{
    std::ostringstream s;
    auto vec = x.get_vec();
    for (size_t i = 0;; ++i) {
        if (i == vec.size() - 1) {
            if (neq(*vec[i].second, *boolTrue)) {
                throw SymEngineException(
                    "Code generation requires a (Expr, True) at the end");
            }
            s << "(\n   " << apply(vec[i].first) << "\n";
            break;
        } else {
            s << "((";
            s << apply(vec[i].second);
            s << ") ? (\n   ";
            s << apply(vec[i].first);
            s << "\n)\n: ";
        }
    }
    for (size_t i = 0; i < vec.size(); i++) {
        s << ")";
    }
    str_ = s.str();
}
void CodePrinter::bvisit(const BooleanAtom &x)
{
    str_ = print_double(x.get_val() ? 1.0 : 0.0);
}
void CodePrinter::bvisit(const And &x)
{
    std::ostringstream s;
    const auto &container = x.get_container();
    s << "(";
    for (auto it = container.begin(); it != container.end(); ++it) {
        if (it != container.begin()) {
            s << " && ";
        }
        s << "(" << apply(*(*it)) << ")";
    }
    s << ")";
    str_ = s.str();
}
void CodePrinter::bvisit(const Or &x)
{
    std::ostringstream s;
    const auto &container = x.get_container();
    s << "(";
    for (auto it = container.begin(); it != container.end(); ++it) {
        if (it != container.begin()) {
            s << " || ";
        }
        s << "(" << apply(*(*it)) << ")";
    }
    s << ")";
    str_ = s.str();
}
void CodePrinter::bvisit(const Xor &x)
{
    std::ostringstream s;
    const auto &container = x.get_container();
    s << "(";
    for (auto it = container.begin(); it != container.end(); ++it) {
        if (it != container.begin()) {
            s << " != ";
        }
        s << "((" << apply(*(*it)) << ") != 0)";
    }
    s << ")";
    str_ = s.str();
}
void CodePrinter::bvisit(const Not &x)
{
    std::ostringstream s;
    s << "!(" << apply(*x.get_arg()) << ")";
    str_ = s.str();
}
void CodePrinter::bvisit(const Rational &x)
{
    std::ostringstream o;
    double n = mp_get_d(get_num(x.as_rational_class()));
    double d = mp_get_d(get_den(x.as_rational_class()));
    o << print_double(n) << "/" << print_double(d);
    str_ = o.str();
}
void CodePrinter::bvisit(const Reals &x)
{
    throw SymEngineException("Not supported");
}
void CodePrinter::bvisit(const Rationals &x)
{
    throw SymEngineException("Not supported");
}
void CodePrinter::bvisit(const Integers &x)
{
    throw SymEngineException("Not supported");
}
void CodePrinter::bvisit(const EmptySet &x)
{
    throw SymEngineException("Not supported");
}
void CodePrinter::bvisit(const FiniteSet &x)
{
    throw SymEngineException("Not supported");
}
void CodePrinter::bvisit(const UniversalSet &x)
{
    throw SymEngineException("Not supported");
}
void CodePrinter::bvisit(const Abs &x)
{
    std::ostringstream s;
    s << "fabs(" << apply(x.get_arg()) << ")";
    str_ = s.str();
}
void CodePrinter::bvisit(const Ceiling &x)
{
    std::ostringstream s;
    s << "ceil(" << apply(x.get_arg()) << ")";
    str_ = s.str();
}
void CodePrinter::bvisit(const Truncate &x)
{
    std::ostringstream s;
    s << "trunc(" << apply(x.get_arg()) << ")";
    str_ = s.str();
}
void CodePrinter::bvisit(const Max &x)
{
    std::ostringstream s;
    const auto &args = x.get_args();
    switch (args.size()) {
        case 0:
        case 1:
            throw SymEngineException("Impossible");
        case 2:
            s << "fmax(" << apply(args[0]) << ", " << apply(args[1]) << ")";
            break;
        default: {
            vec_basic inner_args(args.begin() + 1, args.end());
            auto inner = max(inner_args);
            s << "fmax(" << apply(args[0]) << ", " << apply(inner) << ")";
            break;
        }
    }
    str_ = s.str();
}
void CodePrinter::bvisit(const Min &x)
{
    std::ostringstream s;
    const auto &args = x.get_args();
    switch (args.size()) {
        case 0:
        case 1:
            throw SymEngineException("Impossible");
        case 2:
            s << "fmin(" << apply(args[0]) << ", " << apply(args[1]) << ")";
            break;
        default: {
            vec_basic inner_args(args.begin() + 1, args.end());
            auto inner = min(inner_args);
            s << "fmin(" << apply(args[0]) << ", " << apply(inner) << ")";
            break;
        }
    }
    str_ = s.str();
}
void CodePrinter::bvisit(const Constant &x)
{
    if (eq(x, *E)) {
        str_ = "exp(1)";
    } else if (eq(x, *pi)) {
        str_ = "acos(-1)";
    } else {
        str_ = x.get_name();
    }
}
void CodePrinter::bvisit(const NaN &x)
{
    std::ostringstream s;
    s << "NAN";
    str_ = s.str();
}
void CodePrinter::bvisit(const Equality &x)
{
    std::ostringstream s;
    s << apply(x.get_arg1()) << " == " << apply(x.get_arg2());
    str_ = s.str();
}
void CodePrinter::bvisit(const Unequality &x)
{
    std::ostringstream s;
    s << apply(x.get_arg1()) << " != " << apply(x.get_arg2());
    str_ = s.str();
}
void CodePrinter::bvisit(const LessThan &x)
{
    std::ostringstream s;
    s << apply(x.get_arg1()) << " <= " << apply(x.get_arg2());
    str_ = s.str();
}
void CodePrinter::bvisit(const StrictLessThan &x)
{
    std::ostringstream s;
    s << apply(x.get_arg1()) << " < " << apply(x.get_arg2());
    str_ = s.str();
}
void CodePrinter::bvisit(const Function &x)
{
    str_ = print_codegen_function(*this, x);
}
void CodePrinter::bvisit(const Sign &x)
{
    const std::string arg = apply(x.get_arg());
    const std::string zero = print_double(0.0);
    const std::string one = print_double(1.0);
    const std::string minus_one = print_double(-1.0);
    std::ostringstream s;
    s << "((" << arg << " == " << zero << ") ? (" << zero << ") : ((" << arg
      << " < " << zero << ") ? (" << minus_one << ") : (" << one << ")))";
    str_ = s.str();
}
void CodePrinter::bvisit(const UnevaluatedExpr &x)
{
    str_ = apply(x.get_arg());
}
void CodePrinter::bvisit(const UnivariateSeries &x)
{
    throw SymEngineException("Not supported");
}
void CodePrinter::bvisit(const Derivative &x)
{
    throw SymEngineException("Not supported");
}
void CodePrinter::bvisit(const Subs &x)
{
    throw SymEngineException("Not supported");
}
void CodePrinter::bvisit(const GaloisField &x)
{
    throw SymEngineException("Not supported");
}

void C89CodePrinter::bvisit(const Infty &x)
{
    std::ostringstream s;
    if (x.is_negative_infinity())
        s << "-HUGE_VAL";
    else if (x.is_positive_infinity())
        s << "HUGE_VAL";
    else
        throw SymEngineException("Not supported");
    str_ = s.str();
}
void C89CodePrinter::_print_pow(std::ostringstream &o,
                                const RCP<const Basic> &a,
                                const RCP<const Basic> &b)
{
    if (eq(*a, *E)) {
        o << "exp(" << apply(b) << ")";
    } else if (eq(*b, *minus_one)) {
        o << apply(*one) << "/" << parenthesizeLE(a, PrecedenceEnum::Mul);
    } else if (eq(*b, *rational(1, 2))) {
        o << "sqrt(" << apply(a) << ")";
    } else {
        o << "pow(" << apply(a) << ", " << apply(b) << ")";
    }
}

void C99CodePrinter::bvisit(const Infty &x)
{
    std::ostringstream s;
    if (x.is_negative_infinity())
        s << "-INFINITY";
    else if (x.is_positive_infinity())
        s << "INFINITY";
    else
        throw SymEngineException("Not supported");
    str_ = s.str();
}
void C99CodePrinter::_print_pow(std::ostringstream &o,
                                const RCP<const Basic> &a,
                                const RCP<const Basic> &b)
{
    if (eq(*a, *E)) {
        o << "exp(" << apply(b) << ")";
    } else if (eq(*b, *minus_one)) {
        o << apply(*one) << "/" << parenthesizeLE(a, PrecedenceEnum::Mul);
    } else if (eq(*b, *rational(1, 2))) {
        o << "sqrt(" << apply(a) << ")";
    } else if (eq(*b, *rational(1, 3))) {
        o << "cbrt(" << apply(a) << ")";
    } else {
        o << "pow(" << apply(a) << ", " << apply(b) << ")";
    }
}
void C99CodePrinter::bvisit(const Gamma &x)
{
    std::ostringstream s;
    s << "tgamma(" << apply(x.get_arg()) << ")";
    str_ = s.str();
}
void C99CodePrinter::bvisit(const LogGamma &x)
{
    std::ostringstream s;
    s << "lgamma(" << apply(x.get_arg()) << ")";
    str_ = s.str();
}

void CudaCodePrinter::bvisit(const Integer &x)
{
    str_ = print_double(mp_get_d(x.as_integer_class()));
}

void CudaCodePrinter::bvisit(const Function &x)
{
    str_ = print_codegen_function(*this, x);
}

void CudaCodePrinter::bvisit(const Constant &x)
{
    if (eq(x, *E)) {
        str_ = "exp(1.0)";
    } else if (eq(x, *pi)) {
        str_ = "acos(-1.0)";
    } else {
        str_ = x.get_name();
    }
}

void CudaCodePrinter::bvisit(const NaN &x)
{
    str_ = "CUDART_NAN";
}

void CudaCodePrinter::bvisit(const Infty &x)
{
    if (x.is_negative_infinity())
        str_ = "-CUDART_INF";
    else if (x.is_positive_infinity())
        str_ = "CUDART_INF";
    else
        throw SymEngineException("Not supported");
}

void CudaFloatCodePrinter::bvisit(const BooleanAtom &x)
{
    str_ = print_float_literal(x.get_val() ? 1.0 : 0.0);
}

void CudaFloatCodePrinter::bvisit(const Function &x)
{
    str_ = print_codegen_function(*this, x);
}

void CudaFloatCodePrinter::bvisit(const Integer &x)
{
    str_ = print_float_literal(mp_get_d(x.as_integer_class()));
}

void CudaFloatCodePrinter::bvisit(const Rational &x)
{
    std::ostringstream o;
    double n = mp_get_d(get_num(x.as_rational_class()));
    double d = mp_get_d(get_den(x.as_rational_class()));
    o << print_float_literal(n) << "/" << print_float_literal(d);
    str_ = o.str();
}

void CudaFloatCodePrinter::bvisit(const RealDouble &x)
{
    str_ = print_float_literal(x.i);
}

#ifdef HAVE_SYMENGINE_MPFR
void CudaFloatCodePrinter::bvisit(const RealMPFR &x)
{
    StrPrinter::bvisit(x);
    str_ += "f";
}
#endif

void CudaFloatCodePrinter::bvisit(const Constant &x)
{
    if (eq(x, *E)) {
        str_ = "exp(1.0f)";
    } else if (eq(x, *pi)) {
        str_ = "acos(-1.0f)";
    } else {
        str_ = x.get_name();
    }
}

void CudaFloatCodePrinter::bvisit(const NaN &x)
{
    str_ = "CUDART_NAN_F";
}

void CudaFloatCodePrinter::bvisit(const Infty &x)
{
    if (x.is_negative_infinity())
        str_ = "-CUDART_INF_F";
    else if (x.is_positive_infinity())
        str_ = "CUDART_INF_F";
    else
        throw SymEngineException("Not supported");
}

void CudaFloatCodePrinter::bvisit(const Sign &x)
{
    const std::string arg = apply(x.get_arg());
    const std::string zero = print_float_literal(0.0);
    const std::string one = print_float_literal(1.0);
    const std::string minus_one = print_float_literal(-1.0);
    std::ostringstream s;
    s << "((" << arg << " == " << zero << ") ? (" << zero << ") : ((" << arg
      << " < " << zero << ") ? (" << minus_one << ") : (" << one << ")))";
    str_ = s.str();
}

void JSCodePrinter::bvisit(const Constant &x)
{
    if (eq(x, *E)) {
        str_ = "Math.E";
    } else if (eq(x, *pi)) {
        str_ = "Math.PI";
    } else {
        str_ = x.get_name();
    }
}
void JSCodePrinter::_print_pow(std::ostringstream &o, const RCP<const Basic> &a,
                               const RCP<const Basic> &b)
{
    if (eq(*a, *E)) {
        o << "Math.exp(" << apply(b) << ")";
    } else if (eq(*b, *rational(1, 2))) {
        o << "Math.sqrt(" << apply(a) << ")";
    } else if (eq(*b, *rational(1, 3))) {
        o << "Math.cbrt(" << apply(a) << ")";
    } else {
        o << "Math.pow(" << apply(a) << ", " << apply(b) << ")";
    }
}
void JSCodePrinter::bvisit(const Abs &x)
{
    std::ostringstream s;
    s << "Math.abs(" << apply(x.get_arg()) << ")";
    str_ = s.str();
}
void JSCodePrinter::bvisit(const Sin &x)
{
    std::ostringstream s;
    s << "Math.sin(" << apply(x.get_arg()) << ")";
    str_ = s.str();
}
void JSCodePrinter::bvisit(const Cos &x)
{
    std::ostringstream s;
    s << "Math.cos(" << apply(x.get_arg()) << ")";
    str_ = s.str();
}
void JSCodePrinter::bvisit(const Max &x)
{
    const auto &args = x.get_args();
    std::ostringstream s;
    s << "Math.max(";
    for (size_t i = 0; i < args.size(); ++i) {
        s << apply(args[i]);
        s << ((i == args.size() - 1) ? ")" : ", ");
    }
    str_ = s.str();
}
void JSCodePrinter::bvisit(const Min &x)
{
    const auto &args = x.get_args();
    std::ostringstream s;
    s << "Math.min(";
    for (size_t i = 0; i < args.size(); ++i) {
        s << apply(args[i]);
        s << ((i == args.size() - 1) ? ")" : ", ");
    }
    str_ = s.str();
}

std::string ccode(const Basic &x)
{
    C99CodePrinter c;
    return c.apply(x);
}

std::string cudacode(const Basic &x)
{
    CudaCodePrinter p;
    return p.apply(x);
}

std::string cudacode_float(const Basic &x)
{
    CudaFloatCodePrinter p;
    return p.apply(x);
}

std::string jscode(const Basic &x)
{
    JSCodePrinter p;
    return p.apply(x);
}

std::string inline c89code(const Basic &x)
{
    C89CodePrinter p;
    return p.apply(x);
}

std::string inline c99code(const Basic &x)
{
    C99CodePrinter p;
    return p.apply(x);
}

} // namespace SymEngine
