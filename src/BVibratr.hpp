#ifndef BVIBRATR_HPP_
#define BVIBRATR_HPP_

#include <array>
#include "ADSR.hpp"
#include "Oscx3.hpp"
#include "LinearFader.hpp"
#include "Patch.hpp"
#include "RingBuffer.hpp"

#include <cstdint>
#include <lv2/core/lv2.h>
#include <lv2/atom/atom.h>
#include <lv2/atom/forge.h>
#include <lv2/state/state.h>
#include <lv2/worker/worker.h>
#include <utility>

#include "Ports.hpp"
#include "Urids.hpp"
#include "Wavetable.hpp"


class BVibratr
{
private:
	struct Atom_GC{LV2_Atom atom; const Wavetable<>* ptr;};
	struct Atom_WT_Install{LV2_Atom atom; std::pair<Wavetable<>*, Wavetable<>*> wts;};

public:
	BVibratr (double samplerate, const char* bundlePath, const LV2_Feature* const* features);
	~BVibratr ();

	void connect_port (uint32_t port, void *data);
	void activate ();
	void run (uint32_t n_samples);
	void deactivate ();
	LV2_State_Status state_save (LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags, const LV2_Feature* const* features);
	LV2_State_Status state_restore (LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags, const LV2_Feature* const* features);
	LV2_Worker_Status work (LV2_Worker_Respond_Function respond, LV2_Worker_Respond_Handle handle, uint32_t size, const void* data);
	LV2_Worker_Status work_response (uint32_t size, const void* data);

private:
	void start();
	void on_midi_note_on (const uint8_t channel, const uint8_t note, const uint8_t velocity);
	void on_midi_note_off (const uint8_t channel, const uint8_t note, const uint8_t velocity);
	void on_midi_cc (const uint8_t channel, const uint8_t cc, const uint8_t param);
	void on_midi (const uint8_t* const msg);
	void osc1_set_waveform();
	static void on_osc1_restart(LFO<double>& adsr, void* obj);
	static void on_osc2_restart(LFO<double>& adsr, void* obj);
	static void on_osc3_restart(LFO<double>& adsr, void* obj);
	void play (uint32_t start, uint32_t end);
	LV2_Atom_Forge_Ref forge_patch_wavetable(LV2_Atom_Forge& forge, const uint32_t n_elems, const double* data);
	LV2_Atom_Forge_Ref forge_patch_spf(LV2_Atom_Forge& forge, const uint32_t spf);
	bool garbage_collector(const Wavetable<>* ptr);
	Atom_WT_Install work_new_wavetable();

	double rate;

	// Ports
	const LV2_Atom_Sequence* control_in;
	LV2_Atom_Sequence* control_out;
	const float* audio_in_1;
	const float* audio_in_2;
	float* audio_out_1;
	float* audio_out_2;
	std::array<const float*, BVIBRATR_NR_CONTROLLERS> controller_ports;
	float* latency_port;

	// Map and mapped urids
	LV2_URID_Map* map;
	//LV2_URID_Unmap* unmap;
	BVibratrURIDs urids;
	Patch patch;
	LV2_Atom_Forge forge;

	// Worker schedule
	LV2_Worker_Schedule* workerSchedule;

	// Controllers
	std::array<float, BVIBRATR_NR_CONTROLLERS> controllers;

	// Internals
	ADSR<double> adsr;
	Oscx3<double> oscx3; 
	uint8_t note;							// Last NOTE_ON note (or >= 0x80 for none)
	double depth_cc;
	RingBuffer<float> buffer_1;
	RingBuffer<float> buffer_2;
	size_t buffer_offset;
	double depth;
	LinearFader<double> shift;				// Temporal shift (vibrato)
	LinearFader<float> amp;					// Volume change (tremolo)
	LinearFader<float> mix;					// Mix for change in dry/wet and bypass
	Wavetable<> worker_wt;					// (Temporary) wavetable, only used for the worker thread to create the real wavetables for the oscillators
	bool notify;							// Request to send data via CONTROL_OUT
};

#endif /* BVIBRATR_HPP_ */
