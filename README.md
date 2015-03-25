# new-examples-kodo



License
=======

To obtain a valid Kodo license **you must fill out the license request** [form](http://steinwurf.com/license/).

Kodo is available under a research and educational friendly license, see the
details in the LICENSE.rst file.


About
=====

Examples of security schemes based on Kodo-library. The following schemes have been implemented:

- Only encrypted payload (using AES)
- [P-Coding](http://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=5462050)
- P-Coding with recode
- [SPOC](http://www.dcc.fc.up.pt/~barros/publications/icc2008/vilela-lima-barros.pdf)
- SPOC with recode


Build
=====

To configure the examples run:

    python waf configure

This will configure the project and download all the dependencies needed.

To be able to build some examples you need to have installed [Crypto++ Library](http://www.cryptopp.com/). To download and install the library from your repository. There is more information about on the [cryptopp-wiki](http://www.cryptopp.com/wiki/Linux#Distribution_Package).


After configure run the following command to build:

    python waf build

If you experience problems with either of the two previous steps you can
check the Kodo [Manual](https://public.readthedocs.com/steinwurf-kodo/en/latest/) for troubleshooting.


Run
===

After building you run the application typing:

    ./build/linux/encrypted_payload/encrypted_payload
    ./build/linux/p_coding/p_coding
    ./build/linux/p_coding_recode/p_coding_recode
    ./build/linux/spoc/spoc
    ./build/linux/spoc_recode/spoc_recode

note:: If you are on windows or mac the output folder will be different


