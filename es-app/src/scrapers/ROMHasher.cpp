#include "scrapers/ROMHasher.h"
#include <boost/assign.hpp>
#include <boost/filesystem.hpp>
#include <boost/uuid/sha1.hpp>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <map>

namespace ROMHasher
{

std::string Sha1Func::str()
{
	unsigned int digest[5];
	hash.get_digest(digest);
	std::stringstream ss;
	ss << std::hex << std::setfill('0') << std::setw(8) << digest[0];
	ss << std::hex << std::setfill('0') << std::setw(8) << digest[1];
	ss << std::hex << std::setfill('0') << std::setw(8) << digest[2];
	ss << std::hex << std::setfill('0') << std::setw(8) << digest[3];
	ss << std::hex << std::setfill('0') << std::setw(8) << digest[4];
	return ss.str();
}

void Sha1Func::process_bytes(char * b, unsigned int n)
{
	hash.process_bytes(b, n);
}

void Sha1Func::reset()
{
	hash.reset();
}

std::map<std::string, Decoder> decoders = boost::assign::map_list_of
	(".bin", DEC_BINARY)
	(".32x", DEC_BINARY)
	(".a26", DEC_BINARY)
	(".bin", DEC_BINARY)
	(".gb", DEC_BINARY)
	(".gbc", DEC_BINARY)
	(".gba", DEC_BINARY)
	(".gen", DEC_BINARY)
	(".gg", DEC_BINARY)
	(".lyx", DEC_BINARY)
	(".md", DEC_BINARY)
	(".pce", DEC_BINARY)
	(".rom", DEC_BINARY)
	(".sms", DEC_BINARY)
	(".fig", DEC_SNES)
	(".sfc", DEC_SNES)
	(".smc", DEC_SNES)
	(".swc", DEC_SNES)
	// TODO: Switch Sega to parsing ROM header to detect interleaving instead of extension.
	(".mgd", DEC_MGD)
	(".smd", DEC_SMD)
	(".lnx", DEC_LNX)
	(".n64", DEC_N64)
	(".v64", DEC_N64)
	(".z64", DEC_N64)
	(".nes", DEC_NES);
	// TODO: Add .zip and .gz support.

std::string hash_file(HashFunc * hf, std::ifstream& file, std::streampos start)
{
	size_t bs = 16384;
	char * b = new char [bs];
	file.seekg (start, std::ios::beg);
	while (file)
	{
		file.read (b, bs);
		if (file)
		{
			hf->process_bytes(b, bs);
		}
		else if (file.eof())
		{
			// file.gcount() returns an std::streamsize, which is the signed counterpart of std::size_t
			// it only returns a negative value in the constructors of std::strstreambuf, so we can
			// safely cast to size_t (taken from http://en.cppreference.com/w/cpp/io/streamsize)
			hf->process_bytes(b, (size_t)file.gcount());
		}
		else
		{
			hf->reset();
		}
	}
	delete[] b;
	return hf->str();
}

std::string hash_nes(HashFunc * hf, std::ifstream& file)
{
	char * b = new char [16];
	file.seekg (0, std::ios::beg);
	file.read(b, 16);
	unsigned char* header = reinterpret_cast<unsigned char*>(b);
	unsigned int prg_size = (unsigned int)header[4];
	unsigned int chr_size = (unsigned int)header[5];
	if ((header[7] & 12) == 8)
	{
		unsigned char rom_size = (unsigned int)header[9];
		chr_size = ((rom_size & 0x0F) << 8) + chr_size;
		prg_size = ((rom_size & 0xF0) << 4) + prg_size;
	}
	unsigned int prg, chr, start;
	prg = 16 * 1024 * prg_size;
	chr = 8 * 1024 * chr_size;
	start = 16;
	if ((header[6] & 4) == 4)
	{
		start += 512;
	}

	char * rom_data = new char [chr + prg];
	file.seekg (start, std::ios::beg);
	file.read (rom_data, chr + prg);
	if (file)
	{
		hf->process_bytes(rom_data, chr + prg);
	}
	else
	{
		hf->reset();
	}
	delete[] rom_data;
	delete[] b;
	return hf->str();
}

void deinterleave(char* p, unsigned int n)
{
	unsigned int m, i;
	m = n / 2;
	char * b = new char [n];
	for (i=0 ; i<n ; i++)
	{
		if (i < m)
		{
			b[i*2+1] = p[i];
		}
		else
		{
			b[i*2-n] = p[i];
		}
	}
	for (i=0 ; i<n; i++)
	{
		p[i] = b[i];
	}
	delete[] b;
}

std::string hash_block(HashFunc * hf, std::ifstream& file, unsigned int bs, void (*mod)(char*, unsigned int))
{
	char * b = new char [bs];
	file.seekg (0, std::ios::beg);
	while (file)
	{
		file.read (b, bs);
		if (file)
		{
			if (mod)
			{
				mod(b, bs);
			}
			hf->process_bytes(b, bs);
		}
		else if (file.eof())
		{
			break;
		}
		else
		{
			hf->reset();
		}
	}
	delete[] b;
	return hf->str();
}

void n_swap(char* b, unsigned int n)
{
	char tmp;
	unsigned int i;
	if (n % 4 != 0)
	{
		return;
	}
	for (i = 0; i < n; i += 4)
	{
		tmp = b[i+2];
		b[i+2] = b[i];
		b[i] = tmp;
		tmp = b[i+3];
		b[i+3] = b[i+1];
		b[i+1] = tmp;
	}
}

void z_swap(char* b, unsigned int n)
{
	char tmp;
	unsigned int i;
	if (n % 4 != 0)
	{
		return;
	}
	for (i = 0; i < n; i += 4)
	{
		tmp = b[i+1];
		b[i+1] = b[i];
		b[i] = tmp;
		tmp = b[i+3];
		b[i+3] = b[i+2];
		b[i+2] = tmp;
	}
}

std::string hash_n64(HashFunc * hf, std::ifstream& file)
{
	char * b = new char [4];
	void (*swap)(char*, unsigned int);
	file.seekg (0, std::ios::beg);
	file.read (b, 4);
	unsigned char* header = reinterpret_cast<unsigned char*>(b);
	if (header[0] == 0x80)
	{
		swap = &z_swap;
	}
	else if (header[3] == 0x80)
	{
		swap = &n_swap;
	}
	else
	{
		swap = NULL;
	}
	delete[] b;
	return hash_block(hf, file, 16384, swap);
}

std::string hash_rom(HashFunc * hf, boost::filesystem::path path)
{
	std::streampos size;
	boost::filesystem::path ext;
	std::ifstream file (path.c_str(), std::ios::binary | std::ios::ate);
	if (file.is_open())
	{
		size = file.tellg();
		ext = path.extension();
		switch (decoders[ext.string()])
		{
			case DEC_BINARY:
				return hash_file(hf, file, 0);
			case DEC_SNES:
				if (size % 1024 == 512)
					return hash_file(hf, file, 512);
				else
					return hash_file(hf, file, 0);
				break;
			case DEC_MGD:
				return hash_block(hf, file, (size_t)size, deinterleave);
			case DEC_SMD:
				return hash_block(hf, file, 16384, deinterleave);
			case DEC_LNX:
				return hash_file(hf, file, 64);
			case DEC_N64:
				return hash_n64(hf, file);
			case DEC_NES:
				return hash_nes(hf, file);
			case DEC_NOT_DEF:
				return "";
		}
		file.close();
	}
	return "";
}

std::string sha1_rom(boost::filesystem::path path)
{
	Sha1Func sf;
	HashFunc * hf = &sf;
	return hash_rom(hf, path);
}

} // end namespace
