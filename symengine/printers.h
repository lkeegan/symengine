#ifndef SYMENGINE_PRINTER_H
#define SYMENGINE_PRINTER_H

#include <cstdint>

#include <symengine/basic.h>
#include <symengine/matrix.h>

namespace SymEngine
{
enum class CodePrinterPrecision : std::uint32_t { Double = 0, Float = 1 };

// Append new fields only at the end. Older callers set `size` to the older
// struct size, which lets newer libraries ignore fields they do not know.
struct CodePrinterSettings {
    std::uint32_t size;
    CodePrinterPrecision precision;

    explicit CodePrinterSettings(CodePrinterPrecision precision
                                 = CodePrinterPrecision::Double)
        : size(sizeof(CodePrinterSettings)), precision(precision)
    {
    }
};

std::string str(const Basic &x);
std::string str(const DenseMatrix &x);
std::string unicode(const Basic &x);
std::string julia_str(const Basic &x);
std::string sbml(const Basic &x);
std::string ascii_art();

std::string mathml(const Basic &x);

std::string latex(const Basic &x);
std::string latex(const DenseMatrix &x, const unsigned max_rows = 20,
                  const unsigned max_cols = 12);

std::string ccode(const Basic &x);
std::string ccode_settings(const Basic &x,
                           const CodePrinterSettings *settings = nullptr);
std::string cudacode(const Basic &x,
                     const CodePrinterSettings *settings = nullptr);
std::string c89code(const Basic &x);
std::string c99code(const Basic &x);
std::string jscode(const Basic &x);
} // namespace SymEngine

#endif // SYMENGINE_PRINTER_H
