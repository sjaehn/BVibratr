#ifndef URIDS_HPP_
#define URIDS_HPP_

#include <cstddef>
#include <lv2/core/lv2.h>
#include <lv2/core/lv2_util.h>
#include <lv2/atom/atom.h>
#include <lv2/midi/midi.h>
#include "lv2/patch/patch.h"
#include <lv2/urid/urid.h>
#include <stdexcept>

#define BVIBRATR_URI "https://www.jahnichen.de/plugins/lv2/BVibratr"
#define BVIBRATR_GUI_URI "https://www.jahnichen.de/plugins/lv2/BVibratr#gui"

struct BVibratrURIDs
{
	LV2_URID atom_Blank;
	LV2_URID atom_Double;
	LV2_URID atom_Int;
	LV2_URID atom_Long;
	LV2_URID atom_Object;
	LV2_URID atom_Path;
	LV2_URID atom_URID;
	LV2_URID atom_Vector;
	LV2_URID atom_eventTransfer;

	LV2_URID midi_MidiEvent;
	
	LV2_URID patch_Get;
	LV2_URID patch_Set;
	LV2_URID patch_property;
	LV2_URID patch_subject;
	LV2_URID patch_value;

	LV2_URID bvibratr;						// This subject.
	LV2_URID bvibratr_garbage_collecor;		// Notification to a garbage collector, typically followed by a pointer to Wavetable<>.
	LV2_URID bvibratr_wavetable;			// Property associated with a wavetable object
	LV2_URID bvibratr_wavetable_data;		// Property associated with wavetable raw data
	LV2_URID bvibratr_wavetable_path;		// Property associated with a wavetable path to load from
	LV2_URID bvibratr_wavetable_install;	/* Notification to install a wavetable, typically contains a pair of pointers containg again
											   a pointer pointer to a Wavetable<> and a pointer to its integrated Wavetable<>.*/
	LV2_URID bvibratr_wavetable_spf;		// Property associated with wavetable samples per frame.

	void init (const LV2_Feature* const* features, const LV2_URID_Map* const* map);
};

inline void BVibratrURIDs::init (const LV2_Feature* const* features, const LV2_URID_Map* const* m)
{
	// Get feature map
	const char* missing = lv2_features_query (features, LV2_URID__map, m, true, NULL);
    if (missing) throw std::invalid_argument ("Feature map not provided by the host. Can't instantiate plugin.");

	// Map urids
	atom_Blank = (*m)->map((*m)->handle, LV2_ATOM__Blank);
	atom_Double = (*m)->map((*m)->handle, LV2_ATOM__Double);
	atom_Int = (*m)->map((*m)->handle, LV2_ATOM__Int);
	atom_Long = (*m)->map((*m)->handle, LV2_ATOM__Long);
	atom_Object = (*m)->map((*m)->handle, LV2_ATOM__Object);
	atom_Path = (*m)->map((*m)->handle, LV2_ATOM__Path);
	atom_URID = (*m)->map((*m)->handle, LV2_ATOM__URID);
	atom_Vector = (*m)->map((*m)->handle, LV2_ATOM__Vector);
	atom_eventTransfer = (*m)->map((*m)->handle, LV2_ATOM__eventTransfer);

    midi_MidiEvent = (*m)->map((*m)->handle, LV2_MIDI__MidiEvent);

	patch_Get = (*m)->map((*m)->handle, LV2_PATCH__Get);
	patch_Set = (*m)->map((*m)->handle, LV2_PATCH__Set);
	patch_property = (*m)->map((*m)->handle, LV2_PATCH__property);
	patch_subject = (*m)->map((*m)->handle, LV2_PATCH__subject);
	patch_value = (*m)->map((*m)->handle, LV2_PATCH__value);

	bvibratr = (*m)->map((*m)->handle, BVIBRATR_URI);
	bvibratr_garbage_collecor = (*m)->map((*m)->handle, BVIBRATR_URI "#garbage_collector");
	bvibratr_wavetable = (*m)->map((*m)->handle, BVIBRATR_URI "#wavetable");
	bvibratr_wavetable_data = (*m)->map((*m)->handle, BVIBRATR_URI "#wavetable_data");
	bvibratr_wavetable_path = (*m)->map((*m)->handle, BVIBRATR_URI "#wavetable_path");
	bvibratr_wavetable_install = (*m)->map((*m)->handle, BVIBRATR_URI "#wavetable_intstall");
	bvibratr_wavetable_spf = (*m)->map((*m)->handle, BVIBRATR_URI "#wavetable_spf");
}

#endif /* URIDS_HPP_ */
