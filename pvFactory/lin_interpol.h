// -*- C++ -*-
// Linear Interpolation
//
// kasemir@lanl.gov

#ifndef __LINEARINTERPOLATION_H__
#define __LINEARINTERPOLATION_H__

//*
//  The <b>LinearTransformation</b> class
//  provides routines for linear transformation 
//  between two intervals,
//  aptly named <I>source</I> and <I>destination</I>.
class LinearTransformation
{
public:
    //* Default: 1-1 mapping
    LinearTransformation ()
    {
        _scale=1;
        _s0=0;
        _s1=10;
        _d0=0;
        _d1=10;
    }

    //* Get start/end of source area.
    double getS0 () const
    {   return _s0; }
    double getS1 () const
    {   return _s1; }

    //* Get start/end of destination area.
    double getD0 () const
    {   return _d0; }
    double getD1 () const
    {   return _d1; }

    /** Setup <b>LinearTransformation</b> to
     *  transform into <I>destination</I> [d0...d1].
     */
    void setDestination (double d0, double d1)
    {
        setup (_s0, _s1, d0, d1);
    }

    /** Setup <b>LinearTransformation</b> to
     *  transform from <I>source</I> [s0...s1].
     */
    void setSource (double s0, double s1)
    {   setup (s0, s1, _d0, _d1);   }

    /** Setup <b>LinearTransformation</b> to
     *  transform from <I>source</I> [s0...s1]
     *  into <I>destination</I> [d0...d1].
     */
    void setup (double s0, double s1, double d0, double d1)
    {
        double ds = s1-s0;

        if (ds == 0)
            return;

        _s0 = s0;
        _s1 = s1;
        _d0 = d0;
        _d1 = d1;
        _scale = (d1-d0)/ds;
    }

    /** Transform given source value <I>s</I>.<BR>
     *  Caution: If s outside [s0...s1], result will be outside [d0...d1]!
     */
    double transform (double s) const
    {
        return (s-_s0)*_scale + _d0;
    }

    /** Try to transform given destination value <I>d</I> back into source.<BR>
     *  Caution: If d outside [d0...d1], result will be outside [s0...s1]!
     */
    double inverse (double d) const
    {
        if (_scale == 0)
            return 0;
        return (d-_d0)/_scale + _s0;
    }
private:
    double  _scale, _s0, _s1, _d0, _d1;
};

#endif //__LINEARINTERPOLATION_H__
