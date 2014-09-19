/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Common.h"
#include "ByteBuffer.h"
#include "WorldPacket.h"
#include "UpdateData.h"
#include "Log.h"
#include "Opcodes.h"
#include "World.h"
#include "zlib.h"

UpdateData::UpdateData(uint16 map) : m_map(map), m_blockCount(0)
{
}

void UpdateData::AddOutOfRangeGUID(std::set<uint64>& guids)
{
    m_outOfRangeGUIDs.insert(guids.begin(), guids.end());
}

void UpdateData::AddOutOfRangeGUID(uint64 guid)
{
    m_outOfRangeGUIDs.insert(guid);
}

void UpdateData::AddUpdateBlock(const ByteBuffer &block)
{
    m_data.append(block);
    ++m_blockCount;
}

bool UpdateData::BuildPacket(WorldPacket* p_Packet)
{
    ASSERT(p_Packet->empty());                                // shouldn't happen
    p_Packet->Initialize(SMSG_UPDATE_OBJECT, 2 + 4 + (m_outOfRangeGUIDs.empty() ? 0 : 1 + 4 + 9 * m_outOfRangeGUIDs.size()) + m_data.wpos());

    if (!HasData())
        return false;

    *p_Packet << uint32(m_blockCount);
    *p_Packet << uint16(m_map);

    if (p_Packet->WriteBit(!m_outOfRangeGUIDs.empty()))
    {
        p_Packet->FlushBits();

        *p_Packet << uint16(0);
        *p_Packet << uint32(m_outOfRangeGUIDs.size());

        for (std::set<uint64>::const_iterator i = m_outOfRangeGUIDs.begin(); i != m_outOfRangeGUIDs.end(); ++i)
            p_Packet->appendPackGUID(*i);
    }
    else
        p_Packet->FlushBits();

    uint32_t l_Pos = p_Packet->wpos();
    *p_Packet << uint32(0);

    p_Packet->append(m_data);

    uint32_t l_Size = p_Packet->wpos() - (l_Pos + 4);
    p_Packet->wpos(l_Pos);

    *p_Packet << uint32(l_Size);

    return true;
}

void UpdateData::Clear()
{
    m_data.clear();
    m_outOfRangeGUIDs.clear();
    m_blockCount = 0;
    m_map = 0;
}

