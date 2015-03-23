// Copyright Steinwurf ApS 2011.
// Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include <kodo/rlnc/full_vector_codes.hpp>
#include <sak/storage.hpp>

#include "cryptopp/modes.h"
#include "cryptopp/aes.h"
#include "cryptopp/filters.h"


#include "cryptopp/ccm.h"
using CryptoPP::CBC_Mode;
using CryptoPP::AES;
#include <cryptopp/hex.h>


int main()
{
    // Set the number of symbols (i.e. the generation size in RLNC
    // terminology) and the size of a symbol in bytes
    uint32_t symbols = 14;
    uint32_t symbol_size = 128;

    // Typdefs for the encoder/decoder type we wish to use
    using rlnc_encoder = kodo::rlnc::full_vector_encoder<fifi::binary8>;
    using rlnc_decoder = kodo::rlnc::full_vector_decoder<fifi::binary8>;

    // In the following we will make an encoder/decoder factory.
    // The factories are used to build actual encoders/decoders
    rlnc_encoder::factory encoder_factory(symbols, symbol_size);
    auto encoder = encoder_factory.build();

    rlnc_decoder::factory decoder_factory(symbols, symbol_size);
    auto decoder = decoder_factory.build();

    // Allocate some storage for a "payload" the payload is what we would
    // eventually send over a network
    std::vector<uint8_t> payload(encoder->payload_size());

    // Allocate some data to encode. In this case we make a buffer
    // with the same size as the encoder's block size (the max.
    // amount a single encoder can encode)
    std::vector<uint8_t> data_in(encoder->block_size());
    std::vector<uint8_t> data_in_encoded(encoder->block_size());

    // Just for fun - fill the data with random data
    std::generate(data_in.begin(), data_in.end(), rand);


    /// Here we are setting up key and IV (DEFAULT_KEYLENGTH = 16 bytes).
    byte key[ CryptoPP::AES::DEFAULT_KEYLENGTH ], iv[ CryptoPP::AES::BLOCKSIZE ];
    memset( key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH );
    memset( iv, 0x00, CryptoPP::AES::BLOCKSIZE );

    // Those strings are necessary to work with cryptopp
    std::string ciphertext;
    std::string decryptedtext;
    
	/// Here we proceed to encrypt the symbol with AES (Advanced Encryption Standard) using CBC Mode and no padding.
    CBC_Mode< AES >::Encryption encryptor;
    encryptor.SetKeyWithIV( key, CryptoPP::AES::DEFAULT_KEYLENGTH, iv );
    CryptoPP::StringSource ss_1( data_in.data() , symbols*symbol_size, true, 
        new CryptoPP::StreamTransformationFilter( encryptor,
             new CryptoPP::StringSink( ciphertext ),
                    CryptoPP::StreamTransformationFilter::NO_PADDING
         )      
    ); 

    /// We copy the result of the encryption to the buffer that kodo uses.
    std::cout << std::endl << std::endl;
    for(uint32_t i=0;i < symbols*symbol_size; i++){
        *(data_in_encoded.data()+ i) = (ciphertext[i]) ; 
    }

    // Assign the data buffer to the encoder so that we may start
    // to produce encoded symbols from it
    encoder->set_symbols(sak::storage(data_in_encoded));


    while (!decoder->is_complete())
    {
        // Encode a packet into the payload buffer
        encoder->write_payload(payload.data());

        // Pass that packet to the decoder
        decoder->read_payload(payload.data());
    }

    // The decoder is complete, now copy the symbols from the decoder
    std::vector<uint8_t> data_out(decoder->block_size());
    decoder->copy_symbols(sak::storage(data_out));

    /// We decrypt the symbol
    CBC_Mode< AES >::Decryption decryptor;
    decryptor.SetKeyWithIV( key, CryptoPP::AES::DEFAULT_KEYLENGTH, iv );
    CryptoPP::StringSource ss_2(  (data_out.data()) , symbols*symbol_size, true,
            new CryptoPP::StreamTransformationFilter( decryptor,
                new CryptoPP::StringSink( decryptedtext ), CryptoPP::StreamTransformationFilter::NO_PADDING) );

    /// We copy the result of the decryption the buffer of kodo.
    for(uint32_t i=0;i < symbols*symbol_size; i++){
        *(data_out.data() + i) = (decryptedtext[i]) ;
    }   

    // Check we properly decoded the data
    if (std::equal(data_out.begin(), data_out.end(), data_in.begin()))
    {
        std::cout << "Data decoded correctly ;D" << std::endl;
    }
    else
    {
        std::cout << "Unexpected failure to decode "
                  << "please file a bug report :)" << std::endl;
    }
}
