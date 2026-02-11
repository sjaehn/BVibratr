#ifndef WAVETABLEWIDGET_HPP_
#define WAVETABLEWIDGET_HPP_

#include "BWidgets/Widget.hpp"
#include <cairo/cairo.h>
#include <cmath>
#include <cstddef>
#include "Wavetable.hpp"

#ifndef BWIDGETS_DEFAULT_WAVETABLE_WIDTH
#define BWIDGETS_DEFAULT_WAVETABLE_WIDTH 200.0
#endif

#ifndef BWIDGETS_DEFAULT_WAVETABLE_HEIGHT
#define BWIDGETS_DEFAULT_WAVETABLE_HEIGHT BWIDGETS_DEFAULT_WAVETABLE_WIDTH
#endif


class WavetableWidget : public BWidgets::Widget
{
protected:
	Wavetable<> wavetable_;
	int selection_;

public:

	/**
	 *  @brief  Constructs an empty default %WavetableWidget object.
	 */
	WavetableWidget ();

	/**
	 *  @brief  Constructs an empty default %WavetableWidget object.
	 *  @param URID  URID.
	 *  @param title  %Widget title.
	 */
	WavetableWidget (const uint32_t urid, const std::string& title);

	/**
	 *  @brief  Constructs a %WavetableWidget object at the origin with optimized extends.
	 *  @param wavetable  Wavetable data.
	 *  @param urid  Optional, URID (default = BUTILITIES_URID_UNKNOWN_URID).
	 *  @param title  Optional, %WavetableWidget title (default = "").
	 */
	WavetableWidget (const Wavetable<>& wavetable, uint32_t urid = BUTILITIES_URID_UNKNOWN_URID, std::string title = "");

	/**
	 *  @brief  Constructs a %WavetableWidget object at the origin with optimized extends.
	 *  @param x  %WavetableWidget X origin coordinate.
	 *  @param y  %WavetableWidget Y origin coordinate.
	 *  @param width  %WavetableWidget width.
	 *  @param height  %WavetableWidget height.
	 *  @param wavetable  Wavetable data.
	 *  @param urid  Optional, URID (default = BUTILITIES_URID_UNKNOWN_URID).
	 *  @param title  Optional, %WavetableWidget title (default = "").
	 */
	WavetableWidget	(const double x, const double y, const double width, const double height, 
			 const Wavetable<>& wavetable, uint32_t urid = BUTILITIES_URID_UNKNOWN_URID, std::string title = "");

	/**
	 *  @brief  Creates a clone of the %WavetableWidget. 
	 *  @return  Pointer to the new %WavetableWidget.
	 *
	 *  Creates a clone of this %WavetableWidget by copying all properties. But NOT its
	 *  linkage.
	 *
	 *  Allocated heap memory needs to be freed using @c delete if the clone
	 *  in not needed anymore!
	 */
	virtual Widget* clone () const override; 

	/**
	 *  @brief  Copies from another %WavetableWidget. 
	 *  @param that  Other %WavetableWidget.
	 *
	 *  Copies all properties from another %WavetableWidget. But NOT its linkage.
	 */
	void copy (const WavetableWidget* that);

	/**
	 *  @brief  Imports the wavetable data.
	 *  @param wavetable  Wavetable data.
	 */
	virtual void setWavetable (const Wavetable<>& wavetable);

	/**
	 *  @brief  Exports the wavetable data.
	 *  @return   Wavetable data.
	 */
	Wavetable<> getWavetable () const;

	void select(const size_t selection);

	void unselect();

protected:
	/**
     *  @brief  Unclipped draw to the surface (if is visualizable).
     */
    virtual void draw () override;

    /**
     *  @brief  Clipped Draw to the surface (if is visualizable).
     *  @param x0  X origin of the clipped area. 
     *  @param y0  Y origin of the clipped area. 
     *  @param width  Width of the clipped area.
     *  @param height  Height of the clipped area. 
     */
    virtual void draw (const double x0, const double y0, const double width, const double height) override;

    /**
     *  @brief  Clipped Draw to the surface (if is visualizable).
     *  @param area  Clipped area. 
     */
    virtual void draw (const BUtilities::Area<>& area) override;
};

inline WavetableWidget::WavetableWidget () : 
	WavetableWidget (0.0, 0.0, BWIDGETS_DEFAULT_WAVETABLE_WIDTH, BWIDGETS_DEFAULT_WAVETABLE_HEIGHT, Wavetable(), BUTILITIES_URID_UNKNOWN_URID, "") 
{

}

inline WavetableWidget::WavetableWidget (const uint32_t urid, const std::string& title) :
	WavetableWidget (0.0, 0.0, BWIDGETS_DEFAULT_WAVETABLE_WIDTH, BWIDGETS_DEFAULT_WAVETABLE_HEIGHT, Wavetable(), urid, title)
{
}

