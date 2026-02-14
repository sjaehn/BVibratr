#ifndef WAVETABLE_HPP_
#define WAVETABLE_HPP_

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>

/**
Template class for wavetable data. A %Wavetable consists of a number of frames
and each frame consists of a number of samples.
@tparam T   Element (floating point) data type.
@tparam N   Capacity (max number of elements). Must be at least 1.    
*/
template<class T = double, size_t N = 0x10000>
class Wavetable : protected std::array<T, N>
{
public:
    enum ErrorCode 
    {
        NO_ERROR = 0, 
        WAVETABLE_OVERFLOW,
        CORRUPT_RAW_DATA_FORMAT,
        CORRUPT_RAW_DATA_PARAMETER,
        CORRUPT_RAW_DATA_VALUE,
        RAW_DATA_VALUE_EXPECTED,
        UNKNOWN_ERROR
    };

protected:
    /* Actual size of the %Wavetable. Must NOT exceed N. */
    size_t wt_size_;

    /* Samples per frame*/
    size_t wt_spf_ = 128;

    /* Error byte. An error will remain until it gets deleted by read_error() */
    ErrorCode wt_err_ = NO_ERROR;

public:
    /**
    Constructs a default %Wavetable without any data.
     */
    Wavetable();

    using std::array<T, N>::operator[];

    /**
    Gets the %Wavetable data size which is the total number of samples 
    including the ingnored samples.
    @return         Wavetable data size.
     */
    inline constexpr size_t size() const {return wt_size_;}

    /**
    Resizes the %Wavetable.
    @param sz       New data size, but limited to N.
    @param val      Values to fill if new size > old size.
     */
    void resize(const size_t sz, T val = 0);

    /**
    Clears the wavetable and resets samples per frame.
     */
    void clear();

    /**
    Pushs a sample to the wavetable. Sets the error byte if operation failed
    due to container size limits.
     */
    void push_back(const T val);

    /**
    Removes the last sample from the wavetable. May also change the samples
    per frame, if less samples than samples per frame would remain.
     */
    void pop_back();

    /**
    Reads the error byte and resets it after this operation.
    @return     Error byte.
     */
    ErrorCode read_error();

    /**
    Parses a string to a %Wavetable following the rules:
    (1) Each line of the string is separated by the endline char. Each line
    has to contain either a parameter, or a value, or a comment or nothing 
    (empty line). 
    (2) A parameter name is followed by its value. The only supported 
    parameter yet is SAMPLES_PER_FRAME. The default value for SAMPLES_PER_FRAME
    is 128.
    (3) The wavetable sample values have to be provided as positive or 
    negative floating point numbers in a non-scientific notation with decimal 
    points or decimal commas. The values should not exceed [-1, 1].
    (4) A comment starts with # end ends at the end of the line.
    (5) Leading and tailing whitespaces are ignored.
    (6) The total number for samples should be divisible by 
    SAMPLES_PER_FRAME. Otherwise overhanging data are ignored.
    (7) The wavetable data shall continue from the beginning after the last 
    sample without breaks.

    @note This method doesn't allocate any memory, and doesn't throw any 
    exception, and doesn't use any locks. Parsing a C string containing data
    for some 10,000 samples may take less than 1 millisecond, depending on
    the system. Nevertheless, calling this method should be omited within the
    realtime thread, at least for large size wavetables.

    @param c        Data as C string.
    @param spf      Samples per frame.
     */
    void from_string(const char* const c, size_t spf = 128);

    /**
    Parses a string to a %Wavetable following the rules:
    (1) Each line of the string is separated by the endline char. Each line
    has to contain either a parameter, or a value, or a comment or nothing 
    (empty line). 
    (2) A parameter name is followed by its value. The only supported 
    parameter yet is SAMPLES_PER_FRAME. The default value for SAMPLES_PER_FRAME
    is 128.
    (3) The wavetable sample values have to be provided as positive or 
    negative floating point numbers in a non-scientific notation with decimal
    points or decimal commas. The values should not exceed [-1, 1].
    (4) A comment starts with # end ends at the end of the line.
    (5) Leading and tailing whitespaces are ignored.
    (6) The total number for samples should be divisible by 
    SAMPLES_PER_FRAME. Otherwise overhanging data are ignored.
    (7) The wavetable data shall continue from the beginning after the last 
    sample without breaks.

    @note Calling this method is as realtime-safe (or unsafe) as its C 
    string variant from_string(c, spf).

    @param s        Data string.
    @param spf      Samples per frame.
     */
    void from_string(const std::string& s, size_t spf = 128);

    /**
    Loads a .wvt text file and parses its content by from_string() to a 
    %Wavetable. See also from_string().
    @note   This method is NOT realtime-safe. Do NOT use it in a realtime
    thread.
    @param path     Path of the .wvt file.
     */
    void from_wvt(const std::string& path);

    /**
    Loads a %Wavetable from samples data with a defined frame size.
    @param samples  Pointer to samples data.
    @param frame_sz Number of samples per frame.
    @param total_sz Total number of samples.
     */
    template<typename T2>
    void from_samples(const T2* samples, const size_t frame_sz, const size_t total_sz);

