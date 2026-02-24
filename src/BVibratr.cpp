#include "BVibratr.hpp"
#include "Limits.hpp"
#include "Ports.hpp"
#include "Wavetable.hpp"

// Utilities
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <limits>
#include <lv2/atom/atom.h>
#include <lv2/atom/forge.h>
#include <lv2/atom/util.h>
#include <lv2/midi/midi.h>
#include <lv2/state/state.h>
#include <lv2/worker/worker.h>
#include <new>

#define SQRT_12_2 (pow (2.0, 1.0 / 12.0))

BVibratr::BVibratr (double samplerate, const char* bundlePath, const LV2_Feature* const* features) :
	rate (samplerate),
	control_in (nullptr),
	audio_in_1 (nullptr),
	audio_in_2 (nullptr),
	audio_out_1 (nullptr),
	audio_out_2 (nullptr),
	latency_port(nullptr),
	map (nullptr),
	//unmap(nullptr),
	adsr(0, 0, 1, 0, ADSR<double>::INVSQR),
	oscx3(samplerate),
	note(0xFF),
	depth_cc (1.0),
	buffer_1(0x10000),
	buffer_2(0x10000),
	buffer_offset((SQRT_12_2 - 1.0) *	// Up to 1 semitone
					samplerate			// Up to 1 second phase length
				 ),						// TODO report latency
	depth(0.0),
	shift(0.0, (SQRT_12_2 - 1.0)),	// Limit temporal shift to 1 semitone
	amp(1.0f, 0.001f),
	mix(0.0f, 0.001f),
	notify(false)
{
	// Init controllers
	controller_ports.fill(nullptr);
	controllers.fill(std::numeric_limits<float>::infinity());	// Ensure change once ports connected

	// Map, urids, ...
	urids.init (features, &map);
	patch = Patch(map);
	lv2_atom_forge_init (&forge, map);

	//const char* missing1 = lv2_features_query (features, LV2_URID__unmap, &unmap, false, NULL);
	//if (missing1) throw std::invalid_argument ("Feature unmap not provided by the host. Can't instantiate plugin.");
	
	// Get worker
	lv2_features_query (features, LV2_WORKER__schedule, &workerSchedule, true, NULL);
    if (!workerSchedule) throw std::invalid_argument ("Feature worker not provided by the host. Can't instantiate plugin.");

	// Init buffers
	buffer_1.fill(0.0f);
	buffer_2.fill(0.0f);

	oscx3.osc1.garbage_collector = [this](const Wavetable<>* ptr){return garbage_collector(ptr);};

	oscx3.osc1.setCallbackFunction(LFO<double>::PHASE_RESTART, &on_osc1_restart, this);
	oscx3.osc2.setCallbackFunction(LFO<double>::PHASE_RESTART, &on_osc2_restart, this);
	oscx3.osc3.setCallbackFunction(LFO<double>::PHASE_RESTART, &on_osc3_restart, this);
}

BVibratr::~BVibratr () {}

void BVibratr::connect_port (uint32_t port, void *data)
{
	switch (port) 
	{
		case BVIBRATR_CONTROL_IN:	control_in = static_cast<const LV2_Atom_Sequence*>(data);
									break;

		case BVIBRATR_CONTROL_OUT:	control_out = static_cast<LV2_Atom_Sequence*>(data);
									break;

		case BVIBRATR_AUDIO_IN_1:	audio_in_1 = static_cast<const float*>(data);
									break;

		case BVIBRATR_AUDIO_IN_2:	audio_in_2 = static_cast<const float*>(data);
									break;

		case BVIBRATR_AUDIO_OUT_1:	audio_out_1 = static_cast<float*>(data);
									break;

		case BVIBRATR_AUDIO_OUT_2:	audio_out_2 = static_cast<float*>(data);
									break;
		
		default:
			if ((port >= BVIBRATR_NR_PORTS) && (port < BVIBRATR_NR_PORTS + BVIBRATR_NR_CONTROLLERS))
			{
				controller_ports[port - BVIBRATR_NR_PORTS] = static_cast<float*>(data);
			}

			else if (port == BVIBRATR_NR_PORTS + BVIBRATR_LATENCY) 
			{
				latency_port = static_cast<float*>(data);
			}
	}
}

void BVibratr::activate ()
{}

void BVibratr::deactivate ()
{}

