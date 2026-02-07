#ifndef BVIBRATRGUI_HPP_
#define BVIBRATRGUI_HPP_

#include <initializer_list>
#include <lv2/ui/ui.h>
#include <lv2/atom/atom.h>
#include <lv2/atom/forge.h>
#include <string>
#include "BWidgets/BWidgets/Draws/Oops/definitions.hpp"
#include "BWidgets/BStyles/Status.hpp"
#include "BWidgets/BStyles/Types/Border.hpp"
#include "BWidgets/BStyles/Types/Color.hpp"
#include "BWidgets/BWidgets/Widget.hpp"
#include "BWidgets/BWidgets/Window.hpp"
#include "BWidgets/BWidgets/Knob.hpp"
#include "BWidgets/BWidgets/Label.hpp"
#include "BWidgets/BWidgets/ComboBox.hpp"
#include "BWidgets/BWidgets/Image.hpp"
#include "BWidgets/BWidgets/TextButton.hpp"

#include "BDial.hpp"
#include "ValueHSlider.hpp"
#include "Ports.hpp"
#include "Urids.hpp"

#ifndef WWW_BROWSER_CMD
#define WWW_BROWSER_CMD "x-www-browser"
#endif

// #define XREGION_URL "http://www.airwindows.com/xregion/"
// #define HELP_URL "https://github.com/sjaehn/BAngr/blob/master/README.md"
// #define YT_URL "https://www.youtube.com/watch?v=-kWy_1UYazo"

#define BVIBRATR_URI "https://www.jahnichen.de/plugins/lv2/BVibratr"
#define BVIBRATR_GUI_URI "https://www.jahnichen.de/plugins/lv2/BVibratr#gui"
#define URID(x) (BURID(BVIBRATR_GUI_URI x))

class BVibratrGUI : public BWidgets::Window
{
public:
	BVibratrGUI (const char *bundle_path, const LV2_Feature *const *features, PuglNativeView parentWindow);
	~BVibratrGUI ();
	void portEvent (uint32_t port_index, uint32_t buffer_size, uint32_t format, const void *buffer);
	virtual void onConfigureRequest (BEvents::Event* event) override;

	LV2UI_Controller controller;
	LV2UI_Write_Function write_function;


private:
	void drawAdsr ();
	void drawWaveform ();
	static void valueChangedCallback (BEvents::Event* event);
	static void midiChannelsChangedCallback (BEvents::Event* event);
	//static void helpButtonClickedCallback (BEvents::Event* event);
	//static void ytButtonClickedCallback (BEvents::Event* event);

	const std::string pluginPath;

	BVibratrURIDs urids;
	LV2_URID_Map* map;
	// LV2_URID_Unmap* unmap;

	// Widgets
	BWidgets::Image mContainer;
	//BWidgets::Button helpButton;
	//BWidgets::Button ytButton;
	BWidgets::Label bypassLabel;
	BWidgets::Knob bypassButton;
	BWidgets::Label drywetLabel;
    BDial drywetDial;
	BWidgets::Label midiChannelLabel;
	BWidgets::HSlider midiChannelWidget;	// Dummy for handling midiChannelBoxes
	std::array<BWidgets::TextButton*, 16> midiChannelBoxes;
	BWidgets::Label midiNoteLabel;
	BWidgets::ComboBox midiNoteCombobox;
	BWidgets::Label depthIsCcLabel;
	BWidgets::ComboBox depthIsCcCombobox;
	BWidgets::Label depthLabel;
	BDial depthDial;
	BWidgets::Widget depthScreen;
	ValueHSlider attackSlider;
	ValueHSlider decaySlider;
	ValueHSlider sustainSlider;
	ValueHSlider releaseSlider;
	BWidgets::Label osc1FreqLabel;
	BDial osc1FreqDial;
	BWidgets::Label osc1ModeLabel;
	BWidgets::ComboBox osc1ModeCombobox;
	BWidgets::Label osc1WaveformLabel;
	BWidgets::ComboBox osc1WaveformCombobox;
	BWidgets::Label osc2AmpLabel;
	BDial osc2AmpDial;
	BWidgets::Label osc2FreqLabel;
	BDial osc2FreqDial;
	BWidgets::Label osc2ModeLabel;
	BWidgets::ComboBox osc2ModeCombobox;
	BWidgets::Label osc2WaveformLabel;
	BWidgets::ComboBox osc2WaveformCombobox;
	BWidgets::Widget osc2Screen1;
	BWidgets::Widget osc2Screen2;
	BWidgets::Label osc3AmpLabel;
	BDial osc3AmpDial;
	BWidgets::Label osc3FreqLabel;
	BDial osc3FreqDial;
	BWidgets::Label osc3ModeLabel;
	BWidgets::ComboBox osc3ModeCombobox;
	BWidgets::Label osc3WaveformLabel;
	BWidgets::ComboBox osc3WaveformCombobox;
	BWidgets::Widget osc3Screen1;
	BWidgets::Widget osc3Screen2;
	BWidgets::Label tremoloLabel;
	BDial tremoloDial;
	BWidgets::Image adsrDisplay;
	BWidgets::Image waveformDisplay;

