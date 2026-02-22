#include "BVibratrGUI.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <initializer_list>
#include <lv2/atom/atom.h>
#include <lv2/atom/forge.h>
#include <new>
#include <string>
#include "BWidgets/BEvents/Event.hpp"
#include "BWidgets/BEvents/ExposeEvent.hpp"
#include "BWidgets/BEvents/ValueChangeTypedEvent.hpp"
#include "BWidgets/BUtilities/Dictionary.hpp"
#include "BWidgets/BWidgets/ComboBox.hpp"
#include "BWidgets/BWidgets/Label.hpp"
#include "BWidgets/BWidgets/Supports/ValueTransferable.hpp"
#include "BWidgets/BWidgets/Supports/ValueableTyped.hpp"
#include "BWidgets/BWidgets/Symbol.hpp"
#include "BWidgets/BWidgets/TextButton.hpp"
#include "BWidgets/BWidgets/Widget.hpp"
#include "MIDI_CC.hpp"
#include "Ports.hpp"
#include "ADSR.hpp"
#include "Oscx3.hpp"
#include "Limits.hpp"
#include "Wavetable.hpp"
#include "WavetableChooser.hpp"
#include "WavetableWidget.hpp"

#define BVIBRATR_GUI_WIDTH 960
#define BVIBRATR_GUI_HEIGHT 460

#define BVIBRATR_DEFAULT_WAVETABLE_FILE "inc/sine.wvt"


