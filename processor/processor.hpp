// ============================================================================
// ============================================================================
class processor_t {
    private:    
    uint64_t stallFetch;
    uint64_t stallDecode;
    
    public:
		
		// ====================================================================
		/// Attributes
		// ====================================================================
		//control Branches
		bool hasBranch;
		opcode_package_t previousBranch;
		//error at insert fetch buffer
		bool insertError;
		opcode_package_t opcodeError;
		// ====================================================================
		// Control
		// ====================================================================
		bool traceIsOver;
		uint64_t fetchCounter;
		uint64_t decodeCounter;
		uint64_t uopCounter;
		// ====================================================================
		/// Methods
		// ====================================================================
		processor_t();
		~processor_t();
	    void allocate();
	    void clock();
		void statistics();
		// ====================================================================
		// Stage Methods
		// ====================================================================
		void fetch();
		void decode();
		void rename();
		void dispatch();
		void execute();
		void commit();
		bool isBusy();
		// ====================================================================
		// Structures
		// ====================================================================
		//decodeBuffer
		circular_buffer_t<uop_package_t> decodeBuffer;
		circular_buffer_t<opcode_package_t> fetchBuffer;
		
		//RAT
		// ROB
		// MOB
		// ====================================================================
		// Statistics
		// ====================================================================
		INSTANTIATE_GET_SET_ADD(uint64_t,stallFetch);
		INSTANTIATE_GET_SET_ADD(uint64_t,stallDecode);

};
