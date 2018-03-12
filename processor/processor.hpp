// ============================================================================
// ============================================================================
class processor_t {
    private:    
	//=============
	//Fetch Related
	//=============
    uint64_t stall_full_FetchBuffer;
    uint64_t stall_wrong_branch;
	//=============
	//Statistics Decode
	//=============
    uint64_t stall_full_DecodeBuffer;
	//=============
	//Statistics Rename
	//=============
	uint64_t stall_full_MOB_Read;
	uint64_t stall_full_MOB_Write;
	uint64_t stall_full_ROB;
	//=============
	//Statistics Dispatch
	//=============
	uint64_t stall_empty_RS;
	//=============
	//Statistics Execute
	//=============
	//=============
	//Statistics Commit
	//=============
    
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
		uint64_t renameCounter;
		uint64_t uopCounter;
		// ====================================================================
		/// Methods
		// ====================================================================
		processor_t();
		~processor_t();
	    void allocate();
	    void clock();
		void statistics();
		void update_registers(reorder_buffer_line_t *robLine);
		int32_t searchPositionROB();
		void removeFrontROB();
		// ====================================================================
		// Stage Methods
		// ====================================================================
		void fetch();
		void decode();
		void rename();
		void dispatch();
		void execute();
		void mob();
		void commit();
		bool isBusy();
		// ====================================================================
		// Structures
		// ====================================================================
		// =======================
		// Buffers
		// =======================
		circular_buffer_t<uop_package_t> decodeBuffer;
		circular_buffer_t<opcode_package_t> fetchBuffer;
		
		// =======================
		// Register Alias Table - RAT
		// =======================
		reorder_buffer_line_t **register_alias_table;
		// =======================
		// Reorder Buffer
		// =======================
        reorder_buffer_line_t *reorderBuffer;
        uint32_t robStart;
        uint32_t robEnd;
        uint32_t robUsed;

		// ======================
		// Memory Order Buffer
		// ======================
		memory_order_buffer_line_t *memory_order_buffer_read;
		memory_order_buffer_line_t *memory_order_buffer_write;
		// ======================
		//Reservation Station 
		container_ptr_reorder_buffer_line_t unified_reservation_station;
		// ====================== 
		// ======================
		// Funcional Unitis - FUs
		// ======================
		// Integer FUs
		uint64_t *fu_int_alu;
		uint64_t *fu_int_mul;
		uint64_t *fu_int_div;
		// Floating Points FUs
		uint64_t *fu_fp_alu;
		uint64_t *fu_fp_mul;
		uint64_t *fu_fp_div;
		// Memory FUs
		uint64_t *fu_mem_load;
		uint64_t *fu_mem_store;
		//container to accelerate  execution
		container_ptr_reorder_buffer_line_t unified_functional_units;

		// ====================================================================
		// Statistics
		// ====================================================================
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_full_FetchBuffer);
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_wrong_branch);
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_full_DecodeBuffer);
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_full_MOB_Read);
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_full_MOB_Write);
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_full_ROB);
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_empty_RS);
		// ====================================================================
		// Compare methods
		// ====================================================================
		bool inline cmp_fetch_block(uint64_t addressA,uint64_t addressB){
			return ((addressA >> OFFSET_SIZE)==(addressB >> OFFSET_SIZE));
		}
};
