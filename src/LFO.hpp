#include <cmath>

template <class T>
class LFO
{
public:
    enum Waveform {SINE = 1, TRIANGLE, SQUARE};

    /**
    Constructs a new LFO object with default parameters.
    */
    LFO ();

    /**
    Constructs a new LFO object.
    @param waveform Waveform.
    @param freq     Frequency.
    */
    LFO (const Waveform waveform, const T freq);

    /**
    Sets the LFO frequency.
    @param freq LFO frequency.
    */
    void set_frequency (const T freq);

    /**
    Gets the LFO frequency.
    @return LFO frequency.
    */
    T get_frequency () const;

    /**
    Sets the LFO waveform. This will take effect immediately if the LFO phase 
    is 0.0. Otherwise the waveform change is scheduled until the start of the
    next phase.
    @param waveform LFO waveform. 
    */
    void set_waveform (const Waveform waveform);

    /**
    Gets the current LFO waveform.
    @return LFO waveform
    */
    Waveform get_waveform () const;

    /**
    Sets a phase offset.
    @param shift Phase offset with shift == 1 is a full phase shift.
    */
    void set_phase_shift (const T shift);

    /**
    Gets the phase offset.
    @return Phase offset with shift == 1 is a full phase shift.
    */
    T get_phase_shift () const;

    /**
    Starts the LFO and applies scheduled changes.
    */
    void start ();

    /**
    Stops the LFO and applies scheduled changes.
    */
    void stop ();

    /**
    Proceeds the LFO. Applies scheduled changes in the case of a phase switch.
    @param time Time in phases.
    */
    void run (const T time);

    /**
    Gets the current LFO value.
    @return LFO value.
    */
    T get_value () const;

    /**
    Gets the current LFO integral value.
    @return LFO value.
    */
    T get_integral () const;

    /** 
    Tests if the LFO object is running or not.
    @return True if running, otherwise false.
    */
    bool is_active() const;

protected:
    Waveform waveform_;
    Waveform scheduled_waveform_;
    T freq_;
    T phase_;
    T shift_;
    bool active_;
};

template <class T> inline LFO<T>::LFO () : LFO (SINE, 1.0) {}

template <class T> inline LFO<T>::LFO (const Waveform waveform, const T freq) : 
    waveform_(waveform), 
    scheduled_waveform_(waveform), 
    freq_(freq), 
    phase_(0.0), 
    shift_(0.0),
    active_(false)
{

}

template <class T> inline void LFO<T>::set_frequency (const T freq) {freq_ = freq;}

template <class T> inline T LFO<T>::get_frequency () const {return freq_;}

template <class T> inline void LFO<T>::set_waveform (const Waveform waveform) 
{
    scheduled_waveform_ = waveform;
    if (phase_ == 0.0) waveform_ = waveform;
}
    
template <class T> inline typename LFO<T>::Waveform LFO<T>::get_waveform () const {return waveform_;}

template <class T> inline void LFO<T>::set_phase_shift (const T shift) {shift_ = shift;}

template <class T> inline T LFO<T>::get_phase_shift () const {return shift_;}

template <class T> inline void LFO<T>::start () 
{
    phase_ = 0.0;
    waveform_ = scheduled_waveform_;
    active_ = true;
}

template <class T> inline void LFO<T>::stop () 
{
    phase_ = 0.0;
    waveform_ = scheduled_waveform_;
    active_ = false;
}

template <class T> inline void LFO<T>::run (const T time) 
{
    if (!active_) return;

    if (std::floor(phase_ + shift_ + time * freq_) != std::floor (phase_ + shift_)) waveform_ = scheduled_waveform_;
    phase_ += time * freq_;
    if (phase_ >= 1.0) phase_ = std::fmod(phase_, 1.0);
}

template <class T> inline T LFO<T>::get_value () const
{
    const T x = phase_ + shift_ - floor(phase_ + shift_);
    switch (waveform_) 
    {
        case SINE:     return -std::cos (2.0 * M_PI * x);

        case TRIANGLE: return x < 0.25 ? 
                              4.0 * x : 
                              (
                                x < 0.75 ?
                                1.0 - 4.0 * (x - 0.25) :
                                -1.0 + 4.0 * (x- 0.75)
                              );

        case SQUARE:    return ((x < 0.25) || (x>= 0.75)) ? 1.0 : -1.0;

        default:        return 0.0;
    }
}

template <class T> inline T LFO<T>::get_integral () const
{
    const T x = phase_ + shift_ - floor(phase_ + shift_);
    switch (waveform_) 
    {
        case SINE:     return std::sin (2.0 * M_PI * x);

        case TRIANGLE:  {
                            const int sec = x * 4.0;
                            const double rem = 4.0 * x - sec; 
                            switch (sec)
                            {
                                case 0:     return 0.5 * std::pow(rem, 2);
                                case 1:     return 0.5 + rem - 0.5 * std::pow(rem, 2);
                                case 2:     return 1.0 - 0.5 * std::pow(rem, 2);
                                default:    return 1.0 - (0.5 + rem - 0.5 * std::pow(rem, 2));
                            }
                        }

        case SQUARE:    return x < 0.25 ? 
                              4.0 * x: 
                              (
                                x < 0.75 ?
                                1.0 - 4.0 * (x - 0.25) :
                                -1.0 + 4.0 * (x - 0.75)
                              );
        default:        return 0.0;
    }
}

template <class T> inline bool LFO<T>::is_active () const {return active_;}