#pragma once
#include "pch.h"
#include "Types.h"

class VertexBuffer {
    uint m_id;

  public:
    VertexBuffer(const void *data, uint size);
    ~VertexBuffer();

    void bind() const;
    void unbind() const;
};

class IndexBuffer {
    uint m_id;
    uint m_count;

  public:
    IndexBuffer(const uint *data, uint count);
    ~IndexBuffer();

    void bind() const;
    void unbind() const;

    inline uint getCount() { return m_count; };
};