BVibratrGUI::BVibratrGUI (const char *bundle_path, const LV2_Feature *const *features, PuglNativeView parentWindow) :
	Window (BVIBRATR_GUI_WIDTH, BVIBRATR_GUI_HEIGHT, parentWindow, URID(), "B.Vibratr", true, PUGL_MODULE, 0),
	controller (NULL), 
	write_function (NULL),
	pluginPath (bundle_path ? std::string (bundle_path) : std::string ("")),
	map (nullptr),

	mContainer(0, 0, BVIBRATR_GUI_WIDTH, BVIBRATR_GUI_HEIGHT, pluginPath + "inc/surface.png", URID ("/bgimage")),
	//helpButton (918, 508, 24, 24, false, false, URID ("/halobutton"), BDICT ("Help")),
	//ytButton (948, 508, 24, 24, false, false, URID ("/halobutton"), BDICT ("Preview")),
	bypassLabel (820, 60, 60, 20, BDICT ("Bypass"), URID ("/ctlabel")),
	bypassButton (840, 30, 20, 20, 2, true, false, URID ("/redbutton"), BDICT ("Bypass")),
	drywetLabel (880, 60, 60, 20, BDICT ("Dry/wet"), URID ("/ctlabel")),
	drywetDial (885, 15, 50, 50, 1.0, 0.0, 1.0, 0.0, BNOTRANSFERD, BNOTRANSFERD, BDOUBLE_TO_STRING, BSTRING_TO_DOUBLE, URID ("/dial"), BDICT ("Dry/wet")),
	midiChannelLabel(510, 20, 200, 20, BDICT("Channel") + ":", URID("/label")),
	midiChannelWidget (0, 20, 0, 0, 0.1, 0, 0xFFFF, 1.0),
	midiNoteLabel (720, 20, 40, 20, BDICT("Note") + ":", URID("/label")),
	midiNoteCombobox(720, 40, 70, 20, {/* To be filled later */}, 1, URID("/menu")),
	midiNoteScreen (710, 10, 80, 60, URID("/screen")),
	depthIsCcLabel (20, 120, 100, 20, BDICT("Use") + " CC:", URID("/label")),
	depthIsCcCombobox(20, 140, 160, 20, midiCcNames, 1, URID("/menu")),
	depthLabel (50, 250, 100, 20, BDICT("Depth"), URID("/ctlabel")),
	depthDial (50, 160, 100, 100, 0.0, 0.0, 50.0, 0.0, BNOTRANSFERD, BNOTRANSFERD, BDOUBLE_TO_STRING, BSTRING_TO_DOUBLE, URID ("/dial"), BDICT ("Depth")),
	depthScreen (20, 160, 160, 120, URID("/screen")),
	attackSlider (40, 328, 120, 15, 0.1, 0.1, 4.0, 0.0, BNOTRANSFERD, BNOTRANSFERD, BDOUBLE_TO_STRING, BSTRING_TO_DOUBLE, URID ("/dial"), BDICT ("Attack")),
	decaySlider (40, 358, 120, 15, 0.1, 0.1, 4.0, 0.0, BNOTRANSFERD, BNOTRANSFERD, BDOUBLE_TO_STRING, BSTRING_TO_DOUBLE, URID ("/dial"), BDICT ("Decay")),
	sustainSlider (40, 388, 120, 15, 0.0, 0.0, 1.0, 0.0, BNOTRANSFERD, BNOTRANSFERD, BDOUBLE_TO_STRING, BSTRING_TO_DOUBLE, URID ("/dial"), BDICT ("Sustain")),
	releaseSlider (40, 418, 120, 15, 0.1, 0.1, 4.0, 0.0, BNOTRANSFERD, BNOTRANSFERD, BDOUBLE_TO_STRING, BSTRING_TO_DOUBLE, URID ("/dial"), BDICT ("Release")),
	osc1FreqLabel (250, 250, 100, 20, BDICT("Frequency"), URID("/ctlabel")),
	osc1FreqDial (250, 160, 100, 100, 1.0, 1.0, 20.0, 0.0, BNOTRANSFERD, BNOTRANSFERD, BDOUBLE_TO_STRING, BSTRING_TO_DOUBLE, URID ("/dial"), BDICT ("Frequency")),
	osc1ModeLabel (210, 120, 90, 20, BDICT("Mode"), URID("/label")),
	osc1ModeCombobox(210, 140, 90, 20, {BDICT("LFO"), BDICT("Wavetable")}, 1, URID("/menu")),
	osc1WaveformLabel (310, 120, 80, 20, BDICT("Waveform"), URID("/label")),
	osc1WaveformCombobox(310, 140, 80, 20, {BDICT("Sine"), BDICT("Triangle"), BDICT("Square"), BDICT("Saw")}, 1, URID("/menu")),
	loadWavetableButton(220, 250, 30, 20, BWidgets::Symbol::SymbolType::load, false, false, URID("/menu/button")),
	editWavetableButton(260, 250, 30, 20, BWidgets::Symbol::SymbolType::setup, false, false, URID("/menu/button")),
	wavetableChooser(nullptr),
	noWavetableLabel (0, 30, 90, 20, BDICT("No Data"), URID("/bglabel")),
	wavetableWidget(210, 165, 90, 80, Wavetable<>(), URID ("/dial")),
	osc2AmpLabel (420, 250, 80, 20, BDICT("Amplitude"), URID("/ctlabel")),
	osc2AmpDial (420, 170, 80, 80, 0.0, 0.0, 1.0, 0.0, BNOTRANSFERD, BNOTRANSFERD, BDOUBLE_TO_STRING, BSTRING_TO_DOUBLE, URID ("/dial"), BDICT ("Amplitude")),
	osc2FreqLabel (500, 250, 80, 20, BDICT("Frequency"), URID("/ctlabel")),
	osc2FreqDial (500, 170, 80, 80, 1.0, 1.0, 20.0, 0.0, BNOTRANSFERD, BNOTRANSFERD, BDOUBLE_TO_STRING, BSTRING_TO_DOUBLE, URID ("/dial"), BDICT ("Frequency")),
	osc2ModeLabel (410, 120, 90, 20, BDICT("Modulation"), URID("/label")),
	osc2ModeCombobox(410, 140, 90, 20, {BDICT("Off"), BDICT("Add"), "FM 1", "PM 1", "AM 1"}, 1, URID("/menu")),
	osc2WaveformLabel (510, 120, 80, 20, BDICT("Waveform"), URID("/label")),
	osc2WaveformCombobox(510, 140, 80, 20, {BDICT("Sine"), BDICT("Triangle"), BDICT("Square"), BDICT("Saw")}, 1, URID("/menu")),
	osc2Screen1 (420, 160, 160, 120, URID("/screen")),
	osc2Screen2 (505, 115, 90, 50, URID("/screen")),
	osc3AmpLabel (620, 250, 80, 20, BDICT("Amplitude"), URID("/ctlabel")),
	osc3AmpDial (620, 170, 80, 80, 0.0, 0.0, 1.0, 0.0, BNOTRANSFERD, BNOTRANSFERD, BDOUBLE_TO_STRING, BSTRING_TO_DOUBLE, URID ("/dial"), BDICT ("Amplitude")),
	osc3FreqLabel (700, 250, 80, 20, BDICT("Frequency"), URID("/ctlabel")),
	osc3FreqDial (700, 170, 80, 80, 1.0, 1.0, 20.0, 0.0, BNOTRANSFERD, BNOTRANSFERD, BDOUBLE_TO_STRING, BSTRING_TO_DOUBLE, URID ("/dial"), BDICT ("Frequency")),
	osc3ModeLabel (610, 120, 90, 20, BDICT("Modulation"), URID("/label")),
	osc3ModeCombobox(610, 140, 90, 20, {BDICT("Off"), BDICT("Add"), "FM 1", "PM 1", "AM 1", "FM 2", "PM 2", "AM 2"}, 1, URID("/menu")),
	osc3WaveformLabel (710, 120, 80, 20, BDICT("Waveform"), URID("/label")),
	osc3WaveformCombobox(710, 140, 80, 20, {BDICT("Sine"), BDICT("Triangle"), BDICT("Square"), BDICT("Saw")}, 1, URID("/menu")),
	osc3Screen1 (620, 160, 160, 120, URID("/screen")),
	osc3Screen2 (705, 115, 90, 50, URID("/screen")),
	tremoloLabel (840, 250, 80, 20, BDICT("Amount"), URID("/ctlabel")),
	tremoloDial (840, 170, 80, 80, 0.0, 0.0, 0.5, 0.0, BNOTRANSFERD, BNOTRANSFERD, BDOUBLE_TO_STRING, BSTRING_TO_DOUBLE, URID ("/dial"), BDICT ("Tremolo")),
	adsrDisplay (180, 320, 200, 120),
	waveformDisplay (420, 310, 520, 130)
	