	// Controllers
	std::array<BWidgets::Widget*, BVIBRATR_NR_CONTROLLERS> controllerWidgets;

	// Definition of styles
	BStyles::ColorMap fgColors = BStyles::ColorMap {{{1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.5, 0.5, 0.5, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BStyles::ColorMap txColors = BStyles::ColorMap {{{0.8, 0.8, 0.8, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.5, 0.5, 0.5, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BStyles::ColorMap bgColors = BStyles::ColorMap {{{0.2, 0.05, 0.1, 1.0}, {0.4, 0.2, 0.3, 1.0}, {0.1, 0.03, 0.05, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BStyles::ColorMap btColors = BStyles::ColorMap {{{0.0, 0.0, 0.0, 1.0}, {0.4, 0.2, 0.3, 1.0}, {0.0, 0.00, 0.00, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BStyles::ColorMap noColors = BStyles::ColorMap {{{0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0}}};

	BStyles::Border border = {{fgColors[BStyles::Status::normal], 1.0}, 0.0, 2.0, 0.0};
	BStyles::Border menuBorder = {{noColors[BStyles::Status::normal], 1.0}, 0.0, 0.0, 0.0};
	BStyles::Border menuBorder2 = {{bgColors[BStyles::Status::normal], 1.0}, 0.0, 0.0, 0.0};
	BStyles::Border btBorder = BStyles::Border (BStyles::Line (bgColors[BStyles::Status::normal].illuminate (BStyles::Color::darkened), 0.0), 0.0, 0.0, 3.0);
	BStyles::Border labelBorder = {BStyles::noLine, 4.0, 0.0, 0.0};
	BStyles::Fill widgetBg = BStyles::noFill;
	BStyles::Fill screenBg = BStyles::Fill (BStyles::Color ({0.5, 0.0, 0.2, 0.8}));
	BStyles::Font defaultFont =	BStyles::Font 
	(
		"Sans", 
		CAIRO_FONT_SLANT_NORMAL, 
		CAIRO_FONT_WEIGHT_NORMAL, 
		12.0,
		BStyles::Font::TextAlign::center, 
		BStyles::Font::TextVAlign::middle
	);

	BStyles::Font smFont =	BStyles::Font 
	(
		"Sans", 
		CAIRO_FONT_SLANT_NORMAL, 
		CAIRO_FONT_WEIGHT_NORMAL, 
		9.0,
		BStyles::Font::TextAlign::center, 
		BStyles::Font::TextVAlign::middle
	);

	BStyles::Font rFont =	BStyles::Font 
	(
		"Sans", 
		CAIRO_FONT_SLANT_NORMAL, 
		CAIRO_FONT_WEIGHT_NORMAL, 
		12.0,
		BStyles::Font::TextAlign::right, 
		BStyles::Font::TextVAlign::middle
	);

	BStyles::Font lFont =	BStyles::Font 
	(
		"Sans", 
		CAIRO_FONT_SLANT_NORMAL, 
		CAIRO_FONT_WEIGHT_NORMAL, 
		12.0,
		BStyles::Font::TextAlign::left, 
		BStyles::Font::TextVAlign::middle
	);

	BStyles::Theme theme = BStyles::Theme
	{
		// main
		{
			URID ("/main"),
			BStyles::Style 
			({
				{BURID(BSTYLES_STYLEPROPERTY_BACKGROUND_URI), BUtilities::makeAny<BStyles::Fill>(widgetBg)},
				{BURID(BSTYLES_STYLEPROPERTY_BORDER_URI), BUtilities::makeAny<BStyles::Border>(BStyles::noBorder)}
			})
		},

		// screen
		{
			URID ("/screen"), 
			BStyles::Style 
			({
				{BURID(BSTYLES_STYLEPROPERTY_BACKGROUND_URI), BUtilities::makeAny<BStyles::Fill>(screenBg)},
				{BURID(BSTYLES_STYLEPROPERTY_BORDER_URI), BUtilities::makeAny<BStyles::Border>(BStyles::noBorder)}
			})
		},

		// button
		{
			URID ("/button"), 
			BStyles::Style 
			({
				{BURID(BSTYLES_STYLEPROPERTY_BACKGROUND_URI), BUtilities::makeAny<BStyles::Fill>({BStyles::Fill(bgColors[BStyles::Status::normal])})},
				{BURID(BSTYLES_STYLEPROPERTY_BGCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(btColors)},
				{BURID(BSTYLES_STYLEPROPERTY_BORDER_URI), BUtilities::makeAny<BStyles::Border>(btBorder)}
			})
		},
		
		// button/label
		{
			URID ("/button/label"), 
			BStyles::Style
			({	
				{BURID(BSTYLES_STYLEPROPERTY_FONT_URI), BUtilities::makeAny<BStyles::Font>(defaultFont)},
				{BURID(BSTYLES_STYLEPROPERTY_TXCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(txColors)}
			})
		},

		// redbutton
		{
			URID ("/redbutton"), 
			BStyles::Style 
			({
				{BURID(BSTYLES_STYLEPROPERTY_BGCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(bgColors)},
				{BURID(BSTYLES_STYLEPROPERTY_FGCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(BStyles::reds)}
			})
		},

		// halobutton
		{
			URID ("/halobutton"), 
			BStyles::Style 
			({
				{BURID(BSTYLES_STYLEPROPERTY_BACKGROUND_URI), BUtilities::makeAny<BStyles::Fill>({BStyles::noFill})},
				{BURID(BSTYLES_STYLEPROPERTY_BGCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(noColors)},
				{BURID(BSTYLES_STYLEPROPERTY_BORDER_URI), BUtilities::makeAny<BStyles::Border>(BStyles::noBorder)}
			})
		},

		// label
		{
			URID ("/label"), 
			BStyles::Style
			({	
				{BURID(BSTYLES_STYLEPROPERTY_FONT_URI), BUtilities::makeAny<BStyles::Font>(lFont)},
				{BURID(BSTYLES_STYLEPROPERTY_TXCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(txColors)}
			})
		},

		// clabel
		{
			URID ("/ctlabel"), 
			BStyles::Style
			({	
				{BURID(BSTYLES_STYLEPROPERTY_FONT_URI), BUtilities::makeAny<BStyles::Font>(defaultFont)},
				{BURID(BSTYLES_STYLEPROPERTY_TXCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(txColors)}
			})
		},

		// rlabel
		{
			URID ("/rlabel"), 
			BStyles::Style
			({	
				{BURID(BSTYLES_STYLEPROPERTY_FONT_URI), BUtilities::makeAny<BStyles::Font>(rFont)},
				{BURID(BSTYLES_STYLEPROPERTY_TXCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(txColors)}
			})
		},

		// dial
		{
			URID ("/dial"), 
			BStyles::Style
			({	
				{BURID(BSTYLES_STYLEPROPERTY_BGCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(bgColors)},
				{BURID(BSTYLES_STYLEPROPERTY_FGCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(fgColors)},
				{URID ("/dial/label"), BUtilities::makeAny<BStyles::Style>
					(
						BStyles::Style
						{
							{BURID(BSTYLES_STYLEPROPERTY_FONT_URI), BUtilities::makeAny<BStyles::Font>(smFont)},
							{BURID(BSTYLES_STYLEPROPERTY_TXCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(txColors)}
						}
					)}
			})
		}

	};
};

#endif /* BVIBRATRGUI_HPP_ */