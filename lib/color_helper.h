// $Id$ -*- c++ -*-
//
//  color_helper.h

#ifndef __ColorHelper_H
#define __ColorHelper_H

#include "color_pkg.h"
#include "pv_factory.h"

// ColorInfo has the "raw" color information:
// name    -  a color string name
// menu    -  colors available to users in color dialog
// palette -  aka color map: colors available on screen
// index   -  index of color in the current palette/color map
//            (unless we talk about the menu index)
// pixel   -  index in palette, X11 pixel value, used in X calls
// as well as rules and special colors for alarms.
//
// A Widget's foreground might use a base color (index/name/pixel)
// but then modify it according to alarm state or
// a rule (which takes a color "value", by default the main PV).
//
// This ColorHelper will help in finding the current, modified pixel.
//
// (Similar to the pvColorClass but streamlined for the ProcessVariable API)
class ColorHelper
{
private:
    int index;
    bool alarm_sensitive;
    short severity;
    double color_value;

public:
    ColorHelper();
    ColorHelper & operator = (const ColorHelper &rhs);

    // Alarm sensitive: will show MINOR, MAJOR, INVALID alarms.
    // Uses INVALID for disconnected.
    void setAlarmSensitive(bool yes_no)
    {   alarm_sensitive= yes_no; }

    bool isAlarmSensitive()
    {   return alarm_sensitive; }
    
    // Set color value and severity from PV
    void updatePVStatus(const ProcessVariable *pv);
    void updateColorValue(const ProcessVariable *pv);
    void reset();
    
    // Palette index (widget property) and name (EDL file):
    // Color that was originally chosen,
    // this is not the pixel for painting.
    int getIndex()
    {   return index; }

    void setIndex(int i)
    {   index = i; }

    const char *getName(colorInfoClass *ci)
    {   return ci->colorName(index); }

    void setName(colorInfoClass *ci, const char *name)
    {   index = ci->colorIndexByName(name); }
    
    // Pixel: Pixel that should be used for painting.
    // That's the base index, transformed according to rules & alarm state
    // and then tranformed into an X11 Pixel
    int getPixel(colorInfoClass *ci);

private:
    ColorHelper(const ColorHelper &rhs); // not implemented
};

inline ColorHelper::ColorHelper()
{
    index = 0;
    alarm_sensitive = true;
    severity = NO_ALARM;
    color_value = 0.0;
}

inline ColorHelper & ColorHelper::operator = (const ColorHelper &rhs)
{
    index = rhs.index;
    alarm_sensitive = rhs.alarm_sensitive;
    severity = NO_ALARM;
    color_value = 0.0;
    return *this;
}

inline void ColorHelper::updatePVStatus(const ProcessVariable *pv)
{
    if (pv && pv->is_valid())
        severity = pv->get_severity();
    else
        severity = INVALID_ALARM;
}

inline void ColorHelper::updateColorValue(const ProcessVariable *pv)
{
    if (pv && pv->is_valid())
        color_value = pv->get_double();
    else
        color_value = 0.0;
}

inline void ColorHelper::reset()
{
    severity = NO_ALARM;
    color_value = 0.0;
}

inline int ColorHelper::getPixel(colorInfoClass *ci)
{
    int pixel;
    if (ci->isRule(index))
        pixel = ci->getPixelByIndex(ci->evalRule(index, color_value));
    else
        pixel = ci->getPixelByIndex(index);
    // If alarm_sensitive, this overrides all other color determinations:
    if (alarm_sensitive && severity > NO_ALARM)
    {
        switch (severity)
        {
            case MINOR_ALARM:
                pixel = ci->getPixelByIndex(
                    ci->getSpecialIndex(COLORINFO_K_MINOR));
                break;
            case MAJOR_ALARM:
                pixel = ci->getPixelByIndex(
                    ci->getSpecialIndex(COLORINFO_K_MAJOR));
                break;
            case INVALID_ALARM:
                pixel = ci->getPixelByIndex(
                    ci->getSpecialIndex(COLORINFO_K_INVALID));
                break;
        }
    }
    return pixel;
}

#endif
