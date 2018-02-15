class plbp_t{

public:
    int8_t ***W;
    uint32_t *GA;
    uint8_t *GHR;
    float saida;
    void allocate();
    uint32_t predict(uint64_t address);
    void train(uint64_t address,uint32_t taken);
     plbp_t();
    ~plbp_t();
    
};