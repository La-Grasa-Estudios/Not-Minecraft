#include "PortalStructure.h"

#include <array>

constexpr int REQUIRED_FRAME_BLOCKS = 10;
constexpr int REQUIRED_INSIDE_BLOCKS = 6;
constexpr int PORTAL_WIDTH = 4;
constexpr int PORTAL_HEIGHT = 5;

// Cleaned up by Gemini, its 2am and i don't have the brain power to do this

static bool IsValidBlock(int id)
{
    return id == BLOCK_AIR || id == BLOCK_PORTAL || id == BLOCK_FIRE;
}

static void GetAxisOffsets(int dir, int& dx, int& dz) {
    dx = (dir == 0) ? 1 : 0;
    dz = (dir == 1) ? 1 : 0;
}

bool PortalStructure::IsPortalValid(World* world, int x, int y, int z)
{
    int outOriginX, outOriginY, outOriginZ;
    return GetPortalDirection(world, x, y, z, outOriginX, outOriginY, outOriginZ) != -1;
}

void PortalStructure::FillPortal(World* world, int x, int y, int z) {
    int ox, oy, oz;
    int dir = GetPortalDirection(world, x, y, z, ox, oy, oz);

    if (dir == -1) return;

    int dx, dz;
    GetAxisOffsets(dir, dx, dz);

    for (int step = 1; step <= PORTAL_WIDTH - 2; step++) {
        for (int py = 1; py <= PORTAL_HEIGHT - 2; py++) {
            int cx = ox - (step * dx);
            int cz = oz - (step * dz);
            int cy = oy + py;

            world->SetBlock(cx, cy, cz, { BLOCK_PORTAL, (uint8_t)dir });
        }
    }
}

int PortalStructure::GetPortalDirection(World* world, int x, int y, int z, int& outOriginX, int& outOriginY, int& outOriginZ) {
    int oy = y;
    bool foundFloor = false;
    for (int i = 1; i < 4; i++) {
        oy = y - i;
        if (world->GetBlock(x, oy, z).Id == BLOCK_OBSIDIAN) {
            foundFloor = true;
            break;
        }
    }
    if (!foundFloor) return -1;

    for (int dir = 0; dir <= 1; dir++) {
        int dx, dz;
        GetAxisOffsets(dir, dx, dz);

        int ox = x;
        int oz = z;

        if (world->GetBlock(ox + dx, oy, oz + dz).Id == BLOCK_OBSIDIAN &&
            IsValidBlock(world->GetBlock(ox + dx, oy + 1, oz + dz).Id)) {
            ox += dx;
            oz += dz;
        }
        ox += dx;
        oz += dz;

        int validFrameBlocks = 0;
        int validInsideBlocks = 0;

        for (int step = 0; step < PORTAL_WIDTH; step++) {
            for (int py = 0; py < PORTAL_HEIGHT; py++) {

                bool isCorner = (step == 0 && py == 0) ||
                    (step == PORTAL_WIDTH - 1 && py == 0) ||
                    (step == PORTAL_WIDTH - 1 && py == PORTAL_HEIGHT - 1) ||
                    (step == 0 && py == PORTAL_HEIGHT - 1);
                if (isCorner) continue;

                bool isInsideFrame = step >= 1 && step <= PORTAL_WIDTH - 2 &&
                    py >= 1 && py <= PORTAL_HEIGHT - 2;

                int cx = ox - (step * dx);
                int cz = oz - (step * dz);
                int cy = oy + py;

                if (!isInsideFrame) {
                    if (world->GetBlock(cx, cy, cz).Id == BLOCK_OBSIDIAN) {
                        validFrameBlocks++;
                    }
                }
                else {
                    if (IsValidBlock(world->GetBlock(cx, cy, cz).Id)) {
                        validInsideBlocks++;
                    }
                }
            }
        }

        if (validFrameBlocks == REQUIRED_FRAME_BLOCKS && validInsideBlocks == REQUIRED_INSIDE_BLOCKS) {
            outOriginX = ox;
            outOriginY = oy;
            outOriginZ = oz;
            return dir;
        }
    }

    return -1;
}