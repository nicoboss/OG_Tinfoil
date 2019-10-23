#pragma once
#include <switch.h>
#include <vector>
#include "nx/ipc/ncm_ext.h"
#include "nx/ncm.hpp"

#define NCA_HEADER_SIZE 0x4000
#define MAGIC_NCA3 0x3341434E /* "NCA3" */

class NcaFsHeader
{
public:
	u8 _0x0;
	u8 _0x1;
	u8 partition_type;
	u8 fs_type;
	crypt_type_t crypt_type;
	u8 _0x5[0x3];
	u8 superblock_data[0x138];
	/*union { /* FS-specific superblock. Size = 0x138. *//*
		pfs0_superblock_t pfs0_superblock;
		romfs_superblock_t romfs_superblock;
		//nca0_romfs_superblock_t nca0_romfs_superblock;
		bktr_superblock_t bktr_superblock;
	};*/
	union {
		u64 section_ctr;
		struct {
			uint32_t section_ctr_low;
			uint32_t section_ctr_high;
		};
	};
	u8 _0x148[0xB8]; /* Padding. */
} PACKED;

struct NcaSectionEntry
{
	u32 media_start_offset;
	u32 media_end_offset;
	u8 _0x8[0x8]; /* Padding. */
} PACKED;

class NcaHeader
{
public:
	u8 fixed_key_sig[0x100]; /* RSA-PSS signature over header with fixed key. */
	u8 npdm_key_sig[0x100]; /* RSA-PSS signature over header with key in NPDM. */
	u32 magic;
	u8 distribution; /* System vs gamecard. */
	u8 content_type;
	u8 m_cryptoType; /* Which keyblob (field 1) */
	u8 m_kaekIndex; /* Which kaek index? */
	u64 nca_size; /* Entire archive size. */
	u64 m_titleId;
	u8 _0x218[0x4]; /* Padding. */
	union {
		uint32_t sdk_version; /* What SDK was this built with? */
		struct {
			u8 sdk_revision;
			u8 sdk_micro;
			u8 sdk_minor;
			u8 sdk_major;
		};
	};
	u8 m_cryptoType2; /* Which keyblob (field 2) */
	u8 _0x221[0xF]; /* Padding. */
	u64 m_rightsId[2]; /* Rights ID (for titlekey crypto). */
	NcaSectionEntry section_entries[4]; /* Section entry metadata. */
	integer<256> section_hashes[4]; /* SHA-256 hashes for each section header. */
	integer<128> m_keys[4]; /* Encrypted key area. */
	u8 _0x340[0xC0]; /* Padding. */
	NcaFsHeader fs_headers[4]; /* FS section headers. */
} PACKED;

class NcaBodyWriter
{
public:
	NcaBodyWriter(const NcaId& ncaId, u64 offset, ContentStorage& contentStorage);
	virtual ~NcaBodyWriter();
	virtual u64 write(const  u8* ptr, u64 sz);

protected:
	ContentStorage* m_contentStorage;
	NcaId m_ncaId;

	u64 m_offset;
};

class NcaWriter
{
public:
	NcaWriter(const NcaId& ncaId, ContentStorage& contentStorage);
	virtual ~NcaWriter();

	bool close();
	u64 write(const  u8* ptr, u64 sz);

protected:
	NcaId m_ncaId;
	ContentStorage* m_contentStorage;
	std::vector<u8> m_buffer;
	NcaBodyWriter* m_writer;
};
