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
	uint64_t registerWrite;
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
	uint64_t stat_disambiguation_read_false_positive;
	uint64_t stat_disambiguation_write_false_positive;
	uint64_t stat_address_to_address;
	uint64_t times_reach_parallel_requests_read;
	uint64_t times_reach_parallel_requests_write;
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
	// ====================================================================
	/// EMC Attributes
	// ====================================================================
	uint32_t llc_miss_rob_head; //tracks number of times llc miss is rob head

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

		uint32_t mob_read();
		uint32_t mob_write();
		void commit();
		// ====================================================================
		void printStructures();
		// ====================================================================
		// Bool Functions @return 
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
		desambiguation_t *desambiguator;
		// ======================
		//READ
		// ======================
		memory_order_buffer_line_t *memory_order_buffer_read;
        uint32_t memory_order_buffer_read_start;
        uint32_t memory_order_buffer_read_end;
        uint32_t memory_order_buffer_read_used;
		memory_order_buffer_line_t* get_next_op_load();
		// Pointers to retain oldests memory operations
		memory_order_buffer_line_t *oldest_read_to_send;
		// ======================
		//WRITE
		// ======================
		memory_order_buffer_line_t *memory_order_buffer_write;
		uint32_t memory_order_buffer_write_start;
        uint32_t memory_order_buffer_write_end;
        uint32_t memory_order_buffer_write_used;
		memory_order_buffer_line_t* get_next_op_store();
		// Pointers to retain oldests memory operations
		memory_order_buffer_line_t *oldest_write_to_send;
		// ======================
		// Parallel requests
		uint32_t counter_mshr_read;
		uint32_t counter_mshr_write;
		int32_t request_DRAM;
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
		INSTANTIATE_GET_SET_ADD(uint64_t,registerWrite);
		/////
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_full_FetchBuffer);
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_wrong_branch);
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_full_DecodeBuffer);
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_full_MOB_Read);
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_full_MOB_Write);
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_full_ROB);
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_empty_RS);
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_disambiguation_read_false_positive);
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_disambiguation_write_false_positive);
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_address_to_address);
		INSTANTIATE_GET_SET_ADD(uint64_t,times_reach_parallel_requests_read);
		INSTANTIATE_GET_SET_ADD(uint64_t,times_reach_parallel_requests_write);
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
		// EMC Methods and attr
		// ====================================================================
		//  ATTR
		bool has_llc_miss; // have a LLC Miss, verify if is ROB head to add ROB Head on buffer
		bool start_emc_module;//if must start generate dep chain
		bool lock_processor; // lock commit para não deixar completar qualquer instrução antes de o EMC parar de excutar, evitando modificar as estruturas
		int8_t counter_activate_emc;
		// ====================================================================
		uint32_t counter_ambiguation_read;
		uint32_t counter_ambiguation_write;
		// ====================================================================
		container_ptr_reorder_buffer_line_t rob_buffer; // Wait list to propagate registers;
		register_remapping_table_t *rrt;
		uint8_t counter_make_dep_chain;
		// Statistics
		// ====================================================================
		uint32_t instrucoes_inter_load_deps;
		uint32_t soma_instrucoes_deps;
		uint32_t numero_load_deps;
		uint32_t cancel_emc_execution;
		uint32_t started_emc_execution;
		// ====================================================================
		//  Methods
		// void
		void clean_rrt();
		void cancel_execution_emc(); //reverte status dos uops ao core
		void make_dependence_chain(reorder_buffer_line_t* rob_line); //generate dep chain
		void update_counter_emc(int32_t value);
		// boolean
		bool isRobHead(reorder_buffer_line_t* robEntry);//verify if rob entry is rob read
		bool verify_spill_register(reorder_buffer_line_t* rob_line);// Verifying register spill to include store ops on chain
		// =================================================================================
		// Funcoes auxiliares para EMC
		// =================================================================================
		bool verify_dependent_loads(); // verifica se ha loads dependentes -> para execucao
		bool already_exists(reorder_buffer_line_t *candidate);//verifica se já existe a instrução na cadeia
		uint32_t count_registers_rrt(uop_package_t uop);
		bool verify_ambiguation(memory_order_buffer_line_t *mob_line);
		// =================================================================================
		// integer
		int32_t renameEMC(reorder_buffer_line_t *rob_line);//Renaming entry to EMC
		uint32_t broadcast_cdb(uint32_t position_rob,int32_t write_register);//broadcast destiny registers on ROB to pseudo wake up operations.
		uint32_t get_position_rob_bcast(reorder_buffer_line_t *rob_ready);// this function add the operat0in in rob buffer only. 
		// =================================================================================
		int32_t get_next_uop_dependence();

		// =================================================================================




		int32_t search_register(int32_t write_register);//Register remapping table declaration
    	int32_t allocate_new_register(int32_t write_register);
		// ====================================================================
		INSTANTIATE_GET_SET_ADD(uint32_t,llc_miss_rob_head);
		INSTANTIATE_GET_SET_ADD(uint32_t,cancel_emc_execution);
		INSTANTIATE_GET_SET_ADD(uint32_t,started_emc_execution);
		INSTANTIATE_GET_SET_ADD(uint32_t,counter_ambiguation_read);
		INSTANTIATE_GET_SET_ADD(uint32_t,counter_ambiguation_write);
};
