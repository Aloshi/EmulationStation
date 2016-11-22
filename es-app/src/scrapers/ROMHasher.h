#pragma once

#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/uuid/sha1.hpp>

namespace ROMHasher {

enum Decoder
{
	DEC_NOT_DEF,
	DEC_BINARY,
	DEC_SNES,
	DEC_MGD,
	DEC_SMD,
	DEC_LNX,
	DEC_N64,
	DEC_NES
};

// Abstract Base Class for adding more Hashing functions if needed.
class HashFunc
{
	public:
		// Processes a series of bytes.
		virtual void process_bytes(char * b, unsigned int n) =0;

		// Returns a hash encoded string of the hash.
		virtual std::string str() =0;

		// Resets the hash.
		virtual void reset() =0;
};

// Sha1 Implementation of a HashFunc.
class Sha1Func : public HashFunc
{
	public:
		void process_bytes(char * b, unsigned int n);
		std::string str();
		void reset();
	private:
		boost::uuids::detail::sha1 hash;
};

// deinterleaves a block of bytes for Megadrive ROMS.
void deinterleave(char* p, unsigned int n);

// Swap N64 encoded bytes to V64.
void n_swap(char* b, unsigned int n);

// Swap Z64 encoded bytes ti V64.
void z_swap(char* b, unsigned int n);

// Helper function for hashing a file from start to eof..
std::string hash_file(HashFunc * hf, std::ifstream& file, std::streampos start);

// Helper function to hash roms with NES header.
std::string hash_nes(HashFunc * hf, std::ifstream& file);

// Helper function that hashes a file in blocks applying mod function to each block.
std::string hash_block(HashFunc * hf, std::ifstream& file, unsigned int bs, void (*mod)(char*, unsigned int));

// Helper function that hashes N64, using ROM header to determine encoding.
std::string hash_n64(HashFunc * hf, std::ifstream& file);

// Generic hash function for roms. It determines the proper decoding function and hashes the rom.
std::string hash_rom(HashFunc * hf, boost::filesystem::path path);

// Gets the SHA1 of a ROM.
std::string sha1_rom(boost::filesystem::path path);

} // namespace ROMHasher
