#ifndef BWIDGETS_WAVETABLECHOOSER_HPP_
#define BWIDGETS_WAVETABLECHOOSER_HPP_

#include "BWidgets/BWidgets/EditLabel.hpp"
#include "BWidgets/FileChooser.hpp"
#include "Wavetable.hpp"
#include "WavetableWidget.hpp"
#include <cairo/cairo.h>
#include <cstddef>
#include <exception>
#include <new>
#include <string>

#ifndef BWIDGETS_DEFAULT_WAVETABLECHOOSER_WIDTH
#define BWIDGETS_DEFAULT_WAVETABLECHOOSER_WIDTH (BWIDGETS_DEFAULT_FILECHOOSER_WIDTH + 100)
#endif

#ifndef BWIDGETS_DEFAULT_WAVETABLECHOOSER_HEIGHT
#define BWIDGETS_DEFAULT_WAVETABLECHOOSER_HEIGHT BWIDGETS_DEFAULT_FILECHOOSER_HEIGHT
#endif

#ifndef BWIDGETS_DEFAULT_WAVETABLECHOOSER_WAVETABLEFILES_REGEX
#define BWIDGETS_DEFAULT_WAVETABLECHOOSER_WAVETABLEFILES_REGEX std::regex (".*\\.((wav)|(wt)|(wvt))$", std::regex_constants::icase)
#endif

/**
 *  @brief  Menu widget for selection of a file.
 *
 *  The %WavetableChooser is a widget based on FileChooser for the selection of 
 *  audio files and samples. It additionally shows the waveform of the
 *  selected audio file and allows to select a range as a Wavetable.
 */
class WavetableChooser : public BWidgets::FileChooser
{
public:
	WavetableWidget wavetable;
	BWidgets::Label totalSamplesLabel;
	BWidgets::Label totalFramesLabel;
	BWidgets::Label spfLabel;
	BWidgets::EditLabel spfEdit;
	BWidgets::Label noFileLabel;

	/**
	 *  @brief  Constructs a default WavetableChooser object.
	 * 
	 */
	WavetableChooser ();

	/**
	 *  @brief  Constructs a default WavetableChooser object.
	 *  @param URID		URID.
	 *  @param title	%Widget title.
	 */
	WavetableChooser (const uint32_t urid, const std::string& title);

	/**
	 *  @brief  Constructs a WavetableChooser object with default size
	 *  @param path		Wavetable path.
	 *  @param filters  Optional, initializer list with filename search
	 *  filters.
     *  @param urid		Optional, URID (default = BUTILITIES_URID_UNKNOWN_URID).
	 *  @param title	Optional, %WavetableChooser title.
	 */
	WavetableChooser(const std::string& path, std::initializer_list<Filter> filters = {},
					 uint32_t urid = BUTILITIES_URID_UNKNOWN_URID, std::string title = "");

	/**
	 *  @brief  Constructs a WavetableChooser object with default size
	 *  @param x  %WavetableChooser X origin coordinate.
	 *  @param y  %WavetableChooser Y origin coordinate.
	 *  @param width  %WavetableChooser width.
	 *  @param height  %WavetableChooser height.
	 *  @param path  Wavetable path.
	 *  @param filters  Optional, initializer list with filename search
	 *  filters.
     *  @param urid  Optional, URID (default = BUTILITIES_URID_UNKNOWN_URID).
	 *  @param title  Optional, %WavetableChooser title.
	 */
	WavetableChooser(const double x, const double y, const double width, const double height,
					 std::string path = ".", 
					 std::initializer_list<Filter> filters =	{Filter	{BUtilities::Dictionary::get ("All files"), 
					 													 std::regex (".*")},
					 											 Filter	{BUtilities::Dictionary::get ("Wavetables"), 
																  		 BWIDGETS_DEFAULT_WAVETABLECHOOSER_WAVETABLEFILES_REGEX}},
					 uint32_t urid = BUTILITIES_URID_UNKNOWN_URID, std::string title = "");


