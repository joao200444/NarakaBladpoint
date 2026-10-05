#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <detours.h>


// Removed all cheat-related helper functions

void install_hooks(uintptr_t base)
{
    // Detours doesn't need global initialization
    // Hooks are installed individually with DetourTransactionBegin/Commit
    
    // All cheat hooks removed - only ESP functionality remains
}



// All cheat control functions removed - ESP only mode

