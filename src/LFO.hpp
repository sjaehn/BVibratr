#include "MiniGC.hpp"
#include "Wavetable.hpp"
#include <cmath>
#include <cstddef>
#include <functional>
#include <new>

template <class T>
class LFO
{
public:
    enum Waveform {WAVETABLE = 0, SINE, TRIANGLE, SQUARE, SAW};

    enum Event 
    {
        START,          // Event elicited if LFO starts.
        PHASE_RESTART,  // Event elicited if phase (plus shift) ended and restarts.
        STOP            // Event elicited if LFO stopped.
    };          

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

    ~LFO();

    /**
    Makes some boring but important background work, like wavetable garbage
    collection.

    @note Call this frequently from OUTSIDE of realtime threads.
     */
    void worker();

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
    Sets the wavetable data and also calculates an integral wavetable. This 
    will take effect immediately if the LFO phase is 0.0. Otherwise the 
    wavetable change is scheduled until the start of the next phase. TThis 
    method does NOT schedule a switch to the wavetable mode.
    
    @note This method allocates heap memory to copy the wavetable data and
    to create an integral wavetable. Thus, this method is NOT realtime-safe.

    @param wavetable    Reference to wavetable.
     */
    void set_wavetable_data(const Wavetable<T>& wavetable);

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

    /**
    Sets a callback function for one of the events START, PHASE_CHANGE, STOP. The passed callback function will
    be called upon the respective event together with this object and optional arguments.
    @param callback Reference to a callback function of the type void function(LFO<T>&, void*).
    @param args     Pointer to the arguments which will be passed together with this object to the callback function. 
     */
    void setCallbackFunction(const Event event, const std::function<void(LFO<T>&, void*)> &callback, void* args = nullptr);

    /**
    Removes the link to an external callback function for the respective event.
    @param event    Event.
     */
    void removeCallbackFunction(const Event event);

protected:
    Waveform waveform_;
    Waveform scheduled_waveform_;
    Wavetable<T> *wavetable_, *scheduled_wavetable_;
    Wavetable<T> *integral_wavetable_, *scheduled_integral_wavetable_;
    MiniGC<const Wavetable<T>, 32> wavetable_gc_;
    T freq_;
    T phase_;
    T shift_;
    bool active_;

    std::array<std::pair<std::function<void(LFO<T>&, void*)>, void*>, STOP + 1> callbacks_;

    /**
    Tries to apply scheduled wavetable data. In this case, the pointers to the
    current wavetable (and integral wavetable) are replaced by the scheduled
    pointers. Just before, the pointers to the previos wavetable (and integral
    wavetable) are sent to the garbage collector to be removed in the next
    worker cycle.
     */
    void apply_scheduled_wavetable_data_();

    /**
    Distributes the events of this objects by calling the respective callback function together with a reference to 
    this object and a pointer to the optionally provided args. Also see: setCallbackFunction.
    @param event    Event.
     */
    void on_event_(const Event event);

    /**
    Default callback function. Doesn't do anything.
    @param adsr     Reference to the LFO<T> object which caused calling of this callback. Here unused.
    @param args     Pointer to optional parameters. Here unused.
     */
    static void defaultCallback_(LFO& adsr, void* args) {}
};

template <class T> inline LFO<T>::LFO () : LFO (SINE, 1.0) {}

template <class T> inline LFO<T>::LFO (const Waveform waveform, const T freq) : 
    waveform_(waveform), 
    scheduled_waveform_(waveform), 
    wavetable_(nullptr),
    scheduled_wavetable_(nullptr),
    integral_wavetable_(nullptr),
    scheduled_integral_wavetable_(nullptr),
    freq_(freq), 
    phase_(0.0), 
    shift_(0.0),
    active_(false)
{
    callbacks_.fill(std::pair<std::function<void(LFO<T>&, void*)>, void*>(&defaultCallback_, nullptr));
}

template <class T> inline LFO<T>::~LFO()
{
    if (wavetable_) delete wavetable_;
    if (scheduled_wavetable_) delete scheduled_wavetable_;
    if (integral_wavetable_) delete integral_wavetable_;
    if (scheduled_integral_wavetable_) delete scheduled_integral_wavetable_;
}

template <class T> inline void LFO<T>::worker()
{
    wavetable_gc_.purge();
}

template <class T> inline void LFO<T>::set_frequency (const T freq) {freq_ = freq;}

template <class T> inline T LFO<T>::get_frequency () const {return freq_;}

template <class T> inline void LFO<T>::set_waveform (const Waveform waveform) 
{
    scheduled_waveform_ = waveform;
    if (phase_ == 0.0) waveform_ = waveform;
}
    
template <class T> inline typename LFO<T>::Waveform LFO<T>::get_waveform () const {return waveform_;}

