// ============================================================================
class orcs_engine_t {
	private:


    public:
        /// Program input
        char *arg_trace_file_name;
        /// Control the Global Cycle
        uint64_t global_cycle;

        bool simulator_alive;

        /// Components modeled
        trace_reader_t *trace_reader;
        //==================
        //Processor
        //==================
        processor_t *processor;
        //==================
        //Branch Predictor
        //==================
        branch_predictor_t *branchPredictor;
        //cache
        // cache_t *cache;
        // ====================================================================
		/// Methods
		// ====================================================================
		orcs_engine_t();
		void allocate();
        uint64_t get_global_cycle() {
            return this->global_cycle;
        };
};
