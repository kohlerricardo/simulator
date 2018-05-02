#include "./../simulator.hpp"
#include <string>

// ============================================================================
memory_order_buffer_line_t::memory_order_buffer_line_t() {
    this->package_clean();
    this->mem_deps_ptr_array = NULL;
};

// ============================================================================
memory_order_buffer_line_t::~memory_order_buffer_line_t() {
    utils_t::template_delete_array<memory_order_buffer_line_t*>(mem_deps_ptr_array);
};


// ============================================================================
void memory_order_buffer_line_t::package_clean() {
        this->opcode_address=0;
        this->memory_address=0;
        this->memory_size=0;
        this->rob_ptr=NULL;                 /// rob pointer
        this->uop_executed=false;
        this->uop_number = 0;
        this->readyAt = orcs_engine.get_global_cycle();  
        this->status = PACKAGE_STATE_FREE;
        this->memory_operation = MEMORY_OPERATION_FREE;
        this->readyToGo = orcs_engine.get_global_cycle();
        this->wait_mem_deps_number = 0;
};

// ============================================================================
std::string memory_order_buffer_line_t::content_to_string() {
    std::string content_string;
    content_string = "";

    content_string = content_string + " |Exec:" + utils_t::uint64_to_string(this->opcode_address);
    content_string = content_string + " |Executed:" + utils_t::bool_to_string(this->uop_executed);
    content_string = content_string + " |Mem. Operation:" +  get_enum_memory_operation_char(this->memory_operation);
    content_string = content_string + " |Mem. Address:" +  utils_t::uint64_to_string(this->memory_address);
    content_string = content_string + " |Status:" +  get_enum_package_state_char(this->status);
    content_string = content_string + " |Ready At:" +  utils_t::uint64_to_string(this->readyAt);
    return content_string;
};

// ============================================================================
/// STATIC METHODS
// ============================================================================
int32_t memory_order_buffer_line_t::find_free(memory_order_buffer_line_t *input_array, uint32_t size_array) {
    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].status == PACKAGE_STATE_FREE){
            return i;
        }
    }
    return POSITION_FAIL;
};
// ============================================================================
int32_t memory_order_buffer_line_t::find_old_request_state_ready(memory_order_buffer_line_t *input_array, uint32_t size_array, package_state_t state) {
    int32_t old_pos = POSITION_FAIL;
    uint64_t old_uop_number = UINT64_MAX;
    /// Find the oldest UOP inside the MOB.... and it have 0 deps.
    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].status == state &&
        input_array[i].uop_number < old_uop_number &&
        input_array[i].wait_mem_deps_number == 0 &&
        input_array[i].uop_executed == true &&
        input_array[i].readyToGo <= orcs_engine.get_global_cycle()) {
            old_uop_number = input_array[i].uop_number;
            old_pos = i;
        }
    }
    return old_pos;
};
// ============================================================================

// ============================================================================
void memory_order_buffer_line_t::print_all_operation_deps(uint64_t &ld_llc_miss,uint64_t &llc_miss_deps,uint64_t &inst_ld_ld) {
    // ORCS_PRINTF("======\n")
    bool hasLoad = false;
    // uint32_t numbOp=0;
    ld_llc_miss++;
    llc_miss_deps++;
    inst_ld_ld++;
    reorder_buffer_line_t *rob_line = this->rob_ptr;
    container_ptr_reorder_buffer_line_t chain;
    chain.reserve(ROB_SIZE);
    chain.push_back(rob_line);
    for (size_t i = 0; i < chain.size(); i++){  
        if(chain.size()>160)break;
        if(chain[i]->wake_up_elements_counter>0){
            chain.push_back(chain[i]->get_deps());
            // chain[i]->wake_up_elements_counter=0;
        }
    }
    for (size_t i = 1; i < chain.size(); i++){ 
        if(chain[i]->uop.opcode_operation == INSTRUCTION_OPERATION_MEM_LOAD){
            hasLoad=true;
            break;
        }
    }
    if(hasLoad){
        ORCS_PRINTF("==============================\n")
        ORCS_PRINTF("MISS %s\n\n",this->rob_ptr->content_to_string().c_str())
        ORCS_PRINTF("\nChain\n")
        for (size_t i = 0; i < chain.size(); i++){ 
            ORCS_PRINTF("%s\n",chain[i]->content_to_string().c_str())
        }
        ORCS_PRINTF("\n==============================\n")
    }
};
// ============================================================================
// Update status package
// ============================================================================
void memory_order_buffer_line_t::updatePackageUntrated(uint32_t stallTime){

    this->status = PACKAGE_STATE_UNTREATED;
    this->readyAt = orcs_engine.get_global_cycle()+stallTime;
};
void memory_order_buffer_line_t::updatePackageReady(uint32_t stallTime){
    this->status = PACKAGE_STATE_READY;
    this->readyAt = orcs_engine.get_global_cycle()+stallTime;
};
void memory_order_buffer_line_t::updatePackageWait(uint32_t stallTime){
    this->status = PACKAGE_STATE_WAIT;
    this->readyAt = orcs_engine.get_global_cycle()+stallTime;
};
void memory_order_buffer_line_t::updatePackageFree(uint32_t stallTime){
    this->status = PACKAGE_STATE_FREE;
    this->readyAt = orcs_engine.get_global_cycle()+stallTime;
};
