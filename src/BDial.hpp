#ifndef BVIBRATOR_BDIAL_HPP_
#define BVIBRATOR_BDIAL_HPP_

#include "BWidgets/BWidgets/ValueDial.hpp"

class BDial : public BWidgets::ValueDial
{
public:
	BDial ();
	BDial (const uint32_t urid, const std::string& title);
	BDial (const double value, const double min, const double max, double step = 0.0, 
			   uint32_t urid = BUTILITIES_URID_UNKNOWN_URID, std::string title = "");
	BDial (const double x, const double y, const double width, const double height, 
			   const double value, const double min, const double max, double step = 0.0,
			   std::function<double (const double& x)> transferFunc = ValueTransferable<double>::noTransfer,
			   std::function<double (const double& x)> reTransferFunc = ValueTransferable<double>::noTransfer,
			   std::function<std::string (const double& x)> displayFunc = valueToString,
			   std::function<double (const std::string& s)> reDisplayFunc = stringToValue,
			   uint32_t urid = BUTILITIES_URID_UNKNOWN_URID, std::string title = "");
	
	virtual Widget* clone () const override;
	void copy (const BDial* that);
	virtual void update () override;

protected:
	virtual void drawArc (cairo_t* cr, const double xc, const double yc, const double radius,
                          const double min, const double max, 
                          const BStyles::Color fgColor, const BStyles::Color bgColor);
	virtual void draw () override;
    virtual void draw (const double x0, const double y0, const double width, const double height) override;
    virtual void draw (const BUtilities::Area<>& area) override;
};

inline BDial::BDial () :
	BDial	(
				 0.0, 0.0, BWIDGETS_DEFAULT_VALUEDIAL_WIDTH, BWIDGETS_DEFAULT_VALUEDIAL_HEIGHT, 
				 0.0, 0.0, 1.0, 0.0, 
				 ValueTransferable<double>::noTransfer, 
				 ValueTransferable<double>::noTransfer, 
				 valueToString,
				 stringToValue,
				 BUTILITIES_URID_UNKNOWN_URID, "")
{

}

inline BDial::BDial (const uint32_t urid, const std::string& title) : 
	BDial	(0.0, 0.0, BWIDGETS_DEFAULT_VALUEDIAL_WIDTH, BWIDGETS_DEFAULT_VALUEDIAL_HEIGHT, 
				 0.0, 0.0, 1.0, 0.0,  
				 ValueTransferable<double>::noTransfer, 
				 ValueTransferable<double>::noTransfer,
				 valueToString,
				 stringToValue, 
				 urid, title) 
{

}

inline BDial::BDial (const double value, const double min, const double max, double step, uint32_t urid, std::string title) : 
	BDial	(0.0, 0.0, BWIDGETS_DEFAULT_VALUEDIAL_WIDTH, BWIDGETS_DEFAULT_VALUEDIAL_HEIGHT, 
				 value, min, max, step, 
				 ValueTransferable<double>::noTransfer, 
				 ValueTransferable<double>::noTransfer,
				 valueToString,
				 stringToValue, 
				 urid, title) 
{

}

inline BDial::BDial	(const double  x, const double y, const double width, const double height, 
							 double value, const double min, const double max, double step, 
							 std::function<double (const double& x)> transferFunc,
				 			 std::function<double (const double& x)> reTransferFunc,
							 std::function<std::string (const double& x)> displayFunc,
							 std::function<double (const std::string& s)> reDisplayFunc,
							 uint32_t urid, std::string title) :
	ValueDial (x, y, width, height, value, min, max, step, transferFunc, reTransferFunc, displayFunc, reDisplayFunc, urid, title)
{
}

inline BWidgets::Widget* BDial::clone () const 
{
	Widget* f = new BDial (urid_, title_);
	f->copy (this);
	return f;
}

inline void BDial::copy (const BDial* that)
{
	ValueDial::copy (that);
}

inline void BDial::update ()
{
	ValueDial::update();
	label.moveTo(label.center(), label.middle());
}

inline void BDial::drawArc     (cairo_t* cr, const double xc, const double yc, const double radius,
                         const double min, const double max, 
                         const BStyles::Color fgColor, const BStyles::Color bgColor)
{
    cairo_set_line_width (cr, 0.0);
    
    // Arc
    cairo_set_source_rgba (cr, CAIRO_RGBA(bgColor));
    cairo_arc (cr, xc, yc, 0.96 * radius, BWIDGETS_DEFAULT_DRAWARC_START, BWIDGETS_DEFAULT_DRAWARC_END);
    cairo_arc_negative (cr, xc, yc, 0.70 * radius, BWIDGETS_DEFAULT_DRAWARC_END, BWIDGETS_DEFAULT_DRAWARC_START);
    cairo_close_path (cr);
    cairo_fill (cr);

    // Fill
	cairo_set_source_rgba (cr, CAIRO_RGBA(fgColor));
    cairo_arc (cr, xc, yc,  0.96 * radius - 0.2, BWIDGETS_DEFAULT_DRAWARC_START + min * BWIDGETS_DEFAULT_DRAWARC_SIZE, BWIDGETS_DEFAULT_DRAWARC_START + max * BWIDGETS_DEFAULT_DRAWARC_SIZE);
    cairo_arc_negative (cr, xc, yc,  0.7 * radius - 0.2, BWIDGETS_DEFAULT_DRAWARC_START + max * BWIDGETS_DEFAULT_DRAWARC_SIZE, BWIDGETS_DEFAULT_DRAWARC_START + min * BWIDGETS_DEFAULT_DRAWARC_SIZE);
    cairo_close_path (cr);
    cairo_fill (cr);
}

inline void BDial::draw ()
{
	draw (0, 0, getWidth(), getHeight());
}

inline void BDial::draw (const double x0, const double y0, const double width, const double height)
{
	draw (BUtilities::Area<> (x0, y0, width, height));
}

inline void BDial::draw (const BUtilities::Area<>& area)
{
	if ((!cairoSurface()) || (cairo_surface_status (cairoSurface()) != CAIRO_STATUS_SUCCESS)) return;

	// Draw super class widget elements first
	Widget::draw (area);

	// Draw only if minimum requirements satisfied
	if ((getHeight () >= 1) && (getWidth () >= 1))
	{
		cairo_t* cr = cairo_create (cairoSurface());

		if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
		{
			// Limit cairo-drawing area
			cairo_rectangle (cr, area.getX (), area.getY (), area.getWidth (), area.getHeight ());
			cairo_clip (cr);

			const double rval = getRatioFromValue (getValue());
			const double rad = 0.5 * (scale_.getWidth() < scale_.getHeight() ? scale_.getWidth() : scale_.getHeight());
			const BStyles::Color fgColor = getFgColors()[getStatus()];
			const BStyles::Color bgColor = getBgColors()[getStatus()];

			if (step_ >= 0.0)
			{
				drawArc (cr, getXOffset() + 0.5 * getEffectiveWidth(), getYOffset() + 0.5 * getEffectiveHeight(), rad - 1.0, 0.0, rval, fgColor, bgColor);
			}

			else
			{
				drawArc (cr, getXOffset() + 0.5 * getEffectiveWidth(), getYOffset() + 0.5 * getEffectiveHeight(), rad - 1.0, 1.0 - rval, 1.0, fgColor, bgColor);
			}
		}

		cairo_destroy (cr);
	}
}
#endif /* BVIBRATOR_BDIAL_HPP_ */
