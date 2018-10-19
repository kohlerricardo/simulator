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
std::string get_status_execution(){
    std::string final_report;
    char report[1000];
    //Benchmark name
 snprintf(report,sizeof(report),"%s","==========================================================================\n");
    final_report+=report;
 snprintf(report,sizeof(report),"Benchmark %s\n",orcs_engine.arg_trace_file_name);
    final_report+=report;
    // get actual cicle
 snprintf(report,sizeof(report),"Actual Cycle %lu\n",orcs_engine.get_global_cycle());
    final_report+=report;

    // Get  status opcodes total, executed -> calculate percent 
    uint64_t total_opcodes = orcs_engine.trace_reader->get_trace_opcode_max();
    uint64_t fetched_opcodes = orcs_engine.trace_reader->get_fetch_instructions();
    snprintf(report,sizeof(report),"opcodes Processed: %lu of %lu\n",fetched_opcodes,total_opcodes);
    final_report+=report;
    ////////
    double percentage_complete = 100.0 * (static_cast<double>(fetched_opcodes) / static_cast<double>(total_opcodes));
    //    
    snprintf(report,sizeof(report),"Total Progress %8.4lf%%\n",percentage_complete);
    final_report+=report;
    // IPC parcial
    snprintf(report,sizeof(report)," IPC(%5.3lf)\n", static_cast<double>(fetched_opcodes) / static_cast<double>(orcs_engine.get_global_cycle()));
    final_report+=report;
    //get time of execution
    gettimeofday(&orcs_engine.stat_timer_end, NULL);
    double seconds_spent = orcs_engine.stat_timer_end.tv_sec - orcs_engine.stat_timer_start.tv_sec;
    
    double seconds_remaining = (100*(seconds_spent / percentage_complete)) - seconds_spent;
        snprintf(report,sizeof(report), "ETC(%02.0f:%02.0f:%02.0f)\n",
                                                floor(seconds_remaining / 3600.0),
                                                floor(fmod(seconds_remaining, 3600.0) / 60.0),
                                                fmod(seconds_remaining, 60.0));

    final_report+=report;
   snprintf(report,sizeof(report),"%s","==========================================================================\n");
    final_report+=report;
    return final_report;
}

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
    //Memory Controller
    //==================
    orcs_engine.memory_controller->allocate();
    //initializate simulator
    orcs_engine.simulator_alive = true;


    /// Start CLOCK for all the components
    while (orcs_engine.simulator_alive) {
        #if HEARTBEAT
            if(orcs_engine.get_global_cycle()%HEARTBEAT_CLOCKS==0){
                ORCS_PRINTF("%s\n",get_status_execution().c_str())
            }
        #endif
        orcs_engine.memory_controller->clock();
        orcs_engine.processor->clock();
        
        orcs_engine.global_cycle++;
    }
    // to be prinf
	ORCS_PRINTF("End of Simulation\n")
	orcs_engine.trace_reader->statistics();
    orcs_engine.processor->statistics();
    orcs_engine.branchPredictor->statistics();
    orcs_engine.cacheManager->statistics();
    orcs_engine.memory_controller->statistics();

    ORCS_PRINTF("Deleting Processor\n")
    delete orcs_engine.processor;
    ORCS_PRINTF("Deleting Branch predictor\n")
    delete orcs_engine.branchPredictor;
    ORCS_PRINTF("Deleting Cache manager\n")
    delete orcs_engine.cacheManager;
    ORCS_PRINTF("Deleting Memory Controller\n")
    delete orcs_engine.memory_controller;
    
    return(EXIT_SUCCESS);
};
