#ifndef BVIBRATR_VALUEHSLIDER_HPP_
#define BVIBRATR_VALUEHSLIDER_HPP_

#include "BWidgets/BWidgets/ValueHSlider.hpp"


/**
 *  @brief  %ValueHSlider widget.
 *
 *  %ValueHSlider is a HSlider Widget with an additional editable label for
 *  displaying its value. 
 */
class ValueHSlider : public BWidgets::ValueHSlider
{
public:

	/**
	 *  @brief  Constructs a default %ValueHSlider object.
	 */
	ValueHSlider () : BWidgets::ValueHSlider () {}

	/**
	 *  @brief  Constructs a default %ValueHSlider object.
	 *  @param URID  URID.
	 *  @param title  %Widget title.
	 */
	ValueHSlider (const uint32_t urid, const std::string& title) : 
		BWidgets::ValueHSlider (urid, title) {}

	/**
	 *  @brief  Creates a %ValueHSlider with default size.
	 *  @param value  Initial value.
	 *  @param min  Lower value limit.
	 *  @param max  Upper value limit.
	 *  @param step  Optional, value increment steps.
	 *  @param urid  Optional, URID (default = BUTILITIES_URID_UNKNOWN_URID).
	 *  @param title  Optional, %Widget title (default = "").
	 */
	ValueHSlider	(const double value, const double min, const double max, double step = 0.0, 
					 uint32_t urid = BUTILITIES_URID_UNKNOWN_URID, std::string title = "") :
		BWidgets::ValueHSlider	(value, min, max, step, urid, title) {}

	/**
	 *  @brief  Creates a %ValueHSlider.
	 *  @param x  %ValueHSlider X origin coordinate.
	 *  @param y  %ValueHSlider Y origin coordinate.
	 *  @param width  %ValueHSlider width.
	 *  @param height  %ValueHSlider height.
	 *  @param value  Initial value.
	 *  @param min  Lower value limit.
	 *  @param max  Upper value limit.
	 *  @param step  Optional, value increment steps.
	 *  @param transferFunc  Optinonal, function to transfer a value from an
	 *  external context to the internal context.
	 *  @param reTransferFunc  Optinonal, function to transfer a value from the
	 *  internal context to an external context.
	 *  @param displayFunc  Optional, function to convert the value to a string
	 *  which will be displayed as a label.
	 *  @param reDisplayFunc  Optional, function to convert the string from
	 *  the (edited) label to the value.
	 *  @param urid  Optional, URID (default = BUTILITIES_URID_UNKNOWN_URID).
	 *  @param title  Optional, %ValueHSlider title (default = "").
	 *
	 *  The optional parameters @a displayFunc and @a reDisplayFunc can be used
	 *  as powerful tools to visualize the value in any way (including units,
	 *  prefixes, postfixes, text substitution, ...) and to parse it. By 
	 *  default, %ValueHSlider displays the value via @c valueToString() and
	 *  thus shows a decimal floating point representation with up to 3 digits
	 *  after the point.
	 */
	ValueHSlider	(const double x, const double y, const double width, const double height, 
					 const double value, const double min, const double max, double step = 0.0,
					 std::function<double (const double& x)> transferFunc = ValueTransferable<double>::noTransfer,
					 std::function<double (const double& x)> reTransferFunc = ValueTransferable<double>::noTransfer,
					 std::function<std::string (const double& x)> displayFunc = valueToString,
					 std::function<double (const std::string& s)> reDisplayFunc = stringToValue,
					 uint32_t urid = BUTILITIES_URID_UNKNOWN_URID, std::string title = "") :
		BWidgets::ValueHSlider	(x, y, width, height, value, min, max, step, 
								 transferFunc, reTransferFunc, displayFunc, reDisplayFunc, 
								 urid, title) {}

	/**
	 *  @brief  Creates a clone of the %ValueHSlider. 
	 *  @return  Pointer to the new %ValueHSlider.
	 *
	 *  Creates a clone of this %ValueHSlider by copying all properties. But NOT its
	 *  linkage.
	 *
	 *  Allocated heap memory needs to be freed using @c delete if the clone
	 *  in not needed anymore!
	 */
	virtual BWidgets::Widget* clone () const override; 

	/**
     *  @brief  Method to be called following an object state change.
     */
    virtual void update () override;
};

inline BWidgets::Widget* ValueHSlider::clone () const 
{
	Widget* f = new ValueHSlider (urid_, title_);
	f->copy (this);
	return f;
}

inline void ValueHSlider::update ()
{
	BWidgets::Label* f = dynamic_cast<BWidgets::Label*>(focus_);
	if (f)
	{
		f->setText(getTitle() + ": " + std::to_string (this->getValue()));
		f->resize();
	}

	scale_ = BUtilities::Area<> 
	(
		getXOffset() + 0.5 * getEffectiveHeight(), 
		getYOffset() + 0.05 * getEffectiveHeight(), 
		getEffectiveWidth() - getEffectiveHeight(), 
		0.9 * getEffectiveHeight()
	);

	const bool lv = label.isValueable();
	label.setValueable (false);
	label.setText (display_ (getValue()));
	label.setValueable (lv);
	label.resize();
	const double rval = getRatioFromValue(getValue());
	label.moveTo (rval > 0.5 ? 1.05 * scale_.getX() : label.right() - 1.05 * scale_.getX(), getYOffset() + 0.25 * getEffectiveHeight());
	label.setTxColors(rval < 0.5 ? getFgColors(): getBgColors());

	Widget::update();
}

#endif /* BVIBRATR_VALUEHSLIDER_HPP_ */
