#pragma once

#include <algorithm>

namespace chip
{
	const size_t SMPL_BUF_SIZE_ = 0x10000;

	enum Stereo : int
	{
		LEFT = 0,
		RIGHT = 1,
	};

    template <typename T>
    inline const T& clamp(const T& value, const T& low, const T& high)
    {
        return std::min(std::max(value, low), high);
    }
}