{
	// Link controllers
	controllerWidgets[BVIBRATR_BYPASS] = &bypassButton;
	controllerWidgets[BVIBRATR_DRY_WET] = &drywetDial;
	controllerWidgets[BVIBRATR_MIDI_CHANNEL] = &midiChannelWidget;
	controllerWidgets[BVIBRATR_MIDI_NOTE] = &midiNoteCombobox;
	controllerWidgets[BVIBRATR_DEPTH_IS_CC] = &depthIsCcCombobox;
	controllerWidgets[BVIBRATR_DEPTH] = &depthDial;
	controllerWidgets[BVIBRATR_DEPTH_ATTACK] = &attackSlider;
	controllerWidgets[BVIBRATR_DEPTH_DECAY] = &decaySlider;
	controllerWidgets[BVIBRATR_DEPTH_SUSTAIN] = &sustainSlider;
	controllerWidgets[BVIBRATR_DEPTH_RELEASE] = &releaseSlider;
	controllerWidgets[BVIBRATR_OSC1_FREQ] = &osc1FreqDial;
	controllerWidgets[BVIBRATR_OSC1_MODE] = &osc1ModeCombobox;
	controllerWidgets[BVIBRATR_OSC1_WAVEFORM] = &osc1WaveformCombobox;
	controllerWidgets[BVIBRATR_OSC2_AMP] = &osc2AmpDial;
	controllerWidgets[BVIBRATR_OSC2_FREQ] = &osc2FreqDial;
	controllerWidgets[BVIBRATR_OSC2_MODE] = &osc2ModeCombobox;
	controllerWidgets[BVIBRATR_OSC2_WAVEFORM] = &osc2WaveformCombobox;
	controllerWidgets[BVIBRATR_OSC3_AMP] = &osc3AmpDial;
	controllerWidgets[BVIBRATR_OSC3_FREQ] = &osc3FreqDial;
	controllerWidgets[BVIBRATR_OSC3_MODE] = &osc3ModeCombobox;
	controllerWidgets[BVIBRATR_OSC3_WAVEFORM] = &osc3WaveformCombobox;
	controllerWidgets[BVIBRATR_TREMOLO] = &tremoloDial;

	// Configure widgets
	for (int i = 0; i < 16; ++i) midiChannelBoxes[i] = new BWidgets::TextButton(510 + (i % 8) * 25, 40 + int(i / 8) * 25, 20, 20, std::to_string(i + 1), true, false, URID("/button"));
	
	const std::array<const std::string, 12> keys {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
	for (int i = 0; i < 128; ++i) midiNoteCombobox.addItem(std::to_string(i) + " - " + keys[i % 12] + std::to_string(static_cast<int>(i / 12) - 1));
	midiNoteCombobox.addItem(BDICT("Any"));

	drywetDial.setClickable(false);
	drywetDial.setScrollable(true);
	drywetDial.label.hide();
	depthDial.setClickable(false);
	depthDial.setScrollable(true);
	attackSlider.setClickable(false);
	attackSlider.setScrollable(true);
	decaySlider.setClickable(false);
	decaySlider.setScrollable(true);
	sustainSlider.setClickable(false);
	sustainSlider.setScrollable(true);
	releaseSlider.setClickable(false);
	releaseSlider.setScrollable(true);
	osc1FreqDial.setClickable(false);
	osc1FreqDial.setScrollable(true);
	osc2AmpDial.setClickable(false);
	osc2AmpDial.setScrollable(true);
	osc2FreqDial.setClickable(false);
	osc2FreqDial.setScrollable(true);
	osc3AmpDial.setClickable(false);
	osc3AmpDial.setScrollable(true);
	osc3FreqDial.setClickable(false);
	osc3FreqDial.setScrollable(true);
	tremoloDial.setClickable(false);
	tremoloDial.setScrollable(true);

	midiChannelWidget.hide();
	midiNoteScreen.hide();
	depthScreen.hide();
	loadWavetableButton.hide();
	editWavetableButton.hide();
	wavetableWidget.hide();
	osc2Screen1.show();
	osc2Screen2.show();
	osc3Screen1.show();
	osc3Screen2.show();

	adsrDisplay.createImage(BStyles::Status::normal);
	waveformDisplay.createImage(BStyles::Status::normal);

	noWavetableLabel.show();

	setTheme(theme);

	// Set callbacks
	for (BWidgets::Widget* c : controllerWidgets) c->setCallbackFunction (BEvents::Event::EventType::valueChangedEvent, BVibratrGUI::valueChangedCallback);
	for (BWidgets::TextButton* m : midiChannelBoxes) m->setCallbackFunction (BEvents::Event::EventType::valueChangedEvent, BVibratrGUI::midiChannelsChangedCallback);
	loadWavetableButton.setCallbackFunction (BEvents::Event::EventType::buttonPressEvent, BVibratrGUI::loadWavetableClickedCallback);
	//helpButton.setCallbackFunction (BEvents::Event::EventType::buttonPressEvent, BVibratrGUI::helpButtonClickedCallback);
	//ytButton.setCallbackFunction (BEvents::Event::EventType::buttonPressEvent, BVibratrGUI::ytButtonClickedCallback);

	// Pack widgets
	wavetableWidget.add(&noWavetableLabel);
	for (BWidgets::Widget* c : controllerWidgets) mContainer.add(c);
	mContainer.add(&bypassLabel);
	mContainer.add(&drywetLabel);
	mContainer.add(&midiChannelLabel);
	for (BWidgets::TextButton* m : midiChannelBoxes) mContainer.add(m);
	mContainer.add(&midiNoteLabel);
	mContainer.add(&midiNoteScreen);
	mContainer.add(&depthIsCcLabel);
	mContainer.add(&depthLabel);
	mContainer.add(&depthScreen);
	mContainer.add(&osc1FreqLabel);
	mContainer.add(&osc1ModeLabel);
	mContainer.add(&osc1WaveformLabel);
	mContainer.add(&loadWavetableButton);
	mContainer.add(&editWavetableButton);
	mContainer.add(&wavetableWidget);
	mContainer.add(&osc2AmpLabel);
	mContainer.add(&osc2FreqLabel);
	mContainer.add(&osc2ModeLabel);
	mContainer.add(&osc2WaveformLabel);
	mContainer.add(&osc2Screen1);
	mContainer.add(&osc2Screen2);
	mContainer.add(&osc3AmpLabel);
	mContainer.add(&osc3FreqLabel);
	mContainer.add(&osc3ModeLabel);
	mContainer.add(&osc3WaveformLabel);
	mContainer.add(&osc3Screen1);
	mContainer.add(&osc3Screen2);
	mContainer.add(&tremoloLabel);
	mContainer.add (&adsrDisplay);
	mContainer.add (&waveformDisplay);
	add (&mContainer);

	//Init map, URID
	urids.init (features, &map);
	patch = Patch(map);
}

BVibratrGUI::~BVibratrGUI() 
{
	for (BWidgets::TextButton* m : midiChannelBoxes) delete m;
	if (wavetableChooser) delete wavetableChooser;
}

void BVibratrGUI::portEvent(uint32_t port_index, uint32_t buffer_size, uint32_t format, const void* buffer)
{
	// Incomming atom events from CONTROL_OUT
	if ((format == urids.atom_eventTransfer) && (port_index == BVIBRATR_CONTROL_OUT))
	{
		const LV2_Atom* atom = static_cast<const LV2_Atom*> (buffer);
		if 
		(
			(patch.get_message_type(atom) == urids.patch_Set) &&
			(patch.get_subject_type(atom) == urids.bvibratr)
		)
		{
			const LV2_Atom* val = patch.get_value_atom(atom);
			if ((patch.get_property_type(atom) == urids.bvibratr_wavetable_data) && val && (val->type == urids.atom_Vector))
			{
				const LV2_Atom_Vector* vec = reinterpret_cast<const LV2_Atom_Vector*>(val);
				const size_t n_elems = (vec->atom.size - sizeof(LV2_Atom_Vector_Body)) / sizeof(double);
				const double* values = reinterpret_cast<const double*>(LV2_ATOM_CONTENTS_CONST(LV2_Atom_Vector, vec));
				Wavetable<> wt;
				if (n_elems > 1) 
				{
					const size_t frame_sz = std::min(wt.get_samples_per_frame(), n_elems);
					wt.from_samples<double>(values, frame_sz, n_elems);
				}
				else wt.clear();
				wavetableWidget.setWavetable(wt);
				drawWaveform();
				if (wt.get_total_samples() > 1) noWavetableLabel.hide();
				else noWavetableLabel.show();
			}

			else if ((patch.get_property_type(atom) == urids.bvibratr_wavetable_spf) && val && (val->type == urids.atom_Int))
			{
				const LV2_Atom_Int* ival = reinterpret_cast<const LV2_Atom_Int*>(val);
				Wavetable<> wt = wavetableWidget.getWavetable();
				wt.set_samples_per_frame(ival->body);
				wavetableWidget.setWavetable(wt);
				drawWaveform();
				if (wt.get_total_samples() > 1) noWavetableLabel.hide();
				else noWavetableLabel.show();
			}
		}
	}

	// Scan controller ports
	if ((format == 0) && (port_index >= BVIBRATR_NR_PORTS) && (port_index < BVIBRATR_NR_PORTS + BVIBRATR_NR_CONTROLLERS))
	{
		const uint32_t idx = port_index - BVIBRATR_NR_PORTS;
		const float* pval = static_cast<const float*> (buffer);

		// Special case midi channels
		if (idx == BVIBRATR_MIDI_CHANNEL)
		{
			midiChannelWidget.setValue(*pval);
			const uint32_t ival = static_cast<uint32_t>(*pval);

			// And bitwise selection of each channel.
			for (int i = 0; i < 16; ++i) midiChannelBoxes[i]->setValue(ival & (1 << i));
		}

		// Comboboxes which require translation
		else if ((idx == BVIBRATR_MIDI_NOTE) || (idx == BVIBRATR_DEPTH_IS_CC))
		{
			BWidgets::ComboBox* combobox = dynamic_cast<BWidgets::ComboBox*>(controllerWidgets[idx]);
			if (combobox) combobox->setValue (*pval + 1);
		}

		// All other control ports
		else
		{
			// All other comboboxes
			BWidgets::ComboBox* combobox = dynamic_cast<BWidgets::ComboBox*>(controllerWidgets[idx]);
			if (combobox) combobox->setValue (*pval);

			// All other widgets
			else
			{
				BWidgets::ValueableTyped<double>* valueable_double = dynamic_cast<BWidgets::ValueableTyped<double>*>(controllerWidgets[idx]);
				if (valueable_double) valueable_double->setValue (*pval);

				else
				{
					BWidgets::ValueableTyped<bool>* valueable_bool = dynamic_cast<BWidgets::ValueableTyped<bool>*>(controllerWidgets[idx]);
					if (valueable_bool) valueable_bool->setValue (*pval);
				};
			}
		}
	}
}

void BVibratrGUI::onConfigureRequest (BEvents::Event* event)
{
	Window::onConfigureRequest (event);

	BEvents::ExposeEvent* ee = dynamic_cast<BEvents::ExposeEvent*>(event);
	if (!ee) return;
	const double sz =	(ee->getArea().getWidth() / BVIBRATR_GUI_WIDTH > ee->getArea().getHeight() / BVIBRATR_GUI_HEIGHT ? 
							ee->getArea().getHeight() / BVIBRATR_GUI_HEIGHT : 
							ee->getArea().getWidth() / BVIBRATR_GUI_WIDTH);
	setZoom (sz);
}

void BVibratrGUI::sendWavetablePath(const std::string path)
{
	if (path.empty()) return;
	if (path.length() > 924)
	{
		std::cerr << "File path \"" << path << "\" too long." << std::endl;
		return;
	}

	LV2_Atom_Forge forge;
	lv2_atom_forge_init(&forge, map);
	uint8_t obj_buf[1024];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));
	LV2_Atom_Forge_Ref msg = patch.write_patch_Set_Path(forge, urids.bvibratr, urids.bvibratr_wavetable_path, path.length() + 1, path.c_str());
	LV2_Atom* atom = reinterpret_cast<LV2_Atom*>(msg);
	if (msg) write_function(controller, BVIBRATR_CONTROL_IN, lv2_atom_total_size(atom), urids.atom_eventTransfer, atom);
}

