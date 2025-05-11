#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace RT {

	using u8 = uint8_t;
	using u16 = uint16_t;
	using u32 = uint32_t;
	using u64 = uint64_t;

	using i8 = int8_t;
	using i16 = int16_t;
	using i32 = int32_t;
	using i64 = int64_t;

	using fp32 = float;
	using fp64 = double;

	using byte = u8;

	template<typename T>
	using Ptr = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;

#define BIT(x) (1 << x)

#ifdef RT_RELEASE
	#define IN_RELEASE_BUILD (1)
#else
	#define IN_RELEASE_BUILD (0)
#endif

#ifdef RT_DEBUG
	#define IN_DEBUG_BUILD (1)
#else 
	#define IN_DEBUG_BUILD (0)
#endif

#if IN_RELEASE_BUILD
#define NO_RELEASE_ONLY(Code)
#else
#define NO_RELEASE_ONLY(Code) Code
#endif

}