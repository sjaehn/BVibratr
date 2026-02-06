#ifndef URIDS_HPP_
#define URIDS_HPP_

#include <cstddef>
#include <lv2/core/lv2.h>
#include <lv2/core/lv2_util.h>
#include <lv2/urid/urid.h>
#include <lv2/midi/midi.h>
#include <stdexcept>

struct BVibratrURIDs
{
	LV2_URID midi_MidiEvent;

	void init (const LV2_Feature* const* features, LV2_URID_Map* map);
};

inline void BVibratrURIDs::init (const LV2_Feature* const* features, LV2_URID_Map* m)
{
	// Get feature map
	const char* missing = lv2_features_query (features, LV2_URID__map, &m, true, NULL);
    if (missing) throw std::invalid_argument ("Feature map not provided by the host. Can't instantiate plugin.");

	// Map urids
    midi_MidiEvent = m->map(m->handle, LV2_MIDI__MidiEvent);
}

#endif /* URIDS_HPP_ */
