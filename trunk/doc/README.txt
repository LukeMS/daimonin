Daimonin Documentation System
=============================

    You're in the Daimonin documentation folder.


    Please do not expect finished documentation here.
    Don't expect it for 2 reasons:
* The documentation is handled in XML source code.
* This is work in progress, we've just started it in mid 2005.

    If you are a normal end user that just wants to read documentation, you're
probably better off reading the documentation provided on the website:

    http://www.daimonin.net/


How the Documentation System works
==================================

Rationale
---------

    The documentation system is meant to enable us to use "write once show
everywhere". Currently, documentation is somewhat spread across different
places inside the CVS repository as well as outside it. The goal is to
harmonize the different sources of documentation. To successfully be able to
do so, the system must also be able to output the documentation in those
formats that are already used at places where we publish documentation.

    This includes output for:
* bbCode as used by the PHP board on the Daimonin Website
* HTML for old web browsers
* XHTML for new web browsers or to have a good input for further processing
* phpWiki as used by the PHP wiki on the Daimonin Website

    To achieve this, I (Cher) have suggested this XML based system. It should
allow us, the developers, to create documentation in a single format (XML) and
use the XSLT transformation sheets provided with this documentation system. It
should also be extensible to add new output formats any time.


Requirements
------------

    To use the documentation system to create documentation


Directory Structure
-------------------

    make/       contains various build systems to run the documentation
                generation

    make/ant/   contains documentation generation based on Apache Ant.
                Get Ant from http://ant.apache.org/

    make/make/  contains documentation generation based on Make (GNUMake etc.)

    src/        contains source files

    src/doc/    contains the documentation source files

    src/dtd/    contains external subset for documentation validation

    src/xslt/   contains the XSLT transformation sheets to generate the
                documentation in other formats


Building the documentation with Ant
-----------------------------------

    To build the documenation with Ant you need an XSLT 2.0 aware XSLT
processor, like Saxon ( http://saxon.sourceforge.net/ ).
    Run "ant -projecthelp" to learn more about building the documentation with
Ant.
