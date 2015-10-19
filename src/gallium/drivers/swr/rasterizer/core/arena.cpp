/****************************************************************************
* Copyright (C) 2014-2015 Intel Corporation.   All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice (including the next
* paragraph) shall be included in all copies or substantial portions of the
* Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*
* @file arena.cpp
*
* @brief Arena memory manager
*        The arena is convenient and fast for managing allocations for any of
*        our allocations that are associated with operations and can all be freed
*        once when their operation has completed. Allocations are cheap since
*        most of the time its simply an increment of an offset. Also, no need to
*        free individual allocations. All of the arena memory can be freed at once.
*
******************************************************************************/

#include "context.h"
#include "arena.h"

#include <cmath>

VOID Arena::Init()
{
    m_memUsed = 0;
    m_pCurBlock = nullptr;
    m_pUsedBlocks = nullptr;
}

VOID* Arena::AllocAligned(uint32_t size, uint32_t align)
{
    if (m_pCurBlock)
    {
        ArenaBlock* pCurBlock = m_pCurBlock;
        pCurBlock->offset = AlignUp(pCurBlock->offset, align);

        if ((pCurBlock->offset + size) < pCurBlock->blockSize)
        {
            BYTE* pMem = (BYTE*)pCurBlock->pMem + pCurBlock->offset;
            pCurBlock->offset += size;
            return pMem;
        }

        // Not enough memory in this arena so lets move to a new block.
        pCurBlock->pNext = m_pUsedBlocks;
        m_pUsedBlocks = pCurBlock;
        m_pCurBlock = nullptr;
    }

    static const uint32_t ArenaBlockSize = 1024*1024;
    uint32_t defaultBlockSize = ArenaBlockSize;
    if (m_pUsedBlocks == nullptr)
    {
        // First allocation after reset. Let's make the first block be the total
        // memory allocated during last set of allocations prior to reset.
        defaultBlockSize = std::max(m_memUsed, defaultBlockSize);
        m_memUsed = 0;
    }

    uint32_t blockSize = std::max(size, defaultBlockSize);
    blockSize = AlignUp(blockSize, KNOB_SIMD_WIDTH*4);

    VOID *pMem = _aligned_malloc(blockSize, KNOB_SIMD_WIDTH*4);    // Arena blocks are always simd byte aligned.
    SWR_ASSERT(pMem != nullptr);

    m_pCurBlock = (ArenaBlock*)malloc(sizeof(ArenaBlock));
    SWR_ASSERT(m_pCurBlock != nullptr);

    if (m_pCurBlock != nullptr)
    {
        m_pCurBlock->pMem = pMem;
        m_pCurBlock->blockSize = blockSize;
        m_pCurBlock->offset = size;
        m_memUsed += blockSize;
    }

    return pMem;
}

VOID* Arena::Alloc(uint32_t size)
{
    return AllocAligned(size, 1);
}

VOID Arena::Reset()
{
    if (m_pCurBlock)
    {
        m_pCurBlock->offset = 0;

        // If we needed to allocate used blocks then reset current.
        // The next time we allocate we'll grow the current block
        // to match all the memory allocated this for this frame.
        if (m_pUsedBlocks)
        {
            m_pCurBlock->pNext = m_pUsedBlocks;
            m_pUsedBlocks = m_pCurBlock;
            m_pCurBlock = nullptr;
        }
    }

    while(m_pUsedBlocks)
    {
        ArenaBlock* pBlock = m_pUsedBlocks;
        m_pUsedBlocks = pBlock->pNext;

        _aligned_free(pBlock->pMem);
        free(pBlock);
    }
}