    /**
    Gets the total number of samples of the %Wavetable.
    @return     Total number of samples.
     */
    size_t get_total_samples() const;

    /**
    Gets the total number of frames of the %Wavetable. A valid %Wavetable
    contains at least 1 frame.
    @return     Total number of frames.
     */
    size_t get_total_frames() const;

    /**
    Sets the number of samples per frame. If the number of samples per frame 
    exceeds the total number of frames, then the %Wavetable is filled up with
    zeros to complete at least one frame.
    @param spf      Number of samples per frame, at least 1.
    @param trim     If true, clips tailing samples.
     */
    void set_samples_per_frame(const size_t spf, bool trim = false);

    /**
    Gets the total number of samples per frame. A valid %Wavetable
    contains at least 1 sample per frame.
    @return     Number of samples per frames.
     */
    size_t get_samples_per_frame() const;

    /**
    Gets the value of a sample at a given relative position. The infractional
    part represents the relative position within this frame. Linear 
    interpolation is performed if the relative position is between two 
    samples. If the relative position exceeds the limits [0, frames), then the
    position count continues from the respecive opposite end of the %Wavetable.
    @param pos      Relativ position as frame index plus the relative position 
    [0.0, 1.0) within this frame.
    @return         (Interpolated) value of the sample.
     */
    T at_rel(T pos) const;

protected:

    /**
    Converts a simple (non-scientific) floating point number-containing string
    to a floating point value of the respective type.
    @param c     Floating point number-containing C string.
    @return  Converted floating point value.

    This method is tolerant over different types of decimal signs (point / 
    comma).
    Sets error byte if no conversion can be performed.
    */
    T strto_ (const char* const c);

    /**
    Interpolates the value at a provided %Wavetable position.
    @param frame    Index of the frame. Must be inside [0, number of frames).
    @param rp       Relative position [0.0, 1.0) within this frame.
    @return         Interpolated value of the sample.    
    */
    T interpolate_(const size_t frame, const T rp) const;
};

template<class T, size_t N> inline Wavetable<T, N>::Wavetable() :
    std::array<T, N>{0},
    wt_size_(0),
    wt_spf_(1),
    wt_err_(NO_ERROR)
{}

template<class T, size_t N> void inline Wavetable<T, N>::resize(const size_t sz, T val)
{
    if (sz == 0) operator[](0) = 0;
    if (sz <= 1)  wt_size_ = sz;
    else
    {
        size_t n_sz = std::min(N, sz);
        for (size_t i = wt_size_; i < n_sz; ++i) operator[](i) = val;
        wt_size_ = n_sz;
    }
}

template<class T, size_t N> void inline Wavetable<T, N>::clear()
{
    wt_size_ = 0;
    operator[](0) = 0;
    wt_spf_= 1;
}

template<class T, size_t N> void inline Wavetable<T, N>::push_back(const T val)
{
    if (wt_size_ < N)
    {
        operator[](wt_size_) = val;
        ++wt_size_; 
    }
    else wt_err_ = WAVETABLE_OVERFLOW;
}

template<class T, size_t N> void inline Wavetable<T, N>::pop_back()
{
    if (wt_size_ < 1) return;
    if (wt_size_ == 1) operator[](0);
    --wt_size_;
}

template<class T, size_t N> typename Wavetable<T, N>::ErrorCode inline Wavetable<T, N>::read_error()
{
    ErrorCode temp = wt_err_;
    wt_err_ = NO_ERROR;
    return temp;
}

template<class T, size_t N> void inline Wavetable<T, N>::from_string(const char* const c, size_t spf)
{
    if (!c) return;

    // Empty string: Clear wavetable
    if (!c[0])
    {
        clear();
        set_samples_per_frame(spf);
        return;
    }

    // Create a temporary wavetable with an empty wave
    Wavetable<T, N> n_data;
    const char* ic = c;
    const size_t clen = strlen(c);
    char l[0x10000] = "";

    while (ic < c + clen)
    {
        // Strip leading whitespaces
        while (ic[0] && (isspace(*ic))) ++ic;

        // Find next line break
        const char* const nc = strchr(ic, '\n');

        // Line
        size_t llen = 0;
        if (nc) llen = nc - ic;
        else llen = strlen(ic);
        if (llen >= 0x10000 - 1)
        {
            wt_err_ = CORRUPT_RAW_DATA_FORMAT;
            return;
        }

        strncpy(l, ic, llen);
        l[llen] = 0;

        // Set pos for next line 
        ic = nc + 1;

        // Everything but comments (#)
        if (l[0] != '#')
        {
            // Parameter(s)
            // TODO Use parser if more parameters will be implemented
            const char* cmdstr = strstr(l, "SAMPLES_PER_FRAME");
            if (cmdstr)
            {
                const char* valstr = cmdstr + strlen("SAMPLES_PER_FRAME");
                char* endptr = NULL;
                const long val = strtol(valstr, &endptr, 0);
                if ((valstr == endptr) || (val < 1) || (val >0x10000)) 
                {
                    wt_err_ = CORRUPT_RAW_DATA_VALUE;
                    break;
                }
                else spf = val;
            }

            // String to value
            else 
            {
                double val = strto_(l);
                if (wt_err_) break;
                n_data.push_back(val);
            }
        }
        
        // End of string
        if (!nc) break;

        // Next line 
        ic = nc + 1;
    }

    // Validate spf
    n_data.set_samples_per_frame(spf);

    // Copy data
    operator=(n_data);
}