void BVibratr::run (uint32_t n_samples)
{
	// Check if all ports are connected
	if ((!control_in) || (!control_out) || (!audio_in_1) || (!audio_in_2) || (!audio_out_1) || (!audio_out_2)) return;
	for (const float* c : controller_ports) if (!c) return;
	if (!latency_port) return;

	// Update controllers
	*(latency_port) = buffer_offset;
	const float o_midi_channels = controllers[BVIBRATR_MIDI_CHANNEL];
	for (int i = 0; i < BVIBRATR_NR_CONTROLLERS; ++i) 
	{
		const float value = controller_limits[i].validate(*controller_ports[i]);
		
		// ... but only if value changed
		if (controllers[i] != value)
		{
			controllers[i] = value;

			switch (i)
			{
				case BVIBRATR_BYPASS:
				case BVIBRATR_DRY_WET:
					mix.set((1.0f - controllers[BVIBRATR_BYPASS]) * controllers[BVIBRATR_DRY_WET]);
					break;

				case BVIBRATR_DEPTH_IS_CC:
					depth =	((value == 128) ? (0.01 /* cents */ * controllers[BVIBRATR_DEPTH]) : depth_cc);
					break;

				case BVIBRATR_DEPTH:
					if (controllers[BVIBRATR_DEPTH_IS_CC] == 128) depth = 0.01 /* cents */ * value;
					break;

				case BVIBRATR_OSC1_MODE:
				case BVIBRATR_OSC1_WAVEFORM:
					osc1_set_waveform();
					break;

				case BVIBRATR_OSC2_WAVEFORM:
					oscx3.osc2.set_waveform(static_cast<LFO<double>::Waveform>(value));
					break;

				case BVIBRATR_OSC3_WAVEFORM:
					oscx3.osc3.set_waveform(static_cast<LFO<double>::Waveform>(value));
					break;

				default:
					break;
			}
		}
	}

	// Analysis after all ports updated
	if (o_midi_channels != controllers[BVIBRATR_MIDI_CHANNEL])
	{
		if (controllers[BVIBRATR_MIDI_CHANNEL] == 0)
		{
			start();
			this->note = 0xFF;
		}
		else if (o_midi_channels == 0)
		{
			adsr.stop();
			this->note = 0xFF;
		}
	}

	// Prepare CONTROL_OUT buffer and initialize atom sequence
	const uint32_t space = control_out->atom.size;
	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_set_buffer(&forge, (uint8_t*) control_out, space);
	lv2_atom_forge_sequence_head(&forge, &frame, 0);

	// Playback, parameters and MIDI
	uint32_t last_frame = 0;
    LV2_ATOM_SEQUENCE_FOREACH (control_in, ev)
    {
        /* play frames until event */
        const uint32_t frame = ev->time.frames;
		const LV2_Atom* atom = &ev->body;
        play (last_frame, frame);
        last_frame = frame;

		// Midi
		if (atom->type == urids.midi_MidiEvent) on_midi(reinterpret_cast<const uint8_t*> (ev + 1));
		
		// Object: Patch messages
		else if ((atom->type == urids.atom_Object) || (atom->type == urids.atom_Blank))
		{
			const LV2_Atom_Object* obj = reinterpret_cast<const LV2_Atom_Object*>(atom);

			// Patch_Get: Blindly send wavetable to CONTROL_OUT
			if (obj->body.otype == urids.patch_Get) notify = true;

			// Patch_Set: Forward to worker
			else if (obj->body.otype == urids.patch_Set) workerSchedule->schedule_work(workerSchedule->handle, lv2_atom_total_size(atom), atom);
			
		}
    }

    /* play remaining frames */
    play (last_frame, n_samples);

	// Update CONTROL_OUT
	if (notify)
	{
		const Wavetable<>* wt = oscx3.osc1.get_latest_wavetable();
		if (wt)
		{
			notify = false;
			if (lv2_atom_forge_frame_time(&forge, 0) &&
				patch.write_patch_Set_Vector(forge, urids.bvibratr, urids.bvibratr_wavetable_data, sizeof(double), urids.atom_Double, wt->size(), &((*wt)[0])) &&
				lv2_atom_forge_frame_time(&forge, 0) &&
				patch.write_patch_Set_Int(forge, urids.bvibratr, urids.bvibratr_wavetable_spf, wt->get_samples_per_frame()))
			{} /* pass */
		}
	}

	if (oscx3.osc1.get_waveform() == LFO<double>::WAVETABLE)
	{
		const double pos = oscx3.osc1.get_position();
		if (lv2_atom_forge_frame_time(&forge, 0) &&
			patch.write_patch_Set_Double(forge, urids.bvibratr, urids.bvibratr_wavetable_pos, pos))
		{} /* pass */
	}

	// Close CONTROL_OUT
	lv2_atom_forge_pop(&forge, &frame);
}

