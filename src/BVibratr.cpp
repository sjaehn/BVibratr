#include "BVibratr.hpp"
#include "Ports.hpp"
#include "Limits.hpp"

// Utilities
#include <cstdint>
#include <cstdio>
#include <lv2/atom/util.h>
#include <lv2/midi/midi.h>

#define SQRT_12_2 (pow (2.0, 1.0 / 12.0))

BVibratr::BVibratr (double samplerate, const char* bundlePath, const LV2_Feature* const* features) :
	rate (samplerate),
	midi_in (nullptr),
	audio_in_1 (nullptr),
	audio_in_2 (nullptr),
	audio_out_1 (nullptr),
	audio_out_2 (nullptr),
	latency_port(nullptr),
	map (nullptr),
	adsr(0, 0, 1, 0, ADSR<double>::INVSQR),
	osc1(),
	osc2(),
	osc3(),
	note(0xFF),
	depth_cc (1.0),
	buffer_1(0x10000),
	buffer_2(0x10000),
	buffer_offset((SQRT_12_2 - 1.0) *	// Up to 1 semitone
					samplerate			// Up to 1 second phase length
				 ),						// TODO report latency
	osc1_mode(0),
	osc2_mode(0),
	osc3_mode(0),
	depth(0.0),
	shift(0.0, (SQRT_12_2 - 1.0)),	// Limit temporal shift to 1 semitone
	amp(1.0f, 0.001f),
	mix(0.0f, 0.001f)
{
	// Init controllers
	controller_ports.fill(nullptr);

	// Map urids
    urids.init (features, map);

	// Init buffers
	buffer_1.fill(0.0f);
	buffer_2.fill(0.0f);
}

BVibratr::~BVibratr () {}

void BVibratr::connect_port (uint32_t port, void *data)
{
	switch (port) 
	{
		case BVIBRATR_MIDI_IN:		midi_in = static_cast<LV2_Atom_Sequence*>(data);
									break;

		case BVIBRATR_AUDIO_IN_1:	audio_in_1 = static_cast<float*>(data);
									break;

		case BVIBRATR_AUDIO_IN_2:	audio_in_2 = static_cast<float*>(data);
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
	if ((!midi_in) || (!audio_in_1) || (!audio_in_2) || (!audio_out_1) || (!audio_out_2)) return;
	for (int i = 0; i < BVIBRATR_NR_CONTROLLERS; ++i) if (!controller_ports[i]) return;
	if (!latency_port) return;

	// Update controllers
	*(latency_port) = buffer_offset;
	for (int i = 0; i < BVIBRATR_NR_CONTROLLERS; ++i) 
	{
		const float value = controller_limits[i].validate(*controller_ports[i]);
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
				osc1_mode = value; // TODO Schedule change
				break;

			case BVIBRATR_OSC1_WAVEFORM:
				osc1.set_waveform(static_cast<LFO<double>::Waveform>(value));
				break;

			case BVIBRATR_OSC2_MODE:		
				osc2_mode = value; // TODO Schedule change
				break;

			case BVIBRATR_OSC2_WAVEFORM:
				osc2.set_waveform(static_cast<LFO<double>::Waveform>(value));
				break;

			case BVIBRATR_OSC3_MODE:		
				osc3_mode = value; // TODO Schedule change
				break;

			case BVIBRATR_OSC3_WAVEFORM:
				osc3.set_waveform(static_cast<LFO<double>::Waveform>(value));
				break;

			default:
				break;
		}
	}

	// Playback and MIDI
	uint32_t last_frame = 0;
    LV2_ATOM_SEQUENCE_FOREACH (midi_in, ev)
    {
        /* play frames until event */
        const uint32_t frame = ev->time.frames;
        play (last_frame, frame);
        last_frame = frame;

        if (ev->body.type == urids.midi_MidiEvent) on_midi(reinterpret_cast<const uint8_t*> (ev + 1));
    }

    /* play remaining frames */
    play (last_frame, n_samples);
}