	/**
	 *  @brief  Creates a clone of the %WavetableChooser. 
	 *  @return  Pointer to the new %WavetableChooser.
	 *
	 *  Creates a clone of this widget by copying all properties. But NOT its
	 *  linkage.
	 *
	 *  Allocated heap memory needs to be freed using @c delete if the clone
	 *  in not needed anymore!
	 */
	virtual Widget* clone () const override; 

	/**
	 *  @brief  Copies from another %WavetableChooser. 
	 *  @param that  Other %WavetableChooser.
	 *
	 *  Copies all properties from another %WavetableChooser. But NOT its linkage.
	 */
	void copy (const WavetableChooser* that);

	/**
	 *  @brief  Sets the file name.
	 *  @param filename  File name.
	 */
	virtual void setFileName (const std::string& filename) override;

	/**
	 *  @brief	Sets the number of samples per frame.
	 *  @param spf	Number of samples per frame.
	 */
	virtual void setSamplesPerFrame (const size_t spf);

	/**
	 *  @brief  Gets the number of samples per frame.
	 *  @return	Number of samples per frame.
	 */
	size_t getSamplesPerFrame() const;

	/**
	 * @brief	Gets a copy of current wavetable.
	 * @return	Wavetable.
	 */
	Wavetable<> getWavetable() const;
	
	/**
     *  @brief  Optimizes the widget extends.
     *
	 *  Resizes the widget to include all direct children into the widget
	 *  area. Resizes the widget to its standard size if this widget doesn't 
	 *  have any additional children (execept the built-in child widgets).
	 */
	virtual void resize () override;

    /**
     *  @brief  Resizes the widget extends.
	 *  @param width  New widget width.
	 *  @param height  New widget height.
	 */
	virtual void resize (const double width, const double height) override;

    /**
	 *  @brief  Resizes the widget extends.
	 *  @param extends  New widget extends.
	 */
	virtual void resize (const BUtilities::Point<> extends) override;

	/**
     *  @brief  Method to be called following an object state change.
     */
	virtual void update () override;

protected:


	static void wfileListBoxClickedCallback (BEvents::Event* event);
	static void spfEnteredCallback (BEvents::Event* event);
	static void filenameEnteredCallback (BEvents::Event* event);
	virtual std::function<void (BEvents::Event*)> getFileListBoxClickedCallback() override;
};

inline WavetableChooser::WavetableChooser () : 
	WavetableChooser	(0.0, 0.0, BWIDGETS_DEFAULT_WAVETABLECHOOSER_WIDTH, BWIDGETS_DEFAULT_WAVETABLECHOOSER_HEIGHT, 
					 ".",	
					 {Filter {BUtilities::Dictionary::get ("All files"), std::regex (".*")},
					  Filter {BUtilities::Dictionary::get ("Wavetables"), BWIDGETS_DEFAULT_WAVETABLECHOOSER_WAVETABLEFILES_REGEX}},
					 BUTILITIES_URID_UNKNOWN_URID, "") 
{

}

inline WavetableChooser::WavetableChooser (const uint32_t urid, const std::string& title) :
	WavetableChooser	(0.0, 0.0, BWIDGETS_DEFAULT_WAVETABLECHOOSER_WIDTH, BWIDGETS_DEFAULT_WAVETABLECHOOSER_HEIGHT, 
					 ".",	
					 {Filter {BUtilities::Dictionary::get ("All files"), std::regex (".*")},
					  Filter {BUtilities::Dictionary::get ("Wavetables"), BWIDGETS_DEFAULT_WAVETABLECHOOSER_WAVETABLEFILES_REGEX}},
					 urid, title) 
{

}

inline WavetableChooser::WavetableChooser	(const std::string& path, std::initializer_list<Filter> filters,
									 uint32_t urid, std::string title) :
	WavetableChooser	(0.0, 0.0, BWIDGETS_DEFAULT_WAVETABLECHOOSER_WIDTH, BWIDGETS_DEFAULT_WAVETABLECHOOSER_HEIGHT, 
					 path, filters, urid, title)
{

}

