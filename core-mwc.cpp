#include "rbench.hpp"

// init function, set random seed pair (w,z) to argument( w_ , z_ )
mwc_t::mwc_t( uint32_t seed_w_ , uint32_t seed_z_ ):
w(seed_w_) , z(seed_z_){
    n16 = saved16 = 0 ;
    n8 = saved8 = 0 ;
    n1 = saved1 = 0 ;
}

// clear random buffer
inline void mwc_t::flush(){
    n16 = saved16 = 0 ;
    n8 = saved8 = 0 ;
    n1 = saved1 = 0 ;
}

// dirty mwc reseed, this is expensive as it
// pulls in various system values for the seeding
void mwc_t::reseed(){
    double time_db = time_now() ;
    const uint64_t time_rnd = alias_cast<uint64_t>( time_db ) ;
    const intptr_t p1 = (intptr_t)(this) ;
    const intptr_t p2 = (intptr_t)&time_db ;

    z = (uint32_t)( time_rnd >> 32 ) ;
    w = time_rnd & ( 0xffffffff ) ;
    
    z += (uint32_t) ~( p1 - p2 ) ;
    w += (uint32_t)( (uint64_t)getpid() ^ (uint64_t)getppid() << 12 ) ;
    
    for( int i = 0 , n = (int)z % 1733 ; i < n ; i ++ ) mwc32() ;
    flush() ;
}

// set random seed pair (w,z) to argument( w_ , z_ )
void mwc_t::set_seed( const uint32_t w_ , const uint32_t z_ ){
    this->w = w_ ;
    this->z = z_ ;
}

// get seed pair (w,z)
pair<uint32_t,uint32_t> mwc_t::get_seed(){
    return make_pair( w , z ) ;
}

// restore seed pair (w,z) to default settings
void mwc_t::set_default_seed(){
    set_seed( MWC_SEED_W , MWC_SEED_Z ) ;
}

// debug info 
void mwc_t::print_info() {
    printf( "MWC random generator:\n" ) ;
    printf( "seeds: w = %u, z = %u\n" , w , z ) ;
    printf( "generate random numbers: %u %u %u\n" , mwc32() , mwc32() , mwc32() ) ;
}

// mwc32: Multiply-with-carry random numbers
// fast pseudo random number generator, see
// http://www.cse.yorku.ca/~oz/marsaglia-rng.html
uint32_t HOT OPTIMIZE3 mwc_t::mwc32(){
    z = 36969 * (z & 65535) + (z >> 16);
    w = 18000 * (w & 65535) + (w >> 16);
    return (z << 16) + w;
}

// mwc64: get a 64 bit pseudo random number 
uint64_t HOT OPTIMIZE3 mwc_t::mwc64() {
    return (((uint64_t)mwc32()) << 32 ) | mwc32() ;
}

// mwc16: get a 16 bit pseudo random number 
uint16_t HOT OPTIMIZE3 mwc_t::mwc16(){
    if( n16 ) {
        n16 -- ;
        saved16 >>= 16 ;
    } else {
        n16 = 1 ;
        saved16 = mwc32() ;
    }
    return saved16 & 0xffff ;
}

// mwc8: get a 8 bit pseudo random number 
uint8_t HOT OPTIMIZE3 mwc_t::mwc8() {
    if( n8 ) {
        n8 -- ;
        saved8 >>= 8 ;
    } else {
        n8 = 3 ;
        saved8 = mwc32() ;
    }
    return saved8 & 0xff ;
}

// mwc1: get a 1 bit pseudo random number 
uint8_t HOT OPTIMIZE3 mwc_t::mwc1() {
    if( LIKELY( n1 ) ){
        n1 -- ;
        saved1 >>= 1 ;
    } else {
        n1 = 31 ;
        saved1 = mwc32() ;
    }
    return saved1 & 0x1 ;
}

// see https://research.kudelskisecurity.com/2020/07/28/the-definitive-guide-to-modulo-bias-and-how-to-avoid-it/
// return 8 bit non-modulo biased value 1..max (inclusive)
// return 0 if mmod is 0
uint8_t OPTIMIZE3 mwc_t::mwc8modn( const uint8_t mmod ){
    if( mmod == 0 ) {
        return 0 ;
    }
    register uint8_t threshold = mmod ;
    register uint8_t val = 0 ;
    while( threshold < 0x80U ){
        threshold = (uint8_t)( threshold << 1 ) ;
    }
    do{
        val = mwc8() ;
    } while( val >= threshold ) ;
    return val % mmod ;
}

// return 16 bit non-modulo biased value 1..max (inclusive)
// return 0 if mmod is 0
uint16_t OPTIMIZE3 mwc_t::mwc16modn( const uint16_t mmod ){
    if( mmod == 0 ) {
        return 0 ;
    }
    register uint16_t threshold = mmod ;
    register uint16_t val = 0 ;
    while( threshold < 0x8000U ){
        threshold = (uint16_t)( threshold << 1 ) ;
    }
    do{
        val = mwc16() ;
    } while( val >= threshold ) ;
    return val % mmod ;
}

// return 32 bit non-modulo biased value 1..max (inclusive)
// return 0 if mmod is 0
uint32_t OPTIMIZE3 mwc_t::mwc32modn( const uint32_t mmod ){
    if( mmod == 0 ) {
        return 0 ;
    }
    register uint32_t threshold = mmod ;
    register uint32_t val = 0 ;
    while( threshold < 0x80000000U ){
        threshold <<= 1 ;
    }
    do{
        val = mwc32() ;
    } while( val >= threshold ) ;
    return val % mmod ;
}

uint32_t OPTIMIZE3 mwc_t::mwc32modn_maybe_pwr2( const uint32_t mmod ){
    register const uint32_t mask = mmod - 1;
	if (UNLIKELY(mmod == 0))
		return 0;
    if( ( mmod & mask ) == 0 ){
        return mwc32() & ( mask ) ;
    } else {
        return mwc32modn( mmod ) ;
    }
}

// return 64 bit non-modulo biased value 1..max (inclusive)
// return 0 if mmod is 0
uint64_t OPTIMIZE3 mwc_t::mwc64modn( const uint64_t mmod ){
    if( mmod == 0 ) {
        return 0 ;
    }
    register uint64_t threshold = mmod ;
    register uint64_t val = 0 ;
    if( ( mmod & (mmod - 1) ) == 0 ){
        return mwc64() & ( mmod - 1 ) ;
    }
    while( threshold < 0x8000000000000000U ){
        threshold <<= 1 ;
    }
    do{
        val = mwc64() ;
    } while( val >= threshold ) ;
    return val % mmod ;
}

// fill the giving memory with random number 
void OPTIMIZE3 mwc_t::fill_array( void* p , int32_t len ){
    int __lessthan4 = len % 4 ;
    for( register int i = 0 ; i < len ; i += 4 ){
        *((int*)p) = mwc32() ;
    }
    for( register int i = 0 ; i < __lessthan4 ; i ++ ){
        *((char*)p+len-1-i) = mwc8() ;
    }
}