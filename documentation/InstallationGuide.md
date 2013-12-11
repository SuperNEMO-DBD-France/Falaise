Installing Falaise from Source {#installationguide}
==============================

\tableofcontents

Introduction {#intro}
============
This walkthrough is intended to get you up and running with Falaise
as fast as possible. It therefore adopts a specific layout of where
working copies of sources, builds of these sources and installs of the
builds. You do not have to follow this layout, but we recommend it
as a fast and convenient way to organise things.

Falaise is currently at the Alpha stage of development,
so you should consider
this document to be a work in progress. We welcome constructive comments,
criticisms and suggestions for improvement, and these should be directed to
the SuperNEMO Software Working Group.

* Current Alpha Status : Alpha 1


Preparing Your System for Installation {#preparation}
======================================
To install the Falaise stack, you first need the following:

* UNIX Operating System
  * Linux
  * Mac OS X 10.7/8 (Lion/Mountain Lion) (10.6 and 10.9 should work, but are not tested)
* Several GB of free disk space:
  * At least 200MB for a source code check out
  * At least 6.5GB for a first from scratch build
* Familiarity with the UNIX command line
* Required Software
  * Subversion 1.6 or above
  * CMake 2.8.8 or above
  * Doxygen 1.8 or above
  * C/C++ compiler supporting at least the C++03 standard
    * GCC 4.5 or above, and Clang 3.1 or above are recommended
    * If available, you should use a C++11 compliant compiler
      * GCC 4.7 or higher, Clang 3.3 or higher
      * This is purely for the best forward compatibility, it is not a requirement.
  * BZip2 Library and Headers
  * zlib Library and Headers

We recommend that the Required Software are installed via the relevant
package manager (e.g. yum, apt on Linux, MacPorts, Homebrew on Mac OS X)
for your system, unless the version requirement listed is not met.

We expect the Falaise stack to compile and run on most modern BSD or Linux
based systems provided the listed Required Software is installed. Note
however that this list is a work in progress, and sections below will
highlight where problems may occur with missing software, and how to
report and resolve these issues.


Getting Falaise {#getting}
===============
Falaise is developed using the Subversion repository hosted by
[LPC Caen](https://nemo.lpc-caen.in2p3.fr). Whilst you need a full account
to commit to the repository, we will be using the provided read-only access.

To work with the software, we recommend using the following layout of code
in directories:

~~~~~
SuperNEMO/
|-- lpc-caen.svn
|-- builds
|-- installs
~~~~~

where the `SuperNEMO` directory can be located where you wish, though it
must be on a filesystem with the required free space.

To create this setup, open a terminal, and choose a directory for your
SuperNEMO workspace. For clarity in this guide, because absolute paths are
needed in certain cases, we will use the dummy directory `/Users/ben`, but
you should choose your own location. The first step is to create the
directory hierarchy:

~~~~~
$ cd /Users/ben
$ mkdir -p SuperNEMO/{lpc-caen.svn, builds, installs}
$ cd SuperNEMO
$ ls
builds  installs  lpc-caen.svn
~~~~~

With the directories in place, we can checkout the code. This is done
via the `svn` command line client, and we be using a "sparse checkout"
pattern. Whilst this requires a few more steps upfront, it provides
a very stable and well organised working copy.

Note that in the following, we demonstrate how create a checkout by the
fastest and easiest means possible. This means that you end up with:

* A read-only repository
  * You can update, view logs and create diffs and patches
  * You cannot commit changes

If you already have a developer's account at LPC Caen, then the following
instructions will also work, simply remove the `--username` and
`--no-auth-cache` arguments from the svn commands.

~~~~~
$ pwd
/path/to/your/chosen/workspace/SuperNEMO
$ svn co https://nemo.lpc-caen.in2p3.fr/svn --username=visitor --no-auth-cache \
--depth empty lpc-caen.svn
~~~~~

At this point, you may see the following warning about certificates:

~~~~~
Error validating server certificate for 'https://nemo.lpc-caen.in2p3.fr:443':
 - The certificate is not issued by a trusted authority. Use the
   fingerprint to validate the certificate manually!
 - The certificate hostname does not match.
 - The certificate has expired.
Certificate information:
 - Hostname: caedev.in2p3.fr
 - Valid: from Thu, 15 May 2008 11:52:02 GMT until Fri, 15 May 2009 11:52:02 GMT
 - Issuer: LPC Caen, CNRS, Caen, France, Fr
 - Fingerprint: ae:89:9e:38:e3:80:a7:a3:b2:1e:df:5e:eb:33:2d:ba:83:be:b7:11
(R)eject, accept (t)emporarily or accept (p)ermanently? p
~~~~~

Simply press `t` or `p` to accept the certificate. The process will
then continue by asking for the password for `visitor`, for which you
should enter `anonymous` (If you have accessed the LPC repository before,
you may not see either the certificate check or password prompt).

~~~~~
Password for 'visitor':
Checked out revision 13809.
~~~~~

The revision number you see will vary depending on the current state of development. If you now list the contents of the `lpc-caen.svn` directory
it will be empty apart from a `.svn` directory. We now checkout the parts
we need, specifically the `Cadfael` Software Development Kit (SDK) and
the `Falaise` project.

~~~~~
$ pwd
/Users/ben/SuperNEMO
$ cd lpc-caen.svn
$ svn up --set-depth immediates Cadfael Falaise
A    Cadfael
A    Cadfael/trunk
A    Cadfael/branches
A    Cadfael/tags
Updated to revision 13851.
A    Falaise
A    Falaise/trunk
A    Falaise/branches
A    Falaise/tags
Updated to revision 13851.
~~~~~

As above, the revision number will depend on the current state of development.
Each LPC project is organised using the standard Subversion directory layout
of `trunk` for the mainline of development, `branches` for feature
implementation and bug fixes, and `tags` for releases.

An `ls` of the above directories will show that, again, they are empty.
However, if you use Subversion's list command, e.g.

~~~~~
$ svn ls Cadfael/trunk
CMakeLists.txt
CadfaelVersion.txt
License.txt
NEWS.rst
README.rst
cmake/
docs/
ports/
testing/
~~~~~

you will get a listing of the contents in the upstream repository.
This is what we mean by a "sparse checkout". Such a working copy allows you
to query the repository for what is available, and then choose
the parts you want to checkout.

Finally, we checkout the trunk of Cadfael, and the Alpha 1 tag of Falaise

~~~~~
$ svn up --set-depth infinity Cadfael/trunk
A    Cadfael/trunk/License.txt
...
A    Cadfael/trunk/testing/ImportedTargets/CMakeLists.txt
Updated to revision 13851.
$ svn ls Falaise/tags
...
$ svn up --set-depth infinity Falaise/tags/Falaise-1.0.0-alpha1
~~~~~

The basic source code working copy is now in place. For more details on
using Subversion, we recommend the [Subversion Red Bean Guide](http://svnbook.red-bean.com/).


Installing the Cadfael SDK {#cadfael}
==========================
Falaise uses several third party libraries to implement its functionality,
specifically

- [Boost C++ Libraries](http://www.boost.org) 1.48.0 and higher
- [CAMP C++ Reflection Library](https://github.com/tegesoft/camp) 0.7.1 and higher
- [CLHEP C++ HEP Library](http://proj-clhep.web.cern.ch/proj-clhep/) 2.1.3.1 and higher
- [GNU Scientific Library](http://www.gnu.org/software/gsl/) 1.15 and higher
- [ROOT C++ HEP Toolkit](http://root.cern.ch) 5.34.00 and higher
- [Geant4 C++ Toolkit for Detector Simulations](http://geant4.cern.ch) 9.6.1 and higher

You can reuse existing installations of these libraries to build Falaise,
but for the easiest and most robust installation, we recommend using the
`Cadfael` Software Development Kit. This bundles the configuration, build
and installation of these libraries into a single easy to use toolchain.

Cadfael uses the [CMake](http://www.cmake.org) tool for driving the
configure, build and install of the above libraries. For convenience,
a simple bootstrapping script, `cadfael-bootstrap` is provided to automate
this process. If you require more control over the configuration, you
should consult the `README.rst` file in the `Cadfael/trunk` directory.

All SuperNEMO software requires builds and installs to be in separate
directories from source code. This is enforced because builds can generate
many local files which could by accident get committed back to the central
repository. It also allows you quicker switch between builds, or to cleanly
delete a build if anything goes wrong without risk of deleting any sources.

To bootstrap an install of Cadfael, go into your workspace and run the
bootstrapping script as follows:

~~~~~
$ cd /Users/ben/SuperNEMO
$ ./lpc-caen.svn/Cadfael/trunk/cadfael-bootstrap -DCMAKE_INSTALL_PREFIX=$(pwd)/installs/Cadfael \
-DCADFAEL_BUILD_PREFIX=$(pwd)/builds/Cadfael
~~~~~

This will create a build directory under the `builds` area and install
Cadfael into a dedicated root under `installs`. Note that both
`CMAKE_INSTALL_PREFIX` and `CADFAEL_BUILD_PREFIX` must be absolute paths,
hence the use of the shell `$(pwd)`. Note that the bootstrapping script
 assumes you have a `cmake` executable in `/usr/bin`. If that is not the
case, you can run the script using your install of cmake directly:

~~~~~
$ cd /Users/ben/SuperNEMO
$ /path/to/your/cmake -P ./lpc-caen.svn/Cadfael/trunk/cadfael-bootstrap \
-DCMAKE_INSTALL_PREFIX=$(pwd)/installs/Cadfael \
-DCADFAEL_BUILD_PREFIX=$(pwd)/builds/Cadfael
~~~~~

Cadfael will maximally parallelize the builds for speed, but it may take
up to an hour or two on older machines with fewer cores. It should
generally complete in well under an hour on quad core and better systems.
If bootstrapping completes successfully the `installs/Cadfael` directory
will be populated with a POSIX style layout of the third party software
required for SuperNEMO.

~~~~~
installs/Cadfael/
|-- bin
|-- etc
|   |-- cmake
|   |-- daemons
|   |-- html
|   |-- plugins
|   |-- proof
|   `-- vmc
|-- include
|   |-- CLHEP
|   |-- Geant4
|   |-- boost
|   |-- camp
|   |-- camp-xml
|   |-- gsl
|   |-- root
|   `-- xercesc
|-- lib
|   |-- CLHEP-2.1.3.1
|   |-- Geant4-9.6.1
|   |-- cmake
|   |-- pkgconfig
|   `-- root
`-- share
    |-- Geant4-9.6.1
    |-- aclocal
    |-- camp
    |-- doc
    |-- emacs
    |-- info
    |-- man
    `-- root
~~~~~

You do not require any environment setup to use the applications, though
you may, at your preference, prepend the `installs/Cadfael/bin`
path to your UNIX `PATH`. Once you have installed Cadfael successfully,
you can safely remove the build directory created by the bootstrapping to
save disk space. As Cadfael updates are expected to be relatively
infrequent, this "one-shot" installation is generally the easiest way to
manage the SDK.


Building and Installing Falaise {#buildinstall}
===============================
Falaise uses the [CMake](http://www.cmake.org) tool for driving the
configure, build and install of its core library and applications.
As with Cadfael, we create a separate build directory under `builds` first,
 and install into a directory under `installs`. No bootstrapping script is
provided because here you will want more control over the build process.

To start, go into your SuperNEMO workspace and create a build directory:

~~~~~
$ cd /Users/ben/SuperNEMO
$ ls
builds  installs  lpc-caen.svn
$ cd builds
$ mkdir falaise.alpha1
$ cd falaise.alpha1
~~~~~

the naming of the directory is arbitrary, but it's useful to give them
a name related to the build task.

We now run cmake as follows to configure the build to install under our
workspace area, and to use the libraries provided in our prior install
of Cadfael:

~~~~~
$ cmake -DCMAKE_INSTALL_PREFIX=/Users/ben/SuperNEMO/installs/falaise.alpha1 \
 -DCMAKE_PREFIX_PATH=/Users/ben/SuperNEMO/installs/Cadfael \
 ../../lpc-caen.svn/Falaise/tags/Falaise-1.0.0-alpha1
-- The C compiler identification is GNU 4.3.4
-- The CXX compiler identification is GNU 4.3.4
-- Check for working C compiler: /usr/bin/cc
-- Check for working C compiler: /usr/bin/cc -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++
-- Check for working CXX compiler: /usr/bin/c++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Found Doxygen: /usr/bin/doxygen (found suitable version "1.8.1.2", minimum required is "1.8")
Boost  found.
Found Boost components:
   thread
-- Found Doxygen: /usr/bin/doxygen (found suitable version "1.8.1.2", minimum required is "1.8.0")
Boost  found.
Found Boost components:
   program_options;filesystem;system
Boost  found.
Found Boost components:
   program_options
-- Configuring done
-- Generating done
-- Build files have been written to: /Users/ben/SuperNEMO/builds/falaise.alpha1
~~~~~

The `CMAKE_INSTALL_PREFIX` variable tells cmake where to install, and
`CMAKE_PREFIX_PATH` where to look first for headers, libraries and
applications. The exact output that follows will depend on the OS you
are running on, the compiler detected and the exact CMake version (later
versions are more verbose). The important thing to check is that you see
the last three lines

~~~~~
-- Configuring done
-- Generating done
-- Build files have been written to: /Users/ben/SuperNEMO/builds/falaise.alpha1
~~~~~

which indicate a successful configuration. This process has generated
a suitable makefile for building Falaise, so we now run make to build
everything:

~~~~~
$ make -j4
Scanning dependencies of target falaise_io
Scanning dependencies of target Bayeux_mctools_geant4
[  0%] Building CXX object source/flreconstruct/CommissioningIO/CMakeFiles/falaise_io.dir/DriftModel.cc.o
...
~~~~~

You should select the integer passed to `-j` based on the number of
cores your system has, e.g. 4 for a quad core system. With parallel builds
the exact output will vary, but a successful build will generally be
indicated by the last lines being:

~~~~~
[100%] Built target flreconstruct
[100%] Built target flsimulate
~~~~~

If you are in any doubt as to the success of the build, simply run
make again without the `-j` argument and you should see:

~~~~~
$ make
[ 89%] Built target Bayeux
[ 94%] Built target Bayeux_mctools_geant4
[ 94%] Built target bayeux_docs
[ 97%] Built target Falaise
[ 97%] Built target flsimulate
[ 98%] Built target flreconstruct
[ 99%] Built target falaise_io
[100%] Built target testCommissioningIO
[100%] Built target falaise_docs
~~~~~

After a successful build, you can install Falaise to the location you
passed to `CMAKE_INSTALL_PREFIX` with

~~~~~
$ make install
~~~~~

This will install everything required into a POSIX style hierarchy under
the install prefix:

~~~~~
installs/falaise.alpha1/
|-- bin
|-- include
|   |-- bayeux
|   `-- falaise
|-- lib64
|   `-- cmake
`-- share
    |-- Bayeux-1.0.0
    `-- Falaise-1.0.0
~~~~~

Using Falaise {#using}
=============
With an install in place, you do not need any further setup to use
the applications. The `flsimulate` and `flreconstruct` apps may be run
directly from the command line, e.g.

~~~~~
$ cd /Users/ben/SuperNEMO
$ ./installs/falaise.alpha1/bin/flsimulate -h
...
$
~~~~~

Of course you may, at your convenience, prepend the `bin` directory to
your UNIX `PATH` but this is not required.

By default, Falaise installs Doxygen based documentation for the Falaise
C++ API and the `flsimulate` and `flreconstruct` programs. To view this,
simply point your web browser to the file:

~~~~~
installs/falaise.alpha1/share/Falaise-1.0.0/Documentation/API/html/index.html
~~~~~

A quick way to open this on Mac OS X systems is to run

~~~~~
$ open installs/falaise.alpha1/share/Falaise-1.0.0/Documentation/API/html/index.html
~~~~~

which will open the file your preferred HTML viewer.
On Linux, this is can be done via

~~~~~
$ xdg-open installs/falaise.alpha1/share/Falaise-1.0.0/Documentation/API/html/index.html
~~~~~

though the existence of the `xdg-open` program may be dependent on the
system you are running on.

The front page provides guidance on the layout of the documentation, with
links to the detailed guides on using `flsimulate` and `flreconstruct`.


Troubleshooting {#trouble}
===============
Whilst this walkthrough guide should cover most uses cases, we cannot
guarantee that problems will not be encountered. If you do experience
issues with any of the steps, please contact us, in the first instance
email your issue to:

* Ben Morgan (Ben.Morgan@warwick.ac.uk)

Please attach as much information as possible to assist in diagnosing the
issue.

Developing Falaise {#developing}
==================
FULL DESCRIPTION DEFERRED TO ALPHA 2

Development of Falaise falls into two main categories

* Implementing features in the Falaise core
* Implementing new plug-in modules for use in `flreconstruct`

The first of these is generally, though not exclusively, requires a
developer account on the LPC Caen Trac system (one can also create and
submit patches).
Development of plugins does not require a developer account,
though wil do should your module become part of the core.

We still recommend using the simple three level layout described earlier
as the sparse checkout allows you full and easy access to all repository
information. You can also easily add your own repositories if you are
doing private development, e.g.

~~~~~
SuperNEMO/
|-- lpc-caen.svn
|-- my-repo.git
|-- builds
|-- installs
~~~~~

Further documentation on this topic will be added in Alpha 2.