void BVibratr::start()
{
	adsr.set_parameters	(controllers[BVIBRATR_DEPTH_ATTACK],
						 controllers[BVIBRATR_DEPTH_DECAY],
						 controllers[BVIBRATR_DEPTH_SUSTAIN],
						 controllers[BVIBRATR_DEPTH_RELEASE]);

	osc1_set_waveform();
	oscx3.osc2.set_waveform(static_cast<LFO<double>::Waveform>(controllers[BVIBRATR_OSC2_WAVEFORM]));
	oscx3.osc3.set_waveform(static_cast<LFO<double>::Waveform>(controllers[BVIBRATR_OSC3_WAVEFORM]));

	adsr.start();
	oscx3.start();
}

void BVibratr::on_midi_note_on (const uint8_t channel, const uint8_t note, const uint8_t velocity)
{
	if (static_cast<uint16_t>(controllers[BVIBRATR_MIDI_CHANNEL]) & (1 << channel))
	{
		if ((controllers[BVIBRATR_MIDI_NOTE] == note) || (controllers[BVIBRATR_MIDI_NOTE] == 128))
		{
			start();
			this->note = note;
		}
	}
}

void BVibratr::on_midi_note_off (const uint8_t channel, const uint8_t note, const uint8_t velocity)
{
	if (static_cast<uint16_t>(controllers[BVIBRATR_MIDI_CHANNEL]) & (1 << channel))
	{
		if (this->note == note)
		{
			adsr.release();
			this->note = 0xFF;
		}
	}
}

void BVibratr::on_midi_cc (const uint8_t channel, const uint8_t cc, const uint8_t param)
{
	if ((static_cast<uint16_t>(controllers[BVIBRATR_MIDI_CHANNEL]) & (1 << channel)) or 
	    (controllers[BVIBRATR_MIDI_CHANNEL] == 0.0f))
	{
		switch (cc)
		{
			case LV2_MIDI_CTL_ALL_NOTES_OFF:	
				on_midi_note_off	(channel, 
									 static_cast<uint8_t>(controllers[BVIBRATR_MIDI_NOTE]), 
									 0);
				break;

			case LV2_MIDI_CTL_ALL_SOUNDS_OFF:	
				on_midi_note_off	(channel, 
									 static_cast<uint8_t>(controllers[BVIBRATR_MIDI_NOTE]), 
									 0);
				oscx3.stop();
				break;

			default:
				{
					if ((controllers[BVIBRATR_DEPTH_IS_CC] != 128) && 
						(cc == controllers[BVIBRATR_DEPTH_IS_CC]))
					{
						depth_cc = static_cast<double>(param) / 127.0;
						depth = 0.01 /* cents */ * controller_limits[BVIBRATR_DEPTH].max * depth_cc;
					}
				}
		}
	}

}

void BVibratr::on_midi (const uint8_t* const msg)
{
	const uint8_t typ = lv2_midi_message_type (msg);
	const uint8_t status = typ & 0xf0;
	const uint8_t channel = typ & 0x0f;

	switch (status)
	{
		case LV2_MIDI_MSG_NOTE_ON:		on_midi_note_on(channel, msg[1], msg[2]);
										break;

		case LV2_MIDI_MSG_NOTE_OFF:		on_midi_note_off(channel, msg[1], msg[2]);
										break;

		case LV2_MIDI_MSG_CONTROLLER:	on_midi_cc(channel, msg[1], msg[2]);
										break;
		default: break;
	}

}

void BVibratr::osc1_set_waveform()
{
	oscx3.osc1.set_waveform(static_cast<LFO<double>::Waveform>((controllers[BVIBRATR_OSC1_MODE] == BVIBRATR_OSC_MODE_LFO) ?
															   controllers[BVIBRATR_OSC1_WAVEFORM] :
															   0));
}

void BVibratr::on_osc1_restart(LFO<double>& adsr, void* obj)
{
	BVibratr* plugin = static_cast<BVibratr*>(obj);
	plugin->oscx3.mode1 = static_cast<BVibratrOscModes>(plugin->controllers[BVIBRATR_OSC1_MODE]);
}

