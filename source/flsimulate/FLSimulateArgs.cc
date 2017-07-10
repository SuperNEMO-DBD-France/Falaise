// Ourselves
#include "FLSimulateArgs.h"

// Third party:
// - Boost:
#include <boost/algorithm/string.hpp>
// - Bayeux:
#include <bayeux/datatools/utils.h>
#include <bayeux/datatools/urn.h>
#include <bayeux/datatools/kernel.h>
#include <bayeux/datatools/library_info.h>
#include <bayeux/mygsl/random_utils.h>

// This Project:
#include "falaise/property_reader.h"
#include "falaise/configuration_db.h"
#include "FLSimulateCommandLine.h"
#include "FLSimulateErrors.h"
#include "FLSimulateUtils.h"

namespace FLSimulate {

  // static
  const std::string & FLSimulateArgs::default_file_for_output_metadata()
  {
    static const std::string _f("__flsimulate-metadata.log");
    return _f;
  }

  // static
  const std::string & FLSimulateArgs::default_file_for_seeds()
  {
    static const std::string _f("__flsimulate-seeds.log");
    return _f;
  }

  void do_postprocess(FLSimulateArgs & flSimParameters);

  // static
  FLSimulateArgs FLSimulateArgs::makeDefault()
  {
    FLSimulateArgs params;

    // Application specific parameters:
    params.logLevel       = datatools::logger::PRIO_ERROR;
    params.userProfile    = "normal";
    params.numberOfEvents = 1;
    params.doSimulation   = true;
    params.doDigitization = false;
    // Identification of the experimental setup:
    params.experimentalSetupUrn = "";

    // Identification of the simulation setup:
    params.simulationSetupUrn   = default_simulation_setup();
    // Simulation manager internal parameters:
    params.simulationManagerParams.set_defaults();
    params.simulationManagerParams.interactive = false;
    params.simulationManagerParams.logging = "error";
    params.simulationManagerParams.manager_config_filename = "";
    // Seeding is auto (from system) unless explicit file supplied
    params.simulationManagerParams.input_prng_seeds_file = "";
    params.simulationManagerParams.output_prng_seeds_file = "";
    params.simulationManagerParams.vg_seed   = mygsl::random_utils::SEED_AUTO; // PRNG for the vertex generator
    params.simulationManagerParams.eg_seed   = mygsl::random_utils::SEED_AUTO; // PRNG for the primary event generator
    params.simulationManagerParams.shpf_seed = mygsl::random_utils::SEED_AUTO; // PRNG for the back end MC hit processors
    params.simulationManagerParams.mgr_seed  = mygsl::random_utils::SEED_AUTO; // PRNG for the Geant4 engine itself
    params.simulationManagerParams.output_profiles_activation_rule = "";
    params.saveRngSeeding = true;
    params.rngSeeding = "";

    // Variants support:
    params.variantConfigUrn = "";
    params.variantProfileUrn = "";
    params.variantSubsystemParams.config_filename = "";
    params.variantSubsystemParams.reporting_filename = "";
    params.saveVariantSettings = true;

    // Service support:
    params.servicesSubsystemConfigUrn = "";
    params.servicesSubsystemConfig = "";

    // I/O control:
    params.outputMetadataFile = "";
    params.embeddedMetadata   = true;
    params.outputFile         = "";

    return params;
  }

