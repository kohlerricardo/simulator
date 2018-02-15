// ============================================================================
// ============================================================================
class processor_t {
    private:    
    
    
    public:

		// ====================================================================
		/// Methods
		// ====================================================================
		
		processor_t();
	    void allocate();
	    void clock();
		void statistics();

		// BTB attribute
		btb_t *btb;
		// btb_t *btb2Bit;
		//BTB Methods
		void updateLruAll(uint32_t add);
		void replaceBTB(uint32_t address);
		uint32_t searchLine(uint32_t pc);
		uint32_t installLine(opcode_package_t instruction);
		inline uint32_t searchLru(btb_t *btb);
		// Statistics values
		int32_t btbHits;
		int32_t btbMiss;
		int32_t branchTaken;
		int32_t branchNotTaken;
		int32_t branches;
		int32_t BtMiss;
		int32_t BntMiss;
		
		//others
		int32_t index;
		int32_t assoc;
		int32_t has_branch;
		uint32_t nextInstruction;
		//plbt
		int32_t predict;
		int32_t oldAdd;
		//Methods to interact with cache
		uint32_t statusCache;
		void searchCache(uint64_t address);
		void writeCache(uint64_t address);
		
};
