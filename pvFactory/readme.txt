-*- outline -*-

This directory currently generates a library libPV.so
that includes the following.
In the future, the PV factory could be part of EDM,
the EPICS PV Factory would be one shared module
as well as the widgets.

* pv_factory.*
Base class for a process-variable factory.
Handles connection, subscription, retrieval
of meta information.

In principle, the actual implementation should
be in a shared library and loaded on demand,
maybe user can even select the implementation
when configuring the widget (Epics, Vsystem, ...).
Currently this is not possible.
Widgets ask PV factory to ProcessVariable,
PV factory has hardcoded table to pick factory
depending on prefix "EPICS\" or "CALC\".
Default for no prefix: EPICS.

* epics_pv_factory.*
Implementation for EPICS ChannelAccess.

* calc_pv_factory.*
Implementation for CALC

* Widget: Monitors/Textupdate
(in textupdate.*)
Simple text update widget.

* Widget: Controls/Textentry
(in textupdate.*)
Simple text entry widget.

* Widget: Monitors/Stripchart
(in SciPlot.* lin_interpol.h strip*)
Strip-chart based on SciPlot.

Experimental "CA Time" option:
When checked, the time stamp from the IOC is used.
If the IOC's time is out of sync with the host,
you see nothing!
Otherwise you can see that e.g. the IOC is x seconds
behind the host.

kasemir@lanl.gov