void BVibratrGUI::sendPatchGet()
{
	LV2_Atom_Forge forge;
	lv2_atom_forge_init(&forge, map);
	uint8_t obj_buf[256];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));
	LV2_Atom_Forge_Ref msg = patch.write_patch_Get(forge, urids.bvibratr, urids.bvibratr_wavetable_data);
	LV2_Atom* atom = reinterpret_cast<LV2_Atom*>(msg);
	if (msg) write_function(controller, BVIBRATR_CONTROL_IN, lv2_atom_total_size(atom), urids.atom_eventTransfer, atom);
}

void BVibratrGUI::drawAdsr()
{
	cairo_surface_t* surface = adsrDisplay.getImageSurface(BStyles::Status::normal);
    cairoplus_surface_clear(surface);
    cairo_t* cr = cairo_create(surface);
    const double w = cairo_image_surface_get_width(surface) - 2;
    const double h = cairo_image_surface_get_height(surface) - 2;

	const double attack = attackSlider.getValue();
	const double decay = decaySlider.getValue();
	const double sustain = sustainSlider.getValue();
	const double release = releaseSlider.getValue();
	const double totalTime = attack + decay + 2.0 + release;
	const double sampleTime = totalTime / w;

	ADSR<double> adsr(attack, decay, sustain, release, ADSR<double>::INVSQR);
	adsr.start();

	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_set_line_width(cr, 2.0);
	cairo_move_to(cr, 1.0, h + 1.0);

	for (double x = 0; x < w; ++x)
	{
		if ((adsr.getPhase() == adsr.SUSTAIN) && (adsr.getPhaseTime() > 2.0)) adsr.release();
		adsr.run(sampleTime);
		cairo_line_to(cr, x + 1.0, h * (1.0 - adsr.get_value()) + 1.0);
	}

	cairo_line_to(cr, w + 1.0, h + 1.0);
	cairo_stroke_preserve (cr);

	cairo_close_path (cr);
	cairo_set_line_width (cr, 0.0);
	cairo_pattern_t* pat = cairo_pattern_create_linear (0, 0, 0, h);
	cairo_pattern_add_color_stop_rgba (pat, 1, 1.0, 1.0, 1.0, 0.1);
	cairo_pattern_add_color_stop_rgba (pat, 0, 1.0, 1.0, 1.0, 0.6 );
	cairo_set_source (cr, pat);
	cairo_fill (cr);
	cairo_pattern_destroy (pat);

    cairo_destroy (cr);
    adsrDisplay.update();

}

