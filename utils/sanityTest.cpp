#include "../simulator.hpp" 

sanity_test_t::sanity_test_t()
{
}

sanity_test_t::~sanity_test_t()
{
}

void sanity_test_t::calculateStallBranchPredictor(){
    this->set_stallBTB(orcs_engine.branchPredictor->btbMiss*BTB_MISS_PENALITY);
    uint64_t stallBP=(orcs_engine.branchPredictor->branchTakenMiss+orcs_engine.branchPredictor->branchNotTakenMiss)*MISSPREDICTION_PENALITY;
    this->set_stallBP(stallBP);
};
void sanity_test_t::check(){
    this->calculateStallBranchPredictor();
    fprintf(stdout,"BTB Stall %lu\n",this->get_stallBTB());
    fprintf(stdout,"BranchPredictor Stall %lu\n",this->get_stallBP());
    fprintf(stdout,"Fetch Stall %lu\n",orcs_engine.processor->get_stall_full_FetchBuffer());
    fprintf(stdout,"Decode Stall %lu\n",orcs_engine.processor->get_stall_full_DecodeBuffer());

    uint64_t total = (orcs_engine.trace_reader->get_fetch_instructions())/FETCH_WIDTH;
    total += (orcs_engine.trace_reader->get_fetch_instructions())%FETCH_WIDTH;
    total += this->get_stallBTB()+this->get_stallBP()+orcs_engine.processor->get_stall_full_FetchBuffer()
    +orcs_engine.processor->get_stall_full_DecodeBuffer();
    fprintf(stdout,"Total Cycle Checked %lu\n",total);

}