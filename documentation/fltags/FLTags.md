The Falaise tag registration system for configuration files {#fltags}
===========================================================

\tableofcontents


The configuration tag system in Falaise {#fltagssystem}
=======================================

The official/blessed  configurations for software components  used for
SuperNEMO  data production,  simulation  and  processing is  addressed
through a *configuration tag* system.  A *tag* is an unique identifier
which  describes  a  specific   configuration  of  a  given  subsystem
(geometry  model, simulation  setup, reconstruction  pipeline...). The
tag  also permit  to  locate  (*resolve*), if  any,  the  path of  the
associated  resource configuration  file  distributed  by the  Falaise
library.

URN
---

A          tag         is          implemented         as          a
[URN](https://en.wikipedia.org/wiki/Uniform_Resource_Name)    (Uniform
Resource Name) character string, using the `urǹ scheme :

Examples:
- `urn:snemo:demonstrator`
- `urn:snemo:demonstrator:geometry:4.0`
- `urn:snemo:demonstrator:simulation:2.1`
- `urn:snemo:demonstrator:rawdata:run:23`
- `urn:snemo:demonstrator:dataset:42`
- `urn:sw:bayeux:3.1.0`

The leading ``urn`` *prefix* is mandatory.  Then comes a mandatory URN
*namespace*, like   ``snemo:demonstrator``.  The last part  of the URN
tag is arbitrary and can be considered as the *identifier* of the item
with respect to the  *namespace*. This *identifier* typically consists
in a sequence  of some alphabetical tokens, separated  by colons (`:`)
and typically  followed by  a numerical  or version  identifier (`42`,
`3.14`) and possibly more specialized  tokens. This sequence of tokens
may or not express some hierarchical organization.

Examples:
 - `urn:snemo:demonstrator:trigger:42`
 - `urn:snemo:demonstrator:trigger:42-bis`
 - `urn:snemo:demonstrator:trigger:42-ter`
 - `urn:snemo:demonstrator:trigger:43`
 - `urn:snemo:demonstrator:trigger:default`


Configuration resource
----------------------

A configuration  item may  be a standalone  resource or  concept which
depends on no external resource and  is simply associated to an unique
configuration    file,    typically   using    the    Bayeux/datatools
(multi-)properties format.   Some configuration  systems used
to   setup    complex   data   processing    components   (simulation,
reconstruction),   are  built   from   the   aggregation  of   several
configuration  subsystems.  The  configuration  tag  system allows  to
describe the *dependency/composition* relationships between such items.

For example,  a *simulation setup* (for  the `flsimulate` application)
is based on a *Geant4 session setup* which aggregates an *experimental
setup*, a  *vertex generation setup*  and a *decay  generation setup*.
The *experimental setup*  itself is built on top of  a *geometry model
setup* and eventually  a *device setup* (which  may describe/model the
hardware devices, including electronics,  cabling...).  This implies a
*dependency  graph*   that  must  be  traversed   properly  to  pickup
unambiguously  the  configuration  of  a  given  data  processing  and
associate it  to the  output data  (real data  collection, simulation,
reconstruction...).  More, a configuration setup  can be reused, or at
least part of it, by sequenced data processing algorithms.

*Example:*

![A data processing pipeline and its associated graph of resources](@ref fltags_confres-1.png)

Monte  Carlo  data  are  generated   by  `flsimulate`  using  a  given
configuration of  the simulation setup.  When  running `flreconstruct`
to process *Monte Carlo data* and build *reconstructed data*, one must
be sure to reuse the same configuration of the experimental setup that
was used  at the simulation  stage, typically using the  same geometry
and  devices models  and associated  options (*variants*  parameters),
besides the specific reconstruction pipeline..


Services and tools
------------------

Falaise  implements  three  different  services  related  to  the  tag
management:
- URN database service,
- URN path resolver service,
- URN query service.

The *URN  database service*  is responsible of  the definition  of all
official tags published by the Physics  Board, the type of resource to
which  they are  associated and  the dependency  relationships between
tags. The configuration  of the URN database service is  stored in the
̀̀$(flquery --resourcedir)/urn/db/`̀ folder.

The *URN path resolver service*  is responsible to locate the resource
file(s)  associated  to  a  given tag.The  configuration  of  the  URN
database      service     is      stored     in      the
̀̀$(flquery --resourcedir)/urn/resolvers/̀` folder.

Both services register themselves in  the *URN query service* which is
hosted  by the  Bayeux/datatools  kernel and  are  used internally  to
automatically address  and locate some configuration  files from their
tag.

A dedicated `falaise::configuration_db` class is provided to allow the
navigation through the tree of tagged resources.


Dependencies between tags {#fltagsdependencies}
=========================

Example of some dependency relationship between tags. Each arrow points
from the depender to the dependee. Here the `urn:3` item depends on the
`urn:2` item which depends on both  `urn:0` and `urn:1` items.
Thus the `urn:3` item depends on `urn:0` and `urn:1` items.

![Tags with dependencies](@ref fltags_dependencies-1.png)


Categories of URN tags {#fltagscategories}
======================

There  are  different  categories  of  configuration  items  that  are
supported in Falaise published tags:

  * `experiment` : Identifier/tag associated to a specific *experiment*,
  * `geometry` : Identifier/tag associated to the configuration of a geometry model.
  * `devices` : Identifier/tag associated to the configuration of a device model.
  * `expsetup` : Identifier/tag associated to an *experimental setup*,
  * `simsetup` : Identifier/tag associated to a *simulation setup*,
  * `recsetup` : Identifier/tag associated to a *reconstruction setup*,
  * `anasetup` : Identifier/tag associated to an *analysis setup*,
  * `varregistry` : Identifier/tag associated to a specific *variant registry*,
  * `variants` : Identifier/tag associated to a *variant service setup*,
  * `varprofile` : Identifier/tag associated to a specific *variant profile*,
  * `services` :  Identifier/tag associated to a *service management setup*,


`experiment`
--------------

The `experiment` category is used for a tag associated to a specific
experiment. Example: the BiPo3 experiment,  the SuperNEMO  demonstrator experiment, the Atlas experiment :-) !

A  tag of  the `experiment` category  is used  to specify the  general experimental
context within which  software activities are done. It  is an abstract
item associated to no real resources like a configuration file. However, such a tag could
be associated to a web page describing the project.


*Example:* Four different tags used to address four different experiments.

![Tags for experiments](@ref fltags_experiments-1.png)


`geometry`
------------

The `geometry`  category is  used for a  tag associated  to the
configuration  of  some geometry  model.

*Example:* Three  distinct `geometry` tags of  the geometry setup
are associated to different `experiment` tags:

![Tags for geometry models](@ref fltags_geometry-1.png)

Additional concurrent geometry configurations can be added in parallel
to the existing ones.  The directed lines between the tags represent a
dependency relationship  from the *depender* to  the *dependee*.  Here
both  geometry  setup  configurations  on  the  left  depends  on  the
SuperNEMO  demonstrator  experiment (`urn:snemo:demonstrator`),  while
there is only one geometry  setup configuration on the right depending
on the BiPo3 experiment  (`urn:bipo3:detector`).  This graph expresses
that it  makes no sense  to use the  `urn:bipo3:geometry:1.0` geometry
model in the context of the SuperNEMO demonstrator experiment.


`devices`
------------

The  `devices`  category   is  used  for  a  tag   associated  to  the
configuration of  some device model.  It is  not used yet  in Falaise.
The  device  model  typically  consists  in  the  description  of  the
electronics and  the connected devices composing  the readout, control
and monitoring system.

`expsetup`
------------

Identifier/tag associated to an *experimental setup* which implies:

  * the setup of the *geometry manager*
  * the setup of the *device manager* (not used yet)

*Example:*

An *experimental  setup* is built  from a  set of two  components: the
*geometry*  model  of   the  detector,  the  *device*   model  of  the
experiment:

![Tags for experimental setups](@ref fltags_expsetups-1.png)

A new  *experimental setup* can  be published using the  same geometry
model  but   a  refined  *device   model*.  In  such  case,   the  new
*experimental setup* must be associated with  a new tag using a bumped
version number:

![Tags for experimental setups](@ref fltags_expsetups-2.png)

Or course, both geometry and device  models could be modified to build
a  new  experimental setup  which  must  be explicitely  described  as
depending of the new combination of subsystems:

![Tags for experimental setups](@ref fltags_expsetups-3.png)

The final dependency graph is:

![Tags for experimental setups](@ref fltags_expsetups-4.png)


`simsetup`
------------

Identifier/tag associated to a *simulation setup* which implies:

  * an *experimental setup*,
  * the setup of the *vertex* generation manager,
  * the setup of the *primary* event generation manager,
  * the setup of the *Geant4* simulation manager which finally uses the three setups above.

*Example:*

The following simulation setup is  built by the aggregation of several
subsystems with their own configurations:


![Tags for simulation setups](@ref fltags_simsetups-1.png)

Suppose  we  decide  to  enrich   the  vertex  generation  setup  with
additional    vertex    generators     not    implemented    in    the
`urn:snemo:demonstrator:simulation:vertexes:4.1`   configuration.   We
can  create   a  new  `urn:snemo:demonstrator:simulation:vertexes:4.2`
configuration setup.   As the simulation  setup depends on  the vertex
generation   setup,  it   is   necessary  to   also   publish  a   new
`urn:snemo:demonstrator:simulation:3.0`  simulation setup.   This will
preserve the existing one for backward compatibility and make possible
to  use  the  new  one  concurrently of  the  previous  version.   The
dependency scheme is now:

![Tags for simulation setups](@ref fltags_simsetups-2.png)


`recsetup`
------------

Identifier/tag associated to a *reconstruction setup* which implies:

  * an *experimental setup* (see above)
  * the setup of the reconstruction chain (*pipeline*)

*Example:*

The following reconstruction  setup is built by the  aggregation of an
experimental  setup  configuration  and  a  specific  data  processing
pipeline (sequence of processing modules):

![Tags for reconstruction setups](@ref fltags_recsetups-1.png)


`anasetup`
------------

Identifier/tag associated to an *analysis setup* which implies:

  * an *experimental setup* (see above)
  * an *analysis setup* (not used yet)


`varregistry`
---------------

Identifier/tag associated to the definition of a *variant registry*.


`variants`
-----------

Identifier/tag associated to the configuration of a *variant service*.
A  variant  service  is  built   from  a  *variant  repository*  which
aggregates, and thus depends on, variant registries.

*Example:*

The variant  service used  by the  simulation setup  is based  on four
variant registries, but the variant service used by the reconstruction
setup needs  only one  variant registry  (geometry) because  the other
registries make  no sense  in the  reconstruction context.   Here both
simulation and reconstruction setups share the same geometry setup and
thus use the same variant registry associated to the geoemtry setup:

![Tags for variants setups](@ref fltags_variants-1.png)

`varprofile`
--------------

Identifier/tag associated  to a  *variant profile*. A  variant profile
depends  on a  given  *variant service*.   Several concurrent  variant
profiles can be attached to a  given variant service.  Only one can be
selected by the user. If a variant profile is officially registered in
the URN  database service, the user  can select it through  its unique
tag.

*Example:*

A  simulation setup  with its  variant service  and several  available
variant  profiles.   Here  selecting  the simulation  setup  with  tag
`urn:snemo:demonstrator:simulation:2.1`  and  the  associated  profile
with  tag  'urn:snemo:demonstrator:simulation:2.1:variants:profiles:2`
determines  the  complete  configuration  of  the  simulation  session
(static and dynamic parts).

![Tags for variant profiles](@ref fltags_varprofile-1.png)

`services`
------------

Identifier/tag associated to the configuration of a *service management system*.


Configuration of the simulation {#fltagssimulationconfiguration}
===============================

As mentionned  above, `flsimulate` needs two  configuration items in
order  to   describe  unambiguously  the  configuration   of  a  given
simulation session/run:
- the static configuration
- the dynamic configuration

Static configuration
--------------------

The first item consists in the *static* part of the configuration.  It
is  provided   through  the  *simulation  setup*   tag.   This  static
configuration is published  by the Physics Board as  an official setup
which defines  the general context  of a Monte Carlo  production. This
part  of  the configuration  is  thus  hardcoded  in Falaise  and  may
correspond to a large and complex set of non mutable configuration and
data files  (normal users should  never change them).  As  an official
item  distributed  within Falaise,  a  tag  is  associated to  it  and
registered in  Falaise's URN database  service.  Users are  invited to
provide the selected  tag which will be automatically  resolved by the
simulation manager.

Dynamic configuration
----------------------

On the  other hand,  Falaise offers  the possibility  to its  users to
modify  a (generally)  limited set  of parameters  available from  the
static part of the configuration:  the *variant* parameters.  The list
of  *variant*  parameters with  their  associated  values defines  the
*dynamic* part of the configuration.  It is called a *variant profile*
and generally  consists in a simple  human readable file or  a list of
named *variant settings*.

The static configuration item generally depends on a specific *variant
service*  which defines  the rules  used to  manage a  set of  variant
parameters.   The  user  can  thus create  its  own  variant  profile,
following the rules published by the variant service associated to the
selected simulation setup.   The variant profile generated  by the user
is  thus  injected  in  the  variant  system  on  top  of  the  static
configuration. This  mechanism may be  used to choose the  tickness of
the  source  foil,  the  random vertex  generator,  the  random  decay
generator as well as some parameters of the simulation engine...

If a specific variant profile is  acknowledged by the Physics Board as
a blessed/official profile,  it is associated to a  tag and registered
in  the  URN  database  service.   Several  variant  profiles  can  be
officially attached to a variant service and registered, leading to as
many official  tags.  The user is  thus invited to provide  the tag of
the  variant  profile  he/she  has selected  from  the  official  list
associated  to  the  chosen  simulation  setup.  In  such  a  case,  a
simulation session  is fully  described by  both the  blessed *static*
configuration tag and the blessed *dynamic* configuration tag.

*Examples:*

- A simulation configuration made from a simulation setup tag and a specific
variant profile:
![Tags for simulation configuration](@ref fltags_simconf-1.png)
- A simulation configuration made from a simulation setup tag and the default
associated variant profile:
![Tags for simulation configuration](@ref fltags_simconf-2.png)
- A simulation configuration made from a simulation setup tag and a non official user
variant profile file:
![Tags for simulation configuration](@ref fltags_simconf-3.png)



Organization of configuration resource files {#fltagsresourcefilesorg}
============================================

WIP

Extension of the tag system
===========================

It is foreseen that the tag system will also be used in a near future to address
the real and Monte Carlo data file produced by the SuperNEMO collaboration.
An external *database* will store the definition table of the data files and their
associated metadata: URN/tag, type, production date, checksum... Another table will store
the paths to the data files addressed through the URN.

Examples:
 - `urn:snemo:demonstrator:data:real:run:3252`
 - `urn:snemo:demonstrator:data:real:run:3253`
 - `urn:snemo:demonstrator:data:real:run:3254`
 - `urn:snemo:demonstrator:data:mc:run:23`
 - `urn:snemo:demonstrator:data:mc:run:24`
 - `urn:snemo:demonstrator:data:mc:run:25`
