./standard_sample -c -i ainput -o a_encode_standard
./hashnew_sample  -c -i ainput -o a_encode_hashnew

./standard_sample -d -i a_encode_standard -o a_decode_standard
./hashnew_sample  -d -i a_encode_hashnew  -o a_decode_hashnew
