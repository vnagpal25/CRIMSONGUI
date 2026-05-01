#pragma once

#ifdef _MSC_VER
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif

#ifndef EIGEN_NO_DEPRECATED_WARNING
#define EIGEN_NO_DEPRECATED_WARNING
#endif

#ifndef EIGEN_DEPRECATED_WITH_REASON
#define EIGEN_DEPRECATED_WITH_REASON(x)
#endif

#ifndef EIGEN_NOEXCEPT
#define EIGEN_NOEXCEPT noexcept
#endif

#ifndef EIGEN_NOEXCEPT_IF
#define EIGEN_NOEXCEPT_IF(x) noexcept(x)
#endif

#include <Eigen/Core>

#if !defined(EIGEN_WORLD_VERSION) || EIGEN_WORLD_VERSION < 5
namespace Eigen
{
namespace numext
{
template <typename Scalar>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE Scalar madd(const Scalar& a, const Scalar& b, const Scalar& c)
{
    return a * b + c;
}
}
}
#endif