void BVibratr::on_osc2_restart(LFO<double>& adsr, void* obj)
{
	BVibratr* plugin = static_cast<BVibratr*>(obj);
	plugin->oscx3.mode2 = static_cast<BVibratrOscModes>(plugin->controllers[BVIBRATR_OSC2_MODE]);
}

void BVibratr::on_osc3_restart(LFO<double>& adsr, void* obj)
{
	BVibratr* plugin = static_cast<BVibratr*>(obj);
	plugin->oscx3.mode3 = static_cast<BVibratrOscModes>(plugin->controllers[BVIBRATR_OSC3_MODE]);
}

void BVibratr::play (uint32_t start, uint32_t end)
{
	const double sample_time = 1.0 / rate;

	// Update oscillator settings
	oscx3.amp1 = 1.0;
	oscx3.freq1 = controllers[BVIBRATR_OSC1_FREQ];
	oscx3.amp2 = controllers[BVIBRATR_OSC2_AMP];
	oscx3.freq2 = controllers[BVIBRATR_OSC2_FREQ];
	oscx3.amp3 = controllers[BVIBRATR_OSC3_AMP];
	oscx3.freq3 = controllers[BVIBRATR_OSC3_FREQ];

	for (uint32_t i = start; i < end; ++i)
	{
		double signal = 0.0;
		double integral = 0.0;

		// Set modes if adsr (and thus all lfos) is stopped
		if (!adsr.is_active())
		{
			oscx3.mode1 = static_cast<BVibratrOscModes>(controllers[BVIBRATR_OSC1_MODE]);
			oscx3.mode2 = static_cast<BVibratrOscModes>(controllers[BVIBRATR_OSC2_MODE]);
			oscx3.mode3 = static_cast<BVibratrOscModes>(controllers[BVIBRATR_OSC3_MODE]);
		}

		// Only run oscillators if adsr is active
		else 
		{
			oscx3.run(sample_time);
			signal = oscx3.get_value();
			integral = oscx3.get_integral();

			// Apply adsr
			adsr.run(sample_time);
			signal *= adsr.get_value();
			integral *= adsr.get_value();
		}

		// Vibrato depth 
		integral *= depth;
		shift.set((SQRT_12_2 - 1.0) * integral);

		// ... and tremolo
		// Send signal * controller to fader to prevent clicks on square waves
		const double tremolo  = controllers[BVIBRATR_TREMOLO] * signal;
		amp.set(1.0 - tremolo);

		// Proceed dry/wet mix
		mix.proceed();
		const float mix_f = mix.get();

		// Audio output
		buffer_1.push_front(audio_in_1[i]);
		buffer_2.push_front(audio_in_2[i]);
		const float dry_1 = buffer_1[buffer_offset];
		const float dry_2 = buffer_2[buffer_offset];
		const float wet_1 = amp.get() * buffer_1[buffer_offset + shift.get()];
		const float wet_2 = amp.get() * buffer_2[buffer_offset + shift.get()];
		audio_out_1[i] = (1.0f - mix_f) * dry_1 + mix_f * wet_1;
		audio_out_2[i] = (1.0f - mix_f) * dry_2 + mix_f * wet_2;
	}
}

LV2_State_Status BVibratr::state_save (LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags, const LV2_Feature* const* features)
{
	// Use worker_wt
	// Store samples per frame as plain data
	const int spf = worker_wt.get_samples_per_frame();
	store(handle, urids.bvibratr_wavetable_spf, &spf, sizeof(spf), urids.atom_Int, LV2_STATE_IS_POD);

	// Store Wavetable data as Vector body data
	const uint32_t size = worker_wt.size() * sizeof(double) + sizeof(LV2_Atom_Vector);
	uint8_t* buf = new (std::nothrow) uint8_t[size];
	if (!buf) return LV2_STATE_ERR_NO_SPACE;

	LV2_Atom_Forge forge;
	lv2_atom_forge_init(&forge, map);
	lv2_atom_forge_set_buffer(&forge, buf, size);
	LV2_Atom_Vector* vec = reinterpret_cast<LV2_Atom_Vector*>(lv2_atom_forge_vector(&forge, 
																					sizeof(double), 
																					urids.atom_Double, 
																					worker_wt.size(), 
																					&(worker_wt[0])));
	if (!vec)
	{
		delete[] buf;
		return LV2_STATE_ERR_UNKNOWN;
	}
	
	store (handle, urids.bvibratr_wavetable_data, LV2_ATOM_BODY(vec), size - sizeof(LV2_Atom), urids.atom_Vector, LV2_STATE_IS_POD);			
	delete[] buf;
	
	return LV2_STATE_SUCCESS;
}

