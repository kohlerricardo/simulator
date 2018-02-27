#ifndef __SANITY_TEST_HPP
#define __SANITY_TEST_HPP

class sanity_test_t
{
private:
    uint64_t stallFetch;
    uint64_t stallBTB;
    uint64_t stallBP;
    uint64_t stallCache;
    
public:
    sanity_test_t();
    ~sanity_test_t();
    INSTANTIATE_GET_SET(uint64_t,stallFetch);
    INSTANTIATE_GET_SET(uint64_t,stallBTB);
    INSTANTIATE_GET_SET(uint64_t,stallBP);
    INSTANTIATE_GET_SET(uint64_t,stallCache);
    void statistics();
    void allocate();
    //==================
    //Method calculate stall Branch Predictor
    //==================
    void calculateStallBranchPredictor();
    void check();

};



#endif // !__SANITY_TEST_HPP