inline WavetableChooser::WavetableChooser	(const double x, const double y, const double width, const double height,
									 std::string path, std::initializer_list<Filter> filters,
									 uint32_t urid, std::string title) :
	FileChooser (x, y, width, height, path, filters, urid, title),
	wavetable(),
	totalSamplesLabel(BUtilities::Dictionary::get ("Total samples") + ":", BUtilities::Urid::urid (BUtilities::Urid::uri (urid) + "/label")),
	totalFramesLabel(BUtilities::Dictionary::get ("Total frames") + ":", BUtilities::Urid::urid (BUtilities::Urid::uri (urid) + "/label")),
	spfLabel(BUtilities::Dictionary::get ("Samples per frame") + ":", BUtilities::Urid::urid (BUtilities::Urid::uri (urid) + "/label")),
	spfEdit("", BUtilities::Urid::urid (BUtilities::Urid::uri (urid) + "/label")),
	noFileLabel (BUtilities::Dictionary::get ("No wavetable file selected"), BUtilities::Urid::urid (BUtilities::Urid::uri (urid) + "/label"), "")
{
	fileListBox.setCallbackFunction (BEvents::Event::EventType::valueChangedEvent, wfileListBoxClickedCallback);
	spfEdit.setCallbackFunction (BEvents::Event::EventType::valueChangedEvent, spfEnteredCallback);
	fileNameBox.setCallbackFunction (BEvents::Event::EventType::valueChangedEvent, filenameEnteredCallback);

	wavetable.setBorder ({{BStyles::grey, 1.0}, 0.0, 3.0, 0.0});
	
	add (&wavetable);
	add (&totalSamplesLabel);
	add (&totalFramesLabel);
	add (&spfLabel);
	add (&spfEdit);
	add (&noFileLabel);
}

inline BWidgets::Widget* WavetableChooser::clone () const 
{
	Widget* f = new WavetableChooser (urid_, title_);
	f->copy (this);
	return f;
}

inline void WavetableChooser::copy (const WavetableChooser* that)
{
	wavetable.copy (&that->wavetable);
	totalSamplesLabel.copy (&that->totalSamplesLabel);
	totalFramesLabel.copy (&that->totalFramesLabel);
	spfLabel.copy (&that->spfLabel);
	spfEdit.copy (&that->spfEdit);
	noFileLabel.copy (&that->noFileLabel);

	FileChooser::copy (that);
}

inline void WavetableChooser::setFileName (const std::string& filename)
{
	if (filename != fileNameBox.getText())
	{
		FileChooser::setFileName (filename);
		std::string newPath = getPath() + "/" + filename;
		char buf[PATH_MAX];
		char *rp = realpath(newPath.c_str(), buf);
		Wavetable<>* wt = new (std::nothrow) Wavetable<>();
		if (!wt) return;	// Bad alloc: Skip

		try {wt->from_file(std::string(rp));}
		catch (std::exception& exc) 
		{
			wt->clear();
			noFileLabel.setText (BUtilities::Dictionary::get ("No preview"));
		}

		wavetable.setWavetable(*wt);
		spfEdit.setText(std::to_string(wavetable.getWavetable().get_samples_per_frame()));
		update();

		delete wt;
	}
}

inline void WavetableChooser::setSamplesPerFrame (const size_t spf)
{
	spfEdit.setValue(std::to_string(spf));
}

inline size_t WavetableChooser::getSamplesPerFrame () const 
{
	return wavetable.getWavetable().get_samples_per_frame();
}

inline void WavetableChooser::resize ()
{
	// Resize to default size first
	resize (BWIDGETS_DEFAULT_WAVETABLECHOOSER_WIDTH, BWIDGETS_DEFAULT_WAVETABLECHOOSER_HEIGHT);

	// Resize to fit all children widgets
	BUtilities::Area<> a = BUtilities::Area<>();
	for (Linkable* c : children_)
	{
		Widget* w = dynamic_cast<Widget*>(c);
		if (w) a.extend (BUtilities::Area<>(w->getPosition(), w->getPosition() + w->getExtends()));
	}

	resize (a.getExtends() + BUtilities::Point<> (BWIDGETS_DEFAULT_MENU_PADDING + getXOffset(), BWIDGETS_DEFAULT_MENU_PADDING + getYOffset()));
}

