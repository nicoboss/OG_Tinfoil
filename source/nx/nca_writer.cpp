#include "nx/nca_writer.h"
#include <zstd.h>

struct Key256
{
	u8 data[0x20] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 0x00, 0x00, 0x00, 0x00 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 0x00 0x00, 0x00 0x00, 0x00};
} PACKED;

const Key256& headerKey()
{
	static Key256 key;
	return key;
}

void append(std::vector<u8>& buffer, const u8* ptr, u64 sz)
{
	for (u64 i = 0; i < sz; i++)
	{
		buffer.push_back(ptr[i]);
	}
}


NcaBodyWriter::NcaBodyWriter(const NcaId& ncaId, u64 offset, ContentStorage& contentStorage) : m_ncaId(ncaId), m_contentStorage(&contentStorage), m_offset(offset)
{
}

NcaBodyWriter::~NcaBodyWriter()
{
}

u64 NcaBodyWriter::write(const  u8* ptr, u64 sz)
{
	m_contentStorage->writePlaceholder(m_ncaId, m_offset, (void*)ptr, sz);

	m_offset += sz;

	return sz;
}


class NczHeader
{
public:
	static const u64 MAGIC = 0x4E544345535A434E;

	class Section
	{
	public:
		u64 offset;
		u64 size;
		u8 cryptoType;
		u8 padding1[7];
		u64 padding2;
		integer<128> cryptoKey;
		integer<128> cryptoCounter;
	} PACKED;

	class SectionContext : public Section
	{
	public:
		SectionContext(const Section& s) : Section(s), crypto(s.cryptoKey, AesCtr(swapEndian(((u64*)&s.cryptoCounter)[0])))
		{
		}

		virtual ~SectionContext()
		{
		}

		void decrypt(void* p, u64 sz, u64 offset)
		{
			if (this->cryptoType != 3)
			{
				return;
			}

			crypto.seek(offset);
			crypto.decrypt(p, p, sz);
		}

		void encrypt(void* p, u64 sz, u64 offset)
		{
			if (this->cryptoType != 3)
			{
				return;
			}

			crypto.seek(offset);
			crypto.encrypt(p, p, sz);
		}

		Aes128Ctr crypto;
	};

	const bool isValid()
	{
		return m_magic == MAGIC && m_sectionCount < 0xFFFF;
	}

	const u64 size() const
	{
		return sizeof(m_magic) + sizeof(m_sectionCount) + sizeof(Section) * m_sectionCount;
	}

	const Section& section(u64 i) const
	{
		return m_sections[i];
	}

	const u64 sectionCount() const
	{
		return m_sectionCount;
	}

protected:
	u64 m_magic;
	u64 m_sectionCount;
	Section m_sections[1];
} PACKED;

class NczBodyWriter : public NcaBodyWriter
{
public:
	NczBodyWriter(const NcaId& ncaId, u64 offset, ContentStorage& contentStorage) : NcaBodyWriter(ncaId, offset, contentStorage)
	{
		buffIn = malloc(buffInSize);
		buffOut = malloc(buffOutSize);

		dctx = ZSTD_createDCtx();
	}

	virtual ~NczBodyWriter()
	{
		close();

		for (auto& i : sections)
		{
			if (i)
			{
				delete i;
				i = NULL;
			}
		}

		if (dctx)
		{
			ZSTD_freeDCtx(dctx);
			dctx = NULL;
		}
	}

	bool close()
	{
		if (this->m_buffer.size())
		{
			processChunk(m_buffer.data(), m_buffer.size());
		}

		flush();

		return true;
	}

	bool flush()
	{
		if (m_deflateBuffer.size())
		{
			m_contentStorage->writePlaceholder(m_ncaId, m_offset, m_deflateBuffer.data(), m_deflateBuffer.size());
			m_offset += m_deflateBuffer.size();
			m_deflateBuffer.resize(0);
		}
		return true;
	}

	NczHeader::SectionContext& section(u64 offset)
	{
		for (int i = 0; i < sections.size(); i++)
		{
			if (offset >= sections[i]->offset && offset < sections[i]->offset + sections[i]->size)
			{
				return *sections[i];
			}
		}
		return *sections[0];
	}

	bool encrypt(const void* ptr, u64 sz, u64 offset)
	{
		const u8* start = (u8*)ptr;
		const u8* end = start + sz;

		while (start < end)
		{
			auto& s = section(offset);

			u64 sectionEnd = s.offset + s.size;

			u64 chunk = offset + sz > sectionEnd ? sectionEnd - offset : sz;

			s.encrypt((void*)start, chunk, offset);

			offset += chunk;
			start += chunk;
			sz -= chunk;
		}

		return true;
	}

