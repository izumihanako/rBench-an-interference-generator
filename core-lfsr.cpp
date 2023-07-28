#include "rbench.hpp"

// Galois LFSR
#define Galois_LFSR_MASK 0xd0000001u
#define Galois_LFSR_rand ( lfsr = ( lfsr >> 1 ) ^ ( unsigned int ) \
            ( ( 0 - ( lfsr & 1u ) ) & Galois_LFSR_MASK ) )

Galois_LFSR::Galois_LFSR(){
    lfsr = 1 ;
}

void Galois_LFSR::set_LFSR_number( uint32_t lfsr_ ){
    lfsr = lfsr_ ;
}

inline uint32_t Galois_LFSR::lfsr32(){
    return Galois_LFSR_rand ;
}

inline uint64_t Galois_LFSR::lfsr64(){
    uint64_t hi = Galois_LFSR_rand ;
    uint64_t lo = Galois_LFSR_rand ;
    return ( hi << 32 ) | lo ;
}

#undef Galois_LFSR_MASK
#undef Galois_LFSR_rand