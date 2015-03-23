// Copyright Steinwurf ApS 2014.
// Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#include <kodo/rlnc/full_vector_codes.hpp>
#include <iostream>
#include <random>


/// The basic idea here is to get the P-coding scheme implemented, later we can
/// think about make an actual P-coding encoder/decoder.
///
/// I'm going to use a "low" level API here to access the coding
/// coefficients and symbol data more directly.

/// Generation of randum numbers that we will use later for the permutation.
int myrandom (int i) { return std::rand()%i; }

int main()
{
    using field_type = fifi::binary8;

    uint32_t symbols = 4;
    uint32_t symbol_size = 16;

    // Typdefs for the encoder/decoder type we wish to use
    using rlnc_encoder = kodo::rlnc::full_vector_encoder<fifi::binary8>;
    using rlnc_decoder = kodo::rlnc::full_vector_decoder<fifi::binary8>;

    // Create the inner and outer encoder
    rlnc_encoder::factory encoder_factory(symbols, symbol_size);
    auto encoder = encoder_factory.build();

    // Create the inner and outer decoder
    rlnc_decoder::factory decoder_factory(symbols, symbol_size);
    auto decoder = decoder_factory.build();

    // data_in_inner: Holds the data we actually want to send over - in
    //                this case we just fill it with random data
    std::vector<uint8_t> data_in(encoder->block_size());

    // payload: The data with both encoding vector and data
    std::vector<uint8_t> payload(encoder->symbol_size() *
                                 fifi::elements_to_size<field_type>(symbols));

    // Generate some random input data
    std::generate(data_in.begin(), data_in.end(), rand);

    // Systematic is off on the encoder
    encoder->set_systematic_off();
    encoder->set_symbols(sak::storage(data_in));

    std::srand ( unsigned ( std::time(0) ) );

    // Creating a vector with random numbers between 0 and (symbols + symbol_size). In this case between 0 and 19.
    std::vector<int> permutation_vector;
    for (uint32_t i=0; i<(symbols + symbol_size) ; ++i) permutation_vector.push_back(i);
    std::random_shuffle ( permutation_vector.begin(), permutation_vector.end(), myrandom);


    // Print out the content of the vector:
    std::cout << "permutation_vector= ";
    for (std::vector<int>::iterator it=permutation_vector.begin(); it!=permutation_vector.end(); ++it)
    std::cout << *it << ' ';
    std::cout << std::endl << std::endl;
    std::cout << std::endl << std::endl;
    std::cout << std::endl << std::endl;


    while (!decoder->is_complete())
    {
        // Calculate offsets for encoding vector and encoded symbol
        uint8_t* vector = payload.data();
        uint8_t* symbol = vector + fifi::elements_to_size<field_type>(symbols);


        // Generate encoding vector and encode
        encoder->generate(vector);
        encoder->encode_symbol(symbol, vector);


        // Saving in an auxiliar array the vector.
        uint8_t* aux = (uint8_t*) malloc((symbols + symbol_size) * sizeof(uint8_t));;
        for(uint32_t i=0;i < (symbols + symbol_size); i++){
             *(aux + i) = *(vector+i) ;
        }

        // PERMUTATION
        for (uint32_t i=0; i< (symbols + symbol_size); ++i) {
            *(vector + permutation_vector.at(i)) = aux[i]; 
        }

        // Loose some packets - this is our network
        if (rand() % 2)
            continue;

        // Saving in an auxiliar array2 the vector. 
        uint8_t* aux2 = (uint8_t*) malloc((symbols + symbol_size) * sizeof(uint8_t));;
        for(uint32_t i=0;i < (symbols + symbol_size); i++){
             *(aux2 + i) = *(vector+i) ;
        }


        // PERMUTATION (inverse)
        for (uint32_t i=0; i<(symbols + symbol_size); ++i) {
            *(vector + i) = aux2[permutation_vector.at(i)]; 
        }


        decoder->decode_symbol(symbol, vector);

    }


    // Allocate buffers for the decoders
    std::vector<uint8_t> data_out(decoder->block_size());
    decoder->copy_symbols(sak::storage(data_out));


    // Check we properly decoded the data
    if (data_in == data_out)
    {
        std::cout << "Data decoded correctly" << std::endl;
    }
    else
    {
        std::cout << "Unexpected failure to decode "
                  << "please file a bug report :)" << std::endl;
    }

    return 0;
}