inline void WavetableChooser::resize (const double width, const double height) 
{
	resize (BUtilities::Point<> (width, height));
}

inline void WavetableChooser::resize (const BUtilities::Point<> extends) 
{
	Widget::resize (extends);
}

inline void WavetableChooser::update ()
{
	setBackground (BStyles::Fill(getBgColors()[getStatus()].illuminate (-0.75)));
	setBorder (BStyles::Border  (BStyles::Line (getBgColors()[getStatus()].illuminate (BStyles::Color::highLighted), 1.0), 0.0, 0.0));

	const double x0 = getXOffset();
	const double y0 = getYOffset();
	const double w = getEffectiveWidth();
	const double h = getEffectiveHeight();

	if ((w >= 40) && (h >= 20))
	{
		const size_t val = fileListBox.getValue();
		if ((val == 0) || (val > dirs_.size())) okButton.label.setText (BUtilities::Dictionary::get ("OK"));
		else okButton.label.setText (BUtilities::Dictionary::get ("Open"));
		//cancelButton.label.setText(BUtilities::Dictionary::get ("Cancel"));
		//loopLabel.setText(labels[BWIDGETS_DEFAULT_WAVETABLECHOOSER_PLAY_AS_LOOP_INDEX]);

		// Get extends first
		okButton.resize();
		cancelButton.resize ();
		const double okWidth = (okButton.getWidth() > cancelButton.getWidth() ? okButton.getWidth() : cancelButton.getWidth()) + 4;
		const double okHeight = (okButton.getHeight() > cancelButton.getHeight() ? okButton.getHeight() : cancelButton.getHeight()) + 4;
		pathNameBox.resize();
		const double pathNameHeight = pathNameBox.getHeight();
		fileNameBox.resize();
		const double fileNameHeight = fileNameBox.getHeight();
		fileNameLabel.resize();
		const double fileNameWidth = fileNameLabel.getWidth();

		pathNameBox.moveTo (x0 + 10, y0 + 10);
		pathNameBox.resize (w - pathNameHeight - 30, pathNameHeight);

		newFolderButton.moveTo (x0 + w - 12 - pathNameHeight, y0 + 8);
		newFolderButton.resize (pathNameHeight + 4, pathNameHeight + 4);

		okButton.moveTo (x0 + w - okWidth - 10, y0 + h - okHeight - 10);
		okButton.resize (okWidth, okHeight);

		cancelButton.moveTo (x0 + w - 2 * okWidth - 20, y0 + h - okHeight - 10);
		cancelButton.resize (okWidth, okHeight);

		fileNameLabel.moveTo (x0 + 10, y0 + h - okHeight - fileNameHeight - 20);
		fileNameLabel.resize (fileNameWidth, fileNameHeight);

		fileNameBox.moveTo (x0 + fileNameWidth + 30, y0 + h - okHeight - fileNameHeight - 20);
		fileNameBox.resize (w - fileNameWidth - 40, fileNameHeight);

		filterComboBox.moveTo (x0 + 10, y0 + h - okHeight - 10);
		filterComboBox.resize (w - 2 * okWidth - 40, okHeight);
		filterComboBox.setItemHeight (okHeight);
		filterComboBox.resizeItems();
		filterComboBox.resizeListBox (BUtilities::Point<> (w - 2 * okWidth - 40, filters_.size() * okHeight + 20));

		okButton.show();
		cancelButton.show();
		fileNameLabel.show();
		fileNameBox.show();
		filterComboBox.show ();

		if (h > pathNameHeight + okHeight + fileNameHeight + 60)
		{
			const double fileListBoxHeight = h - pathNameHeight - okHeight - fileNameHeight - 50;

			fileListBox.moveTo (x0 + 10, y0 + pathNameHeight + 20);
			fileListBox.resize (0.4 * w - 15, fileListBoxHeight);
			fileListBox.setItemHeight (20);
			fileListBox.resizeItems();
			fileListBox.show();

			double waveformHeight = fileListBoxHeight;

			if (wavetable.getWavetable().size() > 1)
			{
				totalSamplesLabel.setText	
				(
					BUtilities::Dictionary::get ("Total samples") + ": " + 
					std::to_string(wavetable.getWavetable().get_total_samples()) + " " + 
					BUtilities::Dictionary::get ("from") + " " + 
					std::to_string(wavetable.getWavetable().size())
				);
				totalSamplesLabel.resize();
				const double totalSamplesHeight = totalSamplesLabel.getHeight();
				totalFramesLabel.setText(BUtilities::Dictionary::get ("Total frames") + ": " + std::to_string(wavetable.getWavetable().get_total_frames()));
				totalFramesLabel.resize();
				const double totalFramesHeight = totalFramesLabel.getHeight();
				spfLabel.resize();
				const double spfHeight = spfLabel.getHeight();
				const double spfWidth = spfLabel.getWidth();
				spfEdit.resize(BWIDGETS_DEFAULT_EDITLABEL_WIDTH, BWIDGETS_DEFAULT_EDITLABEL_HEIGHT);

				if (fileListBoxHeight > totalSamplesHeight + totalFramesHeight + spfHeight + 50.0)
				{
					waveformHeight = fileListBoxHeight - totalSamplesHeight - totalFramesHeight - spfHeight - 10.0;
					totalSamplesLabel.moveTo (x0 + 0.4 * w + 5, y0 + pathNameHeight + 20.0 + waveformHeight + 10.0);
					totalFramesLabel.moveTo (x0 + 0.4 * w + 5, y0 + pathNameHeight + 20.0 + waveformHeight + 10.0 + totalSamplesHeight);
					spfLabel.moveTo (x0 + 0.4 * w + 5, y0 + pathNameHeight + 20.0 + waveformHeight + 10.0 + totalSamplesHeight + totalFramesHeight);
					spfEdit.moveTo(x0 + 0.4 * w + 5 + spfWidth + 10, spfLabel.getPosition().y);

					totalSamplesLabel.show();
					totalFramesLabel.show();
					spfLabel.show();
					spfEdit.show();
				}

				else
				{
					totalSamplesLabel.hide();
					totalFramesLabel.hide();
					spfLabel.hide();
					spfEdit.hide();
					noFileLabel.hide();
				}
			}

			else
			{
				totalSamplesLabel.hide();
				totalFramesLabel.hide();
				spfLabel.hide();
				spfEdit.hide();
			}

			wavetable.moveTo (x0 + 0.4 * w + 5, y0 + pathNameHeight + 20);
			wavetable.resize (0.6 * w - 15, waveformHeight);
			wavetable.show();

			if (wavetable.getWavetable().size() > 1)
			{
				noFileLabel.hide();
			}

			else
			{
				noFileLabel.resize ();
				noFileLabel.moveTo
				(
					x0 + 0.4 * w + 5 + 0.3 * w - 7.5 - 0.5 * noFileLabel.getWidth(),
					y0 + pathNameHeight + 20 + 0.5 * waveformHeight - 0.5 * noFileLabel.getHeight()
				);
				noFileLabel.show();
			}
		}
		else
		{
			fileListBox.hide();
			wavetable.hide();
			totalSamplesLabel.hide();
			totalFramesLabel.hide();
			spfLabel.hide();
			spfEdit.hide();
			noFileLabel.hide();
		}

		confirmBox.resize();
		confirmBox.moveTo (0.5 * getWidth() - 0.5 * confirmBox.getWidth(), 0.5 * getHeight() - 0.5 * confirmBox.getHeight());
		confirmBox.show();

		createLabel.resize();
		createInput.resize();
		createError.resize();
		const double createLabelsWidth = (createLabel.getWidth() > createError.getWidth() ? createLabel.getWidth() : createError.getWidth());
		const double createBoxWidth = (createLabelsWidth + 40 > 2 * okWidth + 60 ? createLabelsWidth + 40 : 2 * okWidth + 60);
		const double createBoxHeight = createLabel.getHeight() + createInput.getHeight() + createError.getHeight() + okHeight + 80;
		createBox.resize (createBoxWidth, createBoxHeight);
		createBox.moveTo (0.5 * getWidth() - 0.5 * createBoxWidth, 0.5 * getHeight() - 0.5 * createBoxHeight);
		createLabel.moveTo (20, 20);
		createInput.resize (createBoxWidth - 40, createInput.getHeight());
		createInput.moveTo (20, 30 + createLabel.getHeight());
		createError.moveTo (20, 40 + createLabel.getHeight() + createInput.getHeight());
		createBox.show();
	}

	else
	{
		okButton.hide();
		cancelButton.hide();
		fileListBox.hide();
		wavetable.hide();
		wavetable.hide();
		totalSamplesLabel.hide();
		totalFramesLabel.hide();
		spfLabel.hide();
		spfEdit.hide();
		noFileLabel.hide();
		fileNameLabel.hide();
		fileNameBox.hide();
		filterComboBox.hide ();
		confirmBox.hide();
		createBox.hide();
	}

	Widget::update();
}

