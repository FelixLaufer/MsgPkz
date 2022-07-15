#ifndef _X_PLATTFORM_H_
#define _X_PLATTFORM_H_

#ifndef NDEBUG
#define NDEBUG
#endif

#include "has_include.h"
#include "has_libstdcplusplus.h"

namespace std {}

namespace arx
{
  namespace stdx
  {
    using namespace ::std;
  }
}
namespace std
{
  using namespace ::arx::stdx;
}

#include "replace_minmax_macros.h"
#include "type_traits.h"

#if ARX_SYSTEM_HAS_INCLUDE(<cstdint>)
#include <cstdint>
#endif

#if ARX_SYSTEM_HAS_INCLUDE(<cstring>)
#include <cstring>
#endif

#if ARX_SYSTEM_HAS_INCLUDE(<memory>)
#include <memory>
#endif

#define __ASSERT_USE_STDERR
#if ARX_SYSTEM_HAS_INCLUDE(<assert.h>)
#include <assert.h>
#endif

#endif