  void do_configure(int argc, char *argv[], FLSimulateArgs& flSimParameters) {
    // - Default Config
    flSimParameters = FLSimulateArgs::makeDefault();

    // - CL Dialog Config
    FLSimulateCommandLine args;
    try {
      do_cldialog(argc, argv, args);
    } catch (FLDialogHelpRequested& e) {
      throw FLConfigHelpHandled();
    } catch (FLDialogOptionsError& e) {
      throw FLConfigUserError {"bad command line input"};
    }

    // Feed input from command line to params
    flSimParameters.logLevel           = args.logLevel;
    flSimParameters.userProfile        = args.userProfile;
    flSimParameters.outputMetadataFile = args.outputMetadataFile;
    flSimParameters.embeddedMetadata   = args.embeddedMetadata;
    flSimParameters.outputFile         = args.outputFile;
    flSimParameters.mountPoints        = args.mountPoints;
    flSimParameters.variantSubsystemParams.reporting_filename = args.variantReportFile;

    if (flSimParameters.mountPoints.size()) {
      // Apply mount points as soon as possible, because manually set file path below
      // may use this mechanism to locate files:
      datatools::kernel & dtk = datatools::kernel::instance();
      datatools::library_info & dtklLibInfo = dtk.grab_library_info_register();
      for (const std::string & mountDirective : flSimParameters.mountPoints) {
        std::string theLibname;
        std::string theTopic;
        std::string thePath;
        std::string errMsg;
        bool parsed = datatools::library_info::parse_path_registration_directive(mountDirective,
                                                                                 theLibname,
                                                                                 theTopic,
                                                                                 thePath,
                                                                                 errMsg);
        DT_THROW_IF(!parsed, FLConfigUserError,
                    "Cannot parse directory mount directive '" << mountDirective << "' : " << errMsg);
        if (theTopic.empty()) {
          theTopic = datatools::library_info::default_topic_label();
          DT_LOG_DEBUG(flSimParameters.logLevel, "Using default path registration topic: " << theTopic);
        }
        DT_LOG_DEBUG(flSimParameters.logLevel, "Path registration: " << mountDirective);
        DT_LOG_DEBUG(flSimParameters.logLevel, "  Library name : " << theLibname);
        DT_LOG_DEBUG(flSimParameters.logLevel, "  Topic        : " << theTopic);
        DT_LOG_DEBUG(flSimParameters.logLevel, "  Path         : " << thePath);
        try {
          dtklLibInfo.path_registration(theLibname, theTopic, thePath, false);
        } catch (std::exception & error) {
          DT_THROW(FLConfigUserError,
                   "Cannot apply directory mount directive '" << mountDirective << "': " << error.what());
        }
      }
    }

    if (! flSimParameters.embeddedMetadata) {
      if (flSimParameters.outputMetadataFile.empty()) {
        // Force a default metadata log file:
        flSimParameters.outputMetadataFile =
          FLSimulateArgs::default_file_for_output_metadata();
      }
    }

    // If a script was supplied, use that to override params
    if (!args.configScript.empty()) {
      datatools::multi_properties flSimConfig("name", "type");
      std::string configScript = args.configScript;
      datatools::fetch_path_with_env(configScript);
      flSimConfig.read(configScript);
      DT_LOG_DEBUG(flSimParameters.logLevel, "Simulation Configuration:");
      if (datatools::logger::is_debug(flSimParameters.logLevel)) {
        flSimConfig.tree_dump(std::cerr, "", "[debug] ");
      }

      // Now extract and bind values as needed
      // Caution: some parameters are only available for specific user profile

      // Basic system:
      if(flSimConfig.has_key_with_meta("flsimulate", "flsimulate::section")) {
        datatools::properties baseSystem = flSimConfig.get_section("flsimulate");
        // Bind properties in this section to the relevant ones in params:

        // Number of simulated events:
        flSimParameters.numberOfEvents = falaise::properties::getValueOrDefault<int>(baseSystem,
                                                                                     "numberOfEvents",
                                                                                     flSimParameters.numberOfEvents);


        // Printing rate for events:
        flSimParameters.simulationManagerParams.number_of_events_modulo =
          falaise::properties::getValueOrDefault<int>(baseSystem,
                                                      "moduloEvents",
                                                      flSimParameters.simulationManagerParams.number_of_events_modulo);

        // Do simulation:
        flSimParameters.doSimulation = falaise::properties::getValueOrDefault<bool>(baseSystem,
                                                                                    "doSimulation",
                                                                                    flSimParameters.doSimulation);

        // Do digitization:
        flSimParameters.doDigitization = falaise::properties::getValueOrDefault<bool>(baseSystem,
                                                                                      "doDigitization",
                                                                                      flSimParameters.doDigitization);

      }

      // Simulation subsystem:
      if(flSimConfig.has_key_with_meta("flsimulate.simulation", "flsimulate::section")) {
        datatools::properties simSubsystem = flSimConfig.get_section("flsimulate.simulation");
        // Bind properties in this section to the relevant ones in params:

        // Simulation setup URN:
        flSimParameters.simulationSetupUrn
          = falaise::properties::getValueOrDefault<std::string>(simSubsystem,
                                                                "simulationSetupUrn",
                                                                flSimParameters.simulationSetupUrn);

        DT_LOG_DEBUG(flSimParameters.logLevel,
                     "flSimParameters.simulationSetupUrn=" << flSimParameters.simulationSetupUrn);

        // Simulation manager main configuration file:
        if (flSimParameters.userProfile == "production" && simSubsystem.has_key("simulationSetupConfig")) {
          DT_THROW(FLConfigUserError,
                   "User profile '" << flSimParameters.userProfile << "' "
                   << "does not allow to use the '" << "simulationSetupConfig" << "' simulation configuration parameter!");
        }
        flSimParameters.simulationManagerParams.manager_config_filename
          = falaise::properties::getValueOrDefault<std::string>(simSubsystem,
                                                                "simulationSetupConfig",
                                                                flSimParameters.simulationManagerParams.manager_config_filename);

        // File for loading internal PRNG's seeds:
        if (flSimParameters.userProfile == "production" && !simSubsystem.has_key("rngSeedFile")) {
          DT_THROW(FLConfigUserError,
                   "User profile '" << flSimParameters.userProfile << "' "
                   << "must use the '" << "rngSeedFile" << "' simulation configuration parameter!");
        }
        flSimParameters.simulationManagerParams.input_prng_seeds_file =
          falaise::properties::getValueOrDefault<std::string>(simSubsystem,
                                                              "rngSeedFile",
                                                              flSimParameters.simulationManagerParams.input_prng_seeds_file);

        if (flSimParameters.simulationManagerParams.input_prng_seeds_file.empty()) {
          if (flSimParameters.userProfile == "production") {
            DT_THROW_IF(simSubsystem.has_key("rngEventGeneratorSeed"),
                        FLConfigUserError,
                        "User profile '" << flSimParameters.userProfile << "' "
                        << "does not allow to use the '" << "rngEventGeneratorSeed"
                        << "' simulation configuration parameter!");
            DT_THROW_IF(simSubsystem.has_key("rngVertexGeneratorSeed"),
                        FLConfigUserError,
                        "User profile '" << flSimParameters.userProfile << "' "
                        << "does not allow to use the '" << "rngVertexGeneratorSeed"
                        << "' simulation configuration parameter!");
            DT_THROW_IF(simSubsystem.has_key("rngHitPostprocessingGeneratorSeed"),
                        FLConfigUserError,
                        "User profile '" << flSimParameters.userProfile << "' "
                        << "does not allow to use the '" << "rngHitProcessingGeneratorSeed"
                        << "' simulation configuration parameter!");
            DT_THROW_IF(simSubsystem.has_key("rngGeant4GeneratorSeed"),
                        FLConfigUserError,
                        "User profile '" << flSimParameters.userProfile << "' "
                        << "does not allow to use the '" << "rngGeant4GeneratorSeed"
                        << "' simulation configuration parameter!");
          }
          flSimParameters.simulationManagerParams.eg_seed =
            falaise::properties::getValueOrDefault<int>(simSubsystem,
                                                        "rngEventGeneratorSeed",
                                                        flSimParameters.simulationManagerParams.eg_seed);
          flSimParameters.simulationManagerParams.vg_seed =
            falaise::properties::getValueOrDefault<int>(simSubsystem,
                                                        "rngVertexGeneratorSeed",
                                                        flSimParameters.simulationManagerParams.vg_seed);
          flSimParameters.simulationManagerParams.shpf_seed =
            falaise::properties::getValueOrDefault<int>(simSubsystem,
                                                        "rngHitProcessingGeneratorSeed",
                                                        flSimParameters.simulationManagerParams.shpf_seed);
          flSimParameters.simulationManagerParams.mgr_seed =
            falaise::properties::getValueOrDefault<int>(simSubsystem,
                                                        "rngGeant4GeneratorSeed",
                                                        flSimParameters.simulationManagerParams.mgr_seed);
        }

        flSimParameters.simulationManagerParams.output_prng_seeds_file =
          falaise::properties::getValueOrDefault<std::string>(simSubsystem,
                                                              "rngSeedFileSave",
                                                              flSimParameters.simulationManagerParams.output_prng_seeds_file);

        // File for loading internal PRNG's states:
        if (flSimParameters.userProfile != "expert" && simSubsystem.has_key("inputRngStateFile")) {
          DT_THROW(FLConfigUserError,
                   "User profile '" << flSimParameters.userProfile << "' "
                   << "does not allow to use the '" << "inputRngStateFile" << "' simulation configuration parameter!");
        }
        flSimParameters.simulationManagerParams.input_prng_states_file =
          falaise::properties::getValueOrDefault<std::string>(simSubsystem,
                                                              "inputRngStateFile",
                                                              flSimParameters.simulationManagerParams.input_prng_states_file);

        // File for saving internal PRNG's states:
        flSimParameters.simulationManagerParams.output_prng_states_file =
          falaise::properties::getValueOrDefault<std::string>(simSubsystem,
                                                              "outputRngStateFile",
                                                              flSimParameters.simulationManagerParams.output_prng_states_file);

        // Saving rate for internal PRNG's states:
        flSimParameters.simulationManagerParams.prng_states_save_modulo =
          falaise::properties::getValueOrDefault<int>(simSubsystem,
                                                      "rngStateModuloEvents",
                                                      flSimParameters.simulationManagerParams.prng_states_save_modulo);
      }

      // Digitization subsystem:
      if(flSimConfig.has_key_with_meta("flsimulate.digitization", "flsimulate::section")) {
        datatools::properties digiSubsystem = flSimConfig.get_section("flsimulate.digitization");
        // Bind properties in this section to the relevant ones in params:

      }

      // Variants subsystem:
      if (flSimConfig.has_key_with_meta("flsimulate.variantService", "flsimulate::section")) {
        datatools::properties variantSubsystem = flSimConfig.get_section("flsimulate.variantService");
        // Bind properties to relevant ones on params

        // Variant configuration URN:
        flSimParameters.variantConfigUrn
          = falaise::properties::getValueOrDefault<std::string>(variantSubsystem,
                                                                "configUrn",
                                                                flSimParameters.variantConfigUrn);

        // Variant configuration:
        if (flSimParameters.userProfile == "production" && variantSubsystem.has_key("config")) {
          DT_THROW(FLConfigUserError,
                   "User profile '" << flSimParameters.userProfile << "' "
                   << "does not allow to use the '" << "config" << "' variants configuration parameter!");
        }
        flSimParameters.variantSubsystemParams.config_filename
          = falaise::properties::getValueOrDefault<std::string>(variantSubsystem,
                                                                "config",
                                                                flSimParameters.variantSubsystemParams.config_filename);

        // Variant profile URN:
        flSimParameters.variantProfileUrn
          = falaise::properties::getValueOrDefault<std::string>(variantSubsystem,
                                                                "profileUrn",
                                                                flSimParameters.variantProfileUrn);


        // Variant profile:
        flSimParameters.variantSubsystemParams.profile_load
          = falaise::properties::getValueOrDefault<std::string>(variantSubsystem,
                                                                "profile",
                                                                flSimParameters.variantSubsystemParams.profile_load);

        // Variant settings:
        if (flSimParameters.userProfile == "production" && variantSubsystem.has_key("settings")) {
          DT_THROW(FLConfigUserError,
                   "User profile '" << flSimParameters.userProfile << "' "
                   << "does not allow to use the '" << "settings" << "' variants configuration parameter!");
        }
        flSimParameters.variantSubsystemParams.settings
          = falaise::properties::getValueOrDefault<std::vector<std::string> >(variantSubsystem,
                                                                              "settings",
                                                                              flSimParameters.variantSubsystemParams.settings);
      }

      // Services subsystem:
      if (flSimConfig.has_key_with_meta("flsimulate.services", "flsimulate::section")) {
        datatools::properties servicesSubsystem = flSimConfig.get_section("flsimulate.services");

        // Services manager configuration URN:
        flSimParameters.servicesSubsystemConfigUrn =
          falaise::properties::getValueOrDefault<std::string>(servicesSubsystem,
                                                              "configUrn",
                                                              flSimParameters.servicesSubsystemConfigUrn);

        // Services manager main configuration file:
        if (flSimParameters.userProfile == "production" && servicesSubsystem.has_key("config")) {
          DT_THROW(FLConfigUserError,
                   "User profile '" << flSimParameters.userProfile << "' "
                   << "does not allow to use the '" << "config" << "' services configuration parameter!");
        }
        flSimParameters.servicesSubsystemConfig =
          falaise::properties::getValueOrDefault<std::string>(servicesSubsystem,
                                                              "config",
                                                              flSimParameters.servicesSubsystemConfig);
      }

    } // !args.configScript.empty()

    do_postprocess(flSimParameters);
    return;
  }