void BVibratrGUI::drawWaveform()
{
	Wavetable<>* wt1 = new (std::nothrow) Wavetable<>;
	if (!wt1) return;
	Wavetable<>* wt2 = new (std::nothrow) Wavetable<>;
	if (!wt2) 
	{
		delete wt1; 
		return;
	}

	cairo_surface_t* surface = waveformDisplay.getImageSurface(BStyles::Status::normal);
    cairoplus_surface_clear(surface);
    cairo_t* cr = cairo_create(surface);
    const double w = cairo_image_surface_get_width(surface);
    const double h = cairo_image_surface_get_height(surface);

	const double attack = attackSlider.getValue();
	const double decay = decaySlider.getValue();
	const double sustain = sustainSlider.getValue();
	const double release = releaseSlider.getValue();
	const double totalTime = attack + decay + 2.0 + release;

	ADSR<double> adsr(attack, decay, sustain, release, ADSR<double>::INVSQR);
	Oscx3<double> oscx3(w);
	
	oscx3.mode1 = BVIBRATR_OSC_MODE_LFO;
	oscx3.mode2 = static_cast<BVibratrOscModes>(osc2ModeCombobox.getValue());
	oscx3.mode3 = static_cast<BVibratrOscModes>(osc3ModeCombobox.getValue());

	oscx3.osc1.set_waveform((osc1ModeCombobox.getValue() == BVIBRATR_OSC_MODE_WAVETABLE) ?
							LFO<double>::WAVETABLE :
							static_cast<LFO<double>::Waveform>(osc1WaveformCombobox.getValue()));
	oscx3.osc2.set_waveform(static_cast<LFO<double>::Waveform>(osc2WaveformCombobox.getValue()));
	oscx3.osc3.set_waveform(static_cast<LFO<double>::Waveform>(osc3WaveformCombobox.getValue()));

	if (osc1ModeCombobox.getValue() == BVIBRATR_OSC_MODE_WAVETABLE) 
	{
		*wt1 = wavetableWidget.getWavetable();
		*wt2 = wavetableWidget.getWavetable();	// No need to integrate here
		oscx3.osc1.garbage_collector = [](const Wavetable<>* ptr){delete ptr; return true;};
		oscx3.osc1.set_wavetable_data(wt1, wt2);
	}

	oscx3.amp3 = osc3AmpDial.getValue();
	oscx3.freq3 = osc3FreqDial.getValue();
	oscx3.amp2 = osc2AmpDial.getValue();
	oscx3.freq2 = osc2FreqDial.getValue();
	oscx3.amp1 = 1.0;
	oscx3.freq1 = osc1FreqDial.getValue();

	adsr.start();
	oscx3.start();

	const double sampleTime = totalTime / w;		

	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_set_line_width(cr, 2.0);
	cairo_move_to(cr, 0.0, 0.5 * h);

	for (double x = 0; x < w; ++x)
	{
		oscx3.run(sampleTime);
		double signal = oscx3.get_value();

		// Apply adsr
		if ((adsr.getPhase() == adsr.SUSTAIN) && (adsr.getPhaseTime() > 2.0)) adsr.release();
		adsr.run(sampleTime);
		signal *= adsr.get_value();

		// Draw
		cairo_line_to(cr, x, 0.5 * h * (1.0 - signal));
	}

	cairo_stroke (cr);
    cairo_destroy (cr);
    waveformDisplay.update();
}

