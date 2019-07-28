#include <stdio.h>
#include <getopt.h>
#include <defaultoptions.h>
#include "convert-manifest.h"

static void print_usage(const char *command)
{
    printf("Usage: %s -i infrastructure_nix [OPTION] [MANIFEST]\n\n", command);

    puts(
    "The command `disnix-convert-manifest' is used to convert a manifest using the\n"
    "old V1 schema into the V2 format.\n\n"

    "Options:\n"
    "  -i, --infrastructure=infrastructure_nix\n"
    "                              Infrastructure Nix expression which captures\n"
    "                              properties of machines in the network\n"
    "      --interface=INTERFACE   Path to executable that communicates with a Disnix\n"
    "                              interface. Defaults to `disnix-ssh-client'\n"
    "      --target-property=PROP  The target property of an infrastructure model,\n"
    "                              that specifies how to connect to the remote Disnix\n"
    "                              interface. (Defaults to: hostname)\n"
    "  -p, --profile=PROFILE       Name of the profile in which the services are\n"
    "                              registered. Defaults to: default\n"
    "      --coordinator-profile-path=PATH\n"
    "                              Path to the manifest of the previous\n"
    "                              configuration. By default this tool will use the\n"
    "                              manifest stored in the disnix coordinator profile\n"
    "                              instead of the specified one, which is usually\n"
    "                              sufficient in most cases.\n"
    "  -h, --help                  Shows the usage of this command to the user\n"

    "\nEnvironment:\n"
    "  DISNIX_CLIENT_INTERFACE    Sets the client interface (which defaults to\n"
    "                             `disnix-ssh-client')\n"
    "  DISNIX_TARGET_PROPERTY     Specifies which property in the infrastructure Nix\n"
    "                             expression specifies how to connect to the remote\n"
    "                             interface (defaults to: hostname)\n"
    "  DISNIX_PROFILE             Sets the name of the profile that stores the\n"
    "                             manifest on the coordinator machine and the\n"
    "                             deployed services per machine on each target\n"
    "                             (Defaults to: default)\n"
    );
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"infrastructure", required_argument, 0, DISNIX_OPTION_INFRASTRUCTURE},
        {"coordinator-profile-path", required_argument, 0, DISNIX_OPTION_COORDINATOR_PROFILE_PATH},
        {"interface", required_argument, 0, DISNIX_OPTION_INTERFACE},
        {"target-property", required_argument, 0, DISNIX_OPTION_TARGET_PROPERTY},
        {"profile", required_argument, 0, DISNIX_OPTION_PROFILE},
        {"help", no_argument, 0, DISNIX_OPTION_HELP},
        {"version", no_argument, 0, DISNIX_OPTION_VERSION},
        {0, 0, 0, 0}
    };
    char *infrastructure_file = NULL;
    char *interface = NULL;
    char *target_property = NULL;
    char *profile = NULL;
    char *manifest_file = NULL;
    char *coordinator_profile_path = NULL;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "p:i:m:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case DISNIX_OPTION_INFRASTRUCTURE:
                infrastructure_file = optarg;
                break;
            case DISNIX_OPTION_COORDINATOR_PROFILE_PATH:
                coordinator_profile_path = optarg;
                break;
            case DISNIX_OPTION_INTERFACE:
                interface = optarg;
                break;
            case DISNIX_OPTION_TARGET_PROPERTY:
                target_property = optarg;
                break;
            case DISNIX_OPTION_PROFILE:
                profile = optarg;
                break;
            case DISNIX_OPTION_HELP:
                print_usage(argv[0]);
                return 0;
            case DISNIX_OPTION_VERSION:
                print_version(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    /* Validate options */

    interface = check_interface_option(interface);
    target_property = check_target_property_option(target_property);
    profile = check_profile_option(profile);

    if(infrastructure_file == NULL)
    {
        fprintf(stderr, "An infrastructure model is required!\n");
        return 1;
    }

    if(optind >= argc)
        manifest_file = NULL;
    else
        manifest_file = argv[optind];

    /* Execute conversion */
    return convert_manifest_file(manifest_file, infrastructure_file, coordinator_profile_path, profile, target_property, interface);
}
