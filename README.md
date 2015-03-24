# new-examples-kodo


About
=====

Examples of security schemes based on Kodo-library. The following schemes have been implemented:

- Only encrypted payload (using AES)
- P-Coding
- P-Coding with recode
- SPOC
- SPOC with recode


Build
=====

To configure the examples run:
::
  python waf configure

This will configure the project and download all the dependencies needed.

After configure run the following command to build:
::
  python waf build

If you experience problems with either of the two previous steps you can
check the Kodo `Manual`_ for troubleshooting.

.. _`Manual`:
   https://public.readthedocs.com/steinwurf-kodo/en/latest/

Run
===

After building you run the application typing:
::
  ./build/linux/encrypted_payload/encrypted_payload
  ./build/linux/p_coding/p_coding
  ./build/linux/p_coding_recode/p_coding_recode
  ./build/linux/spoc/spoc
  ./build/linux/spoc_recode/spoc_recode

.. note:: If you are on windows or mac the output folder will be different