template <class T> inline void LFO<T>::set_wavetable_data(const Wavetable<T>& wavetable)
{
    // (Other) wavetable already scheduled: delete
    if (scheduled_wavetable_) delete scheduled_wavetable_;
    if (scheduled_integral_wavetable_) delete scheduled_integral_wavetable_;
    scheduled_wavetable_ = scheduled_integral_wavetable_ = nullptr;

    // Create new scheduled wavetable and copy data
    scheduled_wavetable_ = new (std::nothrow) Wavetable<T>();
    if (!scheduled_wavetable_) return;  // TODO error
    *scheduled_wavetable_ = wavetable;

    // New scheduled integral
    scheduled_integral_wavetable_ = new (std::nothrow) Wavetable<T>();
    if (!scheduled_integral_wavetable_) 
    {
        delete scheduled_wavetable_;
        scheduled_wavetable_ = nullptr;
        // TODO error
        return;
    }

    // ... and calculate integral
    const size_t total_samples = wavetable.get_total_samples();
    T integral = 0;
    T max = wavetable[0];
    T min = wavetable[0];
    for (size_t i = 0; i < total_samples; ++i)
    {
        integral += wavetable[i];
        (*scheduled_integral_wavetable_)[i] = integral;
        if (integral > max) max = integral;
        else if (integral < min) min = integral;
    }

    // ... normalize (as integral values outside [-1, 1] are not allowed here)
    const T rng = max - min;
    if (rng > 2.0)
    {
        for (size_t i = 0; i < total_samples; ++i)
        {
            (*scheduled_integral_wavetable_)[i] = -1.0 + ((*scheduled_integral_wavetable_)[i] - min) / rng;
        }
    }

    // Phase start: Apply scheduled wavetables
    worker();
    if (phase_ == 0.0) apply_scheduled_wavetable_data_();
}

template <class T> inline void LFO<T>::set_phase_shift (const T shift) {shift_ = shift;}

template <class T> inline T LFO<T>::get_phase_shift () const {return shift_;}

template <class T> inline void LFO<T>::start () 
{
    phase_ = 0.0;
    waveform_ = scheduled_waveform_;
    active_ = true;
    on_event_(START);
}

template <class T> inline void LFO<T>::stop () 
{
    phase_ = 0.0;
    waveform_ = scheduled_waveform_;
    active_ = false;
    on_event_(STOP);
}

template <class T> inline void LFO<T>::run (const T time) 
{
    if (!active_) return;

    if (std::floor(phase_ + shift_ + time * freq_) != std::floor (phase_ + shift_)) 
    {
        waveform_ = scheduled_waveform_;
        on_event_(PHASE_RESTART);
    }

    phase_ += time * freq_;
    if ((waveform_ == WAVETABLE) && wavetable_)
    {
        if (phase_ >= wavetable_->get_total_frames()) phase_ = std::fmod(phase_, wavetable_->get_total_frames());
    }
        
    else if (phase_ >= 1.0) phase_ = std::fmod(phase_, 1.0);
}

template <class T> inline T LFO<T>::get_value () const
{
    const T x = phase_ + shift_ - floor(phase_ + shift_);
    switch (waveform_) 
    {
        case WAVETABLE: return  wavetable_ ?
                                wavetable_->at_rel(phase_ + shift_) : // Not x!
                                0.0;
        
        case SINE:      return -std::cos (2.0 * M_PI * x);

        case TRIANGLE:  return x < 0.25 ? 
                              4.0 * x : 
                              (
                                x < 0.75 ?
                                1.0 - 4.0 * (x - 0.25) :
                                -1.0 + 4.0 * (x- 0.75)
                              );

        case SQUARE:    return ((x < 0.25) || (x>= 0.75)) ? 1.0 : -1.0;

        case SAW:       return 2.0 * x - 2.0 * (x >= 0.5);

        default:        return 0.0;
    }
}

template <class T> inline T LFO<T>::get_integral () const
{
    const T x = phase_ + shift_ - floor(phase_ + shift_);
    switch (waveform_) 
    {
        case WAVETABLE: return  integral_wavetable_ ? 
                                integral_wavetable_->at_rel(phase_ + shift_) :    // Not x!
                                0.0;

        case SINE:      return std::sin (2.0 * M_PI * x);

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

        case SAW:       return (x < 0.5) ?  std::pow(2.0 * x, 2) : 2.0 * (1.0 - x);

        default:        return 0.0;
    }
}

template <class T> inline bool LFO<T>::is_active () const {return active_;}

template <class T> void LFO<T>::setCallbackFunction(const typename LFO<T>::Event event, const std::function<void(LFO<T>&, void*)> &callback, void* args)
{
    callbacks_[event] = std::pair<std::function<void(LFO<T>&, void*)>, void*>(callback, args);
}

template <class T> void LFO<T>::removeCallbackFunction(const typename LFO<T>::Event event)
{
    callbacks_[event] = std::pair<std::function<void(LFO<T>&, void*)>, void*>(&defaultCallback_, nullptr);
}

template <class T> void LFO<T>::apply_scheduled_wavetable_data_()
{
    if (scheduled_wavetable_ && ((!wavetable_) || wavetable_gc_.add(wavetable_)))
    {
        wavetable_ = scheduled_wavetable_;
        scheduled_wavetable_ = nullptr;
    }

    if (scheduled_integral_wavetable_  && ((!integral_wavetable_) || wavetable_gc_.add(integral_wavetable_)))
    {
        integral_wavetable_ = scheduled_integral_wavetable_;
        scheduled_integral_wavetable_ = nullptr;
    }
}

template <class T> void LFO<T>::on_event_(const typename LFO<T>::Event event)
{
    callbacks_[event].first(*this, callbacks_[event].second);
}