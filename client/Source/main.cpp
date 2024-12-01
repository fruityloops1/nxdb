#include "pe/Client.h"
#include "pe/Render.h"
#include "pe/Util.h"

#define BUDDY_ALLOC_IMPLEMENTATION
#include "buddy_alloc.h"

#include "imgui.h"
#include "main.h"

static buddy* sBuddy = nullptr;

void* buddyMalloc(size_t size) {
    return buddy_malloc(sBuddy, size);
}

void buddyFree(void* ptr) {
    return buddy_free(sBuddy, ptr);
}

static void memoryFull() {
    fprintf(stderr, "balls");
    abort();
}

int main() {
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
    }

    u16 port = 3450;

    const size_t arenaSize = 1024 * 1024 * 256;
    void* buddyMetadata = malloc(buddy_sizeof(arenaSize));
    void* buddyArena = malloc(arenaSize);
    sBuddy = buddy_init(reinterpret_cast<u8*>(buddyMetadata), reinterpret_cast<u8*>(buddyArena), arenaSize);

    nxdb::Client client;
    client.run();

    free(buddyMetadata);
    free(buddyArena);

    return 0;
}