LV2_State_Status BVibratr::state_restore (LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags, const LV2_Feature* const* features)
{

	size_t   size;
	uint32_t type;
	uint32_t valflags;

	// Get worker
	LV2_Worker_Schedule* schedule = nullptr;
	lv2_features_query (features, LV2_WORKER__schedule, &schedule, true, NULL);
    if (!schedule) 
	{
		fprintf(stderr, "Feature worker not provided by the host. Can't restore plugin state.");
		return LV2_STATE_ERR_NO_FEATURE;
	}

	// Restore samples per frame
	const void* spf = retrieve(handle, urids.bvibratr_wavetable_spf, &size, &type, &valflags);
	if (spf && (type == urids.atom_Int)) 
	{
		const int value = std::clamp(*reinterpret_cast<const int*>(spf), 1, 65336);
		LV2_Atom_Forge forge;
		lv2_atom_forge_init(&forge, map);
		uint8_t buf[256];
		lv2_atom_forge_set_buffer(&forge, buf, 256);
		const LV2_Atom* atom =reinterpret_cast<const LV2_Atom*>(patch.write_patch_Set_Int(forge, urids.bvibratr, urids.bvibratr_wavetable_spf, value));
		if (atom) schedule->schedule_work(schedule->handle, lv2_atom_total_size(atom), atom);
	}

	// Restore wavetable data
	const void* data = retrieve(handle, urids.bvibratr_wavetable_data, &size, &type, &valflags);
	if (data && (type == urids.atom_Vector))
	{
		const LV2_Atom_Vector_Body* body = reinterpret_cast<const LV2_Atom_Vector_Body*>(data);
		const double* values = reinterpret_cast<const double*>(body + 1);
		const size_t n_elems = (size - sizeof(LV2_Atom_Vector_Body)) / sizeof(double); 
		LV2_Atom_Forge forge;
		lv2_atom_forge_init(&forge, map);
		uint8_t* buf = new (std::nothrow) uint8_t[256 + n_elems * sizeof(double)];
		if (!buf) return LV2_STATE_ERR_NO_SPACE;
		lv2_atom_forge_set_buffer(&forge, buf, 256 + n_elems * sizeof(double));
		const LV2_Atom* atom =reinterpret_cast<const LV2_Atom*>(patch.write_patch_Set_Vector(forge, 
																							 urids.bvibratr, 
																							 urids.bvibratr_wavetable_data, 
																							 sizeof(double), 
																							 urids.atom_Double, 
																							 n_elems, 
																							 values));
		if (atom) schedule->schedule_work(schedule->handle, lv2_atom_total_size(atom), atom);
		delete[] buf;
    }

	return LV2_STATE_SUCCESS;
}

bool BVibratr::garbage_collector(const Wavetable<>* ptr)
{
	if (!ptr) return false;

	// Send a discreet atom to worker as everything is clear if urid bvibratr_garbage_collecor is provided
	Atom_GC wtptr_atom = {{sizeof(Wavetable<>*), urids.bvibratr_garbage_collecor}, ptr};
	workerSchedule->schedule_work (workerSchedule->handle, sizeof(Atom_GC), &wtptr_atom);
	return true;
}

BVibratr::Atom_WT_Install BVibratr::work_new_wavetable()
{
	// 2x Copy construct the work wavetable to wt1 and wt2 in the heap memory. Needs to be deleted outside.
	// this method (e.g., using a garbage collector)
	Wavetable<>* wt1 = new (std::nothrow) Wavetable<>();
	if (!wt1) return {0, 0, {nullptr, nullptr}};
	Wavetable<>* wt2 = new (std::nothrow) Wavetable<>();
	if (!wt2)
	{
		delete wt1;
		return {0, 0, {nullptr, nullptr}};
	}

	*wt1 = worker_wt;
	*wt2 = worker_wt;

	// Integrate wt2
	wt2->integrate();
	wt2->normalize(-1, 1);

	// Create a patch message with a ptr to the new wavetables
	return  {2 * sizeof(Wavetable<>*), urids.bvibratr_wavetable_install, {wt1, wt2}};
}

