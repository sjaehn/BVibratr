#ifndef LINEARFADER_HPP_
#define LINEARFADER_HPP_

#include <cmath>
#include <cstdlib>

template <class T>
class LinearFader
{
private:
    T destination_;
    T delta_;
    T value_;

public:
    LinearFader (const T destination, const T delta) :
        destination_ (destination),
        delta_ (delta),
        value_ (destination)
    {

    }

    void set (const T destination)
    {
        destination_ = destination;
        proceed();
    }

    void set (const T destination, const T delta)
    {
        delta_ = delta;
        set(destination);
    }

    T get () const
    {
        return value_;
    }

    void proceed ()
    {
        if (std::abs(destination_- value_) <= std::abs(delta_)) value_ = destination_;
        else value_ += (1.0 - 2.0 * std::signbit(destination_ - value_)) * std::abs(delta_);
    }
};

#endif /* LINEARFADER_HPP_ */