template<class T, size_t N> void inline Wavetable<T, N>::from_string(const std::string& s, size_t spf)
{
    from_string(s.c_str(), spf);
}

template<class T, size_t N> void inline Wavetable<T, N>::from_wvt(const std::string& path)
{
    std::string s;
    std::ifstream file(path);

    if (!file.is_open()) throw std::invalid_argument("Can't open file " + path + ".");
    else
    {
        std::string line;
        while (std::getline(file, line)) s += line + '\n';

        file.close();
        from_string(s);
    }
}


template<class T, size_t N>
template<class T2>
inline void Wavetable<T, N>::from_samples(const T2* samples, const size_t frame_sz, const size_t total_sz)
{
    if ((!total_sz) || (!frame_sz)) return;

    Wavetable<T, N> n_data;
    size_t nr = std::min(N, total_sz);

    for (size_t i = 0; i < nr; ++ i) n_data[i] = static_cast<T>(samples[i]);

    // Set spf_ and validate
    set_samples_per_frame(std::min(N, frame_sz));

    // Copy data
    operator=(n_data);
}

template<class T, size_t N> inline size_t Wavetable<T, N>::get_total_samples() const {return get_total_frames() * wt_spf_;}

template<class T, size_t N> inline size_t Wavetable<T, N>::get_total_frames() const {return size() / wt_spf_;}

template<class T, size_t N> inline void Wavetable<T, N>::set_samples_per_frame(const size_t spf, bool trim) 
{
    // At least 1 sample
    if (spf == 0) 
    {
        wt_spf_ = 1;
        if (trim) resize (0);
    }

    else 
    {
        wt_spf_ = std::min(N, spf);
    
        // Fill up with zeros to complete at least 1 frame 
        if (wt_spf_ > wt_size_) resize(wt_spf_, 0);

        // Optional trim to complete frames
        if (trim) resize(wt_spf_ * get_total_frames());
    }

    
}

template<class T, size_t N> inline size_t Wavetable<T, N>::get_samples_per_frame() const {return wt_spf_;}

template<class T, size_t N> inline T Wavetable<T, N>::at_rel(T pos) const 
{
    if (wt_size_ == 0) return 0;

    // Keep pos in [0, frames]
    pos -= floor(pos / get_total_frames()) * get_total_frames();

    pos = std::fmod(pos, get_total_frames());
    T i_pos;                                    // Frame
    const T f_pos = std::modf(pos, &i_pos);     // Rel. pos within the frame

    return interpolate_(i_pos, f_pos);
}

template<class T, size_t N> inline T Wavetable<T, N>::interpolate_(const size_t frame, const T rp) const
{
    if (wt_size_ == 0) return 0;

    const T base = floor(rp * get_samples_per_frame());
    const size_t idx1 = (frame * get_samples_per_frame() + static_cast<size_t>(base)) % get_total_samples();
    const size_t idx2 = (idx1 + 1) % get_total_samples();
    const T f = rp * get_samples_per_frame() - base;
    const T val1 = operator[](idx1);
    const T val2 = operator[](idx2);
    return val1 + f * (val2 - val1);
} 

template<class T, size_t N> inline T Wavetable<T, N>::strto_ (const char* const c)
{
    if ((!c) || (c[0] == 0))
    {
        wt_err_ = RAW_DATA_VALUE_EXPECTED;
        return 0;
    }

        const char* ci = c;
        bool isNumber = false;
        T sign = 1.0;
        T predec = 0.0;
        T dec = 0.0;
        T decfac = 0.1;

        // Ignore spaces before
        while (ci[0] == ' ') ++ci;

        // Check sign
        if ((ci[0] == '+') || (ci[0] == '-'))
        {
                if (ci[0] == '-') sign = -1.0;
                ++ci;
        }

        // Interpret pre-decimal digits
        while ((ci[0] != 0) && (ci[0] >= '0') && (ci[0] <= '9'))
        {
                predec = predec * 10.0 + ci[0] - '0';
                ++ci;
                isNumber = true;
        }

        // Check decimal sign
        if ((ci[0] == '.') || (ci[0] == ','))
        {
                ++ci;

                // Interpret decimal digits
                while ((ci[0] != 0) && (ci[0] >= '0') && (ci[0] <= '9'))
                {
                        dec += (ci[0] - '0') * decfac;
                        decfac *= 0.1;
                        ++ci;
                        isNumber = true;
                }
        }

        // Communicate next position
        if (!isNumber)
        {
            wt_err_ = CORRUPT_RAW_DATA_VALUE;
            return 0;
        }

        return sign * (predec + dec);
}

#endif /* WAVETABLE_HPP_ */