LV2_Worker_Status BVibratr::work (LV2_Worker_Respond_Function respond, LV2_Worker_Respond_Handle handle, uint32_t size, const void* data)
{
	if (!data) return LV2_WORKER_ERR_UNKNOWN;
	const LV2_Atom* atom = reinterpret_cast<const LV2_Atom*>(data);
	if (!atom) return LV2_WORKER_ERR_UNKNOWN;

	// Garbage collector: delete Wavetable<>* and nothing else
	if (atom->type == urids.bvibratr_garbage_collecor)
	{
		const Atom_GC* gc_atom = reinterpret_cast<const Atom_GC*>(atom);
		if (gc_atom) delete reinterpret_cast<const Wavetable<>*>(gc_atom->ptr);
	}

	// Patch data
	else if (patch.is_Patch_Msg(atom) && (patch.get_subject_type(atom) == urids.bvibratr))
	{
		// Wavetable path
		LV2_URID property = patch.get_property_type(atom);
		if (property == urids.bvibratr_wavetable_path)
		{
			// Path propvided: Load and install
			const LV2_Atom* watom = patch.get_value_atom(atom);
			if (watom && (watom->type == urids.atom_Path))
			{
				// Load new wavetable
				const char* path = reinterpret_cast<const char*>(LV2_ATOM_BODY_CONST(watom));
				Wavetable<>* wt = new (std::nothrow) Wavetable<>();
				if (!wt) return LV2_WORKER_ERR_NO_SPACE;

				std::string err = "";

				// Try to load from sndfile
				try {wt->from_soundfile(std::string(path));}
				catch (std::exception& exc) {err = exc.what();}

				// Try to load from .wvt
				if (!err.empty())
				{
					err = "";
					try {wt->from_wvt(std::string(path));}
					catch (std::exception& exc) {err = exc.what();}
				}

				// On success
				if (err.empty())
				{
					worker_wt = *wt;

					// Copy, integrate, and pack pointers to Atom_WT_Install
					const Atom_WT_Install msg = work_new_wavetable();

					// Send whole Atom_WT_Install to LFO1 via work_response
					const LV2_Atom* msg_ptr = reinterpret_cast<const LV2_Atom*>(&msg);
					if (msg.wts.first && msg.wts.second) respond(handle, lv2_atom_total_size(msg_ptr), msg_ptr);

				}

				delete wt;
			}
		}

		// Wavetable data
		else if (property == urids.bvibratr_wavetable_data)	
		{
			// Data provided
			const LV2_Atom* watom = patch.get_value_atom(atom);
			if (watom && (watom->type == urids.atom_Vector))
			{
				const LV2_Atom_Vector* vatom = reinterpret_cast<const LV2_Atom_Vector*>(watom);
				const uint32_t n_elems = (vatom->atom.size - sizeof(LV2_Atom_Vector_Body)) / sizeof(double);
				const double* values = reinterpret_cast<const double*>(LV2_ATOM_CONTENTS_CONST(LV2_Atom_Vector, vatom));
				worker_wt.from_samples(values, worker_wt.get_samples_per_frame(), n_elems);

				// Copy, integrate, and pack pointers to Atom_WT_Install
				const Atom_WT_Install msg = work_new_wavetable();

				// Send whole Atom_WT_Install to LFO1 via work_response
				const LV2_Atom* msg_ptr = reinterpret_cast<const LV2_Atom*>(&msg);
				if (msg.wts.first && msg.wts.second) respond(handle, lv2_atom_total_size(msg_ptr), msg_ptr);
			}
		}
		
		// SPF: Only set samples per frame and send whole worker_wt to LFO1 via work_response
		else if (property == urids.bvibratr_wavetable_spf)
		{
			worker_wt.set_samples_per_frame(patch.get_value_int(atom));
			const Atom_WT_Install msg = work_new_wavetable();
			const LV2_Atom* msg_ptr = reinterpret_cast<const LV2_Atom*>(&msg);
			if (msg.wts.first && msg.wts.second) respond(handle, lv2_atom_total_size(msg_ptr), msg_ptr);
		}
	}
	
	return LV2_WORKER_SUCCESS;
}

