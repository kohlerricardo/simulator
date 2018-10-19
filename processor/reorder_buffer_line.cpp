/*
 * Copyright (C) 2010~2014  Marco Antonio Zanata Alves
 *                          (mazalves at inf.ufrgs.br)
 *                          GPPD - Parallel and Distributed Processing Group
 *                          Universidade Federal do Rio Grande do Sul
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../simulator.hpp"
#include <string>

// ============================================================================
reorder_buffer_line_t::reorder_buffer_line_t() {
    this->package_clean();
    this->reg_deps_ptr_array = NULL;
};

// ============================================================================
reorder_buffer_line_t::~reorder_buffer_line_t() {
    utils_t::template_delete_array<reorder_buffer_line_t*>(reg_deps_ptr_array);
};
 bool reorder_buffer_line_t::operator==(const reorder_buffer_line_t& reorder_buffer_line){
        // cmp uop
        if(this->uop==reorder_buffer_line.uop)return FAIL;                      /// uOP stored
        // cmp entry stafge
        if(this->stage !=reorder_buffer_line.stage)return FAIL;
        /// wait Register Dependencies Control
        if(this->wait_reg_deps_number !=reorder_buffer_line.wait_reg_deps_number)return FAIL;
       //cmp reg deps array
        if(memcmp(this->reg_deps_ptr_array,reorder_buffer_line.reg_deps_ptr_array,sizeof(reorder_buffer_line_t)*ROB_SIZE)!=0) return FAIL;  
       //cmp  counter wake up
        if(this->wake_up_elements_counter !=reorder_buffer_line.wake_up_elements_counter)return FAIL;
       //cmp mob pointer
        if(memcmp(this->mob_ptr,reorder_buffer_line.mob_ptr,sizeof(memory_order_buffer_line_t))!=0) return FAIL;  
        
        if(this->on_chain !=reorder_buffer_line.on_chain)return FAIL;
        if(this->is_poisoned !=reorder_buffer_line.is_poisoned)return FAIL;
        if(this->original_miss !=reorder_buffer_line.original_miss)return FAIL;
        if(this->emc_executed !=reorder_buffer_line.emc_executed)return FAIL;
    return OK;
};
// ============================================================================
void reorder_buffer_line_t::package_clean() {
    this->uop.package_clean(); 
    this->stage = PROCESSOR_STAGE_DECODE;
    this->mob_ptr = NULL;
    this->wait_reg_deps_number = 0; 
    this->wake_up_elements_counter = 0;
    this->on_chain=false;
    this->is_poisoned=false;
    this->original_miss = false;
    this->emc_executed=false;
    this->op_on_emc_buffer=0;
    };

// ============================================================================
std::string reorder_buffer_line_t::content_to_string() {
    std::string content_string;
    content_string = "";

    #ifndef SHOW_FREE_PACKAGE
        if (this->uop.status == PACKAGE_STATE_FREE) {
            return content_string;
        }
    #endif

    content_string = this->uop.content_to_string();
    content_string = content_string + " | Stage:" + get_enum_processor_stage_char(this->stage);
    content_string = content_string + " | Reg.Wait:" + utils_t::uint32_to_string(this->wait_reg_deps_number);
    content_string = content_string + " | Op_On_EMC_B.:" + utils_t::uint32_to_string(this->op_on_emc_buffer);
    content_string = content_string + " | WakeUp:" + utils_t::uint32_to_string(this->wake_up_elements_counter);
    content_string = content_string + " | ReadyAt: " + utils_t::uint64_to_string(this->uop.readyAt);
    content_string = content_string + " | On Dep Chain: " + utils_t::bool_to_string(this->on_chain);
    content_string = content_string + " | Poisoned: " + utils_t::bool_to_string(this->is_poisoned);
    content_string = content_string + " | Executed on EMC: " + utils_t::bool_to_string(this->emc_executed);
    if(this->mob_ptr != NULL){
        content_string = content_string + this->mob_ptr->content_to_string();
    }
    return content_string;
};
std::string reorder_buffer_line_t::content_to_string2() {
    std::string content_string;
    content_string = "";

    #ifndef SHOW_FREE_PACKAGE
        if (this->uop.status == PACKAGE_STATE_FREE) {
            return content_string;
        }
    #endif

    content_string = this->uop.content_to_string();
    content_string = content_string + " | Stage:" + get_enum_processor_stage_char(this->stage);
    content_string = content_string + " | Reg.Wait:" + utils_t::uint32_to_string(this->wait_reg_deps_number);
    content_string = content_string + " | WakeUp:" + utils_t::uint32_to_string(this->wake_up_elements_counter);
    content_string = content_string + " | ReadyAt: " + utils_t::uint64_to_string(this->uop.readyAt);
    content_string = content_string + " | On Dep Chain: " + utils_t::bool_to_string(this->on_chain);
    return content_string;
};
// ============================================================================
void reorder_buffer_line_t::print_dependences(){
    for(uint16_t i=0;i<ROB_SIZE;i++){
        if(this->reg_deps_ptr_array[i]==NULL)break;

        ORCS_PRINTF("%s\n",this->reg_deps_ptr_array[i]->content_to_string().c_str())
    }
};

// ============================================================================
/// STATIC METHODS
// ============================================================================

std::string reorder_buffer_line_t::print_all(reorder_buffer_line_t *input_array, uint32_t size_array) {
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

// ====================================================================
/// Generate Dep Chains to EMC
// ====================================================================

void reorder_buffer_line_t::get_deps(std::vector<reorder_buffer_line_t> *buffer){
    if(this->wake_up_elements_counter>0){
        for (size_t i = 0; i < ROB_SIZE; i++)
        {
            if(this->reg_deps_ptr_array[i]!=NULL){
                buffer->push_back(*this->reg_deps_ptr_array[i]);
            }
            else{
                break;
                }
        }
    }
};
