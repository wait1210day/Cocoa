#include <string>
#include <fstream>
#include <ctime>

#include <zlib.h>

#include "Ciallo/GrBase.h"
#include "Ciallo/SkpSnapshot.h"
CIALLO_BEGIN_NS

namespace {

#define CSKP_HDR_MAGIC                      "CSKP"
#define CSKP_HDR_VERSION                    210213

struct cskp_relative_locator
{
    uint64_t    offset;
    uint64_t    size;
};

struct cskp_metadata_section
{
    uint64_t                timestamp;
    cskp_relative_locator   pictNameLocator;
    cskp_relative_locator   packagerNameLocator;
};

struct cskp_resource
{
    enum cskp_resource_type
    {
        BYTE_TYPE = 0,
        WORD_TYPE,
        DWORD_TYPE,
        QWORD_TYPE
    };

    union cskp_resource_value_any
    {
        uint8_t  byte;
        uint16_t word;
        uint32_t dword;
        uint64_t qword;
    };

    uint8_t            type;
    uint8_t            isArray;
    uint32_t           count;   /* Number of elements in array */
    cskp_resource_value_any value[0];
};

struct cskp_resources_section
{
    uint32_t        resourceCount;
    cskp_resource   resources[0];
};

struct cskp_skp_data_section
{
    enum cskp_compress_algorithm
    {
        COMPRESS_RAW = 0,
        COMPRESS_ZLIB
    };

    uint8_t     algorithm;
    uint64_t    dataSize;
    uint32_t    dataAdler32;
    uint8_t     data[0];
};

struct cskp_section
{
    enum cskp_section_kind
    {
        CSKP_SEC_METADATA,
        CSKP_SEC_RESOURCES,
        CSKP_SEC_SKP_DATA
    };

    union cskp_section_content
    {
        cskp_metadata_section   metadata;
        cskp_resources_section  resources;
        cskp_skp_data_section   skpData;
    };

    uint8_t               kind;
    cskp_section_content  content;
};

struct cskp_header
{
    uint8_t                 magic[8];
    uint32_t                version;
    uint32_t                sectionCount;
    cskp_relative_locator   sections[0];
};

template<typename T>
T *cast_from_relative_locator(uint8_t *pBuffer, const cskp_relative_locator& locator)
{
    if (locator.size != sizeof(T))
        return nullptr;
    return static_cast<T*>(static_cast<void*>(pBuffer + locator.offset));
}

uint8_t cvt_algorithm_flag(SkpSnapshotCompress compress)
{
    switch (compress)
    {
    case SkpSnapshotCompress::kRaw:
        return cskp_skp_data_section::COMPRESS_RAW;
    case SkpSnapshotCompress::kZlib:
        return cskp_skp_data_section::COMPRESS_ZLIB;
    }
}

size_t deflate_compress_data(const uint8_t *pSrc, uint8_t *pDst, size_t size, uint32_t& adler)
{
    ::z_stream stream;
    stream.next_in = const_cast<uint8_t*>(pSrc);
    stream.avail_in = size;
    stream.next_out = pDst;
    stream.avail_out = size;
    stream.zalloc = nullptr;
    stream.zfree = nullptr;
    stream.opaque = nullptr;

    int err = 0;
    err = deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                       MAX_WBITS + -1, 8, Z_DEFAULT_STRATEGY);
    if (err != Z_OK)
    {
        deflateEnd(&stream);
        return 0;
    }

    err = deflate(&stream, Z_FINISH);
    deflateEnd(&stream);
    if (err != Z_STREAM_END)
        return 0;

    adler = stream.adler;
    return stream.total_out;
}

size_t process_skp_data(const std::unique_ptr<uint8_t>& out, const sk_sp<SkData>& in,
                        SkpSnapshotCompress compress,
                        uint32_t& adler32)
{
}

} // namespace anonymous

bool SkpSnapshotWriteToFile(const sk_sp<SkPicture>& picture, const std::string& path,
                            SkpSnapshotCompress compress)
{
    if (picture == nullptr)
        return false;

    std::ofstream fs(path);
    if (!fs.is_open())
        return false;

    sk_sp<SkData> rawData(picture->serialize());
    if (rawData == nullptr || rawData->size() == 0)
        return false;

    cskp_skp_data_section skpDataSec{};
    skpDataSec.algorithm = cvt_algorithm_flag(compress);
    std::unique_ptr<uint8_t> skpDataBuffer(new uint8_t[rawData->size()]);
    skpDataSec.dataSize = process_skp_data(skpDataBuffer, rawData, compress, skpDataSec.dataAdler32);
    if (skpDataSec.dataSize == 0)
        return false;

    constexpr uint64_t hdrSize = sizeof(cskp_header) + 3 * sizeof(cskp_relative_locator);
    std::unique_ptr<cskp_header> hdr(reinterpret_cast<cskp_header*>(new uint8_t[hdrSize]));
    std::strncpy(reinterpret_cast<char*>(hdr->magic), CSKP_HDR_MAGIC, 8);
    hdr->version = CSKP_HDR_VERSION;
    hdr->sectionCount = 3;

    cskp_metadata_section metadata{};
    metadata.timestamp = std::time(nullptr);
}

sk_sp<SkPicture> SkpSnapshotLoad(const std::string& path)
{
}

CIALLO_END_NS