void BVibratr::on_midi_note_on (const uint8_t channel, const uint8_t note, const uint8_t velocity)
{
	if (static_cast<uint16_t>(controllers[BVIBRATR_MIDI_CHANNEL]) & (1 << channel))
	{
		if ((controllers[BVIBRATR_MIDI_NOTE] == note) || (controllers[BVIBRATR_MIDI_NOTE] == 128))
		{
			adsr.set_parameters	(controllers[BVIBRATR_DEPTH_ATTACK],
								 controllers[BVIBRATR_DEPTH_DECAY],
								 controllers[BVIBRATR_DEPTH_SUSTAIN],
							 	 controllers[BVIBRATR_DEPTH_RELEASE]);

			osc1.set_waveform(static_cast<LFO<double>::Waveform>(controllers[BVIBRATR_OSC1_WAVEFORM]));
			osc1.set_frequency(controllers[BVIBRATR_OSC1_FREQ]);
			osc2.set_waveform(static_cast<LFO<double>::Waveform>(controllers[BVIBRATR_OSC2_WAVEFORM]));
			osc2.set_frequency(controllers[BVIBRATR_OSC2_FREQ]);
			osc3.set_waveform(static_cast<LFO<double>::Waveform>(controllers[BVIBRATR_OSC3_WAVEFORM]));
			osc3.set_frequency(controllers[BVIBRATR_OSC3_FREQ]);

			adsr.start();
			osc1.start();
			osc2.start();
			osc3.start();

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
				osc1.stop();
				osc2.stop();
				osc3.stop();
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

void BVibratr::play (uint32_t start, uint32_t end)
{
	const double sample_time = 1.0 / rate;

	// Oscillator settings
	const double osc1_freq = controllers[BVIBRATR_OSC1_FREQ];
	const double osc2_amp = controllers[BVIBRATR_OSC2_AMP];
	const double osc2_freq = controllers[BVIBRATR_OSC2_FREQ];
	const double osc3_amp = controllers[BVIBRATR_OSC3_AMP];
	const double osc3_freq = controllers[BVIBRATR_OSC3_FREQ];

	const double amp_f = 1.0 +	((controllers[BVIBRATR_OSC2_MODE] == BVIBRATR_OSC_MODE_ADD) ? osc2_amp : 0.0) +
								((controllers[BVIBRATR_OSC3_MODE] == BVIBRATR_OSC_MODE_ADD) ? osc3_amp : 0.0);

	for (uint32_t i = start; i < end; ++i)
	{
		double signal = 0.0;		// To be used for tremolo (amp)
		double integral = 0.0;		// To be used for vibrato (shift)

		// Modulators
		double osc1_freq_m = 1.0;	// Frequency multiplier, range [0.0, 2.0]
		double osc1_phase_d = 0.0;	// Phase delta, range [-1.0, 1.0]
		double osc1_amp_m = 1.0;	// Amplification multiplier, range [0.0, 1.0]

		double osc2_freq_m = 1.0;
		double osc2_phase_d = 0.0;
		double osc2_amp_m = 1.0;

		// Run osc3
		osc3.set_frequency(osc3_freq);
		osc3.run(sample_time);

		switch(osc3_mode)
		{
			case BVIBRATR_OSC_MODE_ADD:	
				signal += osc3_amp * osc3.get_value();
				integral += osc3_amp * osc3.get_integral() * rate / osc3_freq;
				break;

			case BVIBRATR_OSC_MODE_FM1:
				osc1_freq_m *= (1.0 - osc3_amp * osc3.get_value());
				break;

			case BVIBRATR_OSC_MODE_PM1:
				osc1_phase_d += osc3_amp * osc3.get_value();
				break;

			case BVIBRATR_OSC_MODE_AM1:
				osc1_amp_m *= (1.0 - 0.5 * osc3_amp * (1.0 + osc3.get_value()));
				break;

			case BVIBRATR_OSC_MODE_FM2:
				osc2_freq_m *= (1.0 - osc3_amp * osc3.get_value());
				break;

			case BVIBRATR_OSC_MODE_PM2:
				osc2_phase_d += osc3_amp * osc3.get_value();
				break;

			case BVIBRATR_OSC_MODE_AM2:
				osc2_amp_m *= (1.0 - 0.5 * osc3_amp * (1.0 + osc3.get_value()));
				break;

			default:
				break;
		}

		// Run osc2
		osc2.set_frequency(osc2_freq_m * osc2_freq);
		osc1.set_phase_shift(osc2_phase_d);
		osc2.run(sample_time);

		switch(osc2_mode)
		{
			case BVIBRATR_OSC_MODE_ADD:	
				signal += osc2_amp_m * osc2_amp * osc2.get_value();
				integral += osc2_amp_m * osc2_amp * osc2.get_integral() * rate / osc2_freq;
				break;

			case BVIBRATR_OSC_MODE_FM1:
				osc1_freq_m *= (1.0 - osc2_amp_m * osc2_amp * osc2.get_value());
				break;

			case BVIBRATR_OSC_MODE_PM1:
				osc1_phase_d += osc2_amp_m * osc2_amp * osc2.get_value();
				break;

			case BVIBRATR_OSC_MODE_AM1:
				osc1_amp_m *= (1.0 - 0.5 * osc2_amp_m * osc2_amp * (1.0 + osc2.get_value()));
				break;

			default:
				break;
		}

		// Run osc1
		if (osc1_mode == BVIBRATR_OSC_MODE_LFO)
		{
			osc1.set_frequency(osc1_freq_m * osc1_freq);
			osc1.set_phase_shift(osc1_phase_d);
			osc1.run(sample_time);
			signal += osc1_amp_m * osc1.get_value();
			integral += osc1_amp_m * osc1.get_integral() * rate / osc1_freq;
		}

		else /* BVIBRATR_OSC_MODE_USER */ 
		{}

		// Scale signal and integral to not exceed 1.0
		signal /= amp_f;
		integral /= amp_f;

		// Apply adsr
		adsr.run(sample_time);
		signal *= adsr.get_value();
		integral *= adsr.get_value();

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

/*LV2_State_Status BVibratr::state_save (LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags, const LV2_Feature* const* features)
{
	store (handle, urids.lv2plugin_example, &example, sizeof(example), urids.atom_Type_of_example, LV2_STATE_IS_POD);

	return LV2_STATE_SUCCESS;
}*/

/*LV2_State_Status BVibratr::state_restore (LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags, const LV2_Feature* const* features)
{
	size_t   size;
	uint32_t type;
	uint32_t valflags;

	// Restore sharedDataNr
	const void* exampleData = retrieve (handle, urids.lv2plugin_example, &size, &type, &valflags);
	if (exampleData && (type == urids.atom_Type_of_example))
	{
		
	}

	return LV2_STATE_SUCCESS;
}*/

/*LV2_Worker_Status BVibratr::work (LV2_Worker_Respond_Function respond, LV2_Worker_Respond_Handle handle, uint32_t size, const void* data)
{}*/

/*LV2_Worker_Status BVibratr::work_response (uint32_t size, const void* data)
{}*/

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

/*static LV2_State_Status state_save (LV2_Handle instance, LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags,
           const LV2_Feature* const* features)
{
	BVibratr* inst = static_cast<BVibratr*>(instance);
	if (!inst) return LV2_STATE_SUCCESS;

	return inst->state_save (store, handle, flags, features);
}*/

/*static LV2_State_Status state_restore (LV2_Handle instance, LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags,
           const LV2_Feature* const* features)
{
	BVibratr* inst = static_cast<BVibratr*>(instance);
	if (!inst) return LV2_STATE_SUCCESS;

	return inst->state_restore (retrieve, handle, flags, features);
}*/

/*static LV2_Worker_Status work (LV2_Handle instance, LV2_Worker_Respond_Function respond, LV2_Worker_Respond_Handle handle,
	uint32_t size, const void* data)
{
	BVibratr* inst = static_cast<BVibratr*>(instance);
	if (!inst) return LV2_WORKER_SUCCESS;

	return inst->work (respond, handle, size, data);
}*/

/*static LV2_Worker_Status work_response (LV2_Handle instance, uint32_t size,  const void* data)
{
	BVibratr* inst = static_cast<BVibratr*>(instance);
	if (!inst) return LV2_WORKER_SUCCESS;

	return inst->work_response (size, data);
}*/

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
	//static const LV2_State_Interface state  = {state_save, state_restore};
	//if (!strcmp(uri, LV2_STATE__interface)) return &state;

	// Worker
	//static const LV2_Worker_Interface worker = {work, work_response, end_run};
	//if (!strcmp(uri, LV2_WORKER__interface)) return &worker;

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
