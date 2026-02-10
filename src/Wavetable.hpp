#ifndef WAVETABLE_HPP_
#define WAVETABLE_HPP_

#include <cmath>
#include <cstddef>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "BWidgets/BUtilities/sto.hpp"
#include "BWidgets/BUtilities/strip.hpp"

/**
Template class for wavetable data. A %Wavetable consists of a number of frames
and each frame consists of a number of samples.
*/
template<class T = double>
class Wavetable : protected std::vector<T>
{
protected:

    /* samples per frame*/
    size_t spf_ = 128;

public:
    /**
    Constructs a default %Wavetable containing 1 frame and 1 sample with the
    value 0.
     */
    Wavetable<T>();

    using std::vector<T>::operator[];
    using std::vector<T>::size;

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
    @param s        Data string.
     */
    void from_string(const std::string& s);

    /**
    Loads a .wvt text file and parses its content by from_string() to a 
    %Wavetable. See also from_string().
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
    Gets the total number of samples of the %Wavetable. A valid %Wavetable
    contains at least 1 sample.
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
    Interpolates the value at a provided %Wavetable position.
    @param frame    Index of the frame. Must be inside [0, number of frames).
    @param rp       Relative position [0.0, 1.0) within this frame.
    @return         Interpolated value of the sample.    
     */
    T interpolate_(const size_t frame, const T rp) const;
};

template<class T> inline Wavetable<T>::Wavetable() :
    std::vector<T>(0),
    spf_(1)
{}

template<class T> void inline Wavetable<T>::from_string(const std::string& s)
{
    if (s.empty())
    {
        this->clear();
        set_samples_per_frame(spf_);
        return;
    }

    // Create a temporary wavetable with an empty wave
    std::vector<T> n_data;
    size_t pos = 0;

    while (pos < s.length())
    {
        // Find next line break
        const size_t n_idx = s.find('\n', pos);

        std::string line = s.substr(pos, n_idx - pos);
        BUtilities::strip(line);

        // Set pos for next line 
        pos = n_idx + 1;
        
        // Empty line: new wave
        if (line.empty()) continue;

        // Starts with #: comment
        if (line[0] == '#') continue;

        // Parameter(s)
        // TODO Use parser if more parameters will be implemented
        const size_t spf_idx = line.find("SAMPLES_PER_FRAME");
        if (spf_idx != std::string::npos)
        {
            std::string value_str = line.substr(spf_idx + 18);
            try {spf_ = std::stoi(value_str);}
            catch (std::exception& exc) {std::cerr << "Error: " << exc.what() << std::endl;}
            continue;
        }

        // String to value
        try
        {
            const float value = BUtilities::sto<T>(line);
            n_data.push_back(value);
        }
        catch (std::exception& exc)
        {
            std::cerr << "Error: " << exc.what() << std::endl;
            return;
        }

        // EOS
        if (n_idx == std::string::npos) break;
    }

    // Validate spf_
    set_samples_per_frame(spf_);

    // Copy data
    std::vector<T>::operator=(n_data);
}

template<class T> void inline Wavetable<T>::from_wvt(const std::string& path)
{
    std::string s;
    std::ifstream file(path);

    if (!file.is_open()) std::cerr << "IOError: Can't open " << path << std::endl;
    else
    {
        std::string line;
        while (std::getline(file, line)) s += line + '\n';

        file.close();
        from_string(s);
    } 
}

template<class T>
template<class T2>
inline void Wavetable<T>::from_samples(const T2* samples, const size_t frame_sz, const size_t total_sz)
{
    if ((!total_sz) || (!frame_sz)) return;

    std::vector<T> n_data(total_sz, 0);

    for (size_t i = 0; i < total_sz; ++ i) n_data[i] = static_cast<T>(samples[i]);

    // Set spf_ and validate
    set_samples_per_frame(frame_sz);

    // Copy data
    std::vector<T>::operator=(n_data);
}

template<class T> inline size_t Wavetable<T>::get_total_samples() const {return get_total_frames() * spf_;}

template<class T> inline size_t Wavetable<T>::get_total_frames() const {return this->size() / spf_;}

template<class T> inline void Wavetable<T>::set_samples_per_frame(const size_t spf, bool trim) 
{
    // At least 1 sample
    spf_ = (spf == 0) ? 1 : spf;
    
    // Fill up with zeros to complete at least 1 frame 
    if (spf_ > this->size()) this->resize(spf_, 0);

    // Optional trim to complete frames
    if (trim) this->resize(spf_ * get_total_frames());
}

template<class T> inline size_t Wavetable<T>::get_samples_per_frame() const {return spf_;}

template<class T> inline T Wavetable<T>::at_rel(T pos) const 
{
    // Keep pos in [0, frames]
    pos -= floor(pos / get_total_frames()) * get_total_frames();

    pos = std::fmod(pos, get_total_frames());
    T i_pos;                                    // Frame
    const T f_pos = std::modf(pos, &i_pos);     // Rel. pos within the frame

    return interpolate_(i_pos, f_pos);
}

template<class T> inline T Wavetable<T>::interpolate_(const size_t frame, const T rp) const
{
    const T base = floor(rp * get_samples_per_frame());
    const size_t idx1 = static_cast<size_t>(base) % get_total_samples();
    const size_t idx2 = (idx1 + 1) % get_samples_per_frame();
    const T f = rp * get_samples_per_frame() - base;
    const T val1 = this->operator[](idx1);
    const T val2 = this->operator[](idx2);
    return val1 + f * (val2 - val1);
} 

#endif /* WAVETABLE_HPP_ */
