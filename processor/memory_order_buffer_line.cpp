#include "../simulator.hpp"
#include <string>

// ============================================================================
memory_order_buffer_line_t::memory_order_buffer_line_t() {
    this->package_clean();
};

// ============================================================================
memory_order_buffer_line_t::~memory_order_buffer_line_t() {
};


// ============================================================================
void memory_order_buffer_line_t::package_clean() {
        this->opcode_address=0;
        this->memory_address=0;
        this->memory_size=0;
        this->rob_ptr=NULL;                 /// rob pointer
        this->uop_executed=false;
        this->readyAt = 0;        
        this->status = PACKAGE_STATE_FREE;
        this->memory_operation = MEMORY_OPERATION_FREE;
        this->born_cicle=orcs_engine.get_global_cycle();
};

// ============================================================================
std::string memory_order_buffer_line_t::content_to_string() {
    std::string content_string;
    content_string = "";

    content_string = content_string + " |Exec:" + utils_t::uint64_to_string(this->opcode_address);
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
        if (input_array[i].status == PACKAGE_STATE_FREE) {
            return i;
        }
    }
    return POSITION_FAIL;
};
int32_t memory_order_buffer_line_t::find_old_request_state_ready(memory_order_buffer_line_t *input_array, uint32_t size_array, package_state_t state) {
    int32_t old_pos = POSITION_FAIL;
    uint64_t old_uop_number = INT64_MAX;

    /// Find the oldest UOP inside the MOB.... and it have 0 deps.
    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].status == state &&
        input_array[i].rob_ptr->uop.uop_number < old_uop_number &&
        input_array[i].uop_executed == true &&
        input_array[i].readyAt <= orcs_engine.get_global_cycle()) {
            old_uop_number = input_array[i].rob_ptr->uop.uop_number;
            old_pos = i;
        }
    }
    return old_pos;
};
// ============================================================================

// ============================================================================
std::string memory_order_buffer_line_t::print_all(memory_order_buffer_line_t *input_array, uint32_t size_array) {
    std::string content_string;
    std::string final_string;

    final_string = "";
    for (uint32_t i = 0; i < size_array ; i++) {
        content_string = "";
        content_string = input_array[i].content_to_string();
        if (content_string.size() > 1) {
            final_string = final_string + "[" + utils_t::uint32_to_string(i) + "] " + content_string + "\n";
        }
    }
    return final_string;
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