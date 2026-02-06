#ifndef LIMIT_HPP_
#define LIMIT_HPP_

#include <cmath>
#include <array>
#include "Ports.hpp"

struct Limit
{
	float min;
	float max;
	float step;

        float validate (const float value) const
        {
                if (max <= min) return min;
                if (value <= min) return min;
                if (value >= max) return max;
                if (step == 0) return value;
		float newValue = (step > 0 ? min + round ((value - min) / step) * step : max - round ((max - value) / step) * step);
		return (newValue >= min ? (newValue <= max ? newValue : max) : min);
        }
};

constexpr std::array<const Limit, BVIBRATR_NR_CONTROLLERS> controller_limits =
{{
    {0, 1, 1},
    {0, 1, 0},
    {0, 65535, 1},
    {0, 127, 1},
    {0, 128, 1},
    {0.0, 50.0, 0.0},
    {0.1, 4.0, 0.0},
    {0.1, 4.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.1, 4.0, 0.0},
    {1.0, 20.0, 0.0},
    {1, 2, 1},
    {1, 3, 1},
    {0.0, 1.0, 0.0},
    {0.1, 20.0, 0.0},
    {1, 5, 1},
    {1, 3, 1},
    {0.0, 10.0, 0.0},
    {0.1, 20, 0.0},
    {1, 8, 1},
    {1, 3, 1},
    {0.0, 0.5, 0.0}
}};

#endif /* LIMIT_HPP_ */