LV2_Worker_Status BVibratr::work_response (uint32_t size, const void* data)
{
	if (!data) return LV2_WORKER_ERR_UNKNOWN;
	const LV2_Atom* atom = reinterpret_cast<const LV2_Atom*>(data);
	if (!atom) return LV2_WORKER_ERR_UNKNOWN;

	// Wavetable install and schedule notify via CONTROL_OUT
	if (atom->type == urids.bvibratr_wavetable_install)
	{
		const Atom_WT_Install* wti_atom = reinterpret_cast<const Atom_WT_Install*>(atom);
		const Wavetable<>* wt1 = wti_atom->wts.first;
		const Wavetable<>* wt2 = wti_atom->wts.second;
		oscx3.osc1.set_wavetable_data(wt1, wt2);
		notify = true;
	}
	return LV2_WORKER_SUCCESS;
}

/*LV2_Worker_Status BVibratr::end_run ()
{}*/



static LV2_Handle instantiate (const LV2_Descriptor* descriptor, double samplerate, const char* bundle_path, const LV2_Feature* const* features)
{
	// New instance
	BVibratr* instance;
	try {instance = new BVibratr (samplerate, bundle_path, features);}
	catch (std::exception& exc)
	{
		fprintf (stderr, "Plugin instantiation failed. %s\n", exc.what ());
		return NULL;
	}

	return (LV2_Handle)instance;
}

static void connect_port (LV2_Handle instance, uint32_t port, void *data)
{
	BVibratr* inst = static_cast<BVibratr*>(instance);
	if (inst) inst->connect_port (port, data);
}

static void activate (LV2_Handle instance)
{
	BVibratr* inst = static_cast<BVibratr*>(instance);
	if (inst) inst->activate ();
}

static void run (LV2_Handle instance, uint32_t n_samples)
{
	BVibratr* inst = static_cast<BVibratr*>(instance);
	if (inst) inst->run (n_samples);
}

static void deactivate (LV2_Handle instance)
{
	BVibratr* inst = static_cast<BVibratr*>(instance);
	if (inst) inst->deactivate ();
}

static LV2_State_Status state_save (LV2_Handle instance, LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags,
           const LV2_Feature* const* features)
{
	BVibratr* inst = static_cast<BVibratr*>(instance);
	if (!inst) return LV2_STATE_SUCCESS;

	return inst->state_save (store, handle, flags, features);
}

static LV2_State_Status state_restore (LV2_Handle instance, LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags,
           const LV2_Feature* const* features)
{
	BVibratr* inst = static_cast<BVibratr*>(instance);
	if (!inst) return LV2_STATE_SUCCESS;

	return inst->state_restore (retrieve, handle, flags, features);
}

static LV2_Worker_Status work (LV2_Handle instance, LV2_Worker_Respond_Function respond, LV2_Worker_Respond_Handle handle,
	uint32_t size, const void* data)
{
	BVibratr* inst = static_cast<BVibratr*>(instance);
	if (!inst) return LV2_WORKER_SUCCESS;

	return inst->work (respond, handle, size, data);
}

static LV2_Worker_Status work_response (LV2_Handle instance, uint32_t size,  const void* data)
{
	BVibratr* inst = static_cast<BVibratr*>(instance);
	if (!inst) return LV2_WORKER_SUCCESS;

	return inst->work_response (size, data);
}

/*static LV2_Worker_Status end_run (LV2_Handle instance)
{
	BVibratr* inst = static_cast<BVibratr*>(instance);
	if (!inst) return LV2_WORKER_SUCCESS;

	return inst->end_run ();
}*/

static void cleanup (LV2_Handle instance)
{
	BVibratr* inst = static_cast<BVibratr*>(instance);
	if (inst) delete inst;
}

static const void* extension_data (const char* uri)
{
	// State
	static const LV2_State_Interface state  = {state_save, state_restore};
	if (!strcmp(uri, LV2_STATE__interface)) return &state;

	// Worker
	static const LV2_Worker_Interface worker = {work, work_response, nullptr};
	if (!strcmp(uri, LV2_WORKER__interface)) return &worker;

	return NULL;
}

static const LV2_Descriptor descriptor =
{
		BVIBRATR_URI,
		instantiate,
		connect_port,
		activate,
		run,
		deactivate,
		cleanup,
		extension_data
};

// LV2 Symbol Export
LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
	switch (index)
	{
		case 0:		return &descriptor;
		default:	return NULL;
	}
}