  void do_postprocess(FLSimulateArgs & flSimParameters)
  {
    DT_LOG_TRACE_ENTERING(flSimParameters.logLevel);
    // datatools::kernel & dtk = datatools::kernel::instance();
    // const datatools::urn_query_service & dtkUrnQuery = dtk.get_urn_query();
    falaise::configuration_db cfgdb;

    if (flSimParameters.simulationManagerParams.input_prng_seeds_file.empty()) {
      if (!flSimParameters.saveRngSeeding &&
          flSimParameters.simulationManagerParams.output_prng_seeds_file.empty()) {
        // Make sure PRNG seeds are stored in a default log file if
        // seeds are not stored in metadata:
        flSimParameters.simulationManagerParams.output_prng_seeds_file
          = FLSimulateArgs::default_file_for_seeds();
      }
    }

    // Propagate verbosity to variant service:
    flSimParameters.variantSubsystemParams.logging = datatools::logger::get_priority_label(flSimParameters.logLevel);

    if (flSimParameters.simulationSetupUrn.empty()) {

      // Check for hardcoded path to the main simulation setup configuration file:
      if (! flSimParameters.simulationManagerParams.manager_config_filename.empty()) {
        // Only for 'expert' of 'normal' user profiles.

        // Variant configuration:
        if (flSimParameters.variantSubsystemParams.config_filename.empty()) {
          DT_LOG_WARNING(flSimParameters.logLevel, "No variant configuration file is provided!");
        }

        // Services configuration:
        if (flSimParameters.servicesSubsystemConfig.empty()) {
          DT_LOG_WARNING(flSimParameters.logLevel, "No services configuration file is provided!");
        }

      } else {
        DT_THROW(std::logic_error, "Missing simulation setup configuration file!");
      }
    }

    if (!flSimParameters.simulationSetupUrn.empty()) {
      // Check URN registration from the configuration DB:
      {
        DT_THROW_IF(!cfgdb.check_with_category(flSimParameters.simulationSetupUrn,
                                               falaise::configuration_db::category::simulation_setup_label()),
                    std::logic_error,
                    "Cannot find simulation setup URN='" << flSimParameters.simulationSetupUrn << "'!");
      }

      if (flSimParameters.experimentalSetupUrn.empty()) {
        // Automatically determine the experimental setup component associated to this simulation setup:
        DT_THROW_IF(!cfgdb.find_direct_unique_dependee_with_category_from(flSimParameters.experimentalSetupUrn,
                                                                          flSimParameters.simulationSetupUrn,
                                                                          falaise::configuration_db::category::experimental_setup_label()),
                    std::logic_error,
                    "Cannot query unique experimental setup URN from '" << flSimParameters.simulationSetupUrn << "'!");
      }

      // Simulation:
      {
        // Resolve the path of the main simulation configuration file:
        std::string conf_simu_category = "configuration";
        std::string conf_simu_mime;
        std::string conf_simu_path;
        DT_THROW_IF(!cfgdb.resolve(flSimParameters.simulationSetupUrn,
                                   conf_simu_category,
                                   conf_simu_mime,
                                   conf_simu_path),
                    std::logic_error,
                    "Cannot resolve URN='" << flSimParameters.simulationSetupUrn << "'!");
        flSimParameters.simulationManagerParams.manager_config_filename = conf_simu_path;
      }

      // Variant setup:
      if (flSimParameters.variantConfigUrn.empty()) {
        // Automatically determine the variants configuration component:
        DT_THROW_IF(!cfgdb.find_direct_unique_dependee_with_category_from(flSimParameters.variantConfigUrn,
                                                                          flSimParameters.simulationSetupUrn,
                                                                          falaise::configuration_db::category::variants_service_label()),
                    std::logic_error,
                    "Cannot find unique variant setup URN from '" << flSimParameters.simulationSetupUrn << "'!");
      }
      if (!flSimParameters.variantConfigUrn.empty()) {
        // Check URN registration from the system URN query service:
        {
          DT_THROW_IF(!cfgdb.check_with_category(flSimParameters.variantConfigUrn,
                                                 falaise::configuration_db::category::variants_service_label()),
                      std::logic_error,
                      "Cannot find variant setup URN='" << flSimParameters.variantConfigUrn << "'!");
        }

        // Resolve variant configuration file:
        std::string conf_variants_category = "configuration";
        std::string conf_variants_mime;
        std::string conf_variants_path;
        DT_THROW_IF(!cfgdb.resolve(flSimParameters.variantConfigUrn,
                                   conf_variants_category,
                                   conf_variants_mime,
                                   conf_variants_path),
                    std::logic_error,
                    "Cannot resolve URN='" << flSimParameters.variantConfigUrn << "'!");
        flSimParameters.variantSubsystemParams.config_filename = conf_variants_path;
      }

      // Services:
      if (!flSimParameters.servicesSubsystemConfig.empty()) {
        // Force the services configuration path:
        DT_THROW_IF(!flSimParameters.servicesSubsystemConfigUrn.empty(),
                    std::logic_error,
                    "Service configuration URN='" << flSimParameters.servicesSubsystemConfigUrn << "' conflicts with services configuration path='"
                    << flSimParameters.servicesSubsystemConfig << "'!");
      } else {
        if (flSimParameters.servicesSubsystemConfigUrn.empty()) {
          // Automatically determine the variants configuration component:
          DT_THROW_IF(!cfgdb.find_direct_unique_dependee_with_category_from(flSimParameters.servicesSubsystemConfigUrn,
                                                                            flSimParameters.simulationSetupUrn,
                                                                            falaise::configuration_db::category::services_manager_label()),
                      std::logic_error,
                      "Cannot find unique service setup URN from '" << flSimParameters.simulationSetupUrn << "'!");
        }

        if (!flSimParameters.servicesSubsystemConfigUrn.empty()) {
          // Resolve path of the service manager configuration file:
          std::string conf_services_category = "configuration";
          std::string conf_services_mime;
          std::string conf_services_path;
          DT_THROW_IF(!cfgdb.resolve(flSimParameters.servicesSubsystemConfigUrn,
                                     conf_services_category,
                                     conf_services_mime,
                                     conf_services_path),
                      std::logic_error,
                      "Cannot resolve URN='" << flSimParameters.servicesSubsystemConfigUrn << "'!");
          flSimParameters.servicesSubsystemConfig = conf_services_path;
        }
      }
    } // !flSimParameters.simulationSetupUrn.empty()

    if (!flSimParameters.variantSubsystemParams.profile_load.empty()) {
      // Check the variant config path:
      DT_THROW_IF(flSimParameters.variantSubsystemParams.config_filename.empty(),
                  std::logic_error,
                  "Variant configuration file='" << flSimParameters.variantSubsystemParams.config_filename << "' is missing!");
      // Force the variant profile path:
      DT_THROW_IF(!flSimParameters.variantProfileUrn.empty(),
                  std::logic_error,
                  "Variant profile URN='" << flSimParameters.variantProfileUrn << "' conflicts with variant profile path='"
                  << flSimParameters.variantSubsystemParams.profile_load << "'!");
    } else {
      DT_LOG_DEBUG(flSimParameters.logLevel, "No variant profile path is set.");
      if (flSimParameters.variantProfileUrn.empty()) {
        // No variant profile URN is set:
        DT_LOG_DEBUG(flSimParameters.logLevel, "No variant profile URN is set.");
        // We try to find a default one from the list of variant profiles attached on the variant service:
        std::set<std::string> varprofile_dependers;
        // First we extract this list of depender variant profiles:
        if (cfgdb.find_direct_dependers_with_category_from(varprofile_dependers,
                                                           flSimParameters.variantConfigUrn,
                                                           falaise::configuration_db::category::variants_profile_label())) {
          // Scan dependers variant profiles and search for one which has an alias dependee used as the default one:
          for (auto vp : varprofile_dependers) {
            std::string vdpef;
            if (cfgdb.find_direct_unique_depender_with_category_from(vdpef,
                                                                     vp,
                                                                     falaise::configuration_db::category::variants_profile_label())) {
              if (cfgdb.is_alias_of(vdpef, vp)) {
                if (boost::algorithm::ends_with(vdpef,
                                                falaise::configuration_db::category::default_urn_suffix())) {
                  // Pickup the real URN, not the alias:
                  flSimParameters.variantProfileUrn = vp;
                  // std::cerr << "[devel] flSimParameters.variantProfileUrn = " << flSimParameters.variantProfileUrn << std::endl;
                  break;
                }
              }
            }
          }
        }
      }
      if (!flSimParameters.variantProfileUrn.empty()) {
        // Determine the variant profile path from a blessed variant profile URN:
        std::string conf_variantsProfile_category = "configuration";
        std::string conf_variantsProfile_mime;
        std::string conf_variantsProfile_path;
        DT_THROW_IF(!cfgdb.resolve(flSimParameters.variantProfileUrn,
                                   conf_variantsProfile_category,
                                   conf_variantsProfile_mime,
                                   conf_variantsProfile_path),
                    std::logic_error,
                    "Cannot resolve variant profile URN='" << flSimParameters.variantProfileUrn << "'!");
        flSimParameters.variantSubsystemParams.profile_load = conf_variantsProfile_path;
      }
    }

    if (flSimParameters.variantSubsystemParams.config_filename.empty()) {
      DT_LOG_WARNING(flSimParameters.logLevel, "No variant configuration is provided.");
    } else {
      if (flSimParameters.variantSubsystemParams.profile_load.empty()) {
        DT_LOG_WARNING(flSimParameters.logLevel, "No variant profile is provided.");
      } else {
        // Additional variants settings may be allowed but *must* be compatible
        // with selected variants config and optional variants profile.
      }
    }

    if (flSimParameters.servicesSubsystemConfig.empty()) {
      DT_LOG_WARNING(flSimParameters.logLevel, "No services configuration is provided.");
    }

    // Print:
    if (datatools::logger::is_debug(flSimParameters.logLevel)) {
      flSimParameters.print(std::cerr);
    }

    DT_LOG_TRACE_EXITING(flSimParameters.logLevel);
    return;
  }

