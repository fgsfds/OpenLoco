#pragma once

#include "Object.h"
#include "Types.hpp"
#include <OpenLoco/Core/Span.hpp>

namespace OpenLoco
{
    namespace ObjectManager
    {
        struct DependentObjects;
    }
    namespace Gfx
    {
        struct RenderTarget;
    }

#pragma pack(push, 1)
    namespace AirportMovementNodeFlags
    {
        constexpr uint16_t terminal = (1 << 0);
        constexpr uint16_t takeoffEnd = (1 << 1);
        constexpr uint16_t flag2 = (1 << 2);
        constexpr uint16_t taxiing = (1 << 3);
        constexpr uint16_t inFlight = (1 << 4);
        constexpr uint16_t heliTakeoffBegin = (1 << 5);
        constexpr uint16_t takeoffBegin = (1 << 6);
        constexpr uint16_t heliTakeoffEnd = (1 << 7);
        constexpr uint16_t touchdown = (1 << 8);
    }

    struct AirportObject
    {
        static constexpr auto kObjectType = ObjectType::airport;

        struct MovementNode
        {
            int16_t x;      // 0x00
            int16_t y;      // 0x02
            int16_t z;      // 0x04
            uint16_t flags; // 0x06 AirportMovementNodeFlags
        };

        struct MovementEdge
        {
            uint8_t var_00;
            uint8_t curNode;  // 0x01
            uint8_t nextNode; // 0x02
            uint8_t var_03;
            uint32_t mustBeClearEdges;     // 0x04 Which edges must be clear to use the transition edge
            uint32_t atLeastOneClearEdges; // 0x08 Which edges must have at least one clear to use transition edge
        };

        string_id name;
        int16_t buildCostFactor; // 0x02
        int16_t sellCostFactor;  // 0x04
        uint8_t costIndex;       // 0x06
        uint8_t var_07;
        uint32_t image; // 0x08
        uint32_t var_0C;
        uint16_t allowedPlaneTypes; // 0x10
        uint8_t numSpriteSets;      // 0x12
        uint8_t numTiles;           // 0x13
        const uint8_t* var_14;
        const uint16_t* var_18;
        const uint8_t* var_1C[32];
        const uint32_t* var_9C;
        uint32_t largeTiles;               // 0xA0
        int8_t minX;                       // 0xA4
        int8_t minY;                       // 0xA5
        int8_t maxX;                       // 0xA6
        int8_t maxY;                       // 0xA7
        uint16_t designedYear;             // 0xA8
        uint16_t obsoleteYear;             // 0xAA
        uint8_t numMovementNodes;          // 0xAC
        uint8_t numMovementEdges;          // 0xAD
        const MovementNode* movementNodes; // 0xAE
        const MovementEdge* movementEdges; // 0xB2
        uint8_t pad_B6[0xBA - 0xB6];

        void drawPreviewImage(Gfx::RenderTarget& rt, const int16_t x, const int16_t y) const;
        void drawDescription(Gfx::RenderTarget& rt, const int16_t x, const int16_t y, [[maybe_unused]] const int16_t width) const;
        bool validate() const;
        void load(const LoadedObjectHandle& handle, stdx::span<const std::byte> data, ObjectManager::DependentObjects*);
        void unload();
    };
#pragma pack(pop)

    static_assert(sizeof(AirportObject) == 0xBA);
}