#include <array>
#include <cmath>

/**
ADSR envelope class.
TODO: Faders, non-linearity.*/
template <class T>
class ADSR
{
    public:
    enum Phase {ATTACK = 0, DECAY, SUSTAIN, RELEASE};

    enum Fader 
    {
        LINEAR,     // Linear fading for A, D and R with y = x in the relative range [0, 1]. 
        INVSQR,        // Non-linear fading for A, D and R with y = 1 - (1 - x)^2 in the relative range [0, 1].
        SQRT,       // Non-linear fading for A, D and R with y = sqrt(x) in the relative range [0, 1].
        SINE_1_4    // Non-linear fading for A, D and R with y = sin(pi/2 * x) in the relative range [0, 1].
    };

    /**
    Constructs a new ADSR object with default parameters
    (A=0.0, D=0.0, S=1.0, R=0.0, LINEAR).
    */
    ADSR ();

    /**
    Contstructs a new ADSR object with provided parameters. Note: 
    Parameters are NOT validated!
    @param attack   Attack (A) time.
    @param decay    Decay (D) time.
    @param sustain  Sustain (S) value.
    @param release  Release (R) time.
    */
    ADSR (const T attack, const T decay, const T sustain, const T release);

    /**
    Contstructs a new ADSR object with provided parameters. Note: 
    Parameters are NOT validated!
    @param attack   Attack (A) time.
    @param decay    Decay (D) time.
    @param sustain  Sustain (S) value.
    @param release  Release (R) time.
    @param fader    LINEAR, SQRT or SINE.
    */
    ADSR (const T attack, const T decay, const T sustain, const T release, const Fader fader);

    /**
    Sets all ADSR parameters. Note: Parameters are NOT validated!
    @param attack   Attack (A) time.
    @param decay    Decay (D) time.
    @param sustain  Sustain (S) value.
    @param release  Release (R) time.
    */
    void set_parameters (const T attack, const T decay, const T sustain, const T release);

    /** 
    Sets an ADSR parameter value. Changes in A, D, and R do not change the 
    ADSR value (get_value()). his method does NOT validate value!
    @param phase: Parameter (ADSR phase).
    @param value: Parameter value.
    */
    void set_parameter (const Phase phase, const T value);

    /**
    Get the ADSR parameter for the respective phase.
    @param phase: ADSR phase
    @return:      Parameter value.
    */
    const T get_parameter (const Phase phase) const;

    /** 
    Gets the actual value.
    @return: Actual value. 
    */
    const T get_value () const;

    /* Starts ADSR. Sets phase to S*/
    void start ();

    /** 
    Proceeds in ADSR without progress in time. May switch from A to D or from D
    to S if called at the end of the respective phase or stops ADSR at the end
    of R.
    */
    void run();

    /** 
    Proceeds in ADSR. May switch from A to D or from D to S or stops ADSR at 
    the end of R.
    @param time: Time to proceed.
    */
    void run (const T time);

    const Phase getPhase() const;

    const T getPhaseTime() const;

    /* Changes phase to R.*/
    void release ();

    /* Stops ASDR. */
    void stop ();

    /** 
    Tests if the ADSR object is running or not.
    @return True if running, otherwise false.
    */
    const bool is_active() const;

    protected:
    bool active_;
    std::array<T, 4> param_;
    Phase phase_;
    Fader fader_;

    /* Time exceeded in phase_. May be negative. */
    T phase_time_;
};

template <class T> inline ADSR<T>::ADSR () : ADSR (0.0, 0.0, 1.0, 0.0, LINEAR) {}

template <class T> inline ADSR<T>::ADSR (const T attack, const T decay, const T sustain, const T release) : 
    ADSR(attack, decay, sustain, release, LINEAR) {}

template <class T> inline ADSR<T>::ADSR (const T attack, const T decay, const T sustain, const T release, const Fader fader) : 
    active_(false),
    param_({attack, decay, sustain, release}),
    phase_(ATTACK),
    fader_(fader),
    phase_time_(0.0)
{
    run ();
}

template <class T> inline void ADSR<T>::set_parameters (const T attack, const T decay, const T sustain, const T release)
{
    param_[ATTACK] = attack;
    param_[DECAY] = decay;
    param_[SUSTAIN] = sustain;
    param_[RELEASE] = release;
    run();
}

template <class T> inline void ADSR<T>::set_parameter(const Phase phase, const T value)
{
    // This piece of code is made for linear fade of A, D and R
    // TODO Nonlinear faders
    if (phase == phase_) 
    {
        switch (phase) 
        {
            case ATTACK:    phase_time_ = get_value() * value;
                            break;
            case DECAY:     if (param_[SUSTAIN] != 1.0) phase_time_ = value * (1.0 - get_value()) / (1.0 - param_[SUSTAIN]);
                            break;
            case SUSTAIN:   break;
            case RELEASE:   if (param_[SUSTAIN] != 0.0) phase_time_ = value * (param_[SUSTAIN] - get_value()) / param_[SUSTAIN];
        }
    }
    param_[phase] = value;
    run ();
}

template <class T> inline const T ADSR<T>::get_parameter (const Phase phase) const {return phase_;}

template <class T> inline const T ADSR<T>::get_value () const
{
    if (!active_) return 0.0;

    if (phase_ == SUSTAIN) return param_[SUSTAIN];

    const T phase_total = param_[phase_];
    const T prev = ((phase_ == ATTACK) ? 0.0 : ((phase_ == DECAY) ? 1.0 : param_[SUSTAIN]));
    const T next = ((phase_ == ATTACK) ? 1.0 : ((phase_ == DECAY) ? param_[SUSTAIN] : 0.0));

    switch (fader_)
    {
        case LINEAR:    return prev + (next - prev) * phase_time_ / phase_total;
        case INVSQR:    return prev + (next - prev) * (1.0 - pow(1.0 - phase_time_ / phase_total, 2.0));
        case SQRT:      return prev + (next - prev) * std::sqrt(std::abs(phase_time_) / phase_total);
        case SINE_1_4:  return prev + (next - prev) * std::sin(0.5 * M_PI * phase_time_/phase_total);
        default: return 0;
    }
    
}

template <class T> inline void ADSR<T>::start ()
{
    active_ = true;
    phase_= ATTACK;
    phase_time_ = 0.0f;
    run();
}

template <class T> inline void ADSR<T>::run (const T time)
{
    if (!active_) return;
    
    phase_time_ += time;
    while ((phase_time_ >= param_[phase_]) and (phase_ != SUSTAIN))
    {
        if (phase_ == RELEASE)
        {
            stop();
            return;
        }
        else
        {
            phase_time_ -= param_[phase_];
            phase_ = static_cast<Phase>(phase_ + 1);
        }
    }
}

template <class T> inline void ADSR<T>::run () {run (0.0);}

template <class T> inline const typename ADSR<T>::Phase ADSR<T>::getPhase () const {return phase_;}

template <class T> inline const T ADSR<T>::getPhaseTime () const {return phase_time_;}
    
template <class T> inline void ADSR<T>::release ()
{
    if (!active_) return;

    phase_time_ = (param_[SUSTAIN] - get_value()) * param_[RELEASE];  // May become negative if release starts before sustain
    phase_ = RELEASE;
    run();
}

template <class T> inline void ADSR<T>::stop () {active_ = false;}

template <class T> inline const bool ADSR<T>::is_active () const {return active_;}