/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "velox/common/compression/Compression.h"
#include "velox/dwio/common/OutputStream.h"
#include "velox/dwio/common/SeekableInputStream.h"
#include "velox/dwio/common/compression/CompressionBufferPool.h"
#include "velox/dwio/common/encryption/Encryption.h"

namespace facebook::velox::dwio::common::compression {

class Compressor {
 public:
  explicit Compressor(int32_t level) : level_{level} {}

  virtual ~Compressor() = default;

  virtual uint64_t compress(const void* src, void* dest, uint64_t length) = 0;

 protected:
  int32_t level_;
};

class Decompressor {
 public:
  explicit Decompressor(uint64_t blockSize, const std::string& streamDebugInfo)
      : blockSize_{blockSize}, streamDebugInfo_{streamDebugInfo} {}

  virtual ~Decompressor() = default;

  virtual uint64_t getUncompressedLength(
      const char* /* unused */,
      uint64_t /* unused */) const {
    return blockSize_;
  }

  virtual uint64_t decompress(
      const char* src,
      uint64_t srcLength,
      char* dest,
      uint64_t destLength) = 0;

 protected:
  uint64_t blockSize_;
  const std::string streamDebugInfo_;
};

/**
 * Create a decompressor for the given compression kind.
 * @param kind the compression type to implement
 * @param input the input stream that is the underlying source
 * @param bufferSize the maximum size of the buffer
 * @param pool the memory pool
 */
std::unique_ptr<dwio::common::SeekableInputStream> createDecompressor(
    facebook::velox::common::CompressionKind kind,
    std::unique_ptr<dwio::common::SeekableInputStream> input,
    uint64_t bufferSize,
    memory::MemoryPool& pool,
    const std::string& streamDebugInfo,
    const dwio::common::encryption::Decrypter* decryptr = nullptr);

/**
 * Create a compressor for the given compression kind.
 * @param kind the compression type to implement
 * @param bufferPool pool for compression buffer
 * @param bufferHolder buffer holder that handles buffer allocation and
 * collection
 * @param level compression level
 */
std::unique_ptr<BufferedOutputStream> createCompressor(
    facebook::velox::common::CompressionKind kind,
    CompressionBufferPool& bufferPool,
    DataBufferHolder& bufferHolder,
    uint32_t compressionThreshold,
    int32_t zlibCompressionLevel,
    int32_t zstdCompressionLevel,
    uint8_t pageHeaderSize,
    const dwio::common::encryption::Encrypter* encrypter = nullptr);

} // namespace facebook::velox::dwio::common::compression
