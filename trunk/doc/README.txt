Daimonin Documentation System
=============================

    You're in the Daimonin documentation folder.


    Please do not yet expect finished documentation here.
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
places inside the CVS repository as well as outside it:
* source code of server, sometimes documented, as well as the source code of
  other subprojects
* the documentation found here in daimonin/doc/
* auto-generated source documentation with doxygen or javadoc
* The Wiki, found at
  http://www.daimonin.net/modules.php?op=modload&name=phpWiki&file=index
* The Forum, found at
  http://www.daimonin.net/modules.php?name=PNphpBB2
* Various sources spread around the web
* The editor's online documentation

    The goal is to harmonize the different sources of documentation and reduce
their number - or at least the work required for maintenance. To successfully be
able to do so, the system must also be able to output the documentation in those
formats that are already used at places where we publish documentation.

    This includes output for:
* bbCode as used by the PHP board on the Daimonin Website
* HTML for old web browsers
* XHTML for new web browsers or to have a good input for further processing
* phpWiki as used by the PHP wiki on the Daimonin Website

    To achieve this, I (Cher) have suggested this XML based system. It should
allow us, the developers, to create documentation in a single format (XML) and
use the XSLT transformation sheets provided with this documentation system. It
should also be extensible to add new output formats any time. It should reduce
the number of places where we really need to maintain documentation.


Requirements
------------

    To use the documentation system to create documentation


Directory Structure
-------------------

    make/       contains various build systems to run the documentation
                generation

    make/ant/   contains documentation generation based on Apache Ant.
                Get Ant from http://ant.apache.org/

    src/        contains source files

    src/doc/    contains the documentation source files

    src/dtd/    contains external subset for documentation validation

    src/xslt/   contains the XSLT transformation sheets to generate the
                documentation in other formats


Todo
----

    The following things are planned but not implemented yet:
* automatic wiki update
* generate documentation specifically for use as online documentation within the
  editor

Building the documentation with Ant
-----------------------------------

    To build the documenation with Ant you need an XSLT 2.0 aware XSLT
processor, like Saxon ( http://saxon.sourceforge.net/ ). Note that if you use
Saxon, it must be Saxon 8.0 or newer. Previous versions are likely to fail
because they do not implement XSLT 2.0. At the time of this writing, Saxon is
the only XSLT processor knowing to support XSLT 2.0. So the standard Xalan that
is included with the SDK or Ant is not sufficient.
    Run "ant -f make/ant/build.xml -projecthelp" to learn more about building
the documentation with Ant.

    Note: building the documentation with make is currently not supported. The
reason for this is that I haven't found a free and small C XSLT 2.0 processor
that would fit the usage with make.




Please don't remove the following line
vim:tw=80