inline WavetableWidget::WavetableWidget (const Wavetable<>& wavetable, uint32_t urid, std::string title) :
	WavetableWidget (0.0, 0.0, BWIDGETS_DEFAULT_WAVETABLE_WIDTH, BWIDGETS_DEFAULT_WAVETABLE_HEIGHT, wavetable, urid, title)
{
}

inline WavetableWidget::WavetableWidget (const double x, const double y, const double width, const double height, const Wavetable<>& wavetable, uint32_t urid, std::string title) :
	Widget (x, y, width, height, urid, title),
	wavetable_ (wavetable),
	selection_(-1)
{
	
}

inline BWidgets::Widget* WavetableWidget::clone () const 
{
	Widget* f = new WavetableWidget (urid_, title_);
	f->copy (this);
	return f;
}

inline void WavetableWidget::copy (const WavetableWidget* that)
{
	wavetable_ = that->wavetable_;
	selection_ = that->selection_;
    Widget::copy (that);
}

inline void WavetableWidget::setWavetable (const Wavetable<>& wavetable)
{
	wavetable_ = wavetable;
	update ();
}

inline Wavetable<> WavetableWidget::getWavetable () const 
{
	return wavetable_;
}

inline void WavetableWidget::select(const size_t selection)
{
	selection_ = selection;
	update();
}

inline void WavetableWidget::unselect()
{
	selection_ = -1;
	update();
}

inline void WavetableWidget::draw ()
{
	draw (0, 0, getWidth(), getHeight());
}

inline void WavetableWidget::draw (const double x0, const double y0, const double width, const double height)
{
	draw (BUtilities::Area<> (x0, y0, width, height));
}

inline void WavetableWidget::draw (const BUtilities::Area<>& area)
{
	if ((!cairoSurface()) || (cairo_surface_status (cairoSurface()) != CAIRO_STATUS_SUCCESS)) return;

	// Draw super class widget elements first
	Widget::draw (area);

	const size_t dim_i = wavetable_.get_total_frames();
	const size_t dim_j = wavetable_.get_samples_per_frame();
	if ((wavetable_.get_total_samples() <= 1) || (dim_i < 1) || (dim_j < 1)) return;

	cairo_t* cr = cairo_create (cairoSurface());

	if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
	{
		// Limit cairo-drawing area
		cairo_rectangle (cr, area.getX (), area.getY (), area.getWidth (), area.getHeight ());
		cairo_clip (cr);

		const double xoffs = getXOffset ();
		const double yoffs = getYOffset ();
		const double weff = getEffectiveWidth ();
		const double heff = getEffectiveHeight ();

		
		const double d = 0.375 * heff;
		const double h = 0.125 * heff;
		const double w = weff - d;
		const double x0 = xoffs;
		const double y0 = yoffs + h;

		const double step_i = (dim_i < 5) ? 0.2 : 1.0 / dim_i;			// Distance between every wave, max. 0.2
		const double step_j = 1.0 / dim_j;								// Distance between every wave point
		const double ori_i = 0.5 - 0.5 * step_i * (dim_i - 1);			// Origin of the wave
		const double ori_j = 0.5 - 0.5 * step_j * (dim_j);				// Origin of the wave
		const double alpha = dim_i < h ? 0.667 : 0.667 * h / dim_i;		// Alpha channel

		// Draw table
		/*
		cairo_move_to(cr, x0 + ori_j * w + d, y0 - ori_i * d + h);
		cairo_line_to(cr, x0 + ori_j * w + d + w, y0 - ori_i * d + d + h);
		cairo_line_to(cr, x0 + ori_j * w + w, y0 - ori_i * d + 2 * d + h);
		cairo_line_to(cr, x0 + ori_j * w, y0 - ori_i * d + d + h);
		cairo_close_path(cr);
		cairo_set_source_rgba (cr, CAIRO_RGBA (getBgColors()[getStatus()]));
		cairo_fill(cr);
		*/
		
		// Draw waves
		cairo_set_source_rgba (cr, CAIRO_RGB (getFgColors()[getStatus()]), alpha);

		for (double i = dim_i - 1; i >= 0; --i)
		{
			const double x0_i = x0 + ori_j * w + d * i / static_cast<double>(dim_i);	// ... translated to x coords
			const double y0_i = y0 + (1.0 - (ori_i + i * step_i)) * d;					// ... and y coords

			cairo_move_to(cr, x0_i, y0_i - wavetable_.at_rel(i) * h);
			for (double j = 0.0; j < 1.0; j += step_j) 
			{
				cairo_line_to	(cr, 
								 x0_i + j * w, 
								 y0_i - wavetable_.at_rel(i + j) * h + j * d);
			}
			cairo_line_to(cr, x0_i + w, y0_i - wavetable_.at_rel(i + 1) * h + d);
			cairo_set_line_width(cr, (selection_ == i) ? 2.0 : 1.0);
			cairo_stroke(cr);
		}

	cairo_destroy (cr);
}

}

#endif /* WAVETABLEWIDGET_HPP_ */
