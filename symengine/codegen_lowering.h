#ifndef SYMENGINE_CODEGEN_LOWERING_H
#define SYMENGINE_CODEGEN_LOWERING_H

#include <symengine/functions.h>

namespace SymEngine
{

RCP<const Basic> lower_codegen_expr(const RCP<const Basic> &x);
RCP<const Basic> lower_codegen_expr(const Basic &x);

} // namespace SymEngine

#endif // SYMENGINE_CODEGEN_LOWERING_H