inline void WavetableChooser::wfileListBoxClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	BWidgets::ListBox* w = dynamic_cast<BWidgets::ListBox*>(event->getWidget());
	if (!w) return;
	WavetableChooser* fc = dynamic_cast<WavetableChooser*>(w->getParent());
	if (!fc) return;

	const size_t val = w->getValue();

	if ((val != 0) && (!fc->fileNameBox.getEditMode()))
	{
		// Directory selected -> one click chdir
		if (val <= fc->dirs_.size())
		{
			fc->fileNameBox.setText ("");
			fc->wavetable.getWavetable().clear();
			BEvents::ValueChangeTypedEvent<bool> dummyEvent = BEvents::ValueChangeTypedEvent<bool> (&fc->okButton, true);
			fc->okButtonClickedCallback (&dummyEvent);
			//fc->noFileLabel.setText (fc->labels[BWIDGETS_DEFAULT_WAVETABLECHOOSER_NO_FILE_INDEX]);
		}

		// File selected
		else
		{
			BWidgets::Label* l = dynamic_cast<BWidgets::Label*>(w->getItem (val));
			if (l) fc->setFileName (l->getText());
		}

		fc->update();
	}
}

inline void WavetableChooser::spfEnteredCallback (BEvents::Event* event)
{
	if (!event) return;
	BWidgets::EditLabel* e = dynamic_cast<BWidgets::EditLabel*>(event->getWidget());
	if (!e) return;
	WavetableChooser* wc = dynamic_cast<WavetableChooser*>(e->getParent());
	if (!wc) return;

	e->update();
	const std::string s = e->getValue();
	size_t spf = 1;
	try {spf = std::stoul(s);}
	catch (std::exception& exc) 
	{
		std::cerr << exc.what() << std::endl;
		e->setValue(std::to_string(wc->wavetable.getWavetable().get_samples_per_frame()));
		return;
	}

	if ((spf >= 1) && (spf <= 0x10000))
	{
		if (wc->wavetable.getWavetable().get_samples_per_frame() != spf)
		{
			Wavetable wt = wc->wavetable.getWavetable();
			wt.set_samples_per_frame(spf);
			wc->wavetable.setWavetable(wt);
			wc->update();
		}
	}
	else e->setValue(std::to_string(wc->wavetable.getWavetable().get_samples_per_frame()));
}

inline void WavetableChooser::filenameEnteredCallback (BEvents::Event* event)
{
	if (!event) return;
	BWidgets::Label* l = dynamic_cast<BWidgets::Label*>(event->getWidget());
	if (!l) return;
	WavetableChooser* p = (WavetableChooser*)l->getParent();
	if (!p) return;

	const std::string s = l->getText();
	p->setFileName (s);
}

inline std::function<void (BEvents::Event*)> WavetableChooser::getFileListBoxClickedCallback()
{
	return wfileListBoxClickedCallback;
}

#endif /* BWIDGETS_WAVETABLECHOOSER_HPP_ */
