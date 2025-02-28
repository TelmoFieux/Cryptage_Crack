# Cryptage_Crack
A C program that allows basic encryption mechanisms and a naive cracking method.

This program allows the user to encrypt any file using multiple methods: a simple XOR method and a CBC method. Both require a key that can be provided by the user or randomly generated. The CBC method also requires an initialization vector.

There is also a cracking part, which is very basic. In this part, we assume the key is made only of alphanumeric characters and that the message is written in English with no spelling mistakes. We try all possibilities and sort them based on words found in the dictionary or by character frequency.

That is all for the description of this program. You will find documentation in this repository for more information.