void BVibratrGUI::valueChangedCallback (BEvents::Event* event)
{
	
	if (!event) return;

	BWidgets::Widget* widget = event->getWidget ();
	if (!widget) return;

	BVibratrGUI* ui = dynamic_cast<BVibratrGUI*> (widget->getMainWindow());
	if (!ui) return;

	uint32_t idx = BVIBRATR_NR_CONTROLLERS;
	float value = nanf("");

	// Identify controller
	for (int i = 0; i < BVIBRATR_NR_CONTROLLERS; ++i)
	{
		if (widget == ui->controllerWidgets[i])
		{
			idx = i;
			break;
		}
	}

	// Controllers
	if (idx < BVIBRATR_NR_CONTROLLERS)
	{
		// Comboboxes which require translation
		if ((idx == BVIBRATR_MIDI_NOTE) || (idx == BVIBRATR_DEPTH_IS_CC)) 
		{
			BWidgets::ComboBox* combobox = dynamic_cast<BWidgets::ComboBox*>(widget);
			if (combobox)
			{
				const size_t combobox_idx = combobox->getValue();
				value = combobox_idx - 1;
			}
		}

		// All other control ports
		else
		{
			// All other comboboxes
			BWidgets::ComboBox* combobox = dynamic_cast<BWidgets::ComboBox*>(widget);
			if (combobox) 
			{
				const size_t combobox_idx = combobox->getValue();
				value = combobox_idx;
			}

			// All other widgets
			else
			{
				BWidgets::ValueableTyped<double>* valueable_double = dynamic_cast<BWidgets::ValueableTyped<double>*>(widget);
				if (valueable_double) value = valueable_double->getValue();

				else 
				{
					BWidgets::ValueableTyped<bool>* valueable_bool = dynamic_cast<BWidgets::ValueableTyped<bool>*>(widget);
					if (valueable_bool) value = valueable_bool->getValue();

					else return;
				}
			}
		} 

		// Update waveform (ADSR and oscillators)
		if ((idx >= BVIBRATR_DEPTH_ATTACK) && (idx <= BVIBRATR_DEPTH_RELEASE)) ui->drawAdsr();
		if ((idx >= BVIBRATR_DEPTH_ATTACK) && (idx <= BVIBRATR_OSC3_WAVEFORM)) ui->drawWaveform();

		if (idx == BVIBRATR_MIDI_CHANNEL)
		{
			if (ui->midiChannelWidget.getValue() == 0) 
			{
				ui->midiChannelLabel.setText(BDICT("Channel") + ": " + 
											 BDICT("None") + " - " + 
											 BDICT("Autoplay on"));
				ui->midiNoteScreen.show();
			}
			else 
			{
				ui->midiChannelLabel.setText(BDICT("Channel") + ":");
				ui->midiNoteScreen.hide();
			}
		}

		// Controllers which show / hide other controllers
		if (idx == BVIBRATR_DEPTH_IS_CC)
		{
			BWidgets::ComboBox* combobox = dynamic_cast<BWidgets::ComboBox*>(widget);
			if	(combobox && (combobox->getValue() == controller_limits[BVIBRATR_DEPTH_IS_CC].max + 1)) ui->depthScreen.hide();
			else ui->depthScreen.show();
		}

		else if (idx == BVIBRATR_OSC1_MODE)
		{
			BWidgets::ComboBox* combobox = dynamic_cast<BWidgets::ComboBox*>(widget);
			if	(combobox && (combobox->getValue() == 1)) 
			{
				ui->osc1WaveformCombobox.show();
				ui->osc1WaveformLabel.show();
				ui->osc1FreqDial.moveTo(250, 160);
				ui->osc1FreqDial.resize(100, 100);
				ui->osc1FreqLabel.moveTo(250, 250);
				ui->loadWavetableButton.hide();
				ui->editWavetableButton.hide();
				ui->wavetableWidget.hide();
			}
			else 
			{
				ui->osc1WaveformCombobox.hide();
				ui->osc1WaveformLabel.hide();
				ui->osc1FreqDial.moveTo(310, 170);
				ui->osc1FreqDial.resize(80, 80);
				ui->osc1FreqLabel.moveTo(300, 250);
				ui->loadWavetableButton.show();
				ui->editWavetableButton.show();
				ui->wavetableWidget.show();
			}
		}

		else if (idx == BVIBRATR_OSC2_MODE)
		{
			BWidgets::ComboBox* combobox = dynamic_cast<BWidgets::ComboBox*>(widget);
			if	(combobox && (combobox->getValue() == 1)) 
			{
				ui->osc2Screen1.show();
				ui->osc2Screen2.show();
			}
			else 
			{
				ui->osc2Screen1.hide();
				ui->osc2Screen2.hide();
			}
		}

		else if (idx == BVIBRATR_OSC3_MODE)
		{
			BWidgets::ComboBox* combobox = dynamic_cast<BWidgets::ComboBox*>(widget);
			if	(combobox && (combobox->getValue() == 1))
			{
				ui->osc3Screen1.show();
				ui->osc3Screen2.show();
			}
			else 
			{
				ui->osc3Screen1.hide();
				ui->osc3Screen2.hide();
			}
		}



		// Write to ports
		ui->write_function(ui->controller, BVIBRATR_NR_PORTS + idx, sizeof(float), 0, &value);
	}
	
}

