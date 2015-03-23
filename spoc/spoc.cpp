// Copyright Steinwurf ApS 2014.
// Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#include <kodo/rlnc/full_vector_codes.hpp>
#include <iostream>
#include <iomanip>
#include <kodo/trace.hpp>


#include "cryptopp/modes.h"
#include "cryptopp/aes.h"
#include "cryptopp/filters.h"


#include "cryptopp/ccm.h"
using CryptoPP::CBC_Mode;
using CryptoPP::AES;
#include <cryptopp/hex.h>

/// The basic idea here is to get the SPOC scheme implemented, later we can
/// think about make an actual SPOC encoder/decoder.
///
/// I'm going to use a "low" level API here to access the coding
/// coefficients and symbol data more directly.


/// Here we calculate a valid multiple of the key size. And the return value will be how many bytes we will need to encrypt.
uint32_t define_encrypted_bytes_amount(uint32_t symbols){
    if ( symbols % CryptoPP::AES::DEFAULT_KEYLENGTH == 0) {
        return symbols;
    } else {
        return symbols + CryptoPP::AES::DEFAULT_KEYLENGTH - (symbols % CryptoPP::AES::DEFAULT_KEYLENGTH);
        }
}


int main()
{
    using field_type = fifi::binary8;

    uint32_t symbols = 16;
    uint32_t inner_symbol_size = 16;


    uint32_t encoded_count = 0;
    // Here we calculate the size of an outer symbol it will be the symbol
    // size of the inner_code + the size of the encoding vector.

    uint32_t outer_symbol_size =
        inner_symbol_size + fifi::elements_to_size<field_type>(symbols);

    // Typdefs for the encoder/decoder type we wish to use
    using rlnc_encoder = kodo::rlnc::full_vector_encoder<fifi::binary8>;
    using rlnc_decoder = kodo::rlnc::full_vector_decoder<fifi::binary8>;

    // Create the inner and outer encoder
    rlnc_encoder::factory encoder_factory(symbols, outer_symbol_size);
    auto outer_encoder = encoder_factory.build();

    encoder_factory.set_symbol_size(inner_symbol_size);
    auto inner_encoder = encoder_factory.build();

    // Create the inner and outer decoder
    rlnc_decoder::factory decoder_factory(symbols, outer_symbol_size);
    auto outer_decoder = decoder_factory.build();

    decoder_factory.set_symbol_size(inner_symbol_size);
    auto inner_decoder = decoder_factory.build();


    // data_in_inner: Holds the data we actually want to send over - in
    //                this case we just fill it with random data
    std::vector<uint8_t> data_in_inner(inner_encoder->block_size());

    // data_in_outer: Holds the data produced by the inner encoder i.e. the
    //                encoding vector plus encoded symbols
    std::vector<uint8_t> data_in_outer(outer_encoder->block_size());

    // Generate some random input data
    std::generate(data_in_inner.begin(), data_in_inner.end(), rand);

    // Systematic is off on the inner encoder, it is default on so it will
    // be on the outer encoder.
    inner_encoder->set_systematic_off();
    inner_encoder->set_symbols(sak::storage(data_in_inner));


    for (uint32_t i = 0; i < symbols; ++i)
    {
        // Calculate offsets for encoding vector and encoded symbol
        uint8_t* vector = data_in_outer.data() + (i * outer_symbol_size);
        uint8_t* symbol = vector + fifi::elements_to_size<field_type>(symbols);
        ++encoded_count;
        // Generate encoding vector and encode
        inner_encoder->generate(vector);
        inner_encoder->encode_symbol(symbol, vector);
    }

    std::cout << "Encoded count = " << encoded_count << std::endl;


    
    /// Here we are setting up key and IV (DEFAULT_KEYLENGTH = 16 bytes).
    byte key[ CryptoPP::AES::DEFAULT_KEYLENGTH ], iv[ CryptoPP::AES::BLOCKSIZE ];
    memset( key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH );
    memset( iv, 0x00, CryptoPP::AES::BLOCKSIZE );

    /// Call to the function define_encrypted_bytes_amount wich one will give us the 
    /// amount of bytes to encrypt. We do that in order to get a valid multiple of 16.
    uint32_t bytes_to_encrypt = define_encrypted_bytes_amount(symbols);


    // The encoding vectors are now stored in front of every symbol, but
    // not encrypted.
    for (uint32_t i = 0; i < symbols; ++i)
    {
        // Those strings are necessary to work with cryptopp
        std::string ciphertext;
        std::string decryptedtext;

        uint8_t* vector = data_in_outer.data() + (i * outer_symbol_size);
        
        /// Here we proceed to encrypt the symbol with AES (Advanced Encryption Standard) using CBC Mode and no padding.
        CBC_Mode< AES >::Encryption encryptor;
        encryptor.SetKeyWithIV( key, CryptoPP::AES::DEFAULT_KEYLENGTH, iv );
        CryptoPP::StringSource ss( vector , bytes_to_encrypt, true, 
            new CryptoPP::StreamTransformationFilter( encryptor,
                 new CryptoPP::StringSink( ciphertext ),
                        CryptoPP::StreamTransformationFilter::NO_PADDING
             )      
        ); 

        /// We copy the result of the encryption to the buffer that kodo uses.
        std::cout << std::endl << std::endl;
        for(uint32_t i=0;i < bytes_to_encrypt; i++){
            *(vector+ i) = (ciphertext[i]) ; 
        }     
    }

    
    outer_encoder->set_symbols(sak::storage(data_in_outer));

    // Allocate some storage for a "payload" the payload is what we would
    // eventually send over a network
    std::vector<uint8_t> payload(outer_encoder->payload_size());

    // Transmit data between the outer codec
    while (!outer_decoder->is_complete())
    {
        // Encode a packet into the payload buffer
        outer_encoder->write_payload(payload.data());

        // Loose some packets - this is our network
        if (rand() % 2)
            continue;

        // Pass that packet to the decoder
        outer_decoder->read_payload(payload.data());
    }

    // The outer code is finished, so here the place where we
    // decrypt the encoding vectors and decode with the inner code.
    for (uint32_t i = 0; i < symbols; ++i)
    {
        uint8_t* vector = outer_decoder->symbol(i);
        uint8_t* symbol = vector + fifi::elements_to_size<field_type>(symbols);

        std::string decryptedtext;

        
        /// We decrypt the symbol
        CBC_Mode< AES >::Decryption decryptor;
        decryptor.SetKeyWithIV( key, CryptoPP::AES::DEFAULT_KEYLENGTH, iv );
        CryptoPP::StringSource ss( vector , bytes_to_encrypt, true,
                new CryptoPP::StreamTransformationFilter( decryptor,
                    new CryptoPP::StringSink( decryptedtext ), CryptoPP::StreamTransformationFilter::NO_PADDING) );


        /// We copy the result of the decryption the buffer of kodo.
        for(uint32_t i=0;i < bytes_to_encrypt; i++){
            *(vector + i) = (decryptedtext[i]) ;
        }

        inner_decoder->decode_symbol(symbol, vector);
    }

    /// We check the full rank.
    assert(inner_decoder->is_complete());

    // Allocate buffers for the decoders
    std::vector<uint8_t> data_out_inner(inner_decoder->block_size());
    inner_decoder->copy_symbols(sak::storage(data_out_inner));

    // Check we properly decoded the data
    if (data_in_inner == data_out_inner)
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
