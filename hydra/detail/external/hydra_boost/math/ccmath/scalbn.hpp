//  (C) Copyright Matt Borland 2021.
//  (C) Copyright John Maddock 2021.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HYDRA_BOOST_MATH_CCMATH_SCALBN_HPP
#define HYDRA_BOOST_MATH_CCMATH_SCALBN_HPP

#include <cmath>
#include <cfloat>
#include <limits>
#include <type_traits>
#include <hydra/detail/external/hydra_boost/math/tools/is_constant_evaluated.hpp>
#include <hydra/detail/external/hydra_boost/math/ccmath/abs.hpp>
#include <hydra/detail/external/hydra_boost/math/ccmath/isinf.hpp>
#include <hydra/detail/external/hydra_boost/math/ccmath/isnan.hpp>

namespace hydra_boost::math::ccmath {

namespace detail {

template <typename Real, typename Z>
inline constexpr Real scalbn_impl(Real arg, Z exp) noexcept
{
    while(exp > 0)
    {
        arg *= FLT_RADIX;
        --exp;
    }
    while(exp < 0)
    {
        arg /= FLT_RADIX;
        ++exp;
    }

    return arg;
}

} // Namespace detail

template <typename Real, std::enable_if_t<!std::is_integral_v<Real>, bool> = true>
inline constexpr Real scalbn(Real arg, int exp) noexcept
{
    if(HYDRA_BOOST_MATH_IS_CONSTANT_EVALUATED(arg))
    {
        return hydra_boost::math::ccmath::abs(arg) == Real(0) ? arg :
               hydra_boost::math::ccmath::isinf(arg) ? arg :
               hydra_boost::math::ccmath::isnan(arg) ? arg :
               hydra_boost::math::ccmath::detail::scalbn_impl(arg, exp);
    }
    else
    {
        using std::scalbn;
        return scalbn(arg, exp);
    }
}

template <typename Z, std::enable_if_t<std::is_integral_v<Z>, bool> = true>
inline constexpr double scalbn(Z arg, int exp) noexcept
{
    return hydra_boost::math::ccmath::scalbn(static_cast<double>(arg), exp);
}

inline constexpr float scalbnf(float arg, int exp) noexcept
{
    return hydra_boost::math::ccmath::scalbn(arg, exp);
}

#ifndef HYDRA_BOOST_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS
inline constexpr long double scalbnl(long double arg, int exp) noexcept
{
    return hydra_boost::math::ccmath::scalbn(arg, exp);
}
#endif

} // Namespaces

#endif // HYDRA_BOOST_MATH_CCMATH_SCALBN_HPP