  void FLSimulateArgs::print(std::ostream & out_) const
  {
    static const std::string tag("|-- ");
    static const std::string last_tag("`-- ");
    out_ << "FLSimulate setup parameters: " << std::endl;
    out_ << tag << "logLevel                   = " << datatools::logger::get_priority_label(this->logLevel) << std::endl;
    out_ << tag << "userProfile                = " << userProfile << std::endl;
    out_ << tag << "numberOfEvents             = " << numberOfEvents << std::endl;
    out_ << tag << "doSimulation               = " << std::boolalpha << doSimulation << std::endl;
    out_ << tag << "doDigitization             = " << std::boolalpha << doDigitization << std::endl;
    out_ << tag << "experimentalSetupUrn       = " << experimentalSetupUrn << std::endl;
    out_ << tag << "simulationSetupUrn         = " << simulationSetupUrn << std::endl;
    out_ << tag << "simulationSetupConfig      = " << simulationManagerParams.manager_config_filename << std::endl;
    out_ << tag << "saveRngSeeding             = " << std::boolalpha << saveRngSeeding << std::endl;
    out_ << tag << "rngSeeding                 = " << rngSeeding << std::endl;
    out_ << tag << "digitizationSetupUrn       = " << (digitizationSetupUrn.empty() ? "<not used>" : digitizationSetupUrn) << std::endl;
    out_ << tag << "variantConfigUrn           = " << variantConfigUrn << std::endl;
    out_ << tag << "variantProfileUrn          = " << variantProfileUrn << std::endl;
    out_ << tag << "variantSubsystemParams     = " << variantSubsystemParams.config_filename << std::endl;
    out_ << tag << "saveVariantSettings        = " << std::boolalpha << saveVariantSettings << std::endl;
    out_ << tag << "servicesSubsystemConfigUrn = " << servicesSubsystemConfigUrn << std::endl;
    out_ << tag << "servicesSubsystemConfig    = " << servicesSubsystemConfig << std::endl;
    out_ << tag << "outputMetadataFile         = " << outputMetadataFile << std::endl;
    out_ << tag << "embeddedMetadata           = " << std::boolalpha << embeddedMetadata << std::endl;
    out_ << last_tag << "outputFile                 = " << outputFile << std::endl;
    return;
  }

} // namespace FLSimulate
