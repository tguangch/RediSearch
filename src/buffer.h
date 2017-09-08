#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_READ 0
#define BUFFER_WRITE 1
#define BUFFER_FREEABLE 2    // if set, we free the buffer on Release
#define BUFFER_LAZY_ALLOC 4  // only allocate memory in a buffer writer on the first write

typedef struct {
  char *data;
  size_t cap;
  size_t offset;
} Buffer;

typedef struct {
  Buffer *buf;
  size_t pos;
} BufferReader;

#define BUFFER_READ_BYTE(br) br->buf->data[br->pos++]
//++b->buf->offset;

void Buffer_Init(Buffer *b, size_t cap);
size_t Buffer_ReadByte(BufferReader *b, char *c);
/**
Read len bytes from the buffer into data. If offset + len are over capacity
- we do not read and return 0
@return the number of bytes consumed
*/
static inline size_t Buffer_Read(BufferReader *br, void *data, size_t len) {
  // // no capacity - return 0
  // Buffer *b = br->buf;
  // if (br->pos + len > b->cap) {
  //   return 0;
  // }

  memcpy(data, br->buf->data + br->pos, len);
  br->pos += len;
  // b->offset += len;

  return len;
}
size_t Buffer_Seek(BufferReader *b, size_t offset);

static inline size_t BufferReader_Offset(const BufferReader *br) {
  return br->pos;
}

static inline size_t Buffer_Offset(const Buffer *ctx) {
  return ctx->offset;
}

static inline int BufferReader_AtEnd(const BufferReader *br) {
  return br->pos >= br->buf->offset;
}

static inline size_t Buffer_Capacity(const Buffer *ctx) {
  return ctx->cap;
}

static inline int Buffer_AtEnd(const Buffer *ctx) {
  return ctx->offset >= ctx->cap;
}

/**
Skip forward N bytes, returning the resulting offset on success or the end
position if where is outside bounds
*/
static inline size_t Buffer_Skip(BufferReader *br, int bytes) {
  br->pos += bytes;
  return br->pos;
}

typedef struct {
  Buffer *buf;
  char *pos;

} BufferWriter;

size_t Buffer_Write(BufferWriter *b, void *data, size_t len);
size_t Buffer_Truncate(Buffer *b, size_t newlen);

// Ensure that at least extraLen new bytes can be added to the buffer.
// Returns 0 if no realloc was performed. 1 if realloc was performed.
void Buffer_Grow(Buffer *b, size_t extraLen);

static inline size_t Buffer_Reserve(Buffer *buf, size_t n) {
  if (buf->offset + n <= buf->cap) {
    return 0;
  }
  Buffer_Grow(buf, n);
  return 1;
}

/**
 * 'Reserves' a certain amount of space in the buffer, and returns a pointer
 * to the newly reserved memory
 */
static inline void *Buffer_AddToSize(Buffer *b, size_t addedSize) {
  Buffer_Reserve(b, b->offset + addedSize);
  void *ret = b->data + b->offset;
  b->offset += addedSize;
  return ret;
}

/** Add data to the buffer without a BufferWriter */
static inline void Buffer_Append(Buffer *b, const void *data, size_t len) {
  Buffer_Reserve(b, len);
  memcpy(b->data + b->offset, data, len);
  b->offset += len;
}

/**
 * Sets the new size of the buffer, and sets the offset to 0
 */
static inline void Buffer_ReInit(Buffer *b, size_t newSize) {
  if (!b->data) {
    Buffer_Init(b, newSize);
  } else {
    Buffer_Reserve(b, newSize);
  }

  b->offset = 0;
}

#define BUFFER_GETSIZE_AS(b, T) ((b)->offset / sizeof(T))

#define Buffer_GetData(b) (b)->data

BufferWriter NewBufferWriter(Buffer *b);
BufferReader NewBufferReader(Buffer *b);

static inline char *BufferReader_Current(BufferReader *b) {
  return b->buf->data + b->pos;
}

static inline size_t BufferWriter_Offset(BufferWriter *b) {
  return b->pos - b->buf->data;
}

static inline char *BufferWriter_PtrAt(BufferWriter *b, size_t pos) {
  return b->buf->data + pos;
}

size_t BufferWriter_Seek(BufferWriter *b, size_t offset);
size_t Buffer_WriteAt(BufferWriter *b, size_t offset, void *data, size_t len);

Buffer *NewBuffer(size_t len);

Buffer *Buffer_Wrap(char *data, size_t len);
void Buffer_Free(Buffer *buf);

#endif
