This is the DAIMONIN gui-editor for the new 3D client. See http://www.daimonin.net
Daimonin developers may access the gui-editor's SVN folder at daimonin/client3d/gui-editor.
This software is distributed under the terms of the GPL 2, see LICENSE for details.


Configuration
==============

* you need JAVA 1.5 (or 5.0 as Sun calls it) to run the editor.
* Read and optionally edit the file 'editor3d.config'. Defaults should work.


How to run
==============

Windows: editor3d_start.bat all default
Linux  : ./editor3d_start all default


Usage
==============

editor3d_start <operation mode> <configuration file>

<operation mode>     Either 'all' or 'xml' (see below).
<configuration file> 'default' for default configuration (editor3d.config) or specify your own config file.

Examples:
editor3d_start all default
editor3d_start all myConfig.conf

Operation mode 'all':   All PNGs in starting dir are read recursively, packed into one rectangle, XML file with image meta data gets written, PNG containing all PNGs gets written.

Operation mode 'xml':   XML file with meta information is read, PNG containing all PNGs gets written. Use this mode if you want to create or modify the XML by yourself.


TODO's
==============

* a nice GUI.
* more testing


Author
==============

Rumbuff
rumbuff@gmx.de

Please feel free too email me some feedback.
