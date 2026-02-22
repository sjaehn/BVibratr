#ifndef OSCX3_HPP_
#define OSCX3_HPP_

#include "LFO.hpp"
#include "Ports.hpp"

template <class T>
class Oscx3
{
public:
    T rate;

    LFO<T> osc1;
    LFO<T> osc2;
    LFO<T> osc3;

    T freq1;
    T freq2;
    T freq3;

    T amp1;
    T amp2;
    T amp3;

    BVibratrOscModes mode1;
    BVibratrOscModes mode2;
    BVibratrOscModes mode3;

    Oscx3();
    Oscx3(const T rate);

    void start();

    void stop();

    void run(const T time);

    T get_value() const;

    T get_integral() const;

protected:
    T signal_;
    T integral_;
}; 

template <class T> inline Oscx3<T>::Oscx3() : Oscx3<T>::Oscx3(1) {}

template <class T> inline Oscx3<T>::Oscx3(const T rate) :
    rate(rate),
    osc1(),
    osc2(),
    osc3(),
    freq1(1),
    freq2(1),
    freq3(1),
    amp1(1),
    amp2(1),
    amp3(1),
    mode1(BVIBRATR_OSC_MODE_LFO),
    mode2(BVIBRATR_OSC_MODE_PASS),
    mode3(BVIBRATR_OSC_MODE_PASS),
    signal_(0),
    integral_(0)
{}

template <class T> inline void Oscx3<T>::start ()
{
    osc1.start();
    osc2.start();
    osc3.start();
}

template <class T> inline void Oscx3<T>::stop ()
{
    osc1.stop();
    osc2.stop();
    osc3.stop();
}

template <class T> inline void Oscx3<T>::run(const T time)
{
    // Clear values
    signal_ = 0;
    integral_ = 0;

    // Modulators
    double osc1_freq_m = 1.0;	// Frequency multiplier, range [0.0, 2.0]
    double osc1_phase_d = 0.0;	// Phase delta, range [-1.0, 1.0]
    double osc1_amp_m = 1.0;	// Amplification multiplier, range [0.0, 1.0]

    double osc2_freq_m = 1.0;
    double osc2_phase_d = 0.0;
    double osc2_amp_m = 1.0;

    const double amp_f = amp1 +	((mode2 == BVIBRATR_OSC_MODE_ADD) ? amp2 : 0.0) +
								((mode3 == BVIBRATR_OSC_MODE_ADD) ? amp3 : 0.0);

    // Run osc3
    osc3.set_frequency(freq3);
    osc3.set_phase_shift(0);
    osc3.run(time);

    switch(mode3)
    {
        case BVIBRATR_OSC_MODE_ADD:	
            signal_ += amp3 * osc3.get_value();
            integral_ += amp3 * osc3.get_integral() * rate / freq3;
            break;

        case BVIBRATR_OSC_MODE_FM1:
            osc1_freq_m *= (1.0 - amp3 * osc3.get_value());
            break;

        case BVIBRATR_OSC_MODE_PM1:
            osc1_phase_d += amp3 * osc3.get_value();
            break;

        case BVIBRATR_OSC_MODE_AM1:
            osc1_amp_m *= (1.0 - 0.5 * amp3 * (1.0 + osc3.get_value()));
            break;

        case BVIBRATR_OSC_MODE_FM2:
            osc2_freq_m *= (1.0 - amp3 * osc3.get_value());
            break;

        case BVIBRATR_OSC_MODE_PM2:
            osc2_phase_d += amp3 * osc3.get_value();
            break;

        case BVIBRATR_OSC_MODE_AM2:
            osc2_amp_m *= (1.0 - 0.5 * amp3 * (1.0 + osc3.get_value()));
            break;

        default:
            break;
    }

    // Run osc2
    osc2.set_frequency(osc2_freq_m * freq2);
    osc1.set_phase_shift(osc2_phase_d);
    osc2.run(time);

    switch(mode2)
    {
        case BVIBRATR_OSC_MODE_ADD:	
            signal_ += osc2_amp_m * amp2 * osc2.get_value();
            integral_ += osc2_amp_m * amp2 * osc2.get_integral() * rate / freq2;
            break;

        case BVIBRATR_OSC_MODE_FM1:
            osc1_freq_m *= (1.0 - osc2_amp_m * amp2 * osc2.get_value());
            break;

        case BVIBRATR_OSC_MODE_PM1:
            osc1_phase_d += osc2_amp_m * amp2 * osc2.get_value();
            break;

        case BVIBRATR_OSC_MODE_AM1:
            osc1_amp_m *= (1.0 - 0.5 * osc2_amp_m * amp2 * (1.0 + osc2.get_value()));
            break;

        default:
            break;
    }

    // Run osc1
    
    osc1.set_frequency(osc1_freq_m * freq1);
    osc1.set_phase_shift(osc1_phase_d);
    osc1.run(time);
    signal_ += osc1_amp_m * osc1.get_value();
    integral_ += osc1_amp_m * osc1.get_integral() * rate / freq1;

    // Scale signal and integral_ to not exceed 1.0
    signal_ /= amp_f;
    integral_ /= amp_f;
}

template <class T> inline T Oscx3<T>::get_value() const {return signal_;}

template <class T> inline T Oscx3<T>::get_integral() const {return integral_;}

#endif /* OSCX3_HPP_ */