	u64 processChunk(const  u8* ptr, u64 sz)
	{
		ZSTD_inBuffer input = { ptr, sz, 0 };
		m_deflateBuffer.resize(sz);
		m_deflateBuffer.resize(0);

		while (input.pos < input.size)
		{
			ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };
			size_t const ret = ZSTD_decompressStream(dctx, &output, &input);

			if (ZSTD_isError(ret))
			{
				error(ZSTD_getErrorName(ret));
				return false;
			}

			append(m_deflateBuffer, (const u8*)buffOut, output.pos);

			if (m_deflateBuffer.size() >= 0x1000000) // 16 MB
			{
				encrypt(m_deflateBuffer.data(), m_deflateBuffer.size(), m_offset);

				flush();
			}

		}

		if (m_deflateBuffer.size())
		{
			encrypt(m_deflateBuffer.data(), m_deflateBuffer.size(), m_offset);

			flush();
		}
		return 1;
	}

	u64 write(const  u8* ptr, u64 sz) override
	{
		u64 total = 0;

		if (!m_sectionsInitialized)
		{
			if (!m_buffer.size())
			{
				append(m_buffer, ptr, sizeof(u64)*2);
				ptr += sizeof(u64) * 2;
				sz -= sizeof(u64) * 2;
			}

			auto header = (NczHeader*)m_buffer.data();

			if (m_buffer.size() + sz > header->size())
			{
				u64 remainder = header->size() - m_buffer.size();
				append(m_buffer, ptr, remainder);
				ptr += remainder;
				sz -= remainder;
			}
			else
			{
				append(m_buffer, ptr, sz);
				ptr += sz;
				sz = 0;
			}

			header = (NczHeader*)m_buffer.data();

			if (m_buffer.size() == header->size())
			{
				for (u64 i = 0; i < header->sectionCount(); i++)
				{
					sections.push_back(new NczHeader::SectionContext(header->section(i)));
				}

				m_sectionsInitialized = true;
				m_buffer.resize(0);
			}
		}

		while (sz)
		{
			if (m_buffer.size() + sz >= 0x1000000)
			{
				u64 chunk = 0x1000000 - m_buffer.size();
				append(m_buffer, ptr, chunk);

				processChunk(m_buffer.data(), m_buffer.size());
				m_buffer.resize(0);

				sz -= chunk;
				ptr += chunk;
			}
			else
			{
				append(m_buffer, ptr, sz);
				sz = 0;
			}

		}

		return sz;
	}

	size_t const buffInSize = ZSTD_DStreamInSize();
	size_t const buffOutSize = ZSTD_DStreamOutSize();

	void* buffIn = NULL;
	void* buffOut = NULL;

	ZSTD_DCtx* dctx = NULL;

	std::vector<u8> m_buffer;
	std::vector<u8> m_deflateBuffer;

	bool m_sectionsInitialized = false;

	std::vector<NczHeader::SectionContext*> sections;
};


NcaWriter::NcaWriter(const NcaId& ncaId, ContentStorage& contentStorage) : m_ncaId(ncaId), m_contentStorage(&contentStorage), m_writer(NULL)
{
}

NcaWriter::~NcaWriter()
{
	close();
}

bool NcaWriter::close()
{
	if (m_writer)
	{
		delete m_writer;
		m_writer = NULL;
	}
	return true;
}

u64 NcaWriter::write(const  u8* ptr, u64 sz)
{
	u64 total = 0;

	if (m_buffer.size() < NCA_HEADER_SIZE)
	{
		if (m_buffer.size() + sz > NCA_HEADER_SIZE)
		{
			u64 remainder = NCA_HEADER_SIZE - m_buffer.size();
			append(m_buffer, ptr, remainder);

			ptr += remainder;
			sz -= remainder;
		}
		else
		{
			append(m_buffer, ptr, sz);
			ptr += sz;
			sz = 0;
		}

		if (m_buffer.size() == NCA_HEADER_SIZE)
		{
			NcaHeader header;
			memcpy(&header, m_buffer.data(), sizeof(header));
			Crypto crypto(&headerKey(), sizeof(headerKey()), MBEDTLS_CIPHER_AES_128_XTS);
			crypto.xtsDecrypt(&header, &header, sizeof(header), 0, 0x200);

			if (header.magic == MAGIC_NCA3)
			{
				m_contentStorage->createPlaceholder(m_ncaId, m_ncaId, header.nca_size);
			}
			else
			{
				// todo fatal
			}

			m_contentStorage->writePlaceholder(m_ncaId, 0, m_buffer.data(), m_buffer.size());
		}
	}

	if (sz)
	{
		if (!m_writer)
		{
			if (sz >= sizeof(NczHeader::MAGIC))
			{
				if (*(u64*)ptr == NczHeader::MAGIC)
				{
					m_writer = new NczBodyWriter(m_ncaId, m_buffer.size(), *m_contentStorage);
				}
				else
				{
					m_writer = new NcaBodyWriter(m_ncaId, m_buffer.size(), *m_contentStorage);
				}
			}
			else
			{
				// raise fatal
			}
		}

		m_writer->write(ptr, sz);
	}

	return sz;
}

