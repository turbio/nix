#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nix/expr/tests/libexpr.hh"
#include "nix/expr/value.hh"

#include <sstream>
#include <string>

namespace nix {

// Each case:
//   expr:    Nix expression that evaluates to a rational.
//   fval:    The value that the lexer would have produced if the literal
//            were parsed as a NixFloat (strtod-style). We assert that the
//            rational's stringification matches what ostream<<d and
//            std::to_string(d) produce, i.e. the pre-rationals float path.
//   ts:      Expected NixRational::to_string()       (matches ostream<<d, "%g" prec 6)
//   tsp:     Expected NixRational::to_string_padded() (matches std::to_string(d), "%f" prec 6)
struct RationalStringCase
{
    const char * expr;
    double fval;
    const char * ts;
    const char * tsp;
};

class RationalStringTest : public LibExprTest, public ::testing::WithParamInterface<RationalStringCase>
{};

TEST_P(RationalStringTest, matchesExpectedAndFloatParity)
{
    const auto & c = GetParam();

    auto v = eval(c.expr);
    ASSERT_EQ(v.type(), nRational) << "expr: " << c.expr;
    NixRational * r = v.rational();

    EXPECT_EQ(r->to_string(), c.ts) << "rational to_string, expr: " << c.expr;
    EXPECT_EQ(r->to_string_padded(), c.tsp) << "rational to_string_padded, expr: " << c.expr;

    std::ostringstream os;
    os << c.fval;
    EXPECT_EQ(os.str(), c.ts) << "float ostream parity, expr: " << c.expr;
    EXPECT_EQ(std::to_string(c.fval), c.tsp) << "float std::to_string parity, expr: " << c.expr;
}

INSTANTIATE_TEST_SUITE_P(
    RationalString,
    RationalStringTest,
    ::testing::Values(
        // --- Zero ---
        RationalStringCase{"0.0", 0.0, "0", "0.000000"},

        // --- %f mode, X=0 (1 integer digit) ---
        RationalStringCase{"1.0", 1.0, "1", "1.000000"},
        RationalStringCase{"2.0", 2.0, "2", "2.000000"},
        RationalStringCase{"1.5", 1.5, "1.5", "1.500000"},

        // --- Negatives (uses Nix subtraction to avoid relying on unary minus) ---
        RationalStringCase{"(0.0 - 1.5)", -1.5, "-1.5", "-1.500000"},
        RationalStringCase{"(0.0 - 0.0001)", -0.0001, "-0.0001", "-0.000100"},

        // --- Exact binary fractions (double represents exactly) ---
        RationalStringCase{"0.5", 0.5, "0.5", "0.500000"},
        RationalStringCase{"0.25", 0.25, "0.25", "0.250000"},
        RationalStringCase{"0.125", 0.125, "0.125", "0.125000"},
        RationalStringCase{"0.0625", 0.0625, "0.0625", "0.062500"},

        // --- %f mode, X=1..5 (multi-digit integer part) ---
        RationalStringCase{"10.0", 10.0, "10", "10.000000"},
        RationalStringCase{"12.345", 12.345, "12.345", "12.345000"},
        RationalStringCase{"100.0", 100.0, "100", "100.000000"},
        RationalStringCase{"123.45", 123.45, "123.45", "123.450000"},
        RationalStringCase{"1000.0", 1000.0, "1000", "1000.000000"},
        RationalStringCase{"1234.5", 1234.5, "1234.5", "1234.500000"},
        RationalStringCase{"10000.0", 10000.0, "10000", "10000.000000"},
        RationalStringCase{"12345.6", 12345.6, "12345.6", "12345.600000"},
        RationalStringCase{"100000.0", 100000.0, "100000", "100000.000000"},
        RationalStringCase{"123456.0", 123456.0, "123456", "123456.000000"},

        // --- %f/%e boundary at X=6 (first %e) ---
        RationalStringCase{"1000000.0", 1000000.0, "1e+06", "1000000.000000"},
        RationalStringCase{"10000000.0", 10000000.0, "1e+07", "10000000.000000"},

        // --- Large %e via exponent literal ---
        RationalStringCase{"1.0e6", 1.0e6, "1e+06", "1000000.000000"},
        RationalStringCase{"1.0e7", 1.0e7, "1e+07", "10000000.000000"},

        // --- %f mode, X=-1..-4 (fractional) ---
        RationalStringCase{"0.1", 0.1, "0.1", "0.100000"},
        RationalStringCase{"0.123", 0.123, "0.123", "0.123000"},
        RationalStringCase{"0.01", 0.01, "0.01", "0.010000"},
        RationalStringCase{"0.0123", 0.0123, "0.0123", "0.012300"},
        RationalStringCase{"0.001", 0.001, "0.001", "0.001000"},
        RationalStringCase{"0.00123", 0.00123, "0.00123", "0.001230"},
        RationalStringCase{"0.0001", 0.0001, "0.0001", "0.000100"},
        RationalStringCase{"0.0001234", 0.0001234, "0.0001234", "0.000123"},

        // --- %f/%e boundary at X=-5 (first %e on the negative side) ---
        RationalStringCase{"0.00001", 0.00001, "1e-05", "0.000010"},
        RationalStringCase{"1.0e-5", 1.0e-5, "1e-05", "0.000010"},
        RationalStringCase{"1.0e-10", 1.0e-10, "1e-10", "0.000000"},

        // --- %e with multi-digit mantissa ---
        RationalStringCase{"1.23456e7", 1.23456e7, "1.23456e+07", "12345600.000000"},
        RationalStringCase{"1.23456e-5", 1.23456e-5, "1.23456e-05", "0.000012"},

        // --- Signed and zero padded exponents (toml likes 5e+22) ---
        RationalStringCase{"1.0e+6", 1.0e+6, "1e+06", "1000000.000000"},
        RationalStringCase{"5.0e+15", 5.0e+15, "5e+15", "5000000000000000.000000"},
        RationalStringCase{"1.0e-05", 1.0e-05, "1e-05", "0.000010"},
        RationalStringCase{"1.0e010", 1.0e010, "1e+10", "10000000000.000000"},

        // --- Rounding up at 7th digit (not a tie, double and rational agree) ---
        RationalStringCase{"0.1234567", 0.1234567, "0.123457", "0.123457"},
        RationalStringCase{"1.234567", 1.234567, "1.23457", "1.234567"},
        RationalStringCase{"123.4567", 123.4567, "123.457", "123.456700"},

        // --- Rounding down at 7th digit ---
        RationalStringCase{"0.1234561", 0.1234561, "0.123456", "0.123456"},
        RationalStringCase{"9.99995", 9.99995, "9.99995", "9.999950"}));

} // namespace nix
