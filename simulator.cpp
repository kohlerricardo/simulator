#include "./simulator.hpp"

orcs_engine_t orcs_engine;

// =============================================================================
static void display_use() {
    ORCS_PRINTF("**** OrCS - Ordinary Computer Simulator ****\n\n");
    ORCS_PRINTF("Please provide -t <trace_file_basename> -f <output filename>\n");
};

// =============================================================================
static void process_argv(int argc, char **argv) {

    // Name, {no_argument, required_argument and optional_argument}, flag, value
    static struct option long_options[] = {
        {"help",        no_argument, 0, 'h'},
        {"trace",       required_argument, 0, 't'},
        {"output_filename",       optional_argument, 0, 'f'},
        {NULL,          0, NULL, 0}
    };

    // Count number of traces
    int opt;
    int option_index = 0;
    while ((opt = getopt_long_only(argc, argv, "h:t:f:",
                 long_options, &option_index)) != -1) {
        switch (opt) {
        case 0:
            printf ("Option %s", long_options[option_index].name);
            if (optarg)
                printf (" with arg %s", optarg);
            printf ("\n");
            break;

        case 'h':
            display_use();
            break;

        case 't':
            orcs_engine.arg_trace_file_name = optarg;
            break;
        case 'f':
            orcs_engine.output_file_name = optarg;
            break;
        case '?':
            break;

        default:
            ORCS_PRINTF(">> getopt returned character code 0%o ??\n", opt);
        }
    }

    if (optind < argc) {
        ORCS_PRINTF("Non-option ARGV-elements: ");
        while (optind < argc)
            ORCS_PRINTF("%s ", argv[optind++]);
        ORCS_PRINTF("\n");
    }


    if (orcs_engine.arg_trace_file_name == NULL) {
        ORCS_PRINTF("Trace file not defined.\n");
        display_use();
    }

};


// =============================================================================
int main(int argc, char **argv) {
    process_argv(argc, argv);

    /// Call all the allocate's
    orcs_engine.allocate();
    orcs_engine.trace_reader->allocate(orcs_engine.arg_trace_file_name);
    //==================
    //Processor
    //==================
    orcs_engine.processor->allocate();
    //==================
    //Branch Predictor
    //==================
    orcs_engine.branchPredictor->allocate();
    //==================
    //Cache Manager
    //==================
    orcs_engine.cacheManager->allocate();
    //==================
    //Enhaced Memory Controller
    //==================

    //initializate simulator
    orcs_engine.simulator_alive = true;


    /// Start CLOCK for all the components
    while (orcs_engine.simulator_alive) {
        orcs_engine.processor->clock();
        orcs_engine.global_cycle++;
    }
    // to be prinf
	ORCS_PRINTF("End of Simulation\n")
	orcs_engine.trace_reader->statistics();
    orcs_engine.processor->statistics();
    orcs_engine.branchPredictor->statistics();
    orcs_engine.cacheManager->statistics();

    delete orcs_engine.processor;
    delete orcs_engine.branchPredictor;
    delete orcs_engine.cacheManager;
    
    return(EXIT_SUCCESS);
};
