#ifndef BVIBRATR_HPP_
#define BVIBRATR_HPP_

#include <array>
#include "ADSR.hpp"
#include "LFO.hpp"
#include "LinearFader.hpp"
#include "RingBuffer.hpp"

#include <cstdint>
#include <lv2/core/lv2.h>
#include <lv2/atom/atom.h>

#define BVIBRATR_URI "https://www.jahnichen.de/plugins/lv2/BVibratr"
//#define LV2PLUGIN_GUI_URI LV2PLUGIN_URI "#gui"

#include "Ports.hpp"
#include "Urids.hpp"


class BVibratr
{
public:
	BVibratr (double samplerate, const char* bundlePath, const LV2_Feature* const* features);
	~BVibratr ();

	void connect_port (uint32_t port, void *data);
	void activate ();
	void run (uint32_t n_samples);
	void deactivate ();

private:
	void on_midi_note_on (const uint8_t channel, const uint8_t note, const uint8_t velocity);
	void on_midi_note_off (const uint8_t channel, const uint8_t note, const uint8_t velocity);
	void on_midi_cc (const uint8_t channel, const uint8_t cc, const uint8_t param);
	void on_midi (const uint8_t* const msg);
	void play (uint32_t start, uint32_t end);

	double rate;

	// Ports
	LV2_Atom_Sequence* midi_in;
	float* audio_in_1;
	float* audio_in_2;
	float* audio_out_1;
	float* audio_out_2;
	std::array<const float*, BVIBRATR_NR_CONTROLLERS> controller_ports;

	// Optional map feature
	LV2_URID_Map* map;
	BVibratrURIDs urids;

	// Controllers
	std::array<float, BVIBRATR_NR_CONTROLLERS> controllers;
	ADSR<double> adsr;

	// Internals
	LFO<double> osc1, osc2, osc3;
	double depth_cc;
	RingBuffer<float> buffer_1;
	RingBuffer<float> buffer_2;
	size_t buffer_offset;
	int osc1_mode, osc2_mode, osc3_mode;	// TODO Schedule change
	double depth;
	LinearFader<double> shift;				// Temporal shift (vibrato)
	LinearFader<double> amp;				// Volume change (tremolo)
};

#endif /* BVIBRATR_HPP_ */
