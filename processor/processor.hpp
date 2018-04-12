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
	uint64_t stat_inst_int_alu_completed;
	uint64_t stat_inst_mul_alu_completed;
	uint64_t stat_inst_div_alu_completed;
	uint64_t stat_inst_int_fp_completed;
	uint64_t stat_inst_mul_fp_completed;
	uint64_t stat_inst_div_fp_completed;
	uint64_t stat_inst_nop_completed;
	uint64_t stat_inst_load_completed;
	uint64_t stat_inst_store_completed;
	uint64_t stat_inst_branch_completed;
	uint64_t stat_inst_other_completed;
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
		uint64_t commit_uop_counter;
		uint32_t memory_read_executed;
		uint32_t memory_read_received;
		uint32_t memory_write_executed;
		
		// ====================================================================
		/// Methods
		// ====================================================================
		processor_t();
		~processor_t();
	    void allocate();
	    void clock();
		void statistics();
		void printConfiguration();
		// ====================================================================
		// ROB RELATED	
		void update_registers(reorder_buffer_line_t *robLine);
		void solve_registers_dependency(reorder_buffer_line_t *rob_line);
		int32_t searchPositionROB();
		void removeFrontROB();
		// ====================================================================
		// MOB READ RELATED
		int32_t search_position_mob_read();
		void remove_front_mob_read();	
		// ====================================================================
		// MOB WRITE RELATED
		int32_t search_position_mob_write();
		void remove_front_mob_write();
		// ====================================================================
		// Stage Methods
		// ====================================================================
		void fetch();
		void decode();
		void rename();
		void dispatch();
		void execute();
		void mob_read();
		void mob_write();
		void commit();
		// ====================================================================
		bool isBusy();
		void printStructures();
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
		//READ
		// memory_order_buffer_line_t *memory_order_buffer_read;
        // uint32_t memory_order_buffer_read_start;
        // uint32_t memory_order_buffer_read_end;
        // uint32_t memory_order_buffer_read_used;
		std::list<memory_order_buffer_line_t> memory_order_buffer_read; 
		//WRITE
		// memory_order_buffer_line_t *memory_order_buffer_write;
		// uint32_t memory_order_buffer_write_start;
        // uint32_t memory_order_buffer_write_end;
        // uint32_t memory_order_buffer_write_used;
		std::list<memory_order_buffer_line_t> memory_order_buffer_write; 
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
		// Statistics inst completed
		// ====================================================================
		

		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_branch_completed);
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_div_alu_completed);
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_div_fp_completed);
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_int_alu_completed);
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_int_fp_completed);
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_mul_alu_completed);
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_mul_fp_completed);
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_load_completed);
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_store_completed);
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_nop_completed);
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_other_completed);

		// ====================================================================
		// Compare methods
		// ====================================================================
		bool inline cmp_fetch_block(uint64_t addressA,uint64_t addressB){
			return ((addressA >> OFFSET_SIZE)==(addressB >> OFFSET_SIZE));
		}
		// ====================================================================
		// Debug AID
		// ====================================================================
		void solve_for_debug();
		bool front_read_executed;
		bool front_write_executed;

};