void BVibratrGUI::midiChannelsChangedCallback (BEvents::Event* event)
{
	
	if (!event) return;

	BWidgets::Widget* widget = event->getWidget ();
	if (!widget) return;

	BWidgets::TextButton* textButton = dynamic_cast<BWidgets::TextButton*>(widget);
	if (!textButton) return;

	BVibratrGUI* ui = dynamic_cast<BVibratrGUI*> (widget->getMainWindow());
	if (!ui) return;

	// Identify widget
	for (int i = 0; i < 16; ++i)
	{
		if (textButton == ui->midiChannelBoxes[i])
		{
			const uint32_t ival = 1 << i;
			uint32_t mval = ui->midiChannelWidget.getValue();
			mval &= (~ival) & mval;								// Delete bit
			mval |= textButton->getValue() * ival;				// Set bit or not
			ui->midiChannelWidget.setValue(mval);
			break;
		}
	}
}

void BVibratrGUI::loadWavetableClickedCallback (BEvents::Event* event)
{
	if (!event) return;

	BWidgets::Widget* widget = event->getWidget ();
	if (!widget) return;

	BWidgets::SymbolButton* symbolButton = dynamic_cast<BWidgets::SymbolButton*>(widget);
	if (!symbolButton) return;

	BVibratrGUI* ui = dynamic_cast<BVibratrGUI*> (widget->getMainWindow());
	if (!ui) return;

	if (!ui->wavetableChooser)
	{
		ui->wavetableChooser = new WavetableChooser(URID("/menu"), "Wavetables");
		ui->wavetableChooser->confirmIfExists = false;
		ui->wavetableChooser->setCloseable(false);
		ui->wavetableChooser->setCallbackFunction(BEvents::Event::EventType::valueChangedEvent, BVibratrGUI::wavetableFileSelectedCallback);
		ui->wavetableChooser->moveTo(320, 110);
		ui->add(ui->wavetableChooser);
	}

	else 
	{
		delete ui->wavetableChooser;
		ui->wavetableChooser = nullptr;
	}
}

void BVibratrGUI::wavetableFileSelectedCallback (BEvents::Event* event)
{
	if (!event) return;

	BEvents::ValueChangeTypedEvent<std::string>* sev = dynamic_cast<BEvents::ValueChangeTypedEvent<std::string>*>(event);
	if (!sev) return;

	BWidgets::Widget* widget = sev->getWidget ();
	if (!widget) return;

	WavetableChooser* wtc = dynamic_cast<WavetableChooser*>(widget);
	if (!wtc) return;

	BVibratrGUI* ui = dynamic_cast<BVibratrGUI*> (widget->getMainWindow());
	if (!ui) return;

	if (ui->wavetableChooser)
	{
		if (sev->getValue() != "") ui->sendWavetablePath(sev->getValue());
		delete ui->wavetableChooser;
		ui->wavetableChooser = nullptr;
	}
}

/*
void BVibratrGUI::helpButtonClickedCallback (BEvents::Event* event)
{
	char cmd[] = WWW_BROWSER_CMD;
	char param[] = HELP_URL;
	char* argv[] = {cmd, param, NULL};
	std::cerr << "BAngr.lv2#GUI: Call " << HELP_URL << " for help.\n";
	if (BUtilities::vsystem (argv) == -1) std::cerr << "BAngr.lv2#GUI: Couldn't fork.\n";
}

void BVibratrGUI::ytButtonClickedCallback (BEvents::Event* event)
{
	char cmd[] = WWW_BROWSER_CMD;
	char param[] = YT_URL;
	char* argv[] = {cmd, param, NULL};
	std::cerr << "BAngr.lv2#GUI: Call " << YT_URL << " for preview video.\n";
	if (BUtilities::vsystem (argv) == -1) std::cerr << "BAngr.lv2#GUI: Couldn't fork.\n";
}
*/

static LV2UI_Handle instantiate (const LV2UI_Descriptor *descriptor, const char *plugin_uri, const char *bundle_path,
						  LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget *widget,
						  const LV2_Feature *const *features)
{
	PuglNativeView parentWindow = 0;

	if (strcmp(plugin_uri, BVIBRATR_URI) != 0)
	{
		std::cerr << "BVibratr.lv2: GUI does not support plugin with URI " << plugin_uri << std::endl;
		return NULL;
	}

	for (int i = 0; features[i]; ++i)
	{
		if (!strcmp(features[i]->URI, LV2_UI__parent)) parentWindow = reinterpret_cast<PuglNativeView> (features[i]->data);
	}
	if (parentWindow == 0) std::cerr << "BVibratr.lv2#gui: No parent window.\n";

	// New instance
	BVibratrGUI* ui;
	try {ui = new BVibratrGUI (bundle_path, features, parentWindow);}
	catch (std::exception& exc)
	{
		std::cerr << "BVibratr.lv2#gui: Instantiation failed. " << exc.what () << std::endl;
		return NULL;
	}

	ui->controller = controller;
	ui->write_function = write_function;
	ui->sendPatchGet();
	*widget = (LV2UI_Widget) ui->getNativeView ();
	return (LV2UI_Handle) ui;
}

static void cleanup(LV2UI_Handle ui)
{
	BVibratrGUI* pluginGui = static_cast<BVibratrGUI*> (ui);
	if (pluginGui) delete pluginGui;
}

static void portEvent(LV2UI_Handle ui, uint32_t port_index, uint32_t buffer_size,
	uint32_t format, const void* buffer)
{
	BVibratrGUI* pluginGui = static_cast<BVibratrGUI*> (ui);
	if (pluginGui) pluginGui->portEvent(port_index, buffer_size, format, buffer);
}

static int callIdle (LV2UI_Handle ui)
{
	BVibratrGUI* pluginGui = static_cast<BVibratrGUI*> (ui);
	if (pluginGui) pluginGui->handleEvents ();
	return 0;
}

static const LV2UI_Idle_Interface idle = {callIdle};

static const void* extensionData(const char* uri)
{
	if (!strcmp(uri, LV2_UI__idleInterface)) return &idle;
	else return NULL;
}

static const LV2UI_Descriptor guiDescriptor = 
{
		BVIBRATR_GUI_URI,
		instantiate,
		cleanup,
		portEvent,
		extensionData
};

// LV2 Symbol Export
LV2_SYMBOL_EXPORT const LV2UI_Descriptor *lv2ui_descriptor(uint32_t index)
{
	switch (index) {
	case 0: return &guiDescriptor;
	default:return NULL;
